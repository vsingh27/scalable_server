#ifndef WEB_HELPER
#define WEB_HELPER
void error_handler(const char* str);
int establish_tcp_connection(char* serverName, int port);
int send_data(int sd, char* data, int size);
char* read_data(int sd, char* data, int size);
typedef struct
{
        unsigned int timeToServe;
        unsigned int bytesSent;
        unsigned int numRequests;
        unsigned int processID;
}stats;
#endif
