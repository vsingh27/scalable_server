#ifndef MULTI_THREAD_SERVER
#define MULTI_THREAD_SERVER

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

#define SERVER_PORT 8005 //default port
#define BUFFLEN 80
#define MAX_THREADS 200
#define MAX_CLIENTS 100 //maximum clients per thread
#define MAX_IOSIZE 65536
#define USLEEP_TIME 1

#define STATS_FILE "./srvr_stats.csv"

typedef struct 
{
	pthread_t t_id; //the server socket's id
	int current_thread; //current thread number
	int num_client; //clients being served by thread
	int fd_client[MAX_CLIENTS]; //clients file descriptor
	int stats_pos[MAX_CLIENTS]; 
} SocketThread;

typedef struct
{
	char address[17];			/* client's address */
	int port;					/* remote port for client */
	unsigned long requests;		/* total requests generated */
	unsigned long sent_data;	/* total data sent to */
} ClntStats;


SocketThread threads[MAX_THREADS];
ClntStats clnt_stats[MAX_THREADS * MAX_CLIENTS];

static int clnt_pos = 0;

void *serve_clients(void *p);
void accept_connect(int fd);
int find_free_thread(void);
int find_free_client(int thread_num);
void kill_server(int sig);
#endif