#ifndef epoll_utils_h
#define epoll_utils_h
#define MAXEVENTS 1024
struct epoll_event;

int create_epoll(void);
void epoll_add_server(int,int);
void epoll_add_client(int,int);
void epoll_remove_client(int,int);
int wait_epoll(int,struct epoll_event*);

#endif
