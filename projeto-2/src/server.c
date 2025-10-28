#include "server.h"

void *connection() {
  
}

int main(int argc, char **argv) {
  int sock, connected, bytes_recv, t = 1, i, j, sin_size, last_cli = 0, tail = N - 1;
  char send_data[1024] , recv_data[1024];
  struct sockaddr_in server_addr, client_addr[N];
  bool should_stop = false;
  pthread_t cli_t[N];

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
  while (!should_stop) {
    sin_size = sizeof(struct sockaddr_in);
    connected = accept(sock, (struct sockaddr *)&client_addr, &sin_size);

    printf("\nConexão recebida (Cliente: %s , Porta: %d)\n",
           inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
    fflush(stdout);

    // track last cli position

    // loop de todo cliente
    pthread_create();
    pthread_join();
  }
  close(sock);
  return 0;
}
