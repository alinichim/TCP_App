// Copyright (C) Alin Ichim 2023
#ifndef COMMM_STRUCTS_H_
#define COMMM_STRUCTS_H_

#include <string>
#include <cstdint>

#include <sys/socket.h>
#include <arpa/inet.h>

#include "config.h"

#define CONNDATA_SIZE 1024

typedef struct conn_token conn_token_t;
typedef struct client_msg client_msg_t;
typedef struct topic topic_t;

enum token_type {CONNECT, DISCONNECT, SUBS, UNSUB, UPDATE};

struct topic {
  uint8_t type;
  char data[TOPIC_DATA_SIZE];
};

struct client_msg {
  struct sockaddr_in udp_client_addr;
  char topic_name[TOPIC_NAME_SIZE];
  topic_t topic_data;
};

struct conn_token {
  token_type ctype;  // Connection type.
  union {
    char client_id[CLID_SIZE];
    struct {
      bool sf;
      char topic_name[TOPIC_NAME_SIZE];
    } sub_data;
    client_msg_t message;
  } data;
};

#endif  // COMMM_STRUCTS_H_