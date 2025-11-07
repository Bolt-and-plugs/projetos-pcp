#include "server.h"

server s;
queue *slice_queue;
int width, height, finished_threads = 0, **image;

void *get_connection(void *args) {
  thread_args *args_ptr = (thread_args *)args;
  int idx = args_ptr->idx, fd = args_ptr->fd;
  int bytes_recv;
  free(args_ptr);

  char *send_data, *recv_data;

  size_t buffer_size = sizeof(int) * (size_t)width * (size_t)height;
  send_data = malloc(buffer_size);
  recv_data = malloc(buffer_size);
  if (send_data == NULL || recv_data == NULL) {
    perror("Falha ao alocar buffers");
    exit(1);
  }

  while (true) {
    bytes_recv = recv(fd, recv_data, sizeof("ok"), 0);
    recv_data[bytes_recv] = '\0';
    if (strcmp(recv_data, "ok") == 0) {
      printf("Cliente %d: %s\n", idx, recv_data);
      memset(recv_data, 0, width * height);

      sem_wait(&s.mutex);
      if (isEmpty(slice_queue)) {
        finished_threads++;
        sem_post(&s.mutex);
        send(fd, "q", sizeof("q"), 0);
        return NULL;
      }

      int x_ur = slice_queue->queue_x[slice_queue->head];
      int y_ur = slice_queue->queue_y[slice_queue->head];
      dequeue(slice_queue);
      sem_post(&s.mutex);

      printf("Enviando slice [%d %d] da imagem para o cliente\n", x_ur, y_ur);

      int *ptr = (int *)send_data;
      for (int i = 0; i < height; i++) {
        memcpy(ptr, &image[x_ur + i][y_ur], width * sizeof(int));
        ptr += width;
      }

      send(fd, ptr, buffer_size, 0);
      // recebe a mesma seção processada
      bytes_recv = recv(fd, recv_data, buffer_size, 0);
      recv_data[bytes_recv] = '\0';
      // Basicamente pegar os índices enviados da fatia e setar esses bytes
      //  escreve a seção de volta na imagem
      printf("Cliente %d processou: %s\n", idx, recv_data);
      // parse recv data to image
      int curr_val = 0;
      for (int i = 0; i < bytes_recv; i += sizeof(int)) {
        curr_val = atoi(&recv_data[i]);
        image[x_ur][y_ur] = curr_val;
      }
      memset(recv_data, 0, width * height);
    }
  }

  s.curr = idx;
  return args;
}

void init_image() {
  image = malloc(sizeof(int *) * IMAGE_SIZE);
  if (!image) {
    perror("Could not allocate image buffer");
    free(image);
    exit(1);
  }

  for (int i = 0; i < IMAGE_SIZE; i++) {
    image[i] = malloc(sizeof(int) * IMAGE_SIZE);
    if (!image[i]) {
      perror("Could not allocate image buffer");
      for (int j = 0; j <= i; j++)
        free(image[i]);
      exit(1);
    }
  }
}

int main(int argc, char **argv) {
  if (argc < 5) {
    perror("Not enough args");
    exit(-1);
  }

  int sock, connected, t = 1;
  unsigned int sin_size;
  const char *file_name = "assets/time_related/times.dat";
  int num_cli = atoi(argv[2]);
  char file_output[BUFF_SIZE];
  char file_path[BUFF_SIZE];
  FILE *file;

  s.curr = 0;
  s.tail = num_cli;
  sem_init(&s.mutex, 0, 1);

  width = atoi(argv[3]);  // Width
  height = atoi(argv[4]); // Height
  int block_size = atoi(argv[5]), num_blocks = 0;

  slice_queue = (queue *)malloc(sizeof(queue));
  init_queue(slice_queue);

  if (argv[1]) {
    strcpy(file_path, argv[1]);
  }

  init_image();
  read_input(file_path, image, IMAGE_SIZE);
  int num_chunks = (width * height) / block_size;
  for (int i = 0; i < width / block_size; i++) {
    for (int j = 0; j < height / block_size; j++) {
      enqueue(slice_queue, i * block_size, j * block_size);
      num_blocks++;
    }
  }

  if ((sock = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
    perror("Erro na criação do Socket");
    exit(1);
  }

  if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &t, sizeof(int)) == -1) {
    perror("Erro em Setsockopt");
    exit(1);
  }

  s.server_addr.sin_family = AF_INET;
  s.server_addr.sin_port = htons(SERVER_PORT); // host-endian to network-endian
  s.server_addr.sin_addr.s_addr = INADDR_ANY;

  if (bind(sock, (struct sockaddr *)&s.server_addr, sizeof(struct sockaddr)) ==
      -1) {
    perror("Binding não foi possível");
    exit(1);
  }

  if (listen(sock, num_cli) == -1) {
    perror("Erro na definição do tamanho da fila de entrada");
    exit(1);
  }

  printf("\nServidor TCP esperando por cliente na porta 5000\n");
  fflush(stdout);

  file = fopen(file_name, "a");
  struct timespec start, end, _time;

  // loop do servidor
  for (int i = 0; i < num_cli; i++) {
    sin_size = sizeof(struct sockaddr_in);

    connected =
        accept(sock, (struct sockaddr *)&s.client_addr[s.curr], &sin_size);

    clock_gettime(CLOCK_MONOTONIC, &start);
    printf("\nConexão recebida (Cliente: %s , Porta: %d)\n",
           inet_ntoa(s.client_addr[s.curr].sin_addr),
           ntohs(s.client_addr[s.curr].sin_port));

    // pass idx and addr for le(result, N, false); no need anymore for each
    // thread
    thread_args *ta;
    ta = malloc(sizeof(thread_args));
    ta->idx = s.curr;
    ta->fd = connected;

    sem_wait(&s.mutex);
    if (s.tail == s.curr)
      s.curr = s.curr + 1 % num_cli;
    else
      s.curr = s.tail + 1 % num_cli;

    s.tail = s.tail + 1 % num_cli;
    sem_post(&s.mutex);

    pthread_create(&s.cli_t[s.curr], NULL, get_connection, (void *)ta);
    pthread_detach(s.cli_t[s.curr]);
  }

  printf("Todas as %d conexões foram aceitas. Aguardando processamento de %d "
         "chunks de tamanho %d...\n",
         num_cli, num_blocks, block_size);

  // spin lock if there is any data to be processed
  while (true) {
    int empty, finished;

    sem_wait(&s.mutex);
    empty = isEmpty(slice_queue);
    finished = finished_threads;
    sem_post(&s.mutex);

    if (empty && finished == num_cli) {
      break;
    }
    usleep(10000); // 10ms
  }

  sprintf(file_output, "assets/outputs/server-processed-%dx%d.dat", IMAGE_SIZE,
          IMAGE_SIZE);
  write_file(file_output, image, IMAGE_SIZE);
  clock_gettime(CLOCK_MONOTONIC, &end);
  sub_timespec(start, end, &_time);
  printf("Time elapsed: %d.%.9ld | Matrix Size: %d\n ", (int)_time.tv_sec,
         _time.tv_nsec, width * height);
  fprintf(
      file,
      "Time elapsed: %d.%.9ld | Num Blocks: %d | Blocks Dimension: %d x %d ",
      (int)_time.tv_sec, _time.tv_nsec, num_cli, width, height);
  fclose(file);
  close(sock);
  return 0;
}
