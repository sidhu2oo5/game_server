#ifndef server_h
#define server_h
#define MAXROOMS 1024
#include "protocol.h"
typedef struct {
	int type;
	int members;
	int roles[2]; 
	int moves[2];
}room;
int accept_client(int,int);
void create_room(data,int,room*,int*);
void join_room(int,data,room*);
void leave_room(int,data,room*,int*);
void send_result(data,room*);
void handle_player_move(data,room*);
void handle_opponent_move(data,room*);
void handle_rematch_request(data,room*);
void handle_rematch_accept(data,room*);
void handle_rematch_decline(data,room*);
void handle_player_ready(data,room*);
void handle_game_start(data,room*);
void change_room_privacy(data,room*);
void handle_client_request(int,data,room*,int*);
void run_server(void);
int find_client(int,data*,room*);
void handle_client_disconnect(int,int,room*,int*);
void handle_client_connection(int,int);

#endif
