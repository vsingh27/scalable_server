/*---------------------------------------------------------------------------------------
--	SOURCE FILE: multi_thread_srvr.cpp -   A simple multiplexed echo server using TCP
--
--	PROGRAM: mt_srvr
--	g++ -Wall -o mt_srvr multi_thread_srvr.cpp -lpthread
--
--	FUNCTIONS:   int main(int argc, char *argv[])
--               void *serve_clients(void *ptr)
--               void accept_connect(int fd)
--               int find_free_thread(void)
--               int find_free_client(int thread_num)
--               void kill_server(int sig)
--
--	DATE: February 11, 2016
--
--	DESIGNERS: Rizwan Ahmed, Vishav Singh
--
--				
--	PROGRAMMER: Rizwan Ahmed, Vishav Singh
--
--	NOTES:
--	The program will accept TCP connections from multiple client machines.
-- 	The program will read data from each client socket and simply echo it back.
--  When the user presses CTRL+C, the program will close all sockets and write stats
--  to a file before terminating.
---------------------------------------------------------------------------------------*/
#include "multi_thread_srvr.h"

/*---------------------------------------------------------------------------------------
-- FUNCTION: serve_clients
--
-- DATE: February 11, 2016
--
-- DESIGNERS: Rizwan Ahmed, Vishav Singh
--
-- PROGRAMMERS: Rizwan Ahmed, Vishav Singh
--
-- RETURNS: void / NULL
--
-- NOTES:
-- Creates a structure that stores the thread ID and file descriptor of the socket
-- connected to a client. If a client disconnects, the file descriptor gets closed here.
-- If there are no clients connected, then the thread is killed. Otherwise, update
-- the number of requests made and data sent by the client.
---------------------------------------------------------------------------------------*/
void *serve_clients(void *ptr)
{
	SocketThread *thread = (SocketThread *)ptr;
	u_char buf[MAX_IOSIZE];
	int fd, client = 0, rlen = 0, wlen;

	while (1) {
		fd = thread->fd_client[client];

		if (fd != 0) {
			rlen = read(fd, buf, sizeof(buf));

			if (rlen == 0) {
				printf("Client disconnected from socket %d.\n", fd);
				thread->num_client--;
				thread->fd_client[client] = 0;
				close(fd);
			}
			else if (rlen > 0) {
				while ((wlen = write(fd, buf, rlen)) < 0)
					warn("write");

				/* update client info */
				clnt_stats[thread->stats_pos[client]].requests++;

				if (wlen > 0)
					clnt_stats[thread->stats_pos[client]].sent_data += wlen;
			}
		}

		if (client == MAX_CLIENTS -1)
			client = 0;
		else
			client++;

		/* exit the thread if no clients */
		if (thread->num_client == 0) {
			thread->t_id = 0;
			printf("Terminating thread %d\n", thread->current_thread);
			break;
		}
	}
	return NULL;
}

/*---------------------------------------------------------------------------------------
-- FUNCTION: accept_connect
--
-- DATE: February 11, 2016
--
-- DESIGNERS: Rizwan Ahmed, Vishav Singh
--
-- PROGRAMMERS: Rizwan Ahmed, Vishav Singh
--
-- RETURNS: void
--
-- NOTES: Uses the accept() system call to create a new socket, creates a new struct
-- that stores data on a client, and creates a thread for the socket if it hasn't
-- already. It prints an error message if the maximum number of threads or clients
-- has been reached.
---------------------------------------------------------------------------------------*/
void accept_connect(int fd)
{
	int fd_client, thread_num, client_num, socket_buf;
	struct sockaddr_in client_addr;
	socklen_t client_len = sizeof(client_addr);

	/* accept the new connection. */
	fd_client = accept(fd, (struct sockaddr *)&client_addr, &client_len);
	if (fd_client == -1) {
		warn("accept failed");
		return;
	}

	/* increase the buffer size */
	socket_buf = MAX_IOSIZE * 2;
	setsockopt(fd, SOL_SOCKET, SO_SNDBUF, &socket_buf, sizeof(socket_buf));
	setsockopt(fd, SOL_SOCKET, SO_RCVBUF, &socket_buf, sizeof(socket_buf));

	/*thread conditions */
	if ((thread_num = find_free_thread()) == -1)
		perror("No more free threads.");

	if ((client_num = find_free_client(thread_num)) == -1)
		perror("Thread has no more room for clients.");

	threads[thread_num].current_thread = thread_num;
	threads[thread_num].fd_client[client_num] = fd_client;
	/* add client info */
	memcpy(clnt_stats[clnt_pos].address,
		   inet_ntoa(client_addr.sin_addr),
		   strlen(inet_ntoa(client_addr.sin_addr)));
	clnt_stats[clnt_pos].port = client_addr.sin_port;
	clnt_stats[clnt_pos].requests = 0;
	clnt_stats[clnt_pos].sent_data = 0;
	clnt_pos++;

	/* create a new thread if needed */
	if (threads[thread_num].t_id == 0) {
		printf("Creating thread %d\n", thread_num);
		pthread_create(&threads[thread_num].t_id,
					   NULL, &serve_clients, (void *)&threads[thread_num]);
	}

	threads[thread_num].num_client++;

	printf("Accepted connection from %s on socket %d\n",
		   inet_ntoa(client_addr.sin_addr), fd_client);

	fflush(stdout);
}

