#include "client.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/epoll.h>
#include <unistd.h>
#include "network.h"
#include "protocol.h"
#include "error_handle.h"
#include "game.h"
#include "epoll_utils.h"
void clear_screen(){
	printf("\033[H\033[J");
}
int start_client(){
	int clientfd=create_socket();
	if(connect_socket(clientfd)==0)printf("\nCONNECTED TO GAME SERVER\n");
	return clientfd;
}
int get_stdin(){
	char buff[256];
	int val=-1;
	if(read(STDIN_FILENO,buff,256)>-1){
		val=0;
		for(int i=0;buff[i] && buff[i]<='9' && buff[i]>='0';i++)val=val*10+buff[i]-'0';
		return val;
	}
	return val;
}
void exit_client(data*datum,int cfd,int efd){
	printf("\nCLOSING CLIENT\n");
	free(datum);
	close(cfd);
	close(efd);	
	exit(1);
}
void menu(data*datum,int cfd,int efd){
	int choice;
	printf("\nGAME MENU\n");
	printf("\n1.CREATE ROOM");
	printf("\n2.JOIN ROOM");
	printf("\n0.EXIT GAME\n");
	choice=get_value(0,2,"Choice");
	if(!choice)exit_client(datum,cfd,efd);
	else if(choice==1)create_room_req(datum,cfd);
	else if(choice==2)join_room_req(datum,cfd);
}
void create_room_req(data*datum,int fd){
	int choice;
	datum->status=ROOM_CREATE;
	printf("\n0.PUBLIC\n1.PRIVATE\n");
	choice=get_value(0,1,"Choice");
	datum->rtype=choice;
	if(send_data(fd,datum)<0)perror("SEND");
}
void join_room_req(data*datum,int fd){
	int choice;
	datum->status=ROOM_JOIN;
	printf("\n0.RANDOM\n1.ROOM-NO\n");
	choice=get_value(0,1,"Choice");
	if(choice)datum->rno=get_value(0,1023,"Room no");
	else datum->rno=-1;
	if(send_data(fd,datum)<0)perror("SEND");
}
void leave_room_req(data*datum,int cfd,int efd){
	datum->status=ROOM_LEAVE;
	if(send_data(cfd,datum)<0)perror("SEND");
	datum->rno=-1;
	datum->role=0;
	datum->count=1;
	datum->rtype=0;
	datum->ready=0;
	clear_screen();
	menu(datum,cfd,efd);
}
void lobby(data*datum){
	printf("\nROOM NO:%d\n",datum->rno);
	if(datum->count==1)printf("\nWAITING FOR ANOTHER PLAYER\n");
	else if(datum->role==0 && datum->ready==0)printf("\nWAITING FOR OPPONENT TO GIVE READY\n");
	printf("\nLOBBY");
	printf("\n1.LEAVE ROOM\n");
	if(datum->role==0){
		printf("2.ROOM SETTINGS\n");
		if(datum->ready==1)printf("3.START\n");
	}
	else if(datum->role==1)printf("2.READY\n");
	printf("ENTER CHOICE:");
	fflush(stdout);
}
void handle_server_response(data*datum,int cfd,int efd){
	switch(datum->status){
		case SERVER_CONNECTED:
		{
			clear_screen();
			printf("\nSERVER CONNECTED\n");
			menu(datum,cfd,efd);
			break;
		}
		case ROOM_CREATED:
		{
			datum->status=LOBBY;
			datum->ready=0;
			datum->count=1;
			clear_screen();
			printf("\nROOM CREATED\n");
			lobby(datum);
			break;
		}	
		case ROOM_JOINED:
		{
			datum->status=LOBBY;
			datum->count=2;
			datum->ready=0;
			clear_screen();
			printf("\nROOM JOINED\n");
			lobby(datum);
			break;
		}
		case ROOM_PRIVACY_CHANGED:
		{
			clear_screen();
			if(datum->role==0)printf("\nROOM PRIVACY CHANGED SUCCESSFULLY\n");
			else printf("\nROOM PRIVACY WAS CHANGED BY HOST TO %s\n",(datum->rtype==0)?"PUBLIC":"PRIVATE");
			datum->status=LOBBY;
			lobby(datum);
			break;
		}
		case PLAYER_JOINED:
		{
			clear_screen();
			printf("\nPLAYER JOINED\n");
			datum->status=LOBBY;
			datum->ready=0;
			datum->count=2;
			lobby(datum);
			break;
		}
		case PLAYER_READY:
		{
			clear_screen();
			printf("\nPLAYER READY\n");
			datum->ready=1;
			datum->status=LOBBY;
			lobby(datum);
			break;
		}
		case PLAYER_LEFT:
		{
			clear_screen();
			printf("\nPLAYER LEFT ROOM\n");
			datum->count=1;
			datum->status=LOBBY;
			datum->ready=0;
			lobby(datum);
			break;
		}
		case GAME_START:
		{
			clear_screen();
			printf("\nGAME STARTED\n");
			send_move(datum,cfd);
			break;
		}
		case OPPONENT_MOVE:
		{
			printf("\nOPPONENT CHOSE ");
			if(datum->val==ROCK)printf("ROCK\n");
			else if (datum->val==PAPER)printf("PAPER\n");
			else printf("SCISSOR\n");
			break;
		}
		case GAME_RESULT:
		{
			if(datum->val==-1)printf("\nYOU LOST\n");
			else if(datum->val==0)printf("\nDRAW\n");
			else printf("\nYOU WON\n");
			game_after_math(datum);
			break;
		}
		case REMATCH_REQUEST:
		{
			clear_screen();
			printf("\nOPPONENT REQUESTS REMATCH");			
			printf("\n1.REJECT");
			printf("\n2.ACCEPT");
			printf("\nENTER CHOICE:");
			fflush(stdout);
			break;
		}
		case REMATCH_ACCEPTED:
		{
			clear_screen();
			printf("\nOPPONENT ACCEPTED REMATCH\n");
			printf("\nGAME STARTED\n");
			send_move(datum,cfd);
			break;
		}
		case REMATCH_DECLINED:
		{
			clear_screen();
			printf("\nOPPONENT DECLINED REMATCH\n");
			game_after_math(datum);
			break;
		}
		case OPPONENT_DISCONNECTED:
		{
			clear_screen();
			printf("\nOPPONENT DISCONNECTED\n");
			datum->status=LOBBY;
			datum->count=1;
			datum->ready=0;
			lobby(datum);
			break;
		}
		case ROOMS_FULL:
		{
			clear_screen();
			printf("\nTHERE IS NO EMPTY ROOM\n");
			menu(datum,cfd,efd);
			break;	
		}
		case ROOM_FULL:
		{
			clear_screen();
			printf("\nROOM IS FULL\n");
			menu(datum,cfd,efd);
			break;
		}
		default:
		{
			break;
		}
	}
}
void game_after_math(data*datum){
	printf("\n1.LEAVE ROOM");
	printf("\n2.REQUEST REMATCH\n");
	printf("\nENTER CHOICE:");
	fflush(stdout);
	datum->status=GAME_AFTERMATH;
}
void room_conf(data*datum,int cfd){
	int choice;
	printf("\nROOM PRIVACY:%s\n",(datum->rtype==1)?"PRIVATE":"PUBLIC");
	printf("DO YOU WANT TO CHANGE IT\n0.NO\n1.YES\n");
	choice=get_value(0,1,"Choice");
	if(choice==1){
		datum->status=ROOM_PRIVACY_CHANGE;
		if(send_data(cfd,datum)<0)perror("SEND");
	}
	datum->status=LOBBY;
	lobby(datum);
}
void send_move(data*datum,int fd){
	int choice;
	printf("\n0.ROCK");
	printf("\n1.PAPER");
	printf("\n2.SCISSOR\n");
	choice=get_value(0,2,"Move");
	datum->status=PLAYER_MOVE;
	datum->val=choice;
	printf("\nYOU CHOSE %s\n",(choice==ROCK)?"ROCK":(choice==PAPER)?"PAPER":"SCISSOR");
	if(send_data(fd,datum)<0)perror("SEND");
	printf("\nWAITING FOR OPPONENT MOVE\n");
}
void handle_stdin(data*datum,int cfd,int efd){
	int input=get_stdin();
	switch(datum->status){
		case LOBBY:
		{
			if(input==1)leave_room_req(datum,cfd,efd);
			else if(datum->role==0){
				if(input==2)room_conf(datum,cfd);
				else if(datum->ready==1 && input==3){
					datum->status=GAME_START;
					if(send_data(cfd,datum)<0)perror("SEND");
				}
				else printf("\nINVALID INPUT\nRENTER:");
			}
			else if(datum->role==1 && input==2){
				datum->status=PLAYER_READY;
				if(send_data(cfd,datum)<0)perror("SEND");
			}
			else printf("\nINVALID INPUT\nRENTER:");
			break;
		}
		case GAME_AFTERMATH:
		{
			if(input==1)leave_room_req(datum,cfd,efd);
			else if(input==2){
				datum->status=REMATCH_REQUEST;
				if(send_data(cfd,datum)<0)perror("SEND");
			}
			else printf("\nINVALID INPUT\nRENTER:");
			break;
		}
		case REMATCH_REQUEST:
		{
			if(input==1){
				datum->status=REMATCH_DECLINE;
				if(send_data(cfd,datum)<0)perror("SEND");
				clear_screen();
				game_after_math(datum);
			}
			else if(input==2){
				datum->status=REMATCH_ACCEPT;
				if(send_data(cfd,datum)<0)perror("SEND");
				clear_screen();
				printf("\nGAME STARTED\n");
				send_move(datum,cfd);
			}
			else printf("\nINVALID INPUT\nRENTER:");
			break;
		}
		default:
		{
			if(input==0)exit_client(datum,cfd,efd);
			else printf("\nINVALID INPUT\nRENTER:");
		}
	}
}
void run_client(){
	int clientfd,epollfd,nfds;
	struct epoll_event evs[2];
	data * datum = (data*)malloc(sizeof(data));

	epollfd=create_epoll();
	epoll_add(epollfd,STDIN_FILENO);
	clientfd=start_client();
	epoll_add_socket(epollfd,clientfd);

	while(1){
		if(datum->status!=LOBBY && datum->status!=GAME_AFTERMATH && datum->status!=REMATCH_REQUEST){
			printf("\nWaiting For Server\nIf Server Took Too Long To Respond Press 0 To Exit\n");
		}
		nfds=wait_epoll(epollfd,evs);	
		for(int i=0;i<nfds;i++){
			if(evs[i].data.fd==STDIN_FILENO){
				handle_stdin(datum,clientfd,epollfd);
			}
			else if(evs[i].events &(EPOLLHUP|EPOLLERR|EPOLLRDHUP)){
				printf("\nServer Disconnected\n");
				exit_client(datum,clientfd,epollfd);	
			}
			else{
				if(receive_data(clientfd,datum)<0)perror("RECEIVE");
		               	handle_server_response(datum,clientfd,epollfd);
			}
		}
	}
}
