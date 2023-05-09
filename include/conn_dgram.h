// Copyright (C) Alin Ichim 2023
#ifndef CONN_DGRAM_H_
#define CONN_DGRAM_H_

#include <cstdint>

#include "config.h"

typedef struct conn_dgram conn_dgram_t;

struct conn_dgram {
  char topic_name[TOPIC_NAME_SIZE];
  uint8_t type;
  char data[TOPIC_DATA_SIZE];
};

#endif  // CONN_DGRAM_H_