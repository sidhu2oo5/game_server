#ifndef client_h
#define client_h
#include "protocol.h"

void clear_screen(void);
int start_client(void);
int get_stdin(void);
void exit_client(data*,int,int);
void menu(data*,int,int);
void create_room_req(data*,int);
void join_room_req(data*,int);
void leave_room_req(data*,int,int);
void lobby(data*);
void handle_server_response(data*,int,int);
void game_after_math(data*);
void room_conf(data*,int);
void send_move(data*,int);
void handle_stdin(data*,int,int);
void run_client(void);

#endif
