#ifndef _EPOLL_CLIENT_
#define _EPOLL_CLIENT_

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <sys/param.h>
#include <netdb.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <err.h>
#include <time.h>
#include <signal.h>
#include <pthread.h>



#define NO_OF_CONNECTIONS       10      //Default Number of Threads

typedef struct
{
    char *IP[17];               //IP Address of the Client
    unsinged int port;          //Port No Used by the Client to write Data
    unsinged int no_of_requests;//No of requests made by Client to the Server
} client_stats

#endif
