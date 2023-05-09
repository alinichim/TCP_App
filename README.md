# TCP Application

## Description

A TCP Application that consists of a server, TCP clients and UDP clients that act as workers.

### Main Protocol

The protocol that is used by the server and TCP clients is represented by the messages that have a specific structure.
A token stores the type of the token and the data it carries. These tokens are sent in an organized order based. The
first time a client connects to the server, it sends a CONNECT token with its client ID. The server processes this
token and may send a DISCONNECT token if the client is already connected to the application. After this stage, the
server may receive from clients SUBS and UNSUB tokens that are used to process subscriptions, or DISCONNECT tokens to
signal that the client is closing the connection. The server may send UPDATE tokens to clients and attach the data
needed to print the requested output. When the server shuts down, it sends DISCONNECT tokens to all of its connected
clients.

### Store-and-Forward Implementation

The store-and-forward implementation consists of a queue that is updated with new notifications every time a client
(with SF=1) is disconnected and it's cleared right after the client establishes a new connection to the server.

### Configurations

The application parameters can be tweaked in `config.h` file. You can disable the logger used for debugging or change
the output log files. Connection-related parameters like the *chunk size* for send/receive functions, listening backlog
or poll timeout can also be changed.

### Building

To compile both server and subscriber, you can run the command:

```
make
```

This is going to build object files in `build/` directory, while the `server` and `subscriber` executables are built in
the main directory.

To clean everything, run the command:

```
make clean
```