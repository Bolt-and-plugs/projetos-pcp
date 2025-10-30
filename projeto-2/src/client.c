#include "client.h"

int main(int argc, char **argv) {
  int sock, bytes_recv;
  char send_data[BUFF_SIZE], recv_data[BUFF_SIZE];
  struct hostent *host;
  struct sockaddr_in server_addr;

  host = gethostbyname(argv[1]);
  printf("%d", host->h_addrtype);

  if ((sock = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
    perror("Erro na criação do Socket");
    exit(1);
  }
  fflush(stdout);

  server_addr.sin_family = AF_INET;
  server_addr.sin_port = htons(5000);
  server_addr.sin_addr = *((struct in_addr *)host->h_addr);
  bzero(&(server_addr.sin_zero),8);

  // Conecta com o servidor definido
  if (connect(sock, (struct sockaddr *)&server_addr,
              sizeof(struct sockaddr)) == -1)
  { perror("Erro na conexão");
    exit(1);
  }
  // Repete enquanto a conexão for mantida
  while(true) { // Lê entrada do usuário
    printf ("\n TEXTO (q or Q to quit) : ");
    fgets(send_data, sizeof(send_data), stdin);
    // Envia texto ou pedido de desconexão ao servidor
    send(sock, send_data, strlen(send_data), 0);
    // Recebe resposta do servidor
    bytes_recv=recv(sock,recv_data,1024,0);
    recv_data[bytes_recv] = '\0';
    // Imprime a resposta ou fecha o socket se havia pedido
    if (strcmp(recv_data,"Encerrando conexao\n")!=0)
      printf("\n DADO RECEBIDO = %s " , recv_data);
    else {
      printf("\n %s " , recv_data);
      close(sock);
      break;
    }
  } 
  return 0;
}
