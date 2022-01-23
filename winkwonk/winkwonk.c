#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdarg.h>
#include <stddef.h>
#include <string.h>
#include <ctype.h>
#include <time.h>

#include "dyad.h"


#define GAME_TITLE "WINKWONK"
#define PFX "."



static int prt = 6667;
static char *srv = "sakura.jp.as.dal.net";
static char *chn = "#pantasya";
static char *nck = "siesti";
static char *pss = NULL;



static char *cmd=NULL, *usr=NULL, *par=NULL, *txt=NULL;

static bool isReg=false;

static size_t ticks=0;



typedef enum GameState GameState;

enum GameState {
	GAME_STATE_INIT=0,
	GAME_STATE_START,
	GAME_STATE_MAX
};

GameState gamestate=GAME_STATE_INIT;



typedef enum PlayerRole PlayerRole;

enum PlayerRole {
	PLAYER_ROLE_BULLY=0,
	PLAYER_ROLE_STUDENT,
	PLAYER_ROLE_COUNCILOR,
	PLAYER_ROLE_PRINCIPAL,
	PLAYER_ROLE_MAX
};



typedef struct Player Player;

struct Player {
	char *nick;
	int score;
	PlayerRole role;
	bool bullied;
};

static Player **players=NULL;
static size_t nplayers=0;



static void sendf(dyad_Stream * s, char *fmt, ...);
static char *trim(char *a);
static char *skip(char *s, char c);

static Player *CreatePlayer(char *nick,int score);
static void DestroyPlayer(Player **player);
static void DestroyPlayers(Player ***players,size_t *nplayers);
static void AddPlayer(Player ***players,size_t *nplayers,char *nick,int score);
static ssize_t FindPlayer(Player **players,size_t nplayers,char *nick);

static void onConnect(dyad_Event *e);
static void onError(dyad_Event *e);
static void onTick(dyad_Event *e);
static void onLine(dyad_Event *e);



int main(void) {

	dyad_Stream *stm;

	srand((unsigned int)time(NULL));

	dyad_init();

	stm = dyad_newStream();

	dyad_addListener(stm, DYAD_EVENT_CONNECT, onConnect, NULL);
	dyad_addListener(stm, DYAD_EVENT_ERROR, onError, NULL);
	dyad_addListener(stm, DYAD_EVENT_LINE, onLine, NULL);
	dyad_addListener(stm, DYAD_EVENT_TICK, onTick, NULL);

	printf("Connecting...\n");

	dyad_connect(stm, srv, prt);

	while (dyad_getStreamCount() > 0) {
		dyad_update();
	}

	dyad_shutdown();

	return 0;
}



static void sendf(dyad_Stream * s, char *fmt, ...) {

	char *buf1=NULL;
	char *buf2=NULL;
	ssize_t buflen=0;

	va_list args;

	va_start(args, fmt);
	buflen=vsnprintf( NULL, 0, fmt, args);
	va_end(args);

	buf1=calloc(buflen+5,sizeof(*buf1));
	buf2=calloc(buflen+5,sizeof(*buf2));

	va_start(args, fmt);
	vsprintf(buf1,fmt,args);
	va_end(args);

	sprintf(buf2,"<-- %s",buf1);

	printf("%s\n",buf2);

	dyad_writef(s, "%s\r\n", buf1);

	free(buf2);
	buf2=NULL;
	free(buf1);
	buf1=NULL;
}



static char *trim(char *a) {
    char *p = a, *q = a;
    while (isspace(*q))            ++q;
    while (*q)                     *p++ = *q++;
    *p = '\0';
    while (p > a && isspace(*--p)) *p = '\0';
	return a;
}



static char *skip(char *s, char c) {
	while (*s != c && *s != '\0')
		s++;
	if (*s != '\0')
		*s++ = '\0';
	return s;
}



