#include "server.h"
#include <stdio.h>


void *connection(void *args) {
  int idx, fd;
  sscanf("%d\n%d", (char*)args, &idx, &fd);
  printf("id: %d", idx);

  char send_data[BUFF_SIZE] , recv_data[BUFF_SIZE];
  while (true) {
    recv(fd ,recv_data, BUFF_SIZE, 0);
    if (recv_data[0] == 'q' || recv_data[0] == 'Q') {
      break;
    }
    else if (recv_data[0] != '\0') {
      printf("Cliente %d: %s", idx, recv_data);
      sprintf(send_data, "Servidor recebeu: %s", recv_data);
      memset(recv_data, 0, BUFF_SIZE);
    }
  }

  return (void*)idx;
}

int main(int argc, char **argv) {
  int sock, connected, t = 1, last_cli = 0, tail = N - 1; 
  // this tail and last_cli simply does not work right, I did not put any effort into it. Just ignore those
  unsigned int sin_size;
  struct sockaddr_in server_addr, client_addr[N];
  pthread_t cli_t[N];
  sem_t mutex, waiters;


  sem_init(&mutex, 0, 1);
  // client connection waiters
  sem_init(&waiters, 0, N);

  if ((sock = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
    perror("Erro na criação do Socket");
    exit(1);
  }

  if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &t, sizeof(int))
    == -1)
  { perror("Erro em Setsockopt");
    exit(1);
  }
  server_addr.sin_family = AF_INET;
  server_addr.sin_port = htons(5000); // host-endian to network-endian
  server_addr.sin_addr.s_addr = INADDR_ANY;


  if (bind(sock, (struct sockaddr *)&server_addr, sizeof(struct
           sockaddr)) == -1) {
    perror("binding não foi possível");
    exit(1);
  }

  if (listen(sock, N) == -1) {
    perror("Erro na definição do tamanho da fila de entrada");
    exit(1);
  }


  printf("\nServidor TCP esperando por cliente na porta 5000\n");
  fflush(stdout);

  // loop do servidor
  while (true) {
    sin_size = sizeof(struct sockaddr_in);
    connected = accept(sock, (struct sockaddr *)&client_addr, &sin_size);

    printf("\nConexão recebida (Cliente: %s , Porta: %d)\n",
           inet_ntoa(client_addr[last_cli].sin_addr), ntohs(client_addr[last_cli].sin_port));
    fflush(stdout);
  
    // pass idx and addr for each thread
    char args[BUFF_SIZE];
    sem_wait(&waiters);
    sprintf(args, "%d\n%d", last_cli, client_addr[last_cli].sin_addr.s_addr);

    pthread_create(&cli_t[last_cli], NULL, connection, (void*)args);
    pthread_join(cli_t[last_cli], (void*)&last_cli);

    if (client_addr[last_cli + 1].sin_addr.s_addr == 0){
      sem_wait(&mutex);
      if (last_cli == tail)
        last_cli = 0;
      else
        last_cli++;
      sem_post(&mutex);
    }
    else {
      sem_wait(&mutex);
      last_cli = tail;
      sem_post(&mutex);
    }
    sem_post(&waiters);
    printf("%d", last_cli);
  }
  close(sock);
  return 0;
}
