#include <iostream>
#include <map>
#include <string>
#include <cstring>

#include <sys/socket.h>
#include <arpa/inet.h>
#include <poll.h>
#include <unistd.h>

#include "config.h"
#include "conn_funcs.h"
#include "server_structs.h"
#include "comm_structs.h"
#include "conn_dgram.h"

#define ARGN 2

#ifdef LOG_SE

#include "logger.h"

#endif  // LOG_SE

int main(int argc, char *argv[]) {
  
  #ifdef LOG_SE

  logger_init(LOG_SE_FILE);
  logger_info("Logger initialized!");

  #endif  // LOG_SE

  // Check arguments.
  if (argc != ARGN) {
    #ifdef LOG_SU

    logger_error("Program terminated -- Cause: Invalid number of arguments");

    #endif  // LOG_SU

    exit(-1);
  }

  // Create one socket for TCP connections and one for UDP connections.
  int sockfd_tcp, sockfd_udp;
  if ((sockfd_tcp = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
    #ifdef LOG_SU

    logger_error("Program terminated -- Cause: Couldn't create socket");

    #endif  // LOG_SU

    exit(EXIT_FAILURE);
  }
  if ((sockfd_udp = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
    #ifdef LOG_SU

    logger_error("Program terminated -- Cause: Couldn't create socket");

    #endif  // LOG_SU

    exit(EXIT_FAILURE);
  }

  // TODO: Set socket option TCP_NODELAY.

  // Build server address.
  struct sockaddr_in server_addr;
  socklen_t server_addr_len = sizeof(server_addr);
  server_addr.sin_addr.s_addr = INADDR_ANY;
  server_addr.sin_port = htons(atoi(argv[1]));
  server_addr.sin_family = AF_INET;

  // Bind server socket to it's IP address and port.
  if ((bind(sockfd_tcp, (struct sockaddr *)&server_addr, server_addr_len)) < 0) {
    #ifdef LOG_SE

    logger_error("Program terminated -- Cause: Couldn't bind the server socket to it's address");

    #endif  // LOG_SE

    close(sockfd_tcp);
    close(sockfd_udp);
    exit(EXIT_FAILURE);
  }
  if ((bind(sockfd_udp, (struct sockaddr *)&server_addr, server_addr_len)) < 0) {
    #ifdef LOG_SE

    logger_error("Program terminated -- Cause: Couldn't bind the server socket to it's address");

    #endif  // LOG_SE

    close(sockfd_tcp);
    close(sockfd_udp);
    exit(EXIT_FAILURE);
  }

  // Set server to listen for incoming connections.
  if ((listen(sockfd_tcp, BACKLOG))) {
    #ifdef LOG_SE

    logger_error("Program terminated -- Cause: Couldn't listen on server");

    #endif  // LOG_SE

    close(sockfd_tcp);
    close(sockfd_udp);
    exit(EXIT_FAILURE);
  }

  // TODO: Begin implementation.
  // Clients subscription data map [client_id, client_sub_data].
  std::map<std::string, client_sub_t> client_subs;

  // Create poll with current connections.
  std::vector<struct pollfd> conns;
  // Add the STDIN and the socket to polls.
  struct pollfd client_p;
  // For STDIN.
  client_p.fd = STDIN_FILENO;
  client_p.events = POLLIN;
  conns.push_back(client_p);
  // For UDP socket.
  client_p.fd = sockfd_udp;
  client_p.fd = POLLIN;
  conns.push_back(client_p);

  // Begin communication process.
  int new_client_sockfd;  // Used to register a new client.
  struct sockaddr_in new_client_sockaddr;  // Used to store new client address.
  socklen_t new_client_addr_len = sizeof(new_client_sockaddr);  // Client address length.
  while (conns.size()) {
    // Check for an incoming connection.
    new_client_sockfd =
      accept4(sockfd_tcp, (struct sockaddr *)&new_client_sockaddr, &new_client_addr_len, SOCK_NONBLOCK);
    if (new_client_sockfd != -1) {  // New connection.
      #ifdef LOG_SE

      logger_success("Got a new TCP connection");

      #endif  // LOG_SE

      // Await connection token from client.
      conn_token_t token;
      reliable_receive(new_client_sockfd, &token);

      #ifdef LOG_SE

      logger_success("Token recived");

      #endif  // LOG_SE

      // TODO: Actions based on token type.

      // Check if the client was already registered.
      std::string client_id = token.data.client_id;
      std::map<std::string, client_sub_t>::iterator it = client_subs.find(client_id);
      if (it != client_subs.end()) {
        #ifdef LOG_SE

        logger_info("Client already registered. Checking if client is already connected...");

        #endif  // LOG_SE

        if (it->second.connected) {
          #ifdef LOG_SE

          logger_info("Client already connected. Logging event to STDOUT...");

          #endif  // LOG_SE

          std::cout << "Client " << client_id << " already connected." << std::endl;
          continue;
        } else {
          #ifdef LOG_SE

          logger_info("Sending queued messages to client...");

          #endif  // LOG_SE

          // Update client status.
          client_subs[client_id].connected = true;
          client_subs[client_id].sockfd = new_client_sockfd;

          // Send messages queued for the client.
          conn_token_t send_token;
          while (!client_subs[client_id].msg_q.empty()) {
            send_token.ctype = UPDATE;
            memcpy(&send_token.data.message, &client_subs[client_id].msg_q.front(), sizeof(client_msg_t));
            client_subs[client_id].msg_q.pop();
            reliable_send(new_client_sockfd, &send_token, sizeof(send_token));
          }
        }
      } else {
        #ifdef LOG_SE

        logger_info("Registering new client...");

        #endif  // LOG_SE

        // Register new client.
        client_sub_t client_data;
        client_data.sockfd = new_client_sockfd;
        client_data.connected = true;
        client_data.sf = token.data.sub_data.sf;
        client_data.topics.push_back(token.data.sub_data.topic_name);
        client_subs[client_id] = client_data;
      }

      // Log event to STDOUT.
      char addr[16];
      inet_ntop(AF_INET, &new_client_sockaddr.sin_addr.s_addr, addr, 16);
      std::cout << "New client " << token.data.client_id << " connected from ";
      std::cout << addr << ":" << new_client_sockaddr.sin_port << std::endl;

      #ifdef LOG_SE

      logger_info("Adding new socket to polls...");

      #endif  // LOG_SE

      // Add socket to polls.
      client_p.fd = new_client_sockfd;
      client_p.fd = POLLIN;
      conns.push_back(client_p);

      continue;
    }

    #ifdef LOG_SE

    logger_info("No connection recived. Checking polls...");

    #endif  // LOG_SE

    // Check new input.
    poll(conns.data(), conns.size(), POLL_TIMEOUT);

    for (struct pollfd conn : conns) {
      if (conn.revents & POLLIN) {
        // Check if this is input from STDIN.
        if (conn.fd == STDIN_FILENO) {
          std::string cmd;
          std::cin >> cmd;
          if (cmd == "exit") {
            #ifdef LOG_SE

            logger_info("Recived `exit` command from STDIN. Closing all connections...");

            #endif  // LOG_SE
            // TODO: Close all conections.
            // TODO: Exit program.
          }
          continue;
        }
        // Check if this is a package from an UDP client.
        if (conn.fd == sockfd_udp) {
          #ifdef LOG_SE

          logger_info("Receiving datagram from UDP client...");

          #endif  // LOG_SE

          // Recive datagram from client.
          conn_dgram_t datagram;
          struct sockaddr_in client_udp_addr;
          socklen_t slen = sizeof(client_udp_addr);
          if ((recvfrom(conn.fd, &datagram, sizeof(datagram), MSG_WAITALL, (struct sockaddr *)&client_udp_addr,
          &slen)) == -1) {
            #ifdef LOG_SE

            logger_error("Program terminated -- Cause: Couldn't receive data from UDP client");

            #endif  // LOG_SE

            exit(EXIT_FAILURE);
          }

          // Send the update to all connected TCP clients and enqueue for TCP clients that are not connected and have
          // SF activated.
          std::string topic_name = datagram.topic_name;
          client_msg_t update;
          memcpy(update.topic_name, datagram.topic_name, sizeof(datagram.topic_name));
          memcpy(&update.udp_client_addr, &client_udp_addr, sizeof(client_udp_addr));
          update.topic_data.type = datagram.type;
          memcpy(update.topic_data.data, datagram.data, sizeof(datagram.data));
          for (std::map<std::string, client_sub_t>::iterator it = client_subs.begin(); it != client_subs.end(); ++it) {
            for (std::string tname : it->second.topics) {
              if (tname == topic_name) {
                if (it->second.connected) {
                  // Send update.
                  conn_token_t ctoken;
                  ctoken.ctype = UPDATE;;
                  memcpy(&ctoken.data.message, &update, sizeof(update));
                  reliable_send(it->second.sockfd, &ctoken, sizeof(ctoken));
                } else if (it->second.sf) {
                  // Enqueue update.
                  it->second.msg_q.push(update);
                }
                break;
              }
            }
          }

          // TODO: Check token from TCP client.
        }
        break;
      }
    }
  }

  #ifdef LOG_SE

  logger_success("Server closed successfully");

  #endif  // LOG_SE

  return 0;
}