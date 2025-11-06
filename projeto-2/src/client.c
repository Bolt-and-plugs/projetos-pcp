#include "client.h"

int main(int argc, char **argv) {
  int x = atoi(argv[1]), y = atoi(argv[2]);
  int sock, bytes_recv;
  struct hostent *host;
  struct sockaddr_in server_addr;
  char send_data[sizeof(int)*x*y], recv_data[sizeof(int)*x*y];

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
    bytes_recv=recv(sock,recv_data,x*y,0);
    recv_data[bytes_recv] = '\0';
    // Imprime a resposta ou fecha o socket se havia pedido
    if (strcmp(recv_data,"q\n")!=0 || strcmp(recv_data,"Q\n")!=0) {
      printf("\n DADO RECEBIDO = %s " , recv_data);
      // TODO processar fatia da imagem recebida
      uint8_t *orig = (uint8_t*)malloc((size_t)x * (size_t)y);
      memcpy(orig, recv_data, (size_t)x * (size_t)y);
      for (int r = 0; r < y; r++) {
        for (int c = 0; c < x; c++) {
            int sum = 0, n = 0;
            int idx = r * x + c;
            if (r > 0)     { sum += orig[(r - 1) * x + c]; n++; }
            if (r < y - 1) { sum += orig[(r + 1) * x + c]; n++; }  
            if (c > 0)     { sum += orig[r * x + (c - 1)]; n++; }   
            if (c < x - 1) { sum += orig[r * x + (c + 1)]; n++; }   
            send_data[idx] = (sum / n);
        }
      }
      free(orig);
      send(sock, send_data, x*y, 0);
    }
    else {
      printf("\n %s " , recv_data);
      close(sock);
      break;
    }
  } 
  return 0;
}
