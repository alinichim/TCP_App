// Copyright (C) Alin Ichim 2023
#include <iostream>
#include <cstring>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <poll.h>
#include <unistd.h>

#include "logger.h"
#include "config.h"
#include "conn_funcs.h"

#define ARGN 4

int main(int argc, char *argv[]) {
  #ifdef LOG_SU

  // Initialize logger.
  std::string logfile = LOG_SU_FILE; 
  logfile += "_";
  logfile += argv[1];
  logfile += ".log";
  logger_init(logfile);
  logger_info("Logger initialized");

  #endif  // LOG_SU

  // Check arguments.
  if (argc != ARGN) {
    #ifdef LOG_SU

    logger_error("Program terminated -- Cause: Invalid number of arguments");

    #endif  // LOG_SU

    exit(EXIT_FAILURE);
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

  // Create socket.
  int sockfd;
  if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
    #ifdef LOG_SU

    logger_error("Program terminated -- Cause: Couldn't create socket");

    #endif  // LOG_SU

    exit(EXIT_FAILURE);
  }

  // TODO: Set socket option TCP_NODELAY.

  // Poll through available connection to server and stdin.
  struct pollfd conns[2];
  // For stdin.
  conns[0].fd = STDIN_FILENO;
  conns[0].events = POLLIN;
  // For server.
  conns[1].fd = sockfd;
  conns[1].events = POLLIN | POLLOUT;
  #ifdef LOG_SU

  logger_info("Registered file descriptors");

  #endif  // LOG_SU

  // Build server address.
  struct sockaddr_in server_addr;
  socklen_t server_addr_len = sizeof(server_addr);
  if (inet_pton(AF_INET, argv[2], &server_addr.sin_addr.s_addr) <= 0) {
    #ifdef LOG_SU

    logger_error("Program terminated -- Cause: Invalid server address");

    #endif  // LOG_SU

    close(sockfd);
    exit(EXIT_FAILURE);
  }
  server_addr.sin_port = htons(atoi(argv[3]));
  server_addr.sin_family = AF_INET;
  // Connect to server.
  if (connect(sockfd, (struct sockaddr *)&server_addr, server_addr_len)) {
    #ifdef LOG_SU

    logger_error("Program terminated -- Cause: Couldn't connect to server");

    #endif  // LOG_SU

    close(sockfd);
    exit(EXIT_FAILURE);
  }
  #ifdef LOG_SU

  logger_success("Successfully connected to server");

  #endif  // LOG_SU

  // TODO: Poll connections and communicate with server.

  std::string msg = "A journey of a thousand miles begins with a single step";
  reliable_send(sockfd, (void *)msg.data(), msg.length() + 1);

  close(sockfd);
  return 0;
}