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
#include <sys/wait.h>
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
        int epoll_fd = epoll_create(EPOLL_QUEUE_LEN);
        int num_fds,i;
        if(epoll_fd == -1)
        {
                error_handler("EPOLL CREATE");
        }

        //Add server socket FD to epoll Event loop
        event.events =  EPOLLIN | EPOLLERR | EPOLLHUP | EPOLLET;
        event.data.fd = serverSocFD;
        if (epoll_ctl (epoll_fd, EPOLL_CTL_ADD, serverSocFD, &event) == -1)
        {
                error_handler("epoll_ctl");
        }

        while(TRUE)
        {
                num_fds = epoll_wait(epoll_fd, events, EPOLL_QUEUE_LEN, -1);
                if(num_fds < 0)
                {
                        error_handler("ERROR: EPOLL_WAIT");
                }

                for(i=0; i<num_fds; i++)
                {
                        //Error condition
                        if(events[i].events & (EPOLLHUP | EPOLLERR))
                        {
                                fputs("epoll: EPOLLERR", stderr);
                                close(events[i].data.fd);
                                continue;
                        }
                        assert(events[i].events & EPOLLIN);

                        //Server is receiving a connection request
                        if(events[i].data.fd == serverSocFD)
                        {
                                int fd_new = accept(serverSocFD, 0,0);
                                if (fd_new == -1)
                                {
                                        if (errno != EAGAIN && errno != EWOULDBLOCK)
                                        {
                                                error_handler("accept");
                                        }
                                        continue;
                                }

                                //Configure New Socket to NON Blocking
                                if(fcntl(fd_new, F_SETFL, O_NONBLOCK | fcntl(fd_new, F_GETFL,0)) == -1)
                                {
                                        error_handler("fcntl");
                                }

                                //Add new socket to epoll loop
                                event.data.fd = fd_new;
                                if (epoll_ctl (epoll_fd, EPOLL_CTL_ADD, fd_new, &event) == -1)
                                {
                                        error_handler("epoll_ctl");
                                }
                                continue;
                        }

                        //IF one of the SOCKET has read data

                }
        }

}


int main(int argc, char* argv[])
{
        //File descriptor for server socket
        int fdServerSocket;

        //Listening port for Server Socket
        int serverPort;

        //Number of Processess. User Specified
        int numProcesses;

        char* hostName;

        pid_t pid;

        switch(argc)
        {
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
                serverPort = atoi(argv[2]);
                numProcesses = PROCESS_COUNT;
                break;
        }
        case 4:
        {
                hostName = argv[1];
                serverPort = atoi(argv[2]);
                numProcesses = atoi(argv[3]);
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
        for(int k =0; k<numProcesses; k++)
        {
                if(pid != 0)
                {
                        pid_t childpid = wait(NULL);
                }
        }

        return 0;

}
