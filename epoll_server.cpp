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
#include "web_helper.h"

#define TRUE    1
#define FALSE     0
#define EPOLL_QUEUE_LEN 256
#define BUFLEN    80
#define SERVER_TCP_PORT 7000
#define PROCESS_COUNT            5      //Default No of Process to Spawn
#define LOCAL_HOST "localhost"



int child_process(int serverSocFD)
{
        struct epoll_event events[EPOLL_QUEUE_LEN], event;
        epoll_fd = epoll_create(EPOLL_QUEUE_LEN);
        if(epoll_fd == -1)
        {
                error_handler("EPOLL CREATE");
        }

        //Add server socket FD to epoll Event loop
        event.events =  EPOLLIN | EPOLLERR | EPOLLHUP | EPOLLET;
        event.data.fd = serverSocFD;
        if (epoll_ctl (epoll_fd, EPOLL_CTL_ADD, serverSocFD, &event) == -1)
        {
          SystemFatal("epoll_ctl");
        }


}


int main(argc, char* argv[])
{
        //File descriptor for server socket
        int fdServerSocket;

        //Listening port for Server Socket
        int serverPort;

        //Number of Processess. User Specified
        int numProcesses;

        char* hostName;

        switch(argc)
        {
        case 1:
        {
                hostName = LOCAL_HOST;
                serverPort = SERVER_TCP_PORT;
                numProcesses = PROCESS_COUNT;
                break;
        }
        case 2:
        {
                hostName = argv[1];
                serverPort = SERVER_TCP_PORT;
                numProcesses = PROCESS_COUNT;
                break;
        }
        case 3:
        {
                hostName = argv[1];
                serverPort = argv[2];
                numProcesses = PROCESS_COUNT;
                break;
        }
        case 4:
        {
                hostName = argv[1];
                serverPort = argv[2];
                numProcesses = argv[3];
                break;
        }
        default:
        {
                fprintf(stderr, "Usage: %s [port] [num Process]\n", argv[0]);
                exit(EXIT_FAILURE);
        }
        }

        fdServerSocket = server_socket_non_blocking(serverPort, hostName);
        if(fdServerSocket == -1)
        {
                error_handler("SERVER SOCKET");
        }

        for(int j=0; j<numProcesses; j++)
        {
                pid = fork();
                switch(pid)
                {
                case 0:
                        child_process(fdServerSocket);
                        kill(getpid(), SIGINT);
                        exit(0);
                        break;
                }
        }
        for(int k =0; k<processCount; k++)
        {
                if(pid != 0)
                {
                        pid_t childpid = wait(NULL);
                }
        }

        return 0;

}
