#include "select_server.h"

//friday 20:10 revision
//globals
struct timeval start, end;
pthread_t tm; //main thread
int set_debug = 0;
ClntStats * srvr_stats;
int srvrStats_len = 0;
int fd_server;

int main(int argc, char *argv[])
{
	//(void)signal(SIGINT, kill_server);
	setbuf(stdout, NULL);
	run_server(SERVER_PORT);

}

int client_exists(char * checkAddress)
{
	int c;
	for (c = 0; c < srvrStats_len; c++) {

		if ((strcmp(checkAddress, srvr_stats[c].address)) == 0) {
			return c;
		}
	}
	return -1;
}

ClntStats * get_stats(char * ipAddress)
{
	int c;
	if ((c = client_exists(ipAddress)) != -1) {
		return &srvr_stats[c];

	} else {

		if ((srvr_stats = (ClntStats*)realloc(srvr_stats, sizeof (ClntStats) * (srvrStats_len + 1))) != NULL) {
			srvrStats_len++;

			//Initialize server_stats
			memset(srvr_stats[srvrStats_len - 1].address, 0, 32);
			// Copy address to struct
			strcpy(srvr_stats[srvrStats_len - 1].address, ipAddress);
			srvr_stats[srvrStats_len - 1].total_connect = 0;
			srvr_stats[srvrStats_len - 1].curr_connect = 0;
			srvr_stats[srvrStats_len - 1].total_message = 0;
			srvr_stats[srvrStats_len - 1].message = 0;
			srvr_stats[srvrStats_len - 1].total_data = 0;
			srvr_stats[srvrStats_len - 1].curr_data = 0;

			return &srvr_stats[srvrStats_len - 1];
		}
	}
	return NULL;

}

void* live_stats(void*)
{
	int c = 0;
	int p1 = 0, p2 = 0;
	long p3 = 0, p4 = 0, p5 = 0, p6 = 0;
	int t1 = 0, t2 = 0;
	long t3 = 0, t4 = 0, t5 = 0, t6 = 0;

	char line[108];
	for (c = 0; c < 107; c++)
	line[c] = '-';
	line[c] = '\0';

	while (1) {

		gettimeofday(&end, NULL);
		float total_time = (float) (end.tv_sec - start.tv_sec) + ((float) (end.tv_usec - start.tv_usec) / 1000000);
		printf("\nElapsed Time: %.3fs\n", total_time);
		printf("%-14s%-14s%-14s%-14s%-14s%-14s%-14s\n",\
		"Clients",\
		"Total",\
		"Active",\
		"MessagesRecv",\
		"msg/sec",\
		"BytesRecv",\
		"B/s");
		printf("%s\n", line);

		for (c = 0; c < srvrStats_len; c++) {
			// Pre stats
			t1 += p1 = srvr_stats[c].total_connect;
			t2 += p2 = srvr_stats[c].curr_connect;

			t4 += p4 = srvr_stats[c].message;
			t3 += p3 = srvr_stats[c].total_message;

			t6 += p6 = srvr_stats[c].curr_data;
			t5 += p5 = srvr_stats[c].total_data;


			// Print stats
			printf("%-14s%-14d%-14d%-14ld%-14ld%-14ld%-14ld\n",\
			srvr_stats[c].address,\
			p1,\
			p2,\
			p3,\
			p4,\
			p5,\
			p6);

			// Post stats
			srvr_stats[c].message = 0;
			srvr_stats[c].curr_data = 0;
		}

		// Print totals
		printf("%-14s%-14d%-14d%-14ld%-14ld%-14ld%-14ld\n\n",\
		"Total",\
		t1,\
		t2,\
		t3,\
		t4,\
		t5,\
		t6);

		// Reset totals
		t1 = 0;
		t2 = 0;
		t3 = 0;
		t4 = 0;
		t5 = 0;
		t6 = 0;
		sleep(1);
	}

}

