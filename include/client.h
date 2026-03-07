#ifndef client_h
#define client_h

struct data;

int start_client(void);
int create_room_req(int);
int join_room_req(int);
void handle_server_response(struct data);
void send_move(int,int);
void receive_move(int);
int game_aftermath_req(int,int);
void run_client(void);

#endif
