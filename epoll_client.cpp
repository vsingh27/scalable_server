/*---------------------------------------------------------------------------------------
   --	SOURCE FILE:		epoll_client.cpp - A epoll Client Program
   --
   --	PROGRAM:		eplclnt
   --				g++ -Wall -ggdb -o epollc epoll_client.cpp
   --
   --	FUNCTIONS:		Berkeley Socket API
   --
   --	DATE:			February 10, 2016
   --
   --
   --
   --	DESIGNERS:		Vishav Singh, Rizwan Ahmed
   --
   --	PROGRAMMERS:	Vishav Singh, Rizwan Ahmed
   --
   --	NOTES:
   --	The program will establish a TCP connection to a user specifed server.
   --   The server can be specified using a fully qualified domain name or and
   --	IP address. After the connection has been established the user will be
   --   prompted for the number of times data needs to be sent to the server and
   --  the data length.
   --  The data string is then sent to the server and the
   --   response (echo) back from the server is displayed.
   --	This client application can be used to test the aaccompanying epoll
   --	server: epoll_svr.cpp
   ---------------------------------------------------------------------------------------*/

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
#include <sys/wait.h>
#include "web_helper.h"



#define SERVER_TCP_PORT         7000  // Default port
#define TRANSMISSION_COUNT      10      //Defaut No of Requests to Make
#define PROCESS_COUNT            5      //Default No of Process to Spawn
#define EPOLL_QUEUE             1024    //No if clients
#define BUFFER_LEN              1024    //Buffer Length

//Semaphore to Lock when writing statistics
//sem_t* printLock = 0;

//Session Counter Per Process

int totalSessionCount;
int numRequests;
int timeToServe;



char* generate_data(char* data,int size)
{
        int k=0;
        for (int i=0; i< 26; i++)
        {
                data[i] = (char)(97 + (k++));
        }
        return data;
}

int worker_process(char* serverName, int port, char* sendData, char* recData, int numProcess)
{
        for (size_t i = 0; i < numProcess; i++) {
                /* code */
                int sd = establish_tcp_connection(serverName, port);
                send_data(sd, sendData, (int)BUFFER_LEN);
                read_data(sd,recData, (int)BUFFER_LEN);
                printf ("%s\n", recData);
                fflush(stdout);
                close (sd);
        }
        return (0);
}

int main(int argc, char **argv)
{
        //Name of the server
        char* serverName;

        //Port no of the server
        int serverPort;

        //No of times the user wants to send the Data
        int transmissionCount;

        //No of Processes to Spawn
        int processCount;
        char sbuf[BUFFER_LEN];
        char rbuf[BUFFER_LEN];
        generate_data(sbuf,BUFFER_LEN);
        //printf("%s\n",sbuf );
        pid_t pid;
        switch(argc)
        {
        case 2:
        {
                serverName = argv[1];
                serverPort = SERVER_TCP_PORT;
                transmissionCount = TRANSMISSION_COUNT;
                processCount = PROCESS_COUNT;
                break;
        }
        case 3:
        {
                serverName = argv[1];
                serverPort = atoi(argv[2]);
                transmissionCount = TRANSMISSION_COUNT;
                processCount = PROCESS_COUNT;
                break;
        }
        case 4:
        {        serverName = argv[1];
                 serverPort = atoi(argv[2]);
                 transmissionCount = atoi(argv[3]);
                 processCount = PROCESS_COUNT;
                 break; }
        case 5:
        {        serverName = argv[1];
                 serverPort = atoi(argv[2]);
                 transmissionCount = atoi(argv[3]);
                 processCount = atoi(argv[4]);
                 break; }
        default:
        {
                fprintf(stderr, "Usage: %s host [port] [num Transmission] [num Process]\n", argv[0]);
                exit(EXIT_FAILURE);
        }
        }

        for(int j=0; j<processCount; j++)
        {
                pid = fork();

                switch(pid)
                {
                case 0:
                        worker_process(serverName, serverPort, sbuf, rbuf,transmissionCount);
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
