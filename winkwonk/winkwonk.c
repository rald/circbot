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
static char *srv = "::1";
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
	PLAYER_ROLE_NONE=0,
	PLAYER_ROLE_BULLY,
	PLAYER_ROLE_DUMB,
	PLAYER_ROLE_LIAR,
	PLAYER_ROLE_SUCKER,
	PLAYER_ROLE_TRANSFEREE,
	PLAYER_ROLE_STUDENT,
	PLAYER_ROLE_COUNCILOR,
	PLAYER_ROLE_PRESIDENT,
	PLAYER_ROLE_PRINCIPAL,
	PLAYER_ROLE_MAX
};



typedef struct Player Player;

struct Player {
	char *nick;
	int score;
	PlayerRole role;
	bool bullied;
	bool lie;
};

static Player **players=NULL;
static size_t nplayers=0;

static PlayerRole *roles=NULL;
static size_t nroles=0;;

static size_t numbullied=0;


static void sendf(dyad_Stream * s, char *fmt, ...);
static char *trim(char *a);
static char *skip(char *s, char c);

static Player *CreatePlayer(char *nick,int score,PlayerRole role);
static void DestroyPlayer(Player **player);
static void DestroyPlayers(Player ***players,size_t *nplayers);
static void AddPlayer(Player ***players,size_t *nplayers,char *nick,int score,PlayerRole role);
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
		case PLAYER_ROLE_NONE:			result="none";			break;
		case PLAYER_ROLE_BULLY:			result="bully";			break;
		case PLAYER_ROLE_DUMB:			result="dumb";			break;
		case PLAYER_ROLE_LIAR:			result="liar";			break;
		case PLAYER_ROLE_SUCKER:		result="sucker";		break;
		case PLAYER_ROLE_TRANSFEREE:	result="transferee";	break;
		case PLAYER_ROLE_STUDENT:		result="student";		break;
		case PLAYER_ROLE_COUNCILOR:		result="councilor";		break;
		case PLAYER_ROLE_PRESIDENT:		result="president";		break;
		case PLAYER_ROLE_PRINCIPAL:		result="principal";		break;
		default: break;
	}
	return result;
}



