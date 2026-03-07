#ifndef network_h
#define network_h
#define PORT 12345
#define IP "127.0.0.1"
#define MAXQUEUE 50
struct sockaddr_in;
struct data;
int create_socket(void);
int create_server_socket(void);
int accept_socket(int,struct sockaddr_in *);
int connect_socket(int);
int send_data(int,struct data*);
int receive_data(int,struct data*);
#endif