static char *PlayerRoleToString(PlayerRole role) {
	char *result=NULL;
	switch(role) {
		case PLAYER_ROLE_BULLY:		result="Bully";		break;
		case PLAYER_ROLE_STUDENT:	result="Student";	break;
		case PLAYER_ROLE_COUNCILOR:	result="Councilor";	break;
		case PLAYER_ROLE_PRINCIPAL:	result="Principal";	break;
		default: break;
	}
	return result;
}



static Player *CreatePlayer(char *nick,int score) {
	Player *player=malloc(sizeof(*player));
	if(player) {
		player->nick=strdup(nick);
		player->score=score;
	}
	return player;
}



static void DestroyPlayer(Player **player) {
	free((*player)->nick);
	(*player)->nick=NULL;
	(*player)->score=0;
	free(*player);
	player=NULL;
}



static void DestroyPlayers(Player ***players,size_t *nplayers) {
	size_t i;
	for(i=0; i<(*nplayers); i++) {
		DestroyPlayer(&((*players)[i]));
	}
	free(*players);
	(*players)=NULL;
	(*nplayers)=0;
}



static void AddPlayer(Player ***players,size_t *nplayers,char *nick,int score) {
	(*players)=realloc(*players,sizeof(*players)*((*nplayers)+1));
	(*players)[(*nplayers)++]=CreatePlayer(nick,score);
}



static ssize_t FindPlayer(Player **players,size_t nplayers,char *nick) {
	size_t i;
	ssize_t j=-1;
	for(i=0; i<nplayers; i++) {
		if(!strcasecmp(nick,players[i]->nick)) {
			j=(ssize_t)i;
			break;
		}
	}
	return j;
}



static void onConnect(dyad_Event *e) {
	if (pss) {
		sendf(e->stream, "PASS %s", pss);
	}
	sendf(e->stream, "NICK %s", nck);
	sendf(e->stream, "USER %s %s %s :%s", nck, nck, nck, nck);
}



static void onError(dyad_Event *e) {
	printf("Error: %s\n", e->msg);
}



static void onTick(dyad_Event *e) {
	ticks++;
}



