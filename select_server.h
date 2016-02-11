#ifndef SELECT_SERVER
#define SELECT_SERVER

#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <strings.h>
#include <arpa/inet.h>
#include <unistd.h>

#define SERVER_PORT 8005 //default port
#define BUFFER 1024 

#define STATS_FILE "./select_serv_stats.csv"


typedef struct
{
	char address[17]; /* client's address */
	int port; /* client's port number */
	unsigned long requests; /* total requests from client */
	unsigned long sent_data; /* total data sent from client */
}




#endif