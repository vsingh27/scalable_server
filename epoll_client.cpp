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
-- 	The server can be specified using a fully qualified domain name or and
--	IP address. After the connection has been established the user will be
-- 	prompted for the number of times data needs to be sent to the server and
--  the data length.
--  The data string is then sent to the server and the
-- 	response (echo) back from the server is displayed.
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

#define SERVER_TCP_PORT	    	7000	// Default port
#define BUFLEN			        80  	// Buffer length
