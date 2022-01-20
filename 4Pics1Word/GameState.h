#ifndef GAMESTATE_H
#define GAMESTATE_H

typedef enum GameState GameState;

enum GameState {
	GAME_STATE_INIT=0,
	GAME_STATE_WAIT,
	GAME_STATE_START,
	GAME_STATE_PLAY,
	GAME_STATE_END,
	GAME_STATE_MAX
};

#endif
