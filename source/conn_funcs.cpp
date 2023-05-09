// Copyright (C) Alin Ichim 2023
#include <sys/socket.h>

#include "conn_funcs.h"
#include "config.h"

int reliable_send(int sockfd, void *data, int data_size) {
  int packets_num = 1;
  int off = 0;
  int next_chunk_size = (data_size < CHUNK_SIZE) ? data_size : CHUNK_SIZE;

  while ((send(sockfd, data + off, next_chunk_size, 0)) == CHUNK_SIZE) {
    packets_num++;
    off += next_chunk_size;
    data_size -= next_chunk_size;
    next_chunk_size = (data_size < CHUNK_SIZE) ? data_size : CHUNK_SIZE;
  }

  return packets_num;
}

int reliable_receive(int sockfd, void *data) {
  int packets_num = 1;
  int off = 0;

  while ((recv(sockfd, data + off, CHUNK_SIZE, 0)) == CHUNK_SIZE) {
    packets_num++;
    off += CHUNK_SIZE;
  }

  return packets_num;
}
