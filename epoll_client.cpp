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
#include <csignal>
#include <sys/time.h>
#include <map>
#include <utility>
#include <semaphore.h>
#include <sys/mman.h>
#include <iostream>
#include <fstream>
#include "web_helper.h"

using namespace std;

#define SERVER_TCP_PORT         7000  // Default port
#define TRANSMISSION_COUNT      10      //Defaut No of Requests to Make
#define PROCESS_COUNT            5      //Default No of Process to Spawn
#define EPOLL_QUEUE             1024    //No if clients
#define BUFFER_LEN              1024    //Buffer Length
#define  STATS_FILE              "./client_stats.csv"

//Semaphore to Lock when writing statistics
sem_t* printLock = 0;

stats stats_struc;
map<unsigned int, stats> statData;

map<unsigned int, stats>* ptrMap = &statData;

char* generate_data(char* data,int size)
{
        int k=0;
        for (int i=0; i< size; i++)
        {
          if(k == 25)
          {
                  k = 0;
          }
                data[i] = (char)(97 + (k++));

        }
        return data;
}

void print_stats(int)
{
        sem_wait(printLock);
        ofstream statistics_file;
        statistics_file.open (STATS_FILE, ios::out | ios::app | ios::binary);
        int totalRequests = 0;
        for( map<unsigned int, stats>::const_iterator it = statData.begin(); it != statData.end(); ++it )
        {

                unsigned int key = it->first;
                //printf("The key is %u\n",key );
                stats value = it->second;
                totalRequests = totalRequests+ value.numRequests;
                statistics_file<<key<<","<<value.timeToServe<<","<<value.bytesSent<<","<<value.numRequests<<"\n";

        }
        statistics_file<<"Total Requests"<<","<<","<<totalRequests<<"\n";
        statistics_file.close();
        sem_post(printLock);
        exit(0);
}

int worker_process(char* serverName, int port, char* sendData, char* recData, int numReuest)
{
        int sd = establish_tcp_connection(serverName, port);
        timeval tim;
        unsigned int pid = (unsigned int)getpid();

        for (size_t i = 0; i < numReuest; i++) {
                gettimeofday(&tim,NULL);
                double t1 = tim.tv_sec+(tim.tv_usec);
                int num_bytes = 0;
                num_bytes= send_data(sd, sendData, (int)BUFFER_LEN);
                read_data(sd,recData, (int)BUFFER_LEN);
                gettimeofday(&tim,NULL);
                double t2 = tim.tv_sec+(tim.tv_usec);
                stats_struc.timeToServe = t2-t1;
                stats_struc.bytesSent = num_bytes;
                stats_struc.numRequests = numReuest;
                stats_struc.processID = getpid();
                ptrMap->insert(pair<unsigned int, stats>(pid,stats_struc));
                printf ("%s\n", recData);
                fflush(stdout);
        }
        close (sd);
        return EXIT_SUCCESS;
}

int main(int argc, char **argv)
{
        signal(SIGINT, print_stats);
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

        ofstream statistics_file;
        statistics_file.open (STATS_FILE, ios::out | ios::app | ios::binary);
        statistics_file<<"ProcessID," <<"RTT," << "Bytes Sent,"<<"Number of Requests\n";
        statistics_file.close();

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

      // ptrMap =  (map<unsigned int, stats>*)mmap(0, processCount * sizeof(map<unsigned int, stats>), PROT_READ | PROT_WRITE, MAP_SHARED, -1, 0);

        printLock = (sem_t*) mmap(0,sizeof(sem_t),PROT_READ|PROT_WRITE,MAP_SHARED|MAP_ANONYMOUS,-1,0);


        if (printLock == MAP_FAILED)
        {
                error_handler("Print Lock mmap");
        }

        /*if (ptrMap) {
                error_handler("Map mmap");
        }*/

        if (sem_init(printLock,1,1) < 0)
        {
                error_handler("sem_init");
        }
        for(int j=0; j<processCount; j++)
        {
                pid = fork();
                switch(pid)
                {
                case 0:
                        worker_process(serverName, serverPort, sbuf, rbuf,transmissionCount);
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
