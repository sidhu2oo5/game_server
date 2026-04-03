#ifndef protocol_h
#define protocol_h
#include <stdint.h>
typedef enum {
	SERVER_CONNECTED,
	ROOM_CREATE,
	ROOM_JOIN,
	ROOM_LEAVE,
	ROOM_PRIVACY_CHANGE,
	ROOM_PRIVACY_CHANGED,
	ROOM_FULL,
	ROOMS_FULL,
	PLAYER_JOINED,
	PLAYER_READY,
	PLAYER_MOVE,
	PLAYER_LEFT,
	OPPONENT_MOVE,
	OPPONENT_DISCONNECTED,
	GAME_START,
	GAME_RESULT,
	GAME_AFTERMATH,
	LOBBY,
	REMATCH_REQUEST,
	REMATCH_ACCEPT,
	REMATCH_DECLINE,
	ROOM_CREATED,
	ROOM_JOINED,
	REMATCH_ACCEPTED,
	REMATCH_DECLINED
}room_status;
typedef struct {
	room_status status;
	int rno;
	int rtype;
	int count;
	int val;
	int role;
	int ready;
}data;
#endif
