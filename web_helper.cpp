#include <stdio.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include <stdlib.h>
#include <strings.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include "web_helper.h"
#define LISTENER_QUEUE  1024

void error_handler(const char* str)
{
        fprintf(stderr,"%s: ",str);
        perror(0);
        exit(EXIT_FAILURE);
}

int establish_tcp_connection(char* serverName, int port)
{
        struct sockaddr_in server;
        struct hostent *hp;
        int sd;
        if((sd = socket(AF_INET, SOCK_STREAM,0)) == -1)
        {
                error_handler("Cannot create Socket");
        }

        bzero((char *)&server, sizeof(struct sockaddr_in));
        server.sin_family = AF_INET;
        server.sin_port = htons(port);
        if((hp = gethostbyname(serverName)) == NULL)
        {
                error_handler("Unknown Server Address");
        }
        bcopy(hp->h_addr, (char *)&server.sin_addr, hp->h_length);

        //Connecting to the Server
        if(connect(sd,(struct sockaddr *)&server, sizeof(server)) == -1)
        {
                error_handler("Can't connect to the server");
        }

        printf("connected: Server Name: %s\n", hp->h_name);
        return sd;
}

int server_socket_non_blocking(int port, char* hostName)
{
        //hostAddr to socket details
        struct sockaddr_in hostAddr;

        //SOCKET OPTIONS for LINGER
        struct linger linger;
        memset(&linger,0,sizeof(linger));
        linger.l_onoff = 1;
        linger.l_linger = 0;

        //file descriptor for socket
        int socket_fd;


        //create TCP Server Socket
        if((socket_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
        {
                error_handler("Error while creating TCP Server Socket: Socket()");
        }

        //Set the FLAGs for SERVER SOCKET
        int arg = 1;
        if(setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, &arg, sizeof(arg)) == -1)
        {
                error_handler("Failed while setting SOCKET Options: SOC_REUSEADDR");
        }

        //Set SOCKET OPTION LINGER
        if(setsockopt(socket_fd, SOL_SOCKET, SO_LINGER,(char*)&linger, sizeof(linger)) == -1)
        {
                error_handler("Failed while setting SOCKET Options: SO_LINGER");
        }

        //Configure NON Blocking
        if (fcntl (socket_fd, F_SETFL, O_NONBLOCK | fcntl (socket_fd, F_GETFL, 0)) == -1)
        {
                error_handler("FCTL: while setting SERVER SOCKET NON Blocking");
        }

        //bind server socket to port
        memset(&hostAddr,0,sizeof(struct sockaddr_in));
        hostAddr.sin_family = AF_INET;
        hostAddr.sin_addr.s_addr = htonl(INADDR_ANY);
        hostAddr.sin_port = htons(port);

        if(bind(socket_fd, (struct sockaddr*) &hostAddr, sizeof(hostAddr)) == -1)
        {
                error_handler("Error while binding Server Socket: BIND");
        }

        //Start Listening
        if(listen(socket_fd,LISTENER_QUEUE) == -1)
        {
                error_handler("Error while Listening on Server Socket: LISTEN");
        }

        return socket_fd;

}

int send_data(int sd, char* data, int size)
{
        printf("Sending Data..\n");
        int numBytes;
        numBytes =send (sd, data, size, 0);
        return numBytes;
}

char* read_data(int sd, char* data, int size)
{
        printf("Receiving Data..\n");
        char* bp;
        bp = data;
        int bytes_to_read = size;
        int n = 0;
        while((n = recv(sd, data, bytes_to_read,0)) < size)
        {
                bp += n;
                bytes_to_read -= n;
        }
        return data;
}

int process_socket(int fd, int size, server_stats* server_statistics)
{
    int n, bytes_to_read, bytes_sent;
    char *bp, buf[size];

    while(true)
    {
        bp = buf;
        bytes_to_read = size;

        while((n = recv(fd,bp,bytes_to_read,0)) < size)
        {
            server_statistics->bytesRec = n;
            bp += n;
            bytes_to_read -= n;
        }

        printf("Sending:%s\n", buf);

        bytes_sent = send(fd, buf, size, 0);
        server_statistics->bytesSent = bytes_sent;
        close(fd);
        return true;
    }

    close(fd);
    return (0);
}