/*---------------------------------------------------------------------------------------
-- FUNCTION: find_free_thread
--
-- DATE: February 11, 2016
--
-- DESIGNERS: Rizwan Ahmed, Vishav Singh
--
-- PROGRAMMERS: Rizwan Ahmed, Vishav Singh
--
-- RETURNS: Index in array of threads on success, -1 on failure
--
-- NOTES: Checks for any threads that are not at their limit in serving clients.
---------------------------------------------------------------------------------------*/
int find_free_thread(void)
{
	int i;

	for (i = 0; i < MAX_THREADS; i++)
		if (threads[i].num_client < MAX_CLIENTS)
			return i;

	return -1;
}

/*---------------------------------------------------------------------------------------
-- FUNCTION: find_free_client
--
-- DATE: February 11, 2016
--
-- DESIGNERS: Rizwan Ahmed, Vishav Singh
--
-- PROGRAMMERS: Rizwan Ahmed, Vishav Singh
--
-- RETURNS: Index in array of client file descriptors on success, -1 on failure.
--
-- NOTES: Checks if a thread has any more room for more clients. 
---------------------------------------------------------------------------------------*/
int find_free_client(int thread_num)
{
	int i;

	for (i = 0; i < MAX_CLIENTS; i++)
		if (threads[thread_num].fd_client[i] == 0)
			return i;

	return -1;
}

/*---------------------------------------------------------------------------------------
-- FUNCTION: kill_server
--
-- DATE: February 11, 2016
--
-- DESIGNERS: Rizwan Ahmed, Vishav Singh
--
-- PROGRAMMERS: Rizwan Ahmed, Vishav Singh
--
-- RETURNS: void
--
-- NOTES: This is the signal handler that kills all threads and closes all sockets
-- before writing statistics to a CSV file and exiting.
---------------------------------------------------------------------------------------*/
void kill_server(int sig)
{
	FILE *file;
	int i, k;

	/* kill all threads & terminate connections */
	for (i = 0; i < MAX_THREADS; i++) {
		for (k = 0; k < MAX_CLIENTS; k++) {
			if (threads[i].fd_client[k] != 0)
				close(threads[i].fd_client[k]);
		}
		threads[i].num_client = 0;
	}

	/* write data to file */
	file = fopen(STATS_FILE, "w");
	if (file == NULL)
		err(1, "fopen failed");

	if (clnt_pos == 0) {
		fprintf(file, "%s", "No data stored.\n");
	}

	for (i = 0; i < clnt_pos; i++) {
		fprintf(file, "%s, %d, %lu, %lu\n", clnt_stats[i].address, clnt_stats[i].port,
				clnt_stats[i].requests, clnt_stats[i].sent_data);
	}

	fclose(file);

	printf("\nStats written to %s, server closed.\n", STATS_FILE);

	exit(0);

}

/*---------------------------------------------------------------------------------------
-- FUNCTION: main
--
-- DATE: February 11, 2016
--
-- DESIGNERS: Rizwan Ahmed, Vishav Singh
--
-- PROGRAMMERS: Rizwan Ahmed, Vishav Singh
--
-- RETURNS: 0 on exit
--
-- NOTES: Main entry point of the program. Sets up listening socket and array of thread
-- structs. Accepts connections from clients until it is overwhelmed or user presses
-- CTRL+C to kill the server.
---------------------------------------------------------------------------------------*/
int main(int argc, char *argv[])
{
	struct sockaddr_in listen_addr;
	int listen_fd, i, k, reuseaddr_on = 1;

	/*write stats to file before killing server when user presses CTRL+C */
	(void)signal(SIGINT, kill_server);

	/* create listening socket */
	listen_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (listen_fd < 0)
		err(1, "listen failed");

	/* reuse the socket address */
	if (setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, &reuseaddr_on,
				   sizeof(reuseaddr_on)) == -1)
		err(1, "setsockopt failed");

	/* set to listen on all addresses */
	memset(&listen_addr, 0, sizeof(listen_addr));
	listen_addr.sin_family = AF_INET;
	listen_addr.sin_addr.s_addr = INADDR_ANY;
	listen_addr.sin_port = htons(SERVER_PORT);

	/* bind the socket */
	if (bind(listen_fd, (struct sockaddr *)&listen_addr,
			 sizeof(listen_addr)) < 0)
		err(1, "bind failed");

	/* listen for new connections */
	if (listen(listen_fd, 5) < 0)
		err(1, "listen failed");

	/* initialize thread struct */
	for (i = 0; i < MAX_THREADS; i++) {
		threads[i].t_id = 0;
		threads[i].current_thread = 0;
		threads[i].num_client = 0;
		for (k = 0; k < MAX_CLIENTS; k++) {
			threads[i].fd_client[k] = 0;
		}
	}

	/* accept is blocking, so we loop forever */
	while (1)
		accept_connect(listen_fd);

	return 0;
}
