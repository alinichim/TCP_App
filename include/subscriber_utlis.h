// Copyright (C) Alin Ichim 2023
#ifndef SUBSCRIBER_UTILS_H_
#define SUBSCRIBER_UTILS_H_

#include "comm_structs.h"

/**
 * @brief Processes recived token.
 * 
 * @param token The token.
 */
void process_token(conn_token_t *token);

#endif  // SUBSCRIBER_UTILS_H_