// Copyright (C) Alin Ichim 2023
#ifndef CONFIG_H_
#define CONFIG_H_

// Used for logging the server.
#define LOG_SE
// Used for logging the subscriber.
#define LOG_SU
#define LOG_COM
// Server logs file.
#define LOG_SE_FILE "logs/logs_server.log"
// Subscriber logs file.
#define LOG_SU_FILE "logs/logs_subscriber"

// Clinet ID size.
#define CLID_SIZE 12

// Chunk size.
#define CHUNK_SIZE 128

// Listening backlog.
#define BACKLOG 64

// Size of topic name.
#define TOPIC_NAME_SIZE 50
// Size of topic data.
#define TOPIC_DATA_SIZE 1500

// Timeout for polls check.
#define POLL_TIMEOUT 1500

#endif  // CONFIG_H_