#include "client.h"
#include <stdio.h>
#include <stdlib.h>
#include "network.h"
#include "protocol.h"
#include "error_handle.h"
#include "game.h"

int start_client(){
	int clientfd=create_socket();
	if(connect_socket(clientfd)==0)
		printf("\nCONNECTED TO GAME SERVRE\n");
	return clientfd;
}
int create_room_req(int fd){
	struct data msg;
	msg.type=CREATE_ROOM;
	if(send_data(fd,&msg)<0)perror("SEND");
	if(receive_data(fd,&msg)<0)perror("RECEIVE");
	handle_server_response(msg);
	if(receive_data(fd,&msg)<0)perror("RECEIVE");
	handle_server_response(msg);
	return msg.rno;
}
int join_room_req(int fd){
	struct data msg;
	msg.type=JOIN_ROOM;
	msg.rno=get_value(0,1023,"Room no");
	if(send_data(fd,&msg)<0)perror("SEND");
	if(receive_data(fd,&msg)<0)perror("RECEIVE");
	handle_server_response(msg);
	return msg.rno;
}

void handle_server_response(struct data msg){
	switch(msg.type){
		case ROOM_CREATED:
		{
			printf("\nROOM CREATED\n");
			printf("\nROOM NO:%d\n",msg.rno);
			printf("WAITING FOR PLAYER\n");
			break;
		}
		case ROOM_JOINED:
		{
			printf("\nROOM JOINED\n");
			break;
		}
		case PLAYER_JOINED:
		{
			printf("\nPLAYER JOINED ROOM\n");
			break;
		}
		case RESULT:
		{
			if(msg.val==-1)printf("\nYOU LOST\n");
			else if(msg.val==0)printf("\nDRAW\n");
			else printf("\nYOU WON\n");
			break;
		}
		case OPPONENT_MOVE:
		{
			printf("\nOPPONENT CHOSE ");
			if(msg.val==ROCK)printf("ROCK\n");
			else if (msg.val==PAPER)printf("PAPER\n");
			else printf("SCISSOR\n");
			break;
		}
		case REMATCH:
		{
			printf("\nOPPONENT REQUESTS REMATCH\n");
			break;
		}
		case REMATCH_ACCEPT:
		{
			printf("\nREMATCH ACCEPTED\n");
			break;
		}
		case REMATCH_DECLINE:
		{
			printf("\nREMATCH DECLINED\nOPPONENT LEFT ROOM\n");
			break;
		}
		case LEAVE_ROOM:
		{
			printf("\nLEAVING ROOM\n");
			break;
		}
	}
}
void send_move(int fd,int rno){
	struct data msg;
	int choice;
	printf("\n0.ROCK");
	printf("\n1.PAPER");
	printf("\n2.SCISSOR\n");
	choice=get_value(0,2,"Move");
	msg.type=PLAYER_MOVE;
	msg.val=choice;
	msg.rno=rno;
	printf("\nYOU CHOSE %s\n",(choice==ROCK)?"ROCK":(choice==PAPER)?"PAPER":"SCISSOR");
	if(send_data(fd,&msg)<0)perror("SEND");
	printf("\nWAITING FOR OPPONENT MOVE\n");
}
void receive_move_result(int fd){
	struct data msg;
	if(receive_data(fd,&msg)<0)perror("RECEIVE");
	handle_server_response(msg);
}
int game_aftermath_req(int fd,int rno){
	struct data msg;
	int choice;
	printf("\n0.LEAVE ROOM");
	printf("\n1.REQUEST REMATCH\n");
	choice=get_value(0,1,"Choice");
	msg.type=GAME_AFTERMATH;
	msg.rno=rno;
	msg.val=(choice)?REMATCH:LEAVE_ROOM;
	if(send_data(fd,&msg)<0)perror("SEND");
	if(receive_data(fd,&msg)<0)perror("RECEIVE");
	handle_server_response(msg);
	if(msg.type==REMATCH_DECLINE && choice==1){
		printf("\n0.LEAVE ROOM");
		printf("\n1.WAIT FOR ANOTHER OPPONENT");
		choice=get_value(0,1,"Choice");
		if(choice==0){
			msg.type=LEAVE_ROOM;
			msg.rno=rno;
			if(send_data(fd,&msg)<0)perror("SEND");
		}
		else{
			printf("\nWaiting for Player\n");
			if(receive_data(fd,&msg)<0)perror("RECEIVE");
			handle_server_response(msg);
		}
	}
	else if(msg.type==REMATCH && choice==0){
		printf("\n0.REJECT");
		printf("\n1.ACCEPT");
		choice=get_value(0,1,"Choice");
		msg.type=(choice)?REMATCH_ACCEPT:REMATCH_DECLINE;
		msg.rno=rno;
		if(send_data(fd,&msg)<0)perror("SEND");
	}
	return choice;
}
void run_client(){
	int clientfd,rno,choice;
	clientfd=start_client();
	do{
		printf("\nGAME MENU\n");
		printf("\n1.CREATE ROOM");
		printf("\n2.JOIN ROOM");
		printf("\n0.EXIT GAME\n");
		choice=get_value(0,2,"Choice");
		if(!choice)break;
		else if(choice==1)rno=create_room_req(clientfd);
		else if(choice==2)rno=join_room_req(clientfd);
		printf("GAME STARTED\n");
		do{
			send_move(clientfd,rno);
			receive_move_result(clientfd);
			receive_move_result(clientfd);
			choice=game_aftermath_req(clientfd,rno);
		}while(choice);
	}while(1);
}
