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

#define BOGGLE_IMPLEMENTATION
#include "boggle.h"

#define GAME_TITLE "BOGGLE"
#define PFX "."

#define SCORE_FILE "score.txt"


static int prt = 6667;
static char *srv = "sakura.jp.as.dal.net";
static char *chn = "#philippines";
static char *nck = "siesto";
static char *pss = "paanoanggagawinko";

static size_t allottedtime=180;

static char *cmd=NULL, *usr=NULL, *par=NULL, *txt=NULL;

static int isReg=0;

typedef enum GameState GameState;

enum GameState {
	GAME_STATE_INIT=0,
	GAME_STATE_START,
	GAME_STATE_MAX
};

static GameState gamestate=GAME_STATE_INIT;

typedef struct Player Player;

struct Player {
	char *nick;
	int score;
};

static Player **players=NULL;
static size_t nplayers=0;

static int *guessed=NULL;
static size_t numguessed=0;


static size_t ticks=0;


static void sendf(dyad_Stream * s, char *fmt, ...);
static char *trim(char *str);
static char *skip(char *s, char c);
static char *append(char **a,const char *fmt, ...);

static Player *CreatePlayer(char *nick,int score);
static void DestroyPlayer(Player **player);
static void DestroyPlayers(Player ***players,size_t *nplayers);
static void AddPlayer(Player ***players,size_t *nplayers,char *nick,int score);
static ssize_t FindPlayer(Player **players,size_t nplayers,char *nick);

static int loadscores(char *path,Player ***players,size_t *nplayers);
static int savescores(char *path,Player **players,size_t nplayers);

static int cmpbyscoredesc(const void *a,const void *b);

static void onConnect(dyad_Event *e);
static void onError(dyad_Event *e);
static void onTick(dyad_Event *e);
static void onLine(dyad_Event *e);



int main (void) {

	dyad_Stream *stm;

	srand((unsigned int)time(NULL));

	dyad_init();

	stm = dyad_newStream();

	dyad_addListener(stm, DYAD_EVENT_CONNECT, onConnect, NULL);
	dyad_addListener(stm, DYAD_EVENT_ERROR, onError, NULL);
	dyad_addListener(stm, DYAD_EVENT_LINE, onLine, NULL);
	dyad_addListener(stm, DYAD_EVENT_TICK, onTick, NULL);

	printf("connecting...\n");

	dyad_connect(stm, srv, prt);

	while (dyad_getStreamCount() > 0) {
		dyad_update();
	}

	dyad_shutdown();

	return 0;
}



static void sendf(dyad_Stream * s, char *fmt, ...) {
	va_list args;

	va_start(args, fmt);
	vprintf(fmt, args);
	va_end(args);

	va_start(args, fmt);
	dyad_vwritef(s, fmt, args);
	va_end(args);
}



