#ifndef _COMMON_H_
#define _COMMON_H_


#include <stdio.h> // print
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/select.h> //select
#include <poll.h> // poll
#include <sys/epoll.h> //epoll
#include <sys/stat.h>

#endif

#define FOUNDERROR (-1)
#define PORT 9999
#define MAX_CLIENTS 10
#define MAX_EVENTS 10
#define MAX_BUFFER_SIZE 1024