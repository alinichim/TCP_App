// Copyright (C) Alin Ichim 2023
#include <iostream>
#include <cstring>
#include <sys/socket.h>
#include <arpa/inet.h>

#include "logger.h"
#include "config.h"

#define ARGN 4

int main(int argc, char *argv[]) {
  #ifdef LOG_SU

  // Initialize logger.
  logger_init(LOG_SU_FILE);
  logger_info("Logger initialized");

  #endif  // LOG_SU

  // Check arguments.
  if (argc != ARGN) {
    #ifdef LOG_SU

    logger_error("Program terminated -- Cause: Invalid number of arguments");

    #endif  // LOG_SU

    return -1;
  }

  #ifdef LOG_SU

  std::string log_msg = "Program recieved arguments: ";
  log_msg += argv[1];
  log_msg += " ";
  log_msg += argv[2];
  log_msg += " ";
  log_msg += argv[3];

  logger_info(log_msg);

  #endif  // LOG_SU

  // Get program arguments.
  char client_id[CLID_SIZE];
  int server_addr, server_port;
  strncpy(client_id, argv[1], strlen(argv[1]));
  inet_aton(argv[2], (in_addr *)&server_addr);
  server_port = atoi(argv[3]);

  // Create socket.
  int sockfd = socket(AF_INET, SOCK_STREAM, 0);


  return 0;
}