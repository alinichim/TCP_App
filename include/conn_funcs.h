// Copyright (C) Alin Ichim 2023
#ifndef CONN_FUNCS_H_
#define CONN_FUNCS_H_

/**
 * @brief Reliable sends data to socket.
 * 
 * @param sockfd The socket file descriptor.
 * @param data Data to be sent.
 * @param data_size Size of data.
 * @return Number of packets sent.
 */
int reliable_send(int sockfd, void *data, int data_size);

/**
 * @brief Reliable recieves data from socket.
 * 
 * @param sockfd The socket file descriptor.
 * @param data Location to store data.
 * @return Number of packets recieved.
 */
int reliable_recieve(int sockfd, void *data);

#endif  // CONN_FUNCS_H_