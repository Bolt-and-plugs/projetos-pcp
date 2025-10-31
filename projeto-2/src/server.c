#include "server.h"


server s;

void *get_connection(void *args) {
  thread_args *args_ptr = (thread_args*)args;
  int idx = args_ptr->idx, fd = args_ptr->fd;
  free(args_ptr);

  printf("id: %d\n", idx);

  char send_data[BUFF_SIZE] , recv_data[BUFF_SIZE];
  while (true) {
    recv(fd ,recv_data, BUFF_SIZE, 0);
    if (recv_data[0] == 'q' || recv_data[0] == 'Q') {
      break;
    }
    else if (recv_data[0] != '\0') {
      printf("Cliente %d: %s\n", idx, recv_data);
      sprintf(send_data, "Servidor recebeu: %s\n", recv_data);
      memset(recv_data, 0, BUFF_SIZE);
      // enviar parte do vetor agora
    }
  }

  s.curr = idx; 
  return args;
}

int main(int argc, char **argv) {
  int sock, connected, t = 1;
  unsigned int sin_size;
  s.curr = 0;
  s.tail = CLI_NUM - 1;


  sem_init(&s.mutex, 0, 1);

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

  if (listen(sock, CLI_NUM) == -1) {
    perror("Erro na definição do tamanho da fila de entrada");
    exit(1);
  }


  printf("\nServidor TCP esperando por cliente na porta 5000\n");
  fflush(stdout);

  // loop do servidor
  while (true) {
    sin_size = sizeof(struct sockaddr_in);
    connected = accept(sock, (struct sockaddr *)&s.client_addr[s.curr], &sin_size);

    printf("\nConexão recebida (Cliente: %s , Porta: %d)\n",
           inet_ntoa(s.client_addr[s.curr].sin_addr), ntohs(s.client_addr[s.curr].sin_port));
  
    // pass idx and addr for each thread
    thread_args *ta;
    ta = malloc(sizeof(thread_args));
    ta->idx = s.curr;
    ta ->fd = connected;


    sem_wait(&s.mutex);
    if (s.tail == s.curr)
      s.curr = s.curr + 1 % CLI_NUM;
    else 
      s.curr = s.tail + 1 % CLI_NUM;

    s.tail = s.tail + 1 % CLI_NUM;
    sem_post(&s.mutex);
    
    pthread_create(&s.cli_t[s.curr], NULL, get_connection, (void*)ta);
    pthread_detach(s.cli_t[s.curr]);    
  }
  close(sock);
  return 0;
}
