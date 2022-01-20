#ifndef PLAYER_H
#define PLAYER_H

#include <stdlib.h>

typedef struct Player Player;

struct Player {
	char *nick;
	int score;
};

Player *Player_New(char *nick, int score);

#endif
