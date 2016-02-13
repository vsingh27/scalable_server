#include <stdio.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include <stdlib.h>
#include <strings.h>
#include <arpa/inet.h>
#include <unistd.h>
#include "web_helper.h"

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