static Player *CreatePlayer(char *nick,int score,PlayerRole role) {
	Player *player=malloc(sizeof(*player));
	if(player) {
		player->nick=strdup(nick);
		player->score=score;
		player->role=role;
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



static void AddPlayer(Player ***players,size_t *nplayers,char *nick,int score,PlayerRole role) {
	(*players)=realloc(*players,sizeof(*players)*((*nplayers)+1));
	(*players)[(*nplayers)++]=CreatePlayer(nick,score,role);
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

		if(strncasecmp(txt,PFX "quit",5)==0) {
			char *ins=NULL,*prm=NULL;

			ins=txt;
			prm=skip(ins,' ');

			if(prm!=NULL && *prm!='\0') {
				sendf(e->stream,"QUIT :%s",prm);
			} else {
				sendf(e->stream,"QUIT");
			}

		} else if(strcasecmp(txt,PFX "join")==0) {
			if(gamestate==GAME_STATE_INIT) {
				ssize_t k=FindPlayer(players,nplayers,usr);
				if(k==-1) {
					AddPlayer(&players,&nplayers,usr,0,PLAYER_ROLE_NONE);
					sendf(e->stream,"PRIVMSG %s :" GAME_TITLE " %s, joined the game.",chn,usr);
				} else {
					sendf(e->stream,"PRIVMSG %s :" GAME_TITLE " %s, you are already joined.",chn,usr);
				}
			} else if(gamestate==GAME_STATE_START) {
				sendf(e->stream,"PRIVMSG %s :" GAME_TITLE " %s, you cannot join the game is already running.",chn,usr);
			}
		} else if(strcasecmp(txt,PFX "players")==0) {
			sendf(e->stream,"PRIVMSG %s :" GAME_TITLE " %s, %zu players joined.",chn,usr,nplayers);
		} else if(strcasecmp(txt,PFX "start")==0) {
			if(gamestate==GAME_STATE_INIT) {
				if(nplayers<5) {
					sendf(e->stream,"PRIVMSG %s :" GAME_TITLE " %s, there are only %zu player(s) joined must be 5 and up.",chn,usr,nplayers);
				} else {

					size_t i,j;

					if(roles!=NULL) {
						free(roles);
						roles=NULL;
					}

					nroles=nplayers;
					roles=malloc(sizeof(*roles)*nroles);
					PlayerRole role;

					roles[0]=PLAYER_ROLE_BULLY;
					roles[1]=PLAYER_ROLE_COUNCILOR;
					roles[2]=PLAYER_ROLE_PRESIDENT;
					roles[3]=PLAYER_ROLE_PRINCIPAL;

					for(i=4;i<nroles;i++) {
						switch(rand()%5) {
							case 0: roles[i]=PLAYER_ROLE_DUMB; break;
							case 1: roles[i]=PLAYER_ROLE_LIAR; break;
							case 2: roles[i]=PLAYER_ROLE_SUCKER; break;
							case 3: roles[i]=PLAYER_ROLE_TRANSFEREE; break;
							case 4: roles[i]=PLAYER_ROLE_STUDENT; break;
							default: break;
						}
					}

					for(i=nroles-1;i>0;i--) {
						j=(size_t)(rand()%(i+1));
						role=roles[i];
						roles[i]=roles[j];
						roles[j]=role;
					}

					for(i=0;i<nplayers;i++) {
						players[i]->role=roles[i]==PLAYER_ROLE_TRANSFEREE?(players[i]->role==PLAYER_ROLE_NONE?PLAYER_ROLE_STUDENT:players[i]->role):roles[i];
						players[i]->bullied=false;
						players[i]->lie=false;
					}

					for(i=0;i<nplayers;i++) {
						sendf(e->stream,"PRIVMSG %s :" GAME_TITLE " %s, your role is '%s'.",players[i]->nick,players[i]->nick,PlayerRoleToString(players[i]->role));
					}

					sendf(e->stream,"PRIVMSG %s :" GAME_TITLE " Game is started...",chn);

					numbullied=0;
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
					sendf(e->stream,"PRIVMSG %s :" GAME_TITLE " %s, your role is '%s'.",usr,usr,PlayerRoleToString(players[k]->role));
				} else {
					sendf(e->stream,"PRIVMSG %s :" GAME_TITLE " %s, you are not joined.",chn,usr);
				}
			} else {
				sendf(e->stream,"PRIVMSG %s :" GAME_TITLE " %s, game is not started.",chn,usr);
			}
		} else if(strncasecmp(txt,PFX "status",7)==0) {

			char *ins=NULL,*tgt=NULL;

			if(gamestate==GAME_STATE_START) {

				ins=txt;
				tgt=skip(txt,' ');

				if(tgt!=NULL && *tgt!='\0') {

					ssize_t k=FindPlayer(players,nplayers,tgt);

					if(k!=-1) {
						if(players[k]->role==PLAYER_ROLE_LIAR) {
							sendf(e->stream,"PRIVMSG %s :" GAME_TITLE " %s, '%s' status is '%s'.",usr,usr,players[k]->nick,players[k]->lie?"bullied":"normal");
						} else {
							sendf(e->stream,"PRIVMSG %s :" GAME_TITLE " %s, '%s' status is '%s'.",usr,usr,players[k]->nick,players[k]->bullied?"bullied":"normal");
						}
					} else {
						sendf(e->stream,"PRIVMSG %s :" GAME_TITLE " %s, '%s' is not joined.",chn,usr);
					}

				} else {

					ssize_t k=FindPlayer(players,nplayers,usr);

					if(k!=-1) {
						if(players[k]->role==PLAYER_ROLE_LIAR) {
							sendf(e->stream,"PRIVMSG %s :" GAME_TITLE " %s, your real status is '%s' what they see is '%s'.",usr,usr,(players[k]->bullied?"bullied":"normal"),(players[k]->lie?"bullied":"normal"));
						} else {
							sendf(e->stream,"PRIVMSG %s :" GAME_TITLE " %s, your status is '%s'.",usr,usr,players[k]->bullied?"bullied":"normal");
						}
					} else {
						sendf(e->stream,"PRIVMSG %s :" GAME_TITLE " %s, you are not joined.",chn,usr);
					}

				}

			} else {
				sendf(e->stream,"PRIVMSG %s :" GAME_TITLE " %s, game is not started.",chn,usr);

			}

		} else if(strncasecmp(txt,PFX "hit", 4)==0) {

			char *ins=NULL,*tgt=NULL;
			ssize_t k1,k2;

			if(gamestate==GAME_STATE_START) {

				ins=txt;
				tgt=trim(skip(txt,' '));

				k1=FindPlayer(players,nplayers,usr);

				if(k1!=-1) {

					if(players[k1]->role==PLAYER_ROLE_DUMB || players[k1]->role==PLAYER_ROLE_STUDENT) {
						sendf(e->stream,"PRIVMSG %s :" GAME_TITLE " %s, you cannot hit.",usr,usr);
					} else if(players[k1]->bullied) {
						sendf(e->stream,"PRIVMSG %s :" GAME_TITLE " %s, you cannot hit because you are bullied.",usr,usr);
					} else if(players[k1]->role==PLAYER_ROLE_LIAR) {
						if(tgt==NULL || *tgt=='\0') {
							players[k1]->lie=!players[k1]->lie;
							sendf(e->stream,"PRIVMSG %s :" GAME_TITLE " %s, your real status is '%s' what they see is '%s'.",usr,usr,players[k1]->bullied?"bullied":"normal",players[k1]->lie?"bullied":"normal");
						} else {
							sendf(e->stream,"PRIVMSG %s :" GAME_TITLE " %s, you cannot hit a target.",usr,usr);
						}
					} else {

						k2=FindPlayer(players,nplayers,tgt);

						if(k2==-1) {
							sendf(e->stream,"PRIVMSG %s :" GAME_TITLE " %s, cannot find player '%s'.",usr,usr,tgt);
						} else if(players[k1]->role==PLAYER_ROLE_SUCKER) {								sendf(e->stream,"PRIVMSG %s :" GAME_TITLE " Gossip: '%s' is a bully.",chn,players[k2]->nick);
						} else if(k1==k2) {
							sendf(e->stream,"PRIVMSG %s :" GAME_TITLE " %s, cannot hit yourself.",usr,usr);
						} else {

							switch(players[k1]->role) {

								case PLAYER_ROLE_BULLY:

									if(!players[k2]->bullied) {

										if(players[k2]->role==PLAYER_ROLE_PRESIDENT) {
											sendf(e->stream,"PRIVMSG %s :" GAME_TITLE " %s, the bully hits president '%s' and loses.",chn,usr,players[k2]->nick);
											sendf(e->stream,"PRIVMSG %s :" GAME_TITLE " Game Over.",chn);
											gamestate=GAME_STATE_INIT;

										} else if(players[k2]->role!=PLAYER_ROLE_DUMB) {
											numbullied++;

											players[k2]->bullied=true;

											sendf(e->stream,"PRIVMSG %s :" GAME_TITLE " %s, you are bullied.",players[k2]->nick,players[k2]->nick);

											sendf(e->stream,"PRIVMSG %s :" GAME_TITLE " %s, you bullied '%s'.",usr,usr,players[k2]->nick);

											if(numbullied>=nplayers/3) {
												sendf(e->stream,"PRIVMSG %s :" GAME_TITLE " %s, the bully wins.",chn,usr);
												sendf(e->stream,"PRIVMSG %s :" GAME_TITLE " Game Over.",chn);
												gamestate=GAME_STATE_INIT;
											}

										} else if(players[k2]->role!=PLAYER_ROLE_DUMB) {

											sendf(e->stream,"PRIVMSG %s :" GAME_TITLE " %s, you are bullied.",players[k2]->nick,players[k2]->nick);

											sendf(e->stream,"PRIVMSG %s :" GAME_TITLE " %s, you bullied '%s'.",usr,usr,players[k2]->nick);


										}

									} else {
										sendf(e->stream,"PRIVMSG %s :" GAME_TITLE " %s, '%s' is already bullied.",usr,usr,players[k2]->nick);
									}

								break;
								case PLAYER_ROLE_COUNCILOR:
									if(players[k2]->bullied) {
										players[k2]->bullied=false;
										sendf(e->stream,"PRIVMSG %s :" GAME_TITLE " %s, you are healed.",players[k2]->nick,players[k2]->nick);
										sendf(e->stream,"PRIVMSG %s :" GAME_TITLE " %s, you heal '%s'.",usr,usr,players[k2]->nick);
									} else {
										sendf(e->stream,"PRIVMSG %s :" GAME_TITLE " %s, '%s' is not bullied.",usr,usr,players[k2]->nick);
									}
								break;
								case PLAYER_ROLE_PRESIDENT:
									if(players[k2]->role==PLAYER_ROLE_BULLY) {
										sendf(e->stream,"PRIVMSG %s :" GAME_TITLE " %s, you catched the bully '%s'.",chn,usr,players[k2]->nick);
									} else {
										sendf(e->stream,"PRIVMSG %s :" GAME_TITLE " %s, you mistaken '%s' for a bully.",chn,usr,players[k2]->nick);
									}

									sendf(e->stream,"PRIVMSG %s :" GAME_TITLE " Game Over.",chn);

									gamestate=GAME_STATE_INIT;

								break;
								default: break;
							}
						}
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


