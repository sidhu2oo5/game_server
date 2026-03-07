#include "epoll_utils.h"
#include <sys/epoll.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
int create_epoll(void){
	int efd=epoll_create1(0);
	if(efd<0){
		perror("Epoll");
		exit(EXIT_FAILURE);
	}
	return efd;
}
void epoll_add_server(int efd,int sfd){
	struct epoll_event ev;
	ev.events=EPOLLIN;
	ev.data.fd=sfd;
	if(epoll_ctl(efd,EPOLL_CTL_ADD,sfd,&ev)==-1){
		perror("Epoll Add Server Socket");
		exit(EXIT_FAILURE);
	}
}
void epoll_add_client(int efd,int cfd){
	struct epoll_event ev;
	ev.events=EPOLLIN|EPOLLHUP|EPOLLERR|EPOLLRDHUP;
	ev.data.fd=cfd;
	if(epoll_ctl(efd,EPOLL_CTL_ADD,cfd,&ev)==-1){
		perror("Epoll Add Client Socket");
		close(cfd);
	}
}
void epoll_remove_client(int efd,int cfd){
	if(epoll_ctl(efd,EPOLL_CTL_DEL,cfd,NULL)==-1)perror("Epoll Del");
	close(cfd);
}
int wait_epoll(int efd,struct epoll_event*evs){
	int nfds=epoll_wait(efd,evs,MAXEVENTS,-1);
	if(nfds==-1){
		perror("Epoll Wait");
		exit(EXIT_FAILURE);
	}
	return nfds;
}
