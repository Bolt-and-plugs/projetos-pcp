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
    return NULL;
  }

  while (true) {
    memset(recv_data, 0, buffer_size);
    bytes_recv = recv(fd, recv_data, strlen("ok"), 0);

    if (bytes_recv <= 0) {
      free(send_data);
      free(recv_data);
      return NULL;
    }
    recv_data[bytes_recv] = '\0';

    if (strcmp(recv_data, "ok") == 0) {
      printf("Cliente %d: %s\n", idx, recv_data);
      memset(recv_data, 0, width * height);

      sem_wait(&s.mutex);
      if (isEmpty(slice_queue)) {
        finished_threads++;
        sem_post(&s.mutex);
        free(send_data);
        free(recv_data);
        send(fd, "q", strlen("q"), 0);
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

      send_all(fd, send_data, buffer_size);
      // recebe a mesma seção processada
      printf("esperando\n");
      bytes_recv = recv_all(fd, recv_data, buffer_size);
      ptr = (int *)recv_data;
      for (int i = 0; i < height; i++) {
        memcpy(&image[x_ur + i][y_ur], ptr, width * sizeof(int));
        ptr += width;
      }
      memset(recv_data, 0, buffer_size);
    }
  }

  s.curr = idx;
  return args;
}

void init_image() {
  image = malloc(sizeof(int *) * IMAGE_SIZE);
  if (image == NULL) {
    perror("Could not allocate image rows pointer");
    exit(1);
  }

  for (int i = 0; i < IMAGE_SIZE; i++) {
    image[i] = malloc(sizeof(int) * IMAGE_SIZE);
    if (image[i] == NULL) {
      perror("Could not allocate image row");
      // free previously allocated rows
      for (int j = 0; j < i; j++) {
        free(image[j]);
      }
      free(image);
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
  struct timespec start, end, _time;

  s.curr = 0;
  s.tail = num_cli;
  sem_init(&s.mutex, 0, 1);

  width = atoi(argv[3]);  // Block Width
  height = atoi(argv[4]); // Block Height
  int block_size = width * height, num_blocks = 0;

  slice_queue = (queue *)malloc(sizeof(queue));
  init_queue(slice_queue);

  if (argv[1]) {
    strcpy(file_path, argv[1]);
  }

  init_image();
  read_input(file_path, image, IMAGE_SIZE);

  int num_chunks = (width * height) / block_size;

  for (int i = 0; i < IMAGE_SIZE / height; i++) {
    for (int j = 0; j < IMAGE_SIZE / width; j++) {
      enqueue(slice_queue, i * height, j * width);
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
    ta->idx = i;
    ta->fd = connected;

    pthread_create(&s.cli_t[i], NULL, get_connection, (void *)ta);
    pthread_detach(s.cli_t[i]);
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

  clock_gettime(CLOCK_MONOTONIC, &end);
  sprintf(file_output, "assets/outputs/server-processed-%d_cli-%dx%d.dat",
          num_cli, width, height);
  write_file(file_output, image, IMAGE_SIZE);
  sub_timespec(start, end, &_time);
  fprintf(
      file,
      "Time elapsed: %d.%.9ld | Num Blocks: %d | Blocks Dimension: %d x %d\n",
      (int)_time.tv_sec, _time.tv_nsec, num_cli, width, height);
  fclose(file);
  close(sock);
  return 0;
}
