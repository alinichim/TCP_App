// Copyright (C) Alin Ichim 2023
#ifndef SERVER_STRUCTS_H_
#define SERVER_STRUCTS_H_

#include <string>
#include <vector>
#include <queue>
#include <cstdint>

#include "config.h"
#include "comm_structs.h"

typedef struct client_sub client_sub_t;

struct client_sub {
  int sockfd;
  bool connected;
  bool sf;
  std::vector<std::string> topics;
  std::queue<client_msg_t> msg_q;
};

#endif  // SERVER_STRUCTS_H_