char *trim(char *a) {
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



static char *append(char **a,const char *fmt,...) {

	char *b=NULL;
	ssize_t lenb=0;

	va_list args;

	va_start(args,fmt);
	lenb=vsnprintf(NULL,0,fmt,args);
	va_end(args);

	va_start(args,fmt);
	b=calloc(lenb+1,sizeof(*b));
	vsnprintf(b,lenb+1,fmt,args);
	va_end(args);

	if(*a) {
		(*a)=realloc(*a,sizeof(**a)*(strlen(*a)+lenb+1));
	} else {
		(*a)=calloc(lenb+1,sizeof(**a));
	}

	strcat(*a,b);

	free(b);

	return *a;
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



static int loadscores(char *path,Player ***players,size_t *nplayers) {
	FILE *fh;
	char nick[32];
	int score=0;

	if((fh=fopen(path,"rt"))==NULL) {
		printf("Error: cannot open %s.\n",path);
		return 1;
	}

	if((*players)!=NULL) {
		DestroyPlayers(players,nplayers);
	}

	while(fscanf(fh,"%s %d\n",nick,&score)==2) {
		AddPlayer(players,nplayers,nick,score);
	}

	fclose(fh);

	return 0;
}



static int savescores(char *path,Player **players,size_t nplayers) {
	FILE *fh;
	size_t i;

	if((fh=fopen(path,"wt"))==NULL) {
		printf("Error: cannot open %s.\n",path);
		return 1;
	}

	for(i=0; i<nplayers; i++) {
		fprintf(fh,"%s %d\n",players[i]->nick,players[i]->score);
	}

	fclose(fh);

	return 0;
}



static int cmpbyscoredesc(const void *a,const void *b) {
	Player *l=*(Player**)a;
	Player *r=*(Player**)b;
	if(l->score<r->score) return 1;
	else if(l->score>r->score) return -1;
	return 0;
}



static void onConnect(dyad_Event *e) {
	if (pss) {
		dyad_writef(e->stream, "PASS %s\r\n", pss);
	}
	dyad_writef(e->stream, "NICK %s\r\n", nck);
	dyad_writef(e->stream, "USER %s %s bla :%s\r\n", nck, nck, nck);
}



static void onError(dyad_Event *e) {
	printf("error: %s\n", e->msg);
}



static void onTick(dyad_Event *e) {
	ticks++;
	if(gamestate==GAME_STATE_START) {
		if(ticks>=allottedtime) {
			sendf(e->stream,"PRIVMSG %s :" GAME_TITLE " Time Up\r\n",chn);

			sendf(e->stream,"PRIVMSG %s :" GAME_TITLE " Game Over\r\n",chn);
			gamestate=GAME_STATE_INIT;
		}
	}
}



static void onLine(dyad_Event *e) {

	size_t i,j;

	char *tmp=NULL;

	printf("%s\n", e->data);

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

	if (strcmp(cmd, "PING")==0) {
		sendf(e->stream, "PONG :%s\r\n", txt);
	} else if (strcmp(cmd, "001")==0) {
		printf("connected.\n");
		sendf(e->stream, "JOIN %s\r\n", chn);
		isReg = 1;

		loadscores(SCORE_FILE,&players,&nplayers);

		LoadWords();
		LoadDice();

	} else if (strcmp(cmd, "PRIVMSG")==0) {

		printf("<%s> %s\n", usr, txt);

		if(strcasecmp(txt,PFX "start")==0) {

			if(gamestate==GAME_STATE_INIT) {

				char *tmp=NULL;

				ShuffleDice();
				NewBoard();
				GetWords();

				if(guessed!=NULL) {
					free(guessed);
					guessed=NULL;
				}
				guessed=calloc(nwords,sizeof(*guessed));
				numguessed=0;

				for(j=0;j<BOX_SIZE;j++) {
					for(i=0;i<BOX_SIZE;i++) {
						append(&tmp,"%-2s ",letters[j][i]=='q'?"Qu":(char[]){toupper(letters[j][i]),'\0'});
					}
					sendf(e->stream,"PRIVMSG %s :" GAME_TITLE " %s\r\n",chn,tmp);
					free(tmp);
					tmp=NULL;
				}

				ticks=0;

				gamestate=GAME_STATE_START;
			}

		} else if(strcasecmp(txt,PFX "board")==0) {
			if(gamestate==GAME_STATE_START) {

				char *tmp=NULL;

				for(j=0;j<BOX_SIZE;j++) {
					for(i=0;i<BOX_SIZE;i++) {
						append(&tmp,"%-2s ",letters[j][i]=='q'?"Qu":(char[]){toupper(letters[j][i]),'\0'});
					}
					sendf(e->stream,"PRIVMSG %s :" GAME_TITLE " %s\r\n",chn,tmp);
					free(tmp);
					tmp=NULL;
				}
			}
		} else if(strcasecmp(txt,PFX "time")==0) {
			if(gamestate==GAME_STATE_START) {
				int remainingtime=allottedtime-ticks;
				if(remainingtime<0) remainingtime=0;
				sendf(e->stream,"PRIVMSG %s :" GAME_TITLE " %s: Time left: %d mins %d secs\r\n",chn,usr,remainingtime/60,remainingtime%60);
			}
		} else if(strncasecmp(txt,PFX "score",6)==0) {
			ssize_t k;
			if(strlen(txt)>7) {
				k=FindPlayer(players,nplayers,trim(txt+7));
				if(k!=-1) sendf(e->stream,"PRIVMSG %s :" GAME_TITLE " %s: %s score is %d.\r\n",chn,usr,players[k]->nick,players[k]->score);
			} else {
				k=FindPlayer(players,nplayers,usr);
				if(k!=-1) sendf(e->stream,"PRIVMSG %s :" GAME_TITLE " %s: your score is %d.\r\n",chn,usr,players[k]->score);
			}
		} else if(strcasecmp(txt,PFX "top")==0) {
			char *msg=NULL;
			qsort(players,nplayers,sizeof(*players),cmpbyscoredesc);
			for(i=0; i<(nplayers<10?nplayers:10); i++) {
				if(i!=0) append(&msg,", ");
				append(&msg,"%d. %s %d",i+1,players[i]->nick,players[i]->score);
			}
			if(msg) {
				sendf(e->stream,"PRIVMSG %s :" GAME_TITLE " TOP: %s\r\n",chn,msg);
				free(msg);
				msg=NULL;
			}
		} else {
			if(gamestate==GAME_STATE_START) {
				ssize_t k=FindWord(trim(txt));
				size_t len=0;
				int points=0;

				if(k!=-1) {

					if(guessed[k]==0) {

						guessed[k]=1;

						numguessed++;

						len=strlen(words[k]);

						if(len==4) {
							points=1;
						} else if(len==5) {
							points=2;
						} else if(len==6) {
							points=3;
						} else if(len==7) {
							points=4;
						} else if(len>=8) {
							points=11;
						}

						ssize_t l=FindPlayer(players,nplayers,usr);

						if(l==-1) {
							AddPlayer(&players,&nplayers,usr,points);
						} else {
							players[l]->score+=points;
						}

						savescores(SCORE_FILE,players,nplayers);

						if(points>0) {
							sendf(e->stream,"PRIVMSG %s :" GAME_TITLE " %s guessed '%s' plus %d point(s).\r\n",chn,usr,words[k],points);
						}

						if(numguessed>=nwords) {

							sendf(e->stream,"PRIVMSG %s :" GAME_TITLE " %s finished the game..\r\n",chn,usr);

							gamestate=GAME_STATE_INIT;
						}

					} else {

						sendf(e->stream,"PRIVMSG %s :" GAME_TITLE " %s: '%s' is already guessed.\r\n",chn,usr,words[k]);

					}
 				}
			}
		}
	}
}
