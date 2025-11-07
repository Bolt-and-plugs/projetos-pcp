#include "client.h"

int main(int argc, char **argv) {
  if (argc < 4) {
    return 0;
  }

  int x = atoi(argv[2]), y = atoi(argv[3]);
  int sock, bytes_recv;
  struct hostent *host;
  struct sockaddr_in server_addr;

  char *send_data, *recv_data;

  size_t buffer_size = sizeof(int) * (size_t)x * (size_t)y;
  send_data = malloc(buffer_size);
  recv_data = malloc(buffer_size);
  if (send_data == NULL || recv_data == NULL) {
    perror("Falha ao alocar buffers");
    exit(1);
  }

  host = gethostbyname(argv[1]);
  if (host == NULL) {
    herror("gethostbyname");
    exit(1);
  }

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
    send(sock, "ok", strlen("ok"), 0);
    // Recebe resposta do servidor
    bytes_recv=recv(sock,recv_data,x*y*sizeof(int),0);
    recv_data[bytes_recv] = '\0';
    // Imprime a resposta ou fecha o socket se havia pedido
    if (strcmp(recv_data,"q")!=0 || strcmp(recv_data,"Q")!=0) {
      int *orig = (int*)malloc(x * y * sizeof(int));
      int *processed_data = (int*)send_data; 
      memcpy(orig, recv_data, (int)x * (int)y * sizeof(int));
      for (int r = 0; r < y; r++) {
        for (int c = 0; c < x; c++) {
            int sum = 0, n = 0;
            int idx = r * x + c;
            if (r > 0)     { sum += orig[(r - 1) * x + c]; n++; }
            if (r < y - 1) { sum += orig[(r + 1) * x + c]; n++; }  
            if (c > 0)     { sum += orig[r * x + (c - 1)]; n++; }   
            if (c < x - 1) { sum += orig[r * x + (c + 1)]; n++; }   
            processed_data[idx] = (sum / n);
        }
      }
      free(orig);
      printf("Processed chunk;\n");
      send(sock, processed_data, x*y*sizeof(int), 0);
    }
    else {
      printf("\n %s " , recv_data);
      close(sock);
      break;
    }
  } 

  free(recv_data);
  free(send_data);
  return 0;
}
