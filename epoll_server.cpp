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
#include <csignal>
#include <map>
#include <utility>
#include <semaphore.h>
#include <sys/mman.h>
#include <iostream>
#include <fstream>
#include "web_helper.h"

#define TRUE    1
#define FALSE     0
#define EPOLL_QUEUE_LEN 256
#define BUFLEN    1024
#define SERVER_TCP_PORT 7000
#define PROCESS_COUNT            5      //Default No of Process to Spawn
#define LOCAL_HOST "localhost"
#define  STATS_FILE  "./epoll_server_stats.csv"

using namespace std;


map<unsigned int, server_stats> serverStatMap;
map<unsigned int, server_stats>* ptrMap = &serverStatMap;
sem_t* printLock = 0;

void print_statistics(int)
{
        sem_wait(printLock);
        ofstream statistics_file;
        statistics_file.open (STATS_FILE, ios::out | ios::app | ios::binary);
        for( map<unsigned int, server_stats>::const_iterator it = serverStatMap.begin(); it != serverStatMap.end(); ++it )
        {
                unsigned int key = it->first;
                server_stats value = it->second;
                statistics_file<<value.clientInfo.hostName << "," <<value.clientInfo.port << "," << value.numConnections << "," <<value.bytesSent<<"," << value.bytesRec<<"\n";
        }
        statistics_file.close();
        sem_post(printLock);
        exit(0);
}


int child_process(int serverSocFD, char* hostname, int port)
{
//  printf("I am here");
        struct epoll_event events[EPOLL_QUEUE_LEN], event;
        int epoll_fd = epoll_create(EPOLL_QUEUE_LEN);
        int num_fds,i;
        client_info clientstats_struc;

        server_stats serverstats_struc;
         server_stats* ptrStats = &serverstats_struc;
          //Stats Add Client Info

        clientstats_struc.hostName = hostname;
        clientstats_struc.port = port;

       unsigned int pid = (unsigned int)getpid();


        //serverstats_struc.clientInfo = clientstats_struc;

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

                int numCount;
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
                            numCount++;
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
                        if(!process_socket(events[i].data.fd,BUFLEN))
                        {
                                close(events[i].data.fd);
                        }

                }
                serverstats_struc.numConnections = numCount;
                serverstats_struc.bytesSent = (int)BUFLEN;
                serverstats_struc.bytesRec = (int)BUFLEN;
                serverstats_struc.clientInfo = clientstats_struc;
                ptrMap->insert(pair<unsigned int, server_stats>(pid,serverstats_struc));
              printf("Number of Connections%d\n", numCount);
        }
        close(serverSocFD);
        return EXIT_SUCCESS;

}


int main(int argc, char* argv[])
{
        signal(SIGINT, print_statistics);
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

         printf("I am here");
        fdServerSocket = server_socket_non_blocking(serverPort, hostName);
        if(fdServerSocket == -1)
        {
                error_handler("SERVER SOCKET");
        }

        printLock = (sem_t*) mmap(0,sizeof(sem_t),PROT_READ|PROT_WRITE,MAP_SHARED|MAP_ANONYMOUS,-1,0);


        if (printLock == MAP_FAILED)
        {
                error_handler("Print Lock mmap");
        }
        if (sem_init(printLock,1,1) < 0)
        {
                error_handler("sem_init");
        }

        for(int j=0; j<numProcesses; j++)
        {

                pid = fork();

                switch(pid)
                {
                case 0:
                        child_process(fdServerSocket, hostName, serverPort);
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
