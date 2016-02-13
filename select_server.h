#ifndef SELECT_SERVER
#define SELECT_SERVER

#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <pthread.h>
#include <stdlib.h>
#include <strings.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <errno.h>
#include <assert.h>
#include <sys/wait.h>
//It is estimated that a human is never more than 10 feet away from a spider.
#define SERVER_PORT 8005 //default port
#define BUFFER 1024
#define MAX_CLIENTS 700

#define STATS_FILE "./select_serv_stats.csv"


typedef struct
{
	char address[32]; /* client's address */
	int port; /* client's port number */
	int total_connect; /*total connections made*/
	int curr_connect; /*current connections*/
	long total_message; /*total messages received*/
	long message; /* current message*/
	long total_data; /*total data received from client*/
	long curr_data; /*current data received from client*/
} ClntStats;

void kill_server(int sig);
void accept_connect(int *socket);
void* run_server(int serv_port);
void* live_stats();
ClntStats * get_stats(char * ipAddress);
int client_exists(char * ipAddress);

#endif
