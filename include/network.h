#ifndef network_h
#define network_h
#define PORT 12345
#define IP "127.0.0.1"
#define MAXQUEUE 50
#include "protocol.h"
struct sockaddr_in;
int create_socket(void);
int create_server_socket(void);
int accept_socket(int,struct sockaddr_in *);
int connect_socket(int);
int send_data(int,data*);
int receive_data(int,data*);
#endif
