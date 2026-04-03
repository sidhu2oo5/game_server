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
void create_room(data datum,int fd,room *rooms,int* aroom){
	rooms[aroom[0]].type=datum.rtype;
	rooms[aroom[0]].roles[0]=fd;
	rooms[aroom[0]].roles[1]=-1;
	rooms[aroom[0]].moves[0]=rooms[aroom[0]].moves[1]=-1;
	rooms[aroom[0]].members=1;
	datum.status=ROOM_CREATED;
	datum.rno=aroom[0];
	datum.role=0;
	datum.count=1;
	if(send_data(fd,&datum)<0)perror("SEND");
	for(;aroom[0]<MAXROOMS && rooms[aroom[0]].members;aroom[0]++);
}
void join_room(int fd,data datum,room *rooms){
	if(datum.rno==-1){
		datum.rtype=0;
		int i;
		for(i=0;i<MAXROOMS;i++){
			if(rooms[i].members==1 && rooms[i].type==0){
				datum.rno=i;
				break;
			}
		}
		if(i==MAXROOMS){
			datum.status=ROOMS_FULL;
			if(send_data(fd,&datum)<0)perror("SEND");
			return;
		}
	}
	else if(rooms[datum.rno].members==2){
		datum.status=ROOM_FULL;
		if(send_data(fd,&datum)<0)perror("SEND");
		return;
	}
	rooms[datum.rno].roles[1]=fd; 
	rooms[datum.rno].members+=1;
	datum.rtype=rooms[datum.rno].type;	
	datum.count=2;
	datum.status=ROOM_JOINED;
	datum.role=1;
	if(send_data(fd,&datum)<0)perror("SEND");
	datum.status=PLAYER_JOINED;
	datum.role=0;
	if(send_data(rooms[datum.rno].roles[0],&datum)<0)perror("SEND");
}
void leave_room(int fd,data datum,room*rooms,int* aroom){
	rooms[datum.rno].members-=1;
	if(rooms[datum.rno].roles[0]==fd)rooms[datum.rno].roles[0]=rooms[datum.rno].roles[1];
	rooms[datum.rno].roles[1]=-1;
	if(rooms[datum.rno].roles[0]>-1){
		datum.status=PLAYER_LEFT;
		datum.role=0;
		if(send_data(rooms[datum.rno].roles[0],&datum)<0)perror("SEND");	
	}
	if(rooms[datum.rno].roles[0]==-1){
		rooms[datum.rno].members=0;
		rooms[datum.rno].moves[0]=rooms[datum.rno].moves[1]=-1;
		if(datum.rno<aroom[0])aroom[0]=datum.rno;
	}
}
void send_result(data datum,room*rooms){
	int result;
	result=find_winner(rooms[datum.rno].moves[0],rooms[datum.rno].moves[1]);
	datum.status=GAME_RESULT;
	datum.val=-result;
	datum.role=0;
	if(send_data(rooms[datum.rno].roles[0],&datum)<0)perror("SEND");
	datum.val=result;
	datum.role=1;
	if(send_data(rooms[datum.rno].roles[1],&datum)<0)perror("SEND");	
	rooms[datum.rno].moves[1]=rooms[datum.rno].moves[0]=-1;
}
void handle_opponent_move(data datum,room*rooms){
	datum.status=OPPONENT_MOVE;
	datum.val=rooms[datum.rno].moves[1];
	datum.role=0;
	if(send_data(rooms[datum.rno].roles[0],&datum)<0)perror("SEND");
	datum.val=rooms[datum.rno].moves[0];
	datum.role=1;
	if(send_data(rooms[datum.rno].roles[1],&datum)<0)perror("SEND");
}
void handle_player_move(data datum,room*rooms){
	rooms[datum.rno].moves[datum.role]=datum.val;
	if(rooms[datum.rno].moves[!(datum.role)]>-1){
		handle_opponent_move(datum,rooms);
		send_result(datum,rooms);
	}
}
void handle_rematch_request(data datum,room*rooms){
	datum.role=!(datum.role);
	if(send_data(rooms[datum.rno].roles[datum.role],&datum)<0)perror("SEND");
}
void handle_rematch_accept(data datum,room*rooms){
	datum.status=REMATCH_ACCEPTED;
	datum.role=!(datum.role);
	if(send_data(rooms[datum.rno].roles[datum.role],&datum)<0)perror("SEND");
	rooms[datum.rno].moves[0]=rooms[datum.rno].moves[1]=-1;
}
void handle_rematch_decline(data datum,room*rooms){
	datum.status=REMATCH_DECLINED;
	datum.role=!(datum.role);
	if(send_data(rooms[datum.rno].roles[datum.role],&datum)<0)perror("SEND");
}
void change_room_privacy(data datum,room*rooms){
	rooms[datum.rno].type=!(rooms[datum.rno].type);
	datum.rtype=rooms[datum.rno].type;
	datum.status=ROOM_PRIVACY_CHANGED;
	datum.role=0;
	if(send_data(rooms[datum.rno].roles[0],&datum)<0)perror("SEND");
	if(rooms[datum.rno].roles[1]>-1){
		datum.role=1;
		if(send_data(rooms[datum.rno].roles[1],&datum)<0)perror("SEND");
	}
}
void handle_player_ready(data datum,room*rooms){
	datum.role=0;
	if(send_data(rooms[datum.rno].roles[0],&datum)<0)perror("SEND");
}
void handle_game_start(data datum,room*rooms){
	datum.role=0;
	if(send_data(rooms[datum.rno].roles[0],&datum)<0)perror("SEND");
	datum.role=1;
	if(send_data(rooms[datum.rno].roles[1],&datum)<0)perror("SEND");
}
void handle_client_request(int fd,data datum,room*rooms,int* aroom){
	switch(datum.status){
		case ROOM_CREATE:
		{
			create_room(datum,fd,rooms,aroom);
			break;
		}
		case ROOM_JOIN:
		{
			join_room(fd,datum,rooms);
			break;
		}
		case ROOM_LEAVE:
		{
			leave_room(fd,datum,rooms,aroom);
			break;
		}
		case ROOM_PRIVACY_CHANGE:
		{
			change_room_privacy(datum,rooms);
			break;
		}
		case PLAYER_READY:
		{
			handle_player_ready(datum,rooms);
			break;
		}
		case GAME_START:
		{
			handle_game_start(datum,rooms);
			break;
		}
		case PLAYER_MOVE:
		{	
			handle_player_move(datum,rooms);
			break;
		}	
		case REMATCH_REQUEST:
		{
			handle_rematch_request(datum,rooms);
			break;
		}
		case REMATCH_ACCEPT:
		{
			handle_rematch_accept(datum,rooms);
			break;
		}
		case REMATCH_DECLINE:
		{	
			handle_rematch_decline(datum,rooms);
			break;
		}
		default:
		{
			break;
		}
	}
}
int accept_client(int sfd,int efd){
	int cfd;
	struct sockaddr_in* addr=(struct sockaddr_in*)malloc(sizeof(struct sockaddr_in));
	cfd=accept_socket(sfd,addr);
	epoll_add_socket(efd,cfd);
	time_t now=time(NULL);
	printf("\n[LOG]client(%d) connected from %s:%d at %s",cfd,inet_ntoa(addr->sin_addr),ntohs(addr->sin_port),ctime(&now));
	free(addr);
	return cfd;
}
int find_client(int cfd,data*datum,room*rooms){
	for(int i=0;i<MAXROOMS;i++){
		if(rooms[i].roles[0]==cfd || rooms[i].roles[1]==cfd){
			datum->rno=i;
			datum->role=(cfd==rooms[i].roles[1]);
			return i;
		}
	}
	return -1;
}
void handle_client_disconnect(int efd,int cfd,room*rooms,int*aroom){
	data datum;
	if(find_client(cfd,&datum,rooms)>-1)leave_room(cfd,datum,rooms,aroom);
	epoll_remove_client(efd,cfd);
}
void handle_client_connection(int sfd,int efd){
	data datum;
	int fd=accept_client(sfd,efd);
	datum.status=SERVER_CONNECTED;
	if(send_data(fd,&datum)<0)perror("SEND");
}
void run_server(){
	int serverfd,epollfd,nfds,rec;
	data datum;
	struct epoll_event evs[MAXEVENTS];
	room rooms[MAXROOMS]={0};
	int aroom=0;

	epollfd=create_epoll();
	serverfd=create_server_socket();
	epoll_add(epollfd,serverfd);
	printf("\nSERVER STARTED\n");

	while(1){
		nfds=wait_epoll(epollfd,evs);
		for(int i=0;i<nfds;i++){
			if(evs[i].data.fd==serverfd)handle_client_connection(serverfd,epollfd);
			else if(evs[i].events & (EPOLLHUP|EPOLLERR|EPOLLRDHUP))handle_client_disconnect(epollfd,evs[i].data.fd,rooms,&aroom);
			else{
				if((rec=receive_data(evs[i].data.fd,&datum))<0)perror("RECEIVE");
				if(!rec)handle_client_disconnect(epollfd,evs[i].data.fd,rooms,&aroom);
				else handle_client_request(evs[i].data.fd,datum,rooms,&aroom);
			}
		}
	}
}
