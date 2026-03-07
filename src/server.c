#include "server.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <time.h>
#include <arpa/inet.h>
#include "network.h"
#include "epoll_utils.h"
#include "protocol.h"
#include "game.h"
int create_room(int fd,struct room *rooms,int aroom){
	struct data msg;
	rooms[aroom].creater=fd;
	rooms[aroom].joiner=-1;
	rooms[aroom].c_move=rooms[aroom].j_move=-1;
	rooms[aroom].members=1;
	msg.type=ROOM_CREATED;
	msg.rno=aroom;
	if(send_data(fd,&msg)<0)perror("SEND");
	for(;aroom<MAXROOMS && rooms[aroom].members;aroom++);
	return aroom;
}
void join_room(int fd,struct data msg,struct room *rooms){
	rooms[msg.rno].joiner=fd; 
	rooms[msg.rno].members+=1;
	msg.type=ROOM_JOINED;
	if(send_data(fd,&msg)<0)perror("SEND");
	msg.type=PLAYER_JOINED;
	if(send_data(rooms[msg.rno].creater,&msg)<0)perror("SEND");
}
int leave_room(int fd,struct data msg,struct room*rooms,int aroom){
	if(rooms[msg.rno].creater==fd)rooms[msg.rno].creater=rooms[msg.rno].joiner;
	rooms[msg.rno].joiner=-1;
	if(rooms[msg.rno].creater==-1)rooms[msg.rno].members=0;
	if(msg.rno<aroom)aroom=msg.rno;
	return aroom;
}
void send_result(struct data msg,struct room*rooms){
	int result;
	result=find_winner(rooms[msg.rno].c_move,rooms[msg.rno].j_move);
	msg.type=RESULT;
	msg.val=-result;
	if(send_data(rooms[msg.rno].creater,&msg)<0)perror("SEND");
	msg.val=result;
	if(send_data(rooms[msg.rno].joiner,&msg)<0)perror("SEND");	
	rooms[msg.rno].j_move=rooms[msg.rno].c_move=-1;
}
void handle_opponent_move(struct data msg,struct room*rooms){
	msg.type=OPPONENT_MOVE;
	msg.val=rooms[msg.rno].j_move;
	if(send_data(rooms[msg.rno].creater,&msg)<0)perror("SEND");
	msg.val=rooms[msg.rno].c_move;
	if(send_data(rooms[msg.rno].joiner,&msg)<0)perror("SEND");
}
void handle_player_move(int fd,struct data msg,struct room*rooms){
	if(rooms[msg.rno].creater==fd){
		rooms[msg.rno].c_move=msg.val;
		if(rooms[msg.rno].j_move>-1){
			handle_opponent_move(msg,rooms);
			send_result(msg,rooms);
		}
	}
	else if(rooms[msg.rno].joiner==fd){
		rooms[msg.rno].j_move=msg.val;
		if(rooms[msg.rno].c_move>-1){
			handle_opponent_move(msg,rooms);
			send_result(msg,rooms);
		}
	}	
}
int game_aftermath(struct data msg,struct room*rooms,int aroom){
	int result;
	result=(rooms[msg.rno].c_move==rooms[msg.rno].j_move)?1:0;
	if(result){
		if(msg.val==LEAVE_ROOM){
			msg.type=LEAVE_ROOM;
			if(send_data(rooms[msg.rno].creater,&msg)<0)perror("SEND");
			if(send_data(rooms[msg.rno].joiner,&msg)<0)perror("SEND");
			rooms[msg.rno].creater=rooms[msg.rno].joiner=-1;
			rooms[msg.rno].c_move=rooms[msg.rno].j_move=-1;
			rooms[msg.rno].members=0;
			if(msg.rno<aroom)aroom=msg.rno;
		}
		else if(msg.val==REMATCH){
			msg.type=REMATCH_ACCEPT;
			if(send_data(rooms[msg.rno].creater,&msg)<0)perror("SEND");
			if(send_data(rooms[msg.rno].joiner,&msg)<0)perror("SEND");	
			rooms[msg.rno].c_move=rooms[msg.rno].j_move=-1;
		}
	}
	else{
		msg.type=REMATCH;
		if(rooms[msg.rno].c_move==LEAVE_ROOM){
			if(send_data(rooms[msg.rno].creater,&msg)<0)perror("SEND");
		}
		else if(rooms[msg.rno].j_move==LEAVE_ROOM){
			if(send_data(rooms[msg.rno].joiner,&msg)<0)perror("SEND");
		}
	}
	return aroom;
}
int handle_game_aftermath(int fd,struct data msg,struct room*rooms,int aroom){
	if(rooms[msg.rno].creater==fd){
		rooms[msg.rno].c_move=msg.val;
		if(rooms[msg.rno].j_move>-1)aroom=game_aftermath(msg,rooms,aroom);
	}
	else if(rooms[msg.rno].joiner==fd){
		rooms[msg.rno].j_move=msg.val;
		if(rooms[msg.rno].c_move>-1)aroom=game_aftermath(msg,rooms,aroom);
	}
	return aroom;
}
void handle_rematch_accept(int fd,struct data msg,struct room*rooms){
	if(rooms[msg.rno].creater==fd){
		if(send_data(rooms[msg.rno].joiner,&msg)<0)perror("SEND");
	}
	else if(rooms[msg.rno].joiner==fd){
		if(send_data(rooms[msg.rno].creater,&msg)<0)perror("SEND");
	}
	rooms[msg.rno].c_move=rooms[msg.rno].j_move=-1;
}
void handle_rematch_decline(int fd,struct data msg,struct room*rooms){
	if(rooms[msg.rno].creater==fd){
		if(send_data(rooms[msg.rno].joiner,&msg)<0)perror("SEND");		
		rooms[msg.rno].creater=rooms[msg.rno].joiner;
	}
	else if(rooms[msg.rno].joiner==fd){
		if(send_data(rooms[msg.rno].creater,&msg)<0)perror("SEND");
	}
	rooms[msg.rno].c_move=rooms[msg.rno].j_move=-1;
	rooms[msg.rno].joiner=-1;
	rooms[msg.rno].members-=1;
}
int handle_client_request(int fd,struct data msg,struct room*rooms,int aroom){
	switch(msg.type){
		case CREATE_ROOM:
		{
			aroom=create_room(fd,rooms,aroom);
			break;
		}
		case JOIN_ROOM:
		{
			join_room(fd,msg,rooms);
			break;
		}
		case LEAVE_ROOM:
		{
			aroom=leave_room(fd,msg,rooms,aroom);
			break;
		}
		case PLAYER_MOVE:
		{	
			handle_player_move(fd,msg,rooms);
			break;
		}	
		case GAME_AFTERMATH:
		{
			aroom=handle_game_aftermath(fd,msg,rooms,aroom);
			break;
		}
		case REMATCH_ACCEPT:
		{
			handle_rematch_accept(fd,msg,rooms);
			break;
		}
		case REMATCH_DECLINE:
		{	
			handle_rematch_decline(fd,msg,rooms);
			break;
		}
	}
	return aroom;
}
void accept_client(int sfd,int efd){
	int cfd;
	struct sockaddr_in* addr=(struct sockaddr_in*)malloc(sizeof(struct sockaddr_in));
	cfd=accept_socket(sfd,addr);
	epoll_add_client(efd,cfd);
	time_t now=time(NULL);
	printf("\n[LOG]client(%d) connected from %s:%d at %s",cfd,inet_ntoa(addr->sin_addr),ntohs(addr->sin_port),ctime(&now));
	free(addr);
}
void run_server(){
	int serverfd,epollfd,nfds,rec;
	struct data msg;
	struct epoll_event evs[MAXEVENTS];
	struct room rooms[MAXROOMS]={0};
	int aroom=0;

	epollfd=create_epoll();
	serverfd=create_server_socket();
	epoll_add_server(epollfd,serverfd);

	while(1){
		nfds=wait_epoll(epollfd,evs);
		for(int i=0;i<nfds;i++){
			if(evs[i].data.fd==serverfd)accept_client(serverfd,epollfd);
			else if(evs[i].events & (EPOLLHUP|EPOLLERR|EPOLLRDHUP))epoll_remove_client(epollfd,evs[i].data.fd);
			else{
				if((rec=receive_data(evs[i].data.fd,&msg))<0)perror("RECEIVE");
				if(!rec)epoll_remove_client(epollfd,evs[i].data.fd);
				else aroom=handle_client_request(evs[i].data.fd,msg,rooms,aroom);
			}
		}
	}
}
