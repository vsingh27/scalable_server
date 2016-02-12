#include "select_server.h"

//globals
struct timeval start, end;

int main(int argc, char *argv[])
{

}

void* run_server(int servPort)
{

	gettimeofday (&start, NULL);
	
    int i, maxi, nready, arg, t;
    int listen_sd, new_sd, sockfd, maxfd, client[FD_SETSIZE];
    struct sockaddr_in server, client_addr;
    char buf[BUFLEN];
    ssize_t n;
    fd_set rset, allset;
    socklen_t client_len;
    int port = servPort;

    if (servPort == 0) {

        port = SERVER_PORT; // Use the default port

    } else {
        port = servPort; // Use the defined port
    }
    //printf("%d", FD_SETSIZE);

    // Create a stream socket
    if ((listen_sd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
        SystemFatal("Cannot Create Socket!");

    // set SO_REUSEADDR so port can be reused immediately after exit, i.e., after CTRL-c
    arg = 1;
    if (setsockopt(listen_sd, SOL_SOCKET, SO_REUSEADDR, &arg, sizeof (arg)) == -1)
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
    listen(listen_sd, MAX_CONNECTIONS);

    maxfd = listen_sd; // initialize
    maxi = -1; // index into client[] array

    for (i = 0; i < FD_SETSIZE; i++)
        client[i] = -1; // -1 indicates available entry
    FD_ZERO(&allset);
    FD_SET(listen_sd, &allset);
	
    while (TRUE) {
        rset = allset; // structure assignment
        nready = select(maxfd + 1, &rset, NULL, NULL, NULL);

        if (FD_ISSET(listen_sd, &rset)){ // new client connection
            client_len = sizeof (client_addr);
            if ((new_sd = accept(listen_sd, (struct sockaddr *) &client_addr, &client_len)) == -1)
                SystemFatal("accept error");

            //printf(" Remote Address:  %s\n", inet_ntoa(client_addr.sin_addr));

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
			//char * ip_address = inet_ntoa(client_addr.sin_addr);
			//printf("CONNECTED TO: %s\n",ip_address);
			//if((cstat = get_client_stats(ip_address)) == NULL)
				SystemFatal("get_client_stats");
			//cstat->total_conn++;
			//cstat->conn++;

            if (--nready <= 0)
                continue; // no more readable descriptors
        }

        for (i = 0; i <= maxi; i++) // check all clients for data
        {
        	
        	
            if ((sockfd = client[i]) < 0)
                continue;

            if (FD_ISSET(sockfd, &rset)) {
                n = read(sockfd, buf, BUFLEN);
                if (n == BUFLEN) {
                    write(sockfd, buf, BUFLEN); // echo to client
                    //printf("(sockfd: %d)Sending: %s\n", sockfd, buf);
					
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
					if((cstat = get_client_stats(ip_address)) == NULL)
						SystemFatal("get_client_stats");
					cstat->msg++;
				   	cstat->total_msg++;
				   	cstat->data += n;
					cstat->total_data += n;
									
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
                        ;
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
	exit(0);
}
