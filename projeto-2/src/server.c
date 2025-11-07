#include "server.h"
#include <stdlib.h>
#include <linux/time.h>

server s;
int **image;
int x, y;
queue* slice_queue;
int N;
void *get_connection(void *args) {
  thread_args *args_ptr = (thread_args*)args;
  int idx = args_ptr->idx, fd = args_ptr->fd;
  int bytes_recv;
  free(args_ptr);

  char send_data[sizeof(int) * x * y] , recv_data[sizeof(int) * x * y];
  while (true) {
    bytes_recv=recv(fd ,recv_data, x*y, 0);
    recv_data[bytes_recv] = '\0';
    if (strcmp(recv_data,"q\n")==0 || strcmp(recv_data,"Q\n")==0) {
      break;
    }
    else if (recv_data[0] != '\0') {
      printf("Cliente %d: %s\n", idx, recv_data);
      sprintf(send_data, "Servidor recebeu: %s\n", recv_data);
      memset(recv_data, 0, x*y); 
      
      int x_ur = slice_queue->queue_x[slice_queue->tail+1];
      int y_ur = slice_queue->queue_y[slice_queue->tail+1];
      dequeue(slice_queue);
      memcpy(send_data, &image[x_ur][y_ur], x*y);
      send(fd, send_data, x*y, 0);

      // recebe a mesma seção processada
      bytes_recv = recv(fd ,recv_data, x*y, 0);
      recv_data[bytes_recv] = '\0';
      //Basicamente pegar os índices enviados da fatia e setar esses bytes
      // escreve a seção de volta na imagem
      printf("Cliente %d processou: %s\n", idx, recv_data);
      memcpy(image[x_ur][y_ur], &recv_data, x*y); //não saber sintaxe do c ver se funciona risos risos
      memset(recv_data, 0, x*y);
    }
  }

  s.curr = idx; 
  return args;
}

int main(int argc, char **argv) {
  int sock, connected, t = 1;
  unsigned int sin_size; 
  s.curr = 0;
  s.tail = atoi(argv[2]) - 1;
  sem_init(&s.mutex, 0, 1);
  N = s.tail;
  x = atoi(argv[3]);//Width
  y = atoi(argv[4]);//Height
  int W = x;
  int H = y;
  slice_queue = (queue*) malloc(sizeof(queue));
  init_queue(slice_queue);

  char file_path[BUFF_SIZE];
  if (argv[1]) {
    strcpy(file_path, argv[1]);
  }

  image = malloc(sizeof(int) * IMAGE_SIZE);
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

  read_input(file_path, image, IMAGE_SIZE);

  for(int i = 0; i < (IMAGE_SIZE/x); i++)
    for(int j = 0; j < (IMAGE_SIZE/y); i++)
      enqueue(slice_queue, i*x, j*y);

  if ((sock = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
    perror("Erro na criação do Socket");
    exit(1);
  }

  if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &t, sizeof(int))
    == -1) {
    perror("Erro em Setsockopt");
    exit(1);
  }

  s.server_addr.sin_family = AF_INET;
  s.server_addr.sin_port = htons(SERVER_PORT); // host-endian to network-endian
  s.server_addr.sin_addr.s_addr = INADDR_ANY;


  if (bind(sock, (struct sockaddr *)&s.server_addr, sizeof(struct sockaddr)) == -1) {
    perror("Binding não foi possível");
    exit(1);
  }

  if (listen(sock, N) == -1) {
    perror("Erro na definição do tamanho da fila de entrada");
    exit(1);
  }


  printf("\nServidor TCP esperando por cliente na porta 5000\n");
  fflush(stdout);

  FILE *file;
  puts("Initializing sequential execution");
  const char *file_name = "time_related/times.dat";
  file = fopen(file_name, "a");
  struct timespec start, end, _time;
  // loop do servidor
  while (true) {
    sin_size = sizeof(struct sockaddr_in);
    if(N == s.curr) break;
    connected = accept(sock, (struct sockaddr *)&s.client_addr[s.curr], &sin_size);

    clock_gettime(CLOCK_MONOTONIC, &start);
    printf("\nConexão recebida (Cliente: %s , Porta: %d)\n",
           inet_ntoa(s.client_addr[s.curr].sin_addr), ntohs(s.client_addr[s.curr].sin_port));
  
    // pass idx and addr for each thread
    thread_args *ta;
    ta = malloc(sizeof(thread_args));
    ta->idx = s.curr;
    ta ->fd = connected;

    sem_wait(&s.mutex);
    if (s.tail == s.curr)
      s.curr = s.curr + 1 % N;
    else 
      s.curr = s.tail + 1 % N;

    s.tail = s.tail + 1 % N;
    sem_post(&s.mutex);
    
    pthread_create(&s.cli_t[s.curr], NULL, get_connection, (void*)ta);
    pthread_detach(s.cli_t[s.curr]);

    
  }
  while(!isempty(queue)){// function to be implemented.
  }
        //lógica de remontagem.
          /* 
        _NOT FINISHED YET, IT DEPENDS ON "SENDING" AND "RECEIVING" IMPLEMENTATION
        - fn is the function
        - N is the number os blocks that are being transmitted
        - W is the width of the block
        - H is the height of the block
        - W x H compose block's dimension
        */
  clock_gettime(CLOCK_MONOTONIC, &end);
  //write_x_to_file(result, N, false); no need anymore.
  sub_timespec(start, end, &_time);
  printf("Time elapsed: %d.%.9ld | Matrix Size: %d\n ", (int)_time.tv_sec, _time.tv_nsec, N);
  fprintf(file,"Time elapsed: %d.%.9ld | Num Blocks: %d | Blocks Dimension: %d x %d " , (int)_time.tv_sec, _time.tv_nsec, N, W, H);
  fclose(file);
  close(sock);
  return 0;
}
