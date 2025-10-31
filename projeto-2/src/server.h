#ifndef _SERVER
#define _SERVER

#include "defines.h"
#include "pthread.h"
#include "semaphore.h"


#define SERVER_PORT 5000

typedef struct __thread_args{
  int idx;
  int fd;
} thread_args;

typedef struct __server {
  struct sockaddr_in server_addr, client_addr[CLI_NUM];
  pthread_t cli_t[CLI_NUM];
  sem_t mutex;
  int curr, tail;
} server;

void *get_connection(void *args);

int main(int argc, char **argv);

#endif
