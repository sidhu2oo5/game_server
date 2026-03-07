#ifndef server_h
#define server_h
#define MAXROOMS 1024

struct room{
	int members;
	int creater;
	int joiner;
	int c_move;
	int j_move;
};
struct data;
void accept_client(int,int);
int create_room(int,struct room*,int);
void join_room(int,struct data,struct room*);
int leave_room(int,struct data,struct room*,int);
void send_result(struct data,struct room*);
void handle_player_move(int,struct data,struct room*);
void handle_opponent_move(struct data,struct room*);
int game_aftermath(struct data,struct room*,int);
int handle_game_aftermath(int,struct data,struct room*,int);
void handle_rematch_accept(int,struct data,struct room*);
void handle_rematch_decline(int,struct data,struct room*);
int handle_client_request(int,struct data,struct room*,int);
void run_server(void);

#endif
