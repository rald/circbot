#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <time.h>
#include <ctype.h>

#include "dyad.h"
#include "list.h"
#include "string.h"
#include "GameState.h"
#include "PhotoData.h"
#include "player.h"

#define GAME_TITLE "4PICS1WORD"

#define PFX "."

#define BASE_ADDR "http://srv24711.blue.kundencontroller.de/4Pics1Word"
#define HTML_FILE "index.html"
#define DATA_FILE "PhotoData.txt"
#define SCORE_FILE "score.txt"

#define WORD_POINTS 4
#define REVEAL_PENALTY 2

#define STRING_MAX 512

int prt = 6667;
char *srv = "sakura.jp.as.dal.net";
char *chn = "#pantasya";
char *nck = "sieste";
char *pss = NULL;
char *mst = "siesta";
char *mps = "143445254";

char *cmd = NULL, *usr = NULL, *par = NULL, *txt = NULL;

int isReg = 0;
int isAuth = 0;

int ticks = 0;

GameState gameState = GAME_STATE_INIT;

List *photoDatas = NULL;

PhotoData *photoData = NULL;

List *players=NULL;

char clue[STRING_MAX];
char letters[STRING_MAX];

char *html =
    "<html>\n"
    "\t<head>\n"
    "\t\t<style>\n"
    "\t\t\t.container {\n"
    "\t\t\t\tposition: absolute;\n"
    "\t\t\t\ttop: 50%%;\n"
    "\t\t\t\tleft: 50%%;\n"
    "\t\t\t\ttransform: translateX(-50%%) translateY(-50%%);\n"
    "\t\t\t}\n"
    "\t\t</style>\n"
    "\t</head>\n"
    "\t<body>\n"
    "\t\t<div class='container'>\n"
    "\t\t\t<table>\n"
    "\t\t\t\t<tr>\n"
    "\t\t\t\t\t<td><img src='./pics/_%d_1.jpg'></td>\n"
    "\t\t\t\t\t<td><img src='./pics/_%d_2.jpg'></td>\n"
    "\t\t\t\t</tr>\n"
    "\t\t\t\t<tr>\n"
    "\t\t\t\t\t<td><img src='./pics/_%d_3.jpg'></td>\n"
    "\t\t\t\t\t<td><img src='./pics/_%d_4.jpg'></td>\n"
    "\t\t\t\t</tr>\n" "\t\t\t</table>\n" "\t\t</div>\n" "\t<body>\n" "<html>\n";

double drand()
{
	return rand() / (RAND_MAX + 1.0);
}

static void sendf(dyad_Stream * s, char *fmt, ...)
{
	va_list args;

	va_start(args, fmt);
	vprintf(fmt, args);
	va_end(args);

	va_start(args, fmt);
	dyad_vwritef(s, fmt, args);
	va_end(args);
}

static void onConnect(dyad_Event * e)
{
	if (pss) {
		dyad_writef(e->stream, "PASS %s\r\n", pss);
	}
	dyad_writef(e->stream, "NICK %s\r\n", nck);
	dyad_writef(e->stream, "USER %s %s %s :%s\r\n", nck, nck, nck, nck);
}

static void onError(dyad_Event * e)
{
	printf("error: %s\n", e->msg);
}

Player *Player_Find(List *players,char *nick)
{
	Player *player=NULL;
	List *iter=players;
	while(iter) {
		player=iter->data;
		if(!strcmp(nick,player->nick)) return player;
		iter=iter->next;
	}
	return NULL;
}

List *Letter_Find(char *clue,char letter) {
	size_t i;
	List *indexes=NULL;
	for(i=0;i<strlen(clue);i++) {
		if(clue[i]==letter) {
			int *j=malloc(sizeof(int));
			*j=i;
			List_PushBack(&indexes,j);
		}
	}
	return indexes;
}

void FreeInt(void *x) {
	free(x);
	x=NULL;
}

void swap(List *a,List *b) {
	Player *temp=a->data;
	a->data=b->data;
	b->data=temp;
}

