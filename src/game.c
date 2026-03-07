#include "game.h"
int find_winner(int p1_move,int p2_move){
	int result;
	if(p1_move==p2_move)result=0;
	else if((p1_move+1)%3==p2_move)result=1;
	else result=-1;	
	return result;
}