static void onLine(dyad_Event *e) {

	char *tmp=NULL;

	printf("--> %s\n", e->data);

	tmp = strdup(e->data);

	cmd = tmp;

	usr = srv;

	if (cmd==NULL) {
		return;
	}

	if (*cmd=='\0') {
		free(cmd);
		return;
	}

	if (cmd[0] == ':') {
		usr = cmd + 1;
		cmd = skip(usr, ' ');
		if (cmd[0] == '\0')
			return;
		skip(usr, '!');
	}
	skip(cmd, '\r');
	par = skip(cmd, ' ');
	txt = skip(par, ':');

	trim(par);
	trim(txt);

	if (strcasecmp(cmd, "PING")==0) {
		sendf(e->stream, "PONG :%s", txt);
	} else if (strcasecmp(cmd, "001")==0) {
		printf("Connected.\n");
		sendf(e->stream, "JOIN %s", chn);
		isReg = true;
	} else if (strcasecmp(cmd, "PRIVMSG")==0) {

		printf("<%s> %s\n", usr, txt);

		if(strcasecmp(txt,PFX "join")==0) {
			if(gamestate==GAME_STATE_INIT) {
				ssize_t k=FindPlayer(players,nplayers,usr);
				if(k==-1) {
					AddPlayer(&players,&nplayers,usr,0);
					sendf(e->stream,"PRIVMSG %s :" GAME_TITLE " %s, joined the game.",chn,usr);
				} else {
					sendf(e->stream,"PRIVMSG %s :" GAME_TITLE " %s, you are already joined.",chn,usr);
				}
			} else if(gamestate==GAME_STATE_START) {
				sendf(e->stream,"PRIVMSG %s :" GAME_TITLE " %s, you cannot join the game is already running.",chn,usr);
			}
		} else if(strcasecmp(txt,PFX "start")==0) {
			if(gamestate==GAME_STATE_INIT) {
				if(nplayers<4) {
					sendf(e->stream,"PRIVMSG %s :" GAME_TITLE " %s, there are only %zu player(s) joined must be 4 and up.",chn,usr,nplayers);
				} else {

					size_t i,j;

					size_t nroles=nplayers;
					PlayerRole *roles=malloc(sizeof(*roles)*nroles);
					PlayerRole role;

					roles[0]=PLAYER_ROLE_BULLY;
					roles[1]=PLAYER_ROLE_COUNCILOR;
					roles[2]=PLAYER_ROLE_PRINCIPAL;

					for(i=3;i<nroles;i++) {
						roles[i]=PLAYER_ROLE_STUDENT;
					}

					for(i=nroles-1;i>0;i--) {
						j=(size_t)(rand()%(i+1));
						role=roles[i];
						roles[i]=roles[j];
						roles[j]=role;
					}

					for(i=0;i<nplayers;i++) {
						players[i]->role=roles[i];
						players[i]->bullied=false;
					}

					sendf(e->stream,"PRIVMSG %s :" GAME_TITLE " Game is started...",chn);

					for(i=0;i<nplayers;i++) {
						sendf(e->stream,"PRIVMSG %s :" GAME_TITLE " %s, you role is '%s'.",players[i]->nick,players[i]->nick,PlayerRoleToString(players[i]->role));
					}


					ticks=0;
					gamestate=GAME_STATE_START;
				}
			} else if(gamestate==GAME_STATE_START) {
					sendf(e->stream,"PRIVMSG %s :" GAME_TITLE " %s, game is aready running.",chn,usr);
			}
		} else if(strcasecmp(txt,PFX "role")==0) {
			if(gamestate==GAME_STATE_START) {
				ssize_t k=FindPlayer(players,nplayers,usr);
				if(k!=-1) {
					sendf(e->stream,"PRIVMSG %s :" GAME_TITLE " %s, your role is %s.",usr,usr,PlayerRoleToString(players[k]->role));
				} else {
					sendf(e->stream,"PRIVMSG %s :" GAME_TITLE " %s, you are not joined.",chn,usr);
				}
			} else {
				sendf(e->stream,"PRIVMSG %s :" GAME_TITLE " %s, game is not started.",chn,usr);
			}
		} else if(strcasecmp(txt,PFX "status")==0) {

			ssize_t k=FindPlayer(players,nplayers,usr);

			if(k!=-1) {
				sendf(e->stream,"PRIVMSG %s :" GAME_TITLE " %s, your status is '%s'.",usr,usr,players[k]->bullied?"Bullied":"Normal");
			} else {
				sendf(e->stream,"PRIVMSG %s :" GAME_TITLE " %s, you are not joined.",usr,usr);
			}

		} else if(strncasecmp(txt,PFX "hit", 4)==0) {

			char *ins=NULL,*tgt=NULL;
			ssize_t k1,k2;

			if(gamestate==GAME_STATE_START) {

				ins=txt;
				tgt=skip(txt,' ');
				trim(tgt);

				k1=FindPlayer(players,nplayers,usr);

				if(k1!=-1) {
					k2=FindPlayer(players,nplayers,tgt);

					if(k1==k2) {
						sendf(e->stream,"PRIVMSG %s :" GAME_TITLE " %s, cannot hit yourself.",usr,usr);
					} else if(k2!=-1) {
						switch(players[k1]->role) {
							case PLAYER_ROLE_BULLY:
								players[k2]->bullied=true;
							break;
							case PLAYER_ROLE_STUDENT:

							break;
							case PLAYER_ROLE_COUNCILOR:
								players[k2]->bullied=false;
							break;
							case PLAYER_ROLE_PRINCIPAL:

							break;
							default: break;
						}
					} else {
						sendf(e->stream,"PRIVMSG %s :" GAME_TITLE " %s, cannot find player '%s'.",usr,usr,tgt);
					}
				} else {
					sendf(e->stream,"PRIVMSG %s :" GAME_TITLE " %s, you are not joined.",chn,usr);
				}
			} else {
				sendf(e->stream,"PRIVMSG %s :" GAME_TITLE " %s, game is not started.",chn,usr);
			}
		}
	}

	free(tmp);
	tmp=NULL;
}


