#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <strings.h>
#include <unistd.h>
#include <signal.h>

#define TRUE 		1
#define FALSE 		0
#define EPOLL_QUEUE_LEN	256
#define BUFLEN		80
#define SERVER_TCP_PORT	7000
#define PROCESS_COUNT            5      //Default No of Process to Spawn



int main(argc, char* argv[])
{
  //File descriptor for server socket
  int fdServerSocket;

  //Listening port for Server Socket
  int serverPort;

  //Number of Processess. User Specified
  int numProcesses;

  switch(argc)
  {
    case 1:
    {
      serverPort = SERVER_TCP_PORT;
      numProcesses = PROCESS_COUNT;
      break;
    }
    case 2:
    {
      serverPort = argv[1];
      numProcesses = PROCESS_COUNT;
      break;
    }
    case 3:
    {
      serverPort = argv[1];
      numProcesses = argv[2];
      break;
    }
    default:
    {
      fprintf(stderr, "Usage: %s [port] [num Process]\n", argv[0]);
      exit(EXIT_FAILURE);
    }
  }



}
