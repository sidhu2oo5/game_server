#include "network.h"
#include "protocol.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <errno.h>
#include <netinet/tcp.h>
int create_socket(void){
	int fd;
	do {
		fd=socket(AF_INET,SOCK_STREAM,0);
		if(fd<0){
			perror("Socket Creation");
			sleep(1);
		}
	}while(fd<0);
	return fd;

}
int create_server_socket(void){
	int fd,opt=1;
	int idletime=60,interval=10,count=5;
	struct sockaddr_in addr;
	socklen_t addrlen=sizeof(addr);

	fd=create_socket();

	if(setsockopt(fd,SOL_SOCKET,SO_REUSEADDR,&opt,sizeof(opt))<0){
		perror("SO_REUSEADDR");
		close(fd);
		exit(EXIT_FAILURE);
	}
	if(setsockopt(fd,SOL_SOCKET,SO_KEEPALIVE,&opt,sizeof(opt))<0){
		perror("SO_KEEPALIVE");
		close(fd);
		exit(EXIT_FAILURE);
	}
	if(setsockopt(fd,IPPROTO_TCP,TCP_KEEPIDLE,&idletime,sizeof(idletime))<0)
		perror("TCP_KEEPIDLE");
	if(setsockopt(fd,IPPROTO_TCP,TCP_KEEPINTVL,&interval,sizeof(interval))<0)
		perror("TCP_KEEPINTVL");
	if(setsockopt(fd,IPPROTO_TCP,TCP_KEEPCNT,&count,sizeof(count))<0)
		perror("TCP_KEEPCNT");
	
	addr.sin_family=AF_INET;
	addr.sin_port=htons(PORT);
	addr.sin_addr.s_addr=INADDR_ANY;

	if(bind(fd,(struct sockaddr*)&addr,addrlen)<0){
		perror("Bind");
		close(fd);
		exit(EXIT_FAILURE);
	}
	if(listen(fd,MAXQUEUE)<0){
		perror("Listen");
		close(fd);
		exit(EXIT_FAILURE);
	}
	return fd;
}
int accept_socket(int fd,struct sockaddr_in* addr){
	int cfd;
	socklen_t addrlen=sizeof(struct sockaddr_in);
	cfd=accept(fd,(struct sockaddr*)addr,&addrlen);
	if(cfd<0) {
		perror("Accept");
	      	return -1;
	}
	return cfd; 
}
int connect_socket(int fd){
	int stat,count=10;
	struct sockaddr_in addr;
	addr.sin_family=AF_INET;
	addr.sin_port=htons(PORT);
	addr.sin_addr.s_addr=inet_addr(IP);
	do{
		stat=connect(fd,(struct sockaddr*)&addr,sizeof(addr));
		if(stat){
		        count--;
			perror("Connect");
			if(!count)exit(EXIT_FAILURE);
			sleep(1);
		}
	}while(stat);
	return 0;
}
int send_data(int fd,struct data*d){
	const char*ptr=(const char*)d;
	int len=sizeof(*d),tsent=0,sent;
	while(tsent<len){
		sent=send(fd,ptr+tsent,len-tsent,0);
		if(sent<0){
		        if(errno==EINTR)continue;
                        perror("Send Failed");	
			return -1;
		}
		tsent+=sent;		
	}
	return len;
}
int receive_data(int fd,struct data*d){
	char*ptr=(char*)d;
	int len=sizeof(*d),trcv=0,rcv;
      	while(trcv<len){
		rcv=recv(fd,ptr+trcv,len-trcv,0);
		if(rcv<0){
		       if(errno==EINTR)continue;
		       perror("Receive Failed");
		       return -1;
		}
		if(rcv==0)return 0;
		trcv+=rcv;
	}
	return len;
}
