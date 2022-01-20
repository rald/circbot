#include "player.h"

Player *Player_New(char *nick,int score) {
	Player *player=malloc(sizeof(Player));
	if(player) {
		player->nick=nick;
		player->score=score;
	}
	return player;
}
