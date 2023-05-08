#include <iostream>
#include <sys/socket.h>
#include <poll.h>

#include "config.h"

#ifdef LOG_SE

#include "logger.h"

#endif  // LOG_SE

int main(void) {
  
  #ifdef LOG_SE

  logger_init(LOG_SE_FILE);
  logger_info("Logger initialized!");

  #endif  // LOG_SE

  // TODO: Server functionality.
  
  return 0;
}