#ifndef protocol_h
#define protocol_h
#include <stdint.h>
enum msgtype{
	CREATE_ROOM,
	JOIN_ROOM,
	PLAYER_MOVE,
	OPPONENT_MOVE,
	GAME_AFTERMATH,
	REMATCH,
	REMATCH_ACCEPT,
	REMATCH_DECLINE,
	LEAVE_ROOM,
	ROOM_CREATED,
	ROOM_JOINED,
	PLAYER_JOINED,
	PLAYER_LEFT,
	RESULT
};
struct data{
	int type;
	int rno;
	int val;
};
#endif