void List_Sort(List *head) {
	int swapped=0;
	List *p1=NULL,*p2=NULL;
	if(!head) return;
	do {
		swapped=0;
		p1=head;
		while(p1->next!=p2) {
			Player *player1=p1->data;
			Player *player2=p1->next->data;
			if(player1->score < player2->score) {
				swap(p1,p1->next);
				swapped=1;
			}
			p1=p1->next;
		}
		p2=p1;
	} while(swapped);
}

void Players_Save(List *players,char *path) {
	List *iter=players;
	FILE *fout=fopen(path,"wt");
	while(iter) {
		Player *player=iter->data;
		fprintf(fout,"%s %i\n",player->nick,player->score);
		iter=iter->next;
	}
	fclose(fout);
}

List *Players_Load(char *path) {
	char nick[STRING_MAX];
	int score=0;
	List *players=NULL;
	FILE *fin=fopen(path,"rt");
	if(!fin) return NULL;
	while(fscanf(fin,"%s %i\n",nick,&score)==2) {
		List_PushBack(&players,Player_New(strdup(nick),score));
	}
	fclose(fin);
	return players;
}

static void onLine(dyad_Event * e)
{

	size_t i,j;
	char k;

	char *tmp = NULL;

	cmd = NULL;
	usr = NULL;
	par = NULL;
	txt = NULL;

	printf("%s\n", e->data);

	tmp = strdup(e->data);

	cmd = tmp;
	usr = srv;

	if (!tmp) {
		return;
	}

	if (!*tmp) {
		free(tmp);
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

/*
	printf("usr: %s\n",usr);
	printf("cmd: %s\n",cmd);
	printf("par: %s\n",par);
	printf("txt: %s\n",txt);
	printf("\n");
//*/

	if (!strcmp(cmd, "PING")) {
		sendf(e->stream, "PONG :%s\r\n", txt);
	} else if (!strcmp(cmd, "001")) {
		printf("connected.\n");
		sendf(e->stream,"JOIN :%s\r\n",chn);
		isReg = 1;
	} else if (!strcmp(cmd, "PRIVMSG")) {

		printf("<%s> %s\n", usr, txt);

		if (!strcmp(usr, mst)) {

			if (!strcmp(par, nck)) {

				if (!strncmp(txt, PFX "auth", 5)) {
					if (strlen(txt) > 6
					    && !strcmp(txt + 6, mps)) {
						isAuth = 1;
						sendf(e->stream,
						      "PRIVMSG %s :%s\r\n", usr,
						      "access granted");
					} else {
						isAuth = 0;
						sendf(e->stream,
						      "PRIVMSG %s :%s\r\n", usr,
						      "access denied");
					}
				}

			}

			if (isAuth) {

				if (!strncmp(txt, PFX "quit", 5)) {
					if (strlen(txt) > 5) {
						sendf(e->stream, "QUIT :%s\r\n",
						      trim(txt + 5));
					} else {
						sendf(e->stream, "QUIT\r\n");
					}
				}

			}

		}

		if (par[0] == '#') {

			if (!strcmp(txt, PFX "start")) {

				if(gameState==GAME_STATE_START) {
					sendf(e->stream,"PRIVMSG %s :"GAME_TITLE" %s: game is already started\r\n",par,usr);
				} if (gameState == GAME_STATE_INIT) {
					FILE *fout = NULL;

					photoData =
					    List_Get(photoDatas,
						     rand() %
						     List_Count
						     (photoDatas))->data;

					fout = fopen(HTML_FILE, "wt");

					fprintf(fout, html, photoData->id,
						photoData->id, photoData->id,
						photoData->id);

					fclose(fout);

					clue[0]='\0';
					for(i=0;i<strlen(photoData->solution);i++) {
						strcat(clue,"*");
					}

					letters[0]='\0';
					for(i=0;i<12;i++) {
						if(i<strlen(photoData->solution)) {
							strcat(letters,(char[2]){photoData->solution[i],'\0'});
						} else {
							strcat(letters,(char[2]){rand()%26+65,'\0'});
						}
					}

					for(i=strlen(letters)-1;i>0;i--) {
						j=(size_t)(rand()%(i+1));
						k=letters[i];
						letters[i]=letters[j];
						letters[j]=k;
					}

					sendf(e->stream,
					      "PRIVMSG %s :"GAME_TITLE" %s: clue: %s - %s pics: %s/%s\r\n", par,usr,clue,letters,
					      BASE_ADDR, HTML_FILE);

					gameState = GAME_STATE_START;
				}

			} else if (!strcmp(txt, PFX "reveal")) {

				if(gameState==GAME_STATE_START) {

					Player *player=Player_Find(players,usr);

					if(player && player->score>=REVEAL_PENALTY) {

						int position1,position2;

						List *indexes1=Letter_Find(clue,'*');
						if(indexes1) {
							position1=rand()%List_Count(indexes1);
							position2=*(int*)List_Get(indexes1,position1)->data;
							char letter=photoData->solution[position2];
							clue[position2]=letter;

							List *indexes2=Letter_Find(letters,letter);
							position1=rand()%List_Count(indexes2);
							position2=*(int*)List_Get(indexes2,position1)->data;
							for(i=position2;i<strlen(letters);i++) {
								letters[i]=letters[i+1];
							}

							List_Delete(indexes2,FreeInt);
							player->score-=REVEAL_PENALTY;
							List_Sort(players);
							Players_Save(players,SCORE_FILE);

							sendf(e->stream,"PRIVMSG %s :"GAME_TITLE" %s: reveal penalty minus %i points score %i\r\n",par,usr,REVEAL_PENALTY,player->score);

							sendf(e->stream,
							      "PRIVMSG %s :"GAME_TITLE" %s: clue: %s - %s pics: %s/%s\r\n", par,usr,clue,letters,
							      BASE_ADDR, HTML_FILE);

						} else {
							sendf(e->stream,"PRIVMSG %s :"GAME_TITLE" %s: nothing to reveal",par,usr);
						}
						List_Delete(indexes1,FreeInt);

					} else {
						sendf(e->stream,"PRIVMSG %s :"GAME_TITLE" %s: you must have at least %i points to reveal.\r\n",par,usr,REVEAL_PENALTY);
					}

				} else {
					sendf(e->stream,"PRIVMSG %s :"GAME_TITLE" %s: game not started\r\n",par,usr);
				}

			} else if (!strcmp(txt, PFX "hint")) {
				if(gameState==GAME_STATE_START) {
					sendf(e->stream,
					      "PRIVMSG %s :"GAME_TITLE" %s: clue: %s - %s pics: %s/%s\r\n", par,usr,clue,letters,
					      BASE_ADDR, HTML_FILE);
				} else {
					sendf(e->stream,"PRIVMSG %s :"GAME_TITLE" %s: game not started\r\n",par,usr);
				}
			} else if (!strncmp(txt, PFX "word",5)) {
				if(gameState==GAME_STATE_START) {
					if(strlen(txt)>5) {
						char *word=strupr(trim(txt+5));
						if(!strcmp(word,photoData->solution)) {
							Player *player=Player_Find(players,usr);
							if(!player) {
								player=Player_New(strdup(usr),0);
								List_PushBack(&players,player);
							}
							player->score+=WORD_POINTS;
							List_Sort(players);
							Players_Save(players,SCORE_FILE);
							sendf(e->stream,"PRIVMSG %s :"GAME_TITLE" %s: plus %i points score: %i\r\n",par,usr,4,player->score);
							gameState=GAME_STATE_INIT;
						} else {
							sendf(e->stream,"PRIVMSG %s :"GAME_TITLE" %s: your guess is wrong\r\n",par,usr);
						}
					}
				} else {
					sendf(e->stream,"PRIVMSG %s :"GAME_TITLE" %s: game not started\r\n",par,usr);
				}
			} else if (!strncmp(txt, PFX "score",6)) {

				if(players) {
					Player *player=NULL;

					if(strlen(txt)>6) {
						char *nick=trim(txt+6);

						if(!strcmp(usr,nick)) {
							player=Player_Find(players,usr);
							if(player) {
								sendf(e->stream,"PRIVMSG %s :"GAME_TITLE" %s: your score: %i\r\n",par,usr,player->score);
							} else {
								sendf(e->stream,"PRIVMSG %s :"GAME_TITLE" %s: your score: %i\r\n",par,usr,0);
							}
						} else {
							player=Player_Find(players,nick);
							if(player) {
								sendf(e->stream,"PRIVMSG %s :"GAME_TITLE" %s: %s's score: %i\r\n",par,usr,player->nick,player->score);
							} else {
								sendf(e->stream,"PRIVMSG %s :"GAME_TITLE" %s: player not found\r\n",par,usr);
							}
						}
					} else {
						player=Player_Find(players,usr);
						if(player) {
							sendf(e->stream,"PRIVMSG %s :"GAME_TITLE" %s: your score: %i\r\n",par,usr,player->score);
						} else {
							sendf(e->stream,"PRIVMSG %s :"GAME_TITLE" %s: your score: %i\r\n",par,usr,0);
						}
					}

				} else {
					sendf(e->stream,"PRIVMSG %s :"GAME_TITLE" %s: player list is empty\r\n",par,usr);
				}

			} else if (!strcmp(txt, PFX "top")) {
				char result[STRING_MAX];
				char temp[STRING_MAX];
				result[0]='\0';
				if(players) {
					List_Sort(players);
					List *iter=players;
					i=0;
					while(iter) {
						Player *player=iter->data;
						if(player->score>0) {
							if(i!=0) strcat(result,", ");
							temp[0]='\0';
							sprintf(temp,"%zu. %s %i",i+1,player->nick,player->score);
							strcat(result,temp);
							i++;
						}
						iter=iter->next;
						if(i>=10) break;
					}
				}
				if(strlen(result)) {
					sendf(e->stream,"PRIVMSG %s :"GAME_TITLE" %s: TOP: %s\r\n",par,usr,result);
				} else {
					sendf(e->stream,"PRIVMSG %s :"GAME_TITLE" %s: TOP: empty list\r\n",par,usr);
				}
/*
			} else if (!strcmp(txt, PFX "cheat")) {
				if(gameState==GAME_STATE_START) {
					sendf(e->stream,"PRIVMSG %s :"GAME_TITLE" %s: TOP: %s\r\n",par,usr,photoData->solution);
				} else {
					sendf(e->stream,"PRIVMSG %s :"GAME_TITLE" %s: game not started\r\n",par,usr);
				}
//*/
			}
		}

	} else if (!strcmp(cmd, "JOIN")) {
		printf("%s joined %s\n", usr, strlen(par) ? par : txt);
	} else if (!strcmp(cmd, "PART")) {
		printf("%s parted %s\n", usr, strlen(par) ? par : txt);
	} else if (!strcmp(cmd, "QUIT")) {
		printf("%s exits '%s'\n", usr, txt);
	}

	free(tmp);
	tmp = NULL;

}

static void onTick(dyad_Event * e)
{
	ticks++;
}

List *loadData()
{
	List *photoDatas = NULL;
	FILE *fin;
	int id, poolId;
	char solution[STRING_MAX];

	fin = fopen(DATA_FILE, "rt");

	while (fscanf(fin, "%i,%i,%s\n", &id, &poolId, solution) == 3) {
		List_PushBack(&photoDatas,
			      PhotoData_New(id, poolId, strdup(solution)));
	}

	return photoDatas;
}

int main(void)
{

	dyad_Stream *s;

	srand(time(NULL));

	photoDatas = loadData();

	players=Players_Load(SCORE_FILE);

	dyad_init();

	s = dyad_newStream();

	dyad_addListener(s, DYAD_EVENT_CONNECT, onConnect, NULL);
	dyad_addListener(s, DYAD_EVENT_ERROR, onError, NULL);
	dyad_addListener(s, DYAD_EVENT_LINE, onLine, NULL);
	dyad_addListener(s, DYAD_EVENT_TICK, onTick, NULL);

	printf("connecting...\n");

	dyad_connect(s, srv, prt);

	while (dyad_getStreamCount() > 0) {
		dyad_update();
	}

	dyad_shutdown();

	return 0;
}
