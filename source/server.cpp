#include <iostream>
#include <map>
#include <string>
#include <cstring>

#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/tcp.h>
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
  if ((sockfd_tcp = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0)) < 0) {
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

  // Set socket option TCP_NODELAY.
  int yes = 1;
  if ((setsockopt(sockfd_tcp, IPPROTO_TCP, TCP_NODELAY, (char *) &yes, sizeof(int))) < 0) {
    #ifdef LOG_SU

    logger_error("Program terminated -- Cause: Couldn't set TCP_NODELAY option for TCP socket");

    #endif  // LOG_SU

    exit(EXIT_FAILURE);
  }

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
  #ifdef LOG_SE

  logger_info("Server sockets are binded to their address");

  #endif  // LOG_SE

  // Set server to listen for incoming connections.
  if ((listen(sockfd_tcp, BACKLOG))) {
    #ifdef LOG_SE

    logger_error("Program terminated -- Cause: Couldn't listen on server");

    #endif  // LOG_SE

    close(sockfd_tcp);
    close(sockfd_udp);
    exit(EXIT_FAILURE);
  }
  #ifdef LOG_SE

  logger_info("Server is listening for incoming connections...");

  #endif  // LOG_SE

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
  client_p.events = POLLIN;
  conns.push_back(client_p);

  #ifdef LOG_SE

  logger_info("Polls created");

  #endif  // LOG_SE

  // Begin communication process.
  int new_client_sockfd;  // Used to register a new client.
  struct sockaddr_in new_client_sockaddr;  // Used to store new client address.
  socklen_t new_client_addr_len = sizeof(new_client_sockaddr);  // Client address length.
  while (conns.size()) {
    #ifdef LOG_SE

    logger_info("Server is checking for incoming connections...");

    #endif  // LOG_SE
    // Check for an incoming connection.
    new_client_sockfd =
      accept(sockfd_tcp, (struct sockaddr *)&new_client_sockaddr, &new_client_addr_len);
    if (new_client_sockfd != -1) {  // New connection.
      #ifdef LOG_SE

      logger_success("Got a new TCP connection on socket " + std::to_string(new_client_sockfd));

      #endif  // LOG_SE

      // Await connection token from client.
      conn_token_t token;
      reliable_receive(new_client_sockfd, &token);

      #ifdef LOG_SE

      logger_success("Token recived");

      #endif  // LOG_SE

      // Check if the client was already registered.
      std::string client_id = token.client_id;
      std::map<std::string, client_sub_t>::iterator it = client_subs.find(client_id);
      if (it != client_subs.end()) {
        #ifdef LOG_SE

        logger_info("Client already registered. Checking if client is already connected...");

        #endif  // LOG_SE

        if (it->second.connected) {
          #ifdef LOG_SE

          logger_info("Client already connected. Logging event to STDOUT...");

          #endif  // LOG_SE

          // Close connections.
          conn_token_t ctoken;
          ctoken.ctype = DISCONNECT;
          reliable_send(new_client_sockfd, &ctoken, sizeof(ctoken));

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
        client_subs[client_id] = client_data;
      }

      // Log event to STDOUT.
      char addr[16];
      inet_ntop(AF_INET, &new_client_sockaddr.sin_addr.s_addr, addr, 16);
      std::cout << "New client " << token.client_id << " connected from ";
      std::cout << addr << ":" << new_client_sockaddr.sin_port << std::endl;

      #ifdef LOG_SE

      logger_info("Adding new socket to polls...");

      #endif  // LOG_SE

      // Add socket to polls.
      struct pollfd new_poll;
      new_poll.fd = new_client_sockfd;
      new_poll.events = POLLIN;
      conns.push_back(new_poll);

      #ifdef LOG_SE

      std::string log_msg = "Current polls: ";
      for (auto it = conns.begin(); it != conns.end(); ++it) {
        log_msg += std::to_string(it->fd);
        log_msg += " ";
      }
      logger_info(log_msg);

      #endif  // LOG_SE

      continue;
    }

    #ifdef LOG_SE

    logger_info("No connection recived. Checking polls...");

    #endif  // LOG_SE

    // Check new input.
    poll(conns.data(), conns.size(), POLL_TIMEOUT);

    for (struct pollfd conn : conns) {
      #ifdef LOG_SE

      logger_info("Polls: Checking socket " + std::to_string(conn.fd));

      #endif  // LOG_SE
      if (conn.revents & POLLIN) {
        #ifdef LOG_SE

        logger_info("Found connection with new input");

        #endif  // LOG_SE
        // Check if this is input from STDIN.
        if (conn.fd == STDIN_FILENO) {
          std::string cmd;
          std::cin >> cmd;
          if (cmd == "exit") {
            #ifdef LOG_SE

            logger_info("Recived `exit` command from STDIN. Closing all connections...");

            #endif  // LOG_SE

            // Close connections.
            conn_token_t ctoken;
            ctoken.ctype = DISCONNECT;
            for (auto it = conns.begin(); it != conns.end(); ++it) {
              // Send DISCONNECT token to TCP clients.
              if (it->fd != STDIN_FILENO && it->fd != sockfd_udp)
                reliable_send(it->fd, &ctoken, sizeof(ctoken));

              // Close connection.
              close(it->fd);
            }
            conns.clear();
          }
          break;
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
          break;
        }

        #ifdef LOG_SE

        logger_info("Receiving token from TCP client...");

        #endif  // LOG_SE

        // Check token from TCP client.
        conn_token_t ctoken;
        reliable_receive(conn.fd, &ctoken);

        #ifdef LOG_SE

        logger_info("Received token type: " + std::to_string(ctoken.ctype));

        #endif  // LOG_SE

        if (ctoken.ctype == DISCONNECT) {
          #ifdef LOG_SE

          logger_info("Token type: DISCONNECT");

          #endif  // LOG_SE

          // Close connection.
          close(conn.fd);
          // Remove connection from polls.
          for (auto it = conns.begin(); it != conns.end(); ++it) {
            if (it->fd == conn.fd) {
              conns.erase(it);
              break;
            }
          }
          // Set `connected` flag to false.
          std::string clid = ctoken.client_id;
          client_subs[clid].connected = false;
          client_subs[clid].sockfd = -1;

          // Print to STDOUT.
          std::cout << "Client " << clid << " disconnected." << std::endl;
          break;
  
        } else if (ctoken.ctype == SUBS) {
          #ifdef LOG_SE

          logger_info("Token type: SUBS");

          #endif  // LOG_SE

          // Add new subscription.
          std::string clid = ctoken.client_id;
          client_subs[clid].topics.push_back(ctoken.data.sub_data.topic_name);
          client_subs[clid].sf = ctoken.data.sub_data.sf;
        } else if (ctoken.ctype == UNSUB) {
          #ifdef LOG_SE

          logger_info("Token type: UNSUB");

          #endif  // LOG_SE

          // Remove subscription.
          std::string clid = ctoken.client_id;
          for (auto jt = client_subs[clid].topics.begin(); jt != client_subs[clid].topics.end(); ++jt) {
            if (!strncmp(jt->data(), ctoken.data.sub_data.topic_name, strlen(jt->data()))) {
              client_subs[clid].topics.erase(jt);
              break;
            }
          }
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