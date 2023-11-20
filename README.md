# Multi-Strategy Server-Client Communication

## Overview
This C program demonstrates a server-client communication system using four different strategies to handle multiple requests from multiple clients. The strategies include `fork()`, `select()`, `poll()`, and `epoll()`.

The communication involves clients sending messages to the server. The server, upon receiving a message, writes it to a file and then sends the written file back to the respective client. The sent files will be saved in the `results` directory.

## File Structure
- `client.c`: Client implementation.
- `server.c`: Server implementation.
- `Makefile`: Makefile for compilation.
- `results/`: Directory to store files sent by the server.

## Build and Run Instructions

### Initial Setup
1. Compile the program using the provided Makefile:
   make

## Start Server
Run the server:
./server

## Start Client
Open a new terminal window and run the client:
./client -ip 127.0.0.1 -p 9999

## Strategies
The server utilizes four strategies to handle multiple client requests:

1. fork(): Uses fork to create a new process for each client.
2. select(): Utilizes the select system call for managing multiple file descriptors.
3. poll(): Uses the poll system call for asynchronous I/O operations.
4. epoll(): Employs the epoll mechanism for efficient event monitoring.
Choose the desired strategy by modifying the SERVER_STRATEGY macro in the server.c file.

## Notes
The sent files from the server will be stored in the results directory.
Make sure to have the necessary permissions for file read and write operations.

