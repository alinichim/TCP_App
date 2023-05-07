#include <iostream>
#include <sys/socket.h>
#include <poll.h>

#define DEBUG_SE
#define LOG_SE_FILE "logs_server.log"

#ifdef DEBUG_SE

#include "logger.h"

#endif  // DEBUG_SE

int main(void) {
  
  #ifdef DEBUG_SE

  logger_init(LOG_SE_FILE);
  logger_info("Logger initialized!");

  #endif  // DEBUG_SE

  // TODO: Server functionality.
  
  return 0;
}