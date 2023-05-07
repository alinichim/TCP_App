// Copyright (C) Alin Ichim 2023
#include <iostream>
#include <fstream>
#include <ctime>
#include <string>
#include <cstring>

#include "logger.h"

#define FILENAME_LEN 50

static std::string filename;
static time_t rawtime;

void logger_init(std::string fname) {
  filename = fname;
  std::ofstream fout(filename);
  fout.close();
}

static void logger_msg(std::string prefix, std::string message) {
  time(&rawtime);

  std::ofstream fout(filename, std::ios::app);

  char *t = ctime(&rawtime);
  t[strlen(t) - 1] = '\0';

  fout << "(" << t << ") " << prefix << " " << message << std::endl;

  fout.close();
}

void logger_info(std::string message) {
  logger_msg("[!]", message);
}

void logger_error(std::string message) {
  logger_msg("[-]", message);
}

void logger_success(std::string message) {
  logger_msg("[+]", message);
}