void* run_server(int serv_port)
{
		gettimeofday (&start, NULL);

	//start recording live stats
		pthread_create(&tm, NULL, live_stats, NULL);

	int i, maxi, nready, arg, t;
	int listen_sd, new_sd, sockfd, maxfd, client[FD_SETSIZE];
	struct sockaddr_in server, client_addr;
	char buf[BUFFER];
	ssize_t n;
	fd_set rset, allset;
	socklen_t client_len;
	int port = serv_port; // Use the default port

	//create the socket
	if ((listen_sd = socket(AF_INET, SOCK_STREAM,0))== -1)
	SystemFatal("Cannot create socket!");

	// set SO_REUSEADDR so port can be resused imemediately after exit, i.e., after CTRL-c
	arg = 1;
	if (setsockopt (listen_sd, SOL_SOCKET, SO_REUSEADDR, &arg, sizeof(arg)) == -1)
	SystemFatal("setsockopt");

	// Bind an address to the socket
	bzero((char *) &server, sizeof (struct sockaddr_in));
	server.sin_family = AF_INET;
	server.sin_port = htons(port);
	server.sin_addr.s_addr = htonl(INADDR_ANY); // Accept connections from any client

	if (bind(listen_sd, (struct sockaddr *) &server, sizeof (server)) == -1)
	SystemFatal("bind error");


	// Listen for connections
	// queue up to LISTENQ connect requests
	listen(listen_sd, MAX_CLIENTS);

	maxfd = listen_sd; // initialize
	maxi = -1; // index into client[] array

	for (i = 0; i < FD_SETSIZE; i++)
	client[i] = -1; // -1 indicates available entry
	FD_ZERO(&allset);
	FD_SET(listen_sd, &allset);

		ClntStats * clnt_stats;

		while (1) {
			rset = allset; // structure assignment
			nready = select(maxfd + 1, &rset, NULL, NULL, NULL);

			if (FD_ISSET(listen_sd, &rset)){ // new client connection
				client_len = sizeof (client_addr);
				if ((new_sd = accept(listen_sd, (struct sockaddr *) &client_addr, &client_len)) == -1)
				SystemFatal("accept error");

				for (i = 0; i < FD_SETSIZE; i++)
				if (client[i] < 0) {
					client[i] = new_sd; // save descriptor
					break;
				}
				if (i == FD_SETSIZE) {
					printf("Too many clients\n");
					exit(1);
				}

				FD_SET(new_sd, &allset); // add new descriptor to set
				if (new_sd > maxfd)
				maxfd = new_sd; // for select

				if (i > maxi)
				maxi = i; // new max index in client[] array

				// Get new client stats
				char * ip_address = inet_ntoa(client_addr.sin_addr);
				//printf("CONNECTED TO: %s\n",ip_address);
				if((clnt_stats = get_stats(ip_address)) == NULL)
				SystemFatal("get_stats");
				clnt_stats->total_connect++;
				clnt_stats->curr_connect++;

				if (--nready <= 0)
				continue; // no more readable descriptors
			}

			for (i = 0; i <= maxi; i++) // check all clients for data
			{
				if ((sockfd = client[i]) < 0)
				continue;

				if (FD_ISSET(sockfd, &rset)) {
					n = read(sockfd, buf, BUFFER);
					if (n == BUFFER) {
						write(sockfd, buf, BUFFER); // echo to client
						// Get client's sockaddr_in
						struct sockaddr addr;
						socklen_t size = sizeof(struct sockaddr);
						if((getpeername(sockfd, &addr, &size)) == -1){
							SystemFatal("getpeername");
						}

						struct sockaddr_in * sin;
						if(addr.sa_family == AF_INET){
							sin = (struct sockaddr_in *)&addr;
						}
						else
						SystemFatal("not ip4");

						// Get client stats
						char * ip_address = inet_ntoa(sin->sin_addr);
						if((clnt_stats = get_stats(ip_address)) == NULL)
						SystemFatal("get_client_stats");
						clnt_stats->message++;
						clnt_stats->total_message++;
						clnt_stats->curr_data += n;
						clnt_stats->total_data += n;

					}

					if (n == 0) // connection closed by client
					{
						//printf("(sockfd: %d) Remote Address:  %s closed connection\n", sockfd, inet_ntoa(client_addr.sin_addr));
						close(sockfd);
						FD_CLR(sockfd, &allset);
						client[i] = -1;
						t++;
					}
					if (n == -1) {
						if (errno == EAGAIN || errno == EWOULDBLOCK) {
						} else {
							perror("read fail");
						}
					}
				}
			}
		}
	}

	static void SystemFatal(const char* message)
	{
		perror(message);
		exit(EXIT_FAILURE);
	}

	void kill_server(int sig)
	{
		pthread_kill(tm, 0);
		close(fd_server);
		exit(0);
	}
