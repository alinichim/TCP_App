// Copyright (C) Alin Ichim 2023
#include <iostream>
#include <cstring>

#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/tcp.h>
#include <poll.h>
#include <unistd.h>

#include "logger.h"
#include "config.h"
#include "conn_funcs.h"
#include "comm_structs.h"

#define ARGN 4

int main(int argc, char *argv[]) {

  setvbuf(stdout, NULL, _IONBF, BUFSIZ);

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

  // Create socket.
  int sockfd;
  if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
    #ifdef LOG_SU

    logger_error("Program terminated -- Cause: Couldn't create socket");

    #endif  // LOG_SU

    exit(EXIT_FAILURE);
  }

  // Set socket option TCP_NODELAY.
  int yes = 1;
  if ((setsockopt(sockfd, IPPROTO_TCP, TCP_NODELAY, (char *) &yes, sizeof(int))) < 0) {
    #ifdef LOG_SU

    logger_error("Program terminated -- Cause: Couldn't set TCP_NODELAY option for TCP socket");

    #endif  // LOG_SU

    exit(EXIT_FAILURE);
  }

  // Poll through available connection to server and stdin.
  struct pollfd conns[2];
  // For stdin.
  conns[0].fd = STDIN_FILENO;
  conns[0].events = POLLIN;
  // For server.
  conns[1].fd = sockfd;
  conns[1].events = POLLIN;

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

  logger_success("Successfully reached server. Sending CONNECT token...");

  #endif  // LOG_SU

  // Send CONNECT token.
  conn_token_t ctoken;
  memset(&ctoken, 0, sizeof(ctoken));
  ctoken.ctype = CONNECT;
  memcpy(ctoken.client_id, argv[1], strlen(argv[1]) + 1);
  reliable_send(sockfd, &ctoken, sizeof(ctoken));

  // Begin communication process.
  bool exit = false;
  while (!exit) {
    #ifdef LOG_SU

    logger_info("Checking polls...");

    #endif  // LOG_SU

    // Check polls.
    poll(conns, 2, -1);

    if (conns[0].revents & POLLIN) {
      // Command from STDIN.
      #ifdef LOG_SU

      logger_info("Received command from STDIN");

      #endif  // LOG_SU

      std::string cmd;
      std::cin >> cmd;
      if (cmd == "subscribe") {
        // Get topic name.
        std::cin >> cmd;

        // Prepare token.
        memset(&ctoken, 0, sizeof(ctoken));
        ctoken.ctype = SUBS;
        strncpy(ctoken.client_id, argv[1], strlen(argv[1]));
        memcpy(ctoken.data.sub_data.topic_name, cmd.data(), strlen(cmd.data()));
        std::cin >> cmd;
        ctoken.data.sub_data.sf = cmd[0] == '1';

        #ifdef LOG_SU

        logger_info("Sending SUBS token...");

        #endif  // LOG_SU

        // Send token.
        reliable_send(sockfd, &ctoken, sizeof(ctoken));

        std::cout << "Subscribed to topic." << std::endl;
      } else if (cmd == "unsubscribe") {
        // Get topic name.
        std::cin >> cmd;

        // Prepare token.
        memset(&ctoken, 0, sizeof(ctoken));
        ctoken.ctype = UNSUB;
        strncpy(ctoken.client_id, argv[1], strlen(argv[1]));
        memcpy(ctoken.data.sub_data.topic_name, cmd.data(), strlen(cmd.data()));

        #ifdef LOG_SU

        logger_info("Sending UNSUB token...");

        #endif  // LOG_SU

        // Send token.
        reliable_send(sockfd, &ctoken, sizeof(ctoken));

        std::cout << "Unsubscribed from topic." << std::endl;
      } else if (cmd == "exit") {
        #ifdef LOG_SU

        logger_info("Received `exit` command from STDIN. Closing client...");

        #endif  // LOG_SU

        // Prepare DISCONNECT token.
        memset(&ctoken, 0, sizeof(ctoken));
        ctoken.ctype = DISCONNECT;
        strncpy(ctoken.client_id, argv[1], strlen(argv[1]));

        // Send token.
        reliable_send(sockfd, &ctoken, sizeof(ctoken));

        #ifdef LOG_SU

        logger_info("Sent DISCONNECT token");

        #endif  // LOG_SU

        // Close socket.
        close(sockfd);
        exit = true;
      }
    } else if (conns[1].revents & POLLIN) {
      // Input from server.

      #ifdef LOG_SU

      logger_info("Received input from server");

      #endif  // LOG_SU

      reliable_receive(sockfd, &ctoken, sizeof(ctoken));

      if (ctoken.ctype == DISCONNECT) {
        #ifdef LOG_SU

        logger_info("Server is disconnected. Closing client...");

        #endif  // LOG_SU

        // Close socket.
        close(sockfd);
        exit = true;
        // return 0;
      } else if (ctoken.ctype == UPDATE) {
        #ifdef LOG_SU

        logger_info("Received UPDATE token from server");

        #endif  // LOG_SU

        // Print to STDOUT the message.
        std::string message = "";
        char ipaddr[16];
        inet_ntop(AF_INET, &ctoken.data.message.udp_client_addr.sin_addr.s_addr, ipaddr, sizeof(ipaddr));
        message += ipaddr;
        message += ":";
        message += std::to_string(ntohs(ctoken.data.message.udp_client_addr.sin_port));
        message += " - ";
        message += ctoken.data.message.topic_name;
        message += " - ";
        if (ctoken.data.message.topic_data.type == 0) {
          message += "INT - ";
          if (ctoken.data.message.topic_data.data[0])
            message += "-";
          
          int *p = (int *)(ctoken.data.message.topic_data.data + 1);
          message += std::to_string(ntohl(*p));
        } else if (ctoken.data.message.topic_data.type == 1) {
          message += "SHORT_REAL - ";

          uint16_t *p = (uint16_t *)ctoken.data.message.topic_data.data;
          
          message += std::to_string(ntohs(*p) / 100);
          if (ntohs(*p) % 100) {
            message += ".";
            for (uint8_t i = 1; i <= (2 - (uint8_t)std::to_string(ntohs(*p) % 100).length()); ++i) message += "0";
            message += std::to_string(ntohs(*p) % 100);
          }
        } else if (ctoken.data.message.topic_data.type == 2) {
          message += "FLOAT - ";
          if (ctoken.data.message.topic_data.data[0])
            message += "-";
          
          uint32_t *p = (uint32_t *)(ctoken.data.message.topic_data.data + 1);
          uint8_t q = *(ctoken.data.message.topic_data.data + 5);

          int e = 1;
          for (uint8_t i = 1; i <= q; ++i) e *= 10;
          message += std::to_string(ntohl(*p) / e);
          if (ntohl(*p) % e) {
            message += ".";
            for (uint8_t i = 1; i <= (q - (uint8_t)std::to_string(ntohl(*p) % e).length()); ++i) message += "0";
            message += std::to_string(ntohl(*p) % e);
          }
        } else if (ctoken.data.message.topic_data.type == 3) {
          message += "STRING - ";
          message += ctoken.data.message.topic_data.data;
        }
        std::cout << message << std::endl;
      }
    }
  }

  #ifdef LOG_SU

  logger_success("Client closed successfully");

  #endif  // LOG_SU

  return 0;
}