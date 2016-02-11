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
//end my misery 
#define SERVER_PORT 8005 //default port
#define BUFFER 1024 
#define MAX_CLIENTS 200

#define STATS_FILE "./select_serv_stats.csv"


typedef struct
{
	char address[17]; /* client's address */
	int port; /* client's port number */
	unsigned long requests; /* total requests from client */
	unsigned long sent_data; /* total data sent from client */
} ClntStats;

ClntStats clnt_stats[MAX_CLIENTS];

static int clnt_pos = 0;

void kill_server(int sig);
void accept_connect(int fd);
int create_socket();
int set_socket_reuse(int* socket);
int bindAddyToSocket(int* socket, int* port);


#endif
