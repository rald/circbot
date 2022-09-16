#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stddef.h>
#include <string.h>
#include <ctype.h>
#include <time.h>


#define DYAD_IMPLEMENTATION
#include "dyad.h"

#define CONFIG_IMPLEMENTATION
#include "config.h"



#define CONFIG_PATH "config.cfg"


#define GAME_TITLE "WORDLE"
#define PFX "."

static int prt = 6667;
static char *srv = "sakura.jp.as.dal.net";
static char *chn = "#pantasya";
static char *nck = "siesti";
static char *pss = NULL;

#define STRING_MAX 256

#define RANDLIST_FILE "randlist.txt"
#define WORDLIST_FILE "wordlist.txt"

Config **configs=NULL;
size_t nconfigs=0;

dyad_Stream *stm;



static char *cmd=NULL, *usr=NULL, *par=NULL, *txt=NULL;

int isReg=0;


char *word=NULL;


size_t elapsedtime=0,remainingtime=0,allottedtime=180;


typedef enum GameState {
	GAMESTATE_INIT=0,
	GAMESTATE_START,
	GAMESTATE_MAX
} GameState;

GameState gamestate=GAMESTATE_INIT;




static void sendf(dyad_Stream * s, char *fmt, ...);



static void onConnect(dyad_Event *e);
static void onError(dyad_Event *e);
static void onTick(dyad_Event *e);
static void onLine(dyad_Event *e);


char *GetRandLine(char *path);
int check(char msg[],char *w1,char *w2);
int find(char *path,char *w);



int main(void) {


	size_t i;

	srand((unsigned int)time(NULL));

    if(ReadConfig(CONFIG_PATH,&configs,&nconfigs)!=0) {
        return 1;
    }

    for(i=0; i<nconfigs; i++) {
        if(strcasecmp(configs[i]->key,"port")==0) {
            prt=atoi(configs[i]->val);
        } else if(strcasecmp(configs[i]->key,"server")==0) {
            srv=configs[i]->val;
        } else if(strcasecmp(configs[i]->key,"nick")==0) {
            nck=configs[i]->val;
        } else if(strcasecmp(configs[i]->key,"channel")==0) {
            chn=configs[i]->val;
        } else if(strcasecmp(configs[i]->key,"password")==0) {
            pss=configs[i]->val;
        }
    }



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



static void onConnect(dyad_Event *e) {
	if (pss) {
		sendf(e->stream, "PASS %s", pss);
	}
	sendf(e->stream, "NICK %s", nck);
	sendf(e->stream, "USER %s %s %s :%s", nck, nck, nck, nck);
}



static void onError(dyad_Event *e) {
	printf("error: %s\n", e->msg);
}



static void onTick(dyad_Event *e) {

	if(gamestate==GAMESTATE_START) {
		elapsedtime++;
	
		remainingtime=allottedtime-elapsedtime;
	
		if(remainingtime<=0) remainingtime=0;
	
		if(remainingtime==0) {
			sendf(e->stream,"PRIVMSG %s :" GAME_TITLE " Time Up.",chn);
			sendf(e->stream,"PRIVMSG %s :" GAME_TITLE " The word is \"%s\".",chn,word);
			sendf(e->stream,"PRIVMSG %s :" GAME_TITLE " Game Over.",chn);
			gamestate=GAMESTATE_INIT;
		}
	}

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

	if (strcmp(cmd, "PING")==0) {
		sendf(e->stream, "PONG %s", txt);
	} else if (strcmp(cmd, "001")==0) {
		printf("Connected.\n");
		sendf(e->stream, "JOIN %s", chn);
		isReg = 1;
	} else if (strcmp(cmd, "PRIVMSG")==0) {


		printf("<%s> %s\n", usr, txt);


		if(strcasecmp(txt,PFX "start")==0) {
			if(gamestate==GAMESTATE_INIT) {

				if(word!=NULL) {
					free(word);
					word=NULL;
				}

				word=GetRandLine(RANDLIST_FILE);

				trim(word);

				sendf(e->stream,"PRIVMSG %s :" GAME_TITLE " %s, game is started.",chn,usr);

				elapsedtime=0;
				gamestate=GAMESTATE_START;
			}
		} else if(strncasecmp(txt,PFX "word",5)==0) {
			if(gamestate==GAMESTATE_START) {
				if(strlen(txt)>5) {
					char msg[STRING_MAX];
					char *guess=txt+5;
					int res=0;
					trim(guess);
					res=check(msg,word,guess);
					sendf(e->stream,"PRIVMSG %s :" GAME_TITLE " %s, %s",chn,usr,msg);
					if(res==1) {
						sendf(e->stream,"PRIVMSG %s :" GAME_TITLE " Game Over.",chn);
						gamestate=GAMESTATE_INIT;
					}
				}
			} else {
				
			}	
		}
	}

	free(tmp);
	tmp=NULL;
}



char *GetRandLine(char *path) {
	FILE *fin;

	char *p=NULL;
	size_t n=0;
	char line[STRING_MAX];
	char sline[STRING_MAX]={0};

	if((fin=fopen(path,"rt"))==NULL) {
		return NULL;
	}

	while(fgets(line,STRING_MAX,fin)) {
		p=strchr(line,'\n');
		if(p) *p='\0';
		n++;

		if((double)1/n>=drand()) {
			strncpy(sline,line,STRING_MAX);
		}
	}

	return strdup(strupr(sline));
}


int check(char msg[],char *w1,char *w2) {
	char s1[STRING_MAX];
	char s2[STRING_MAX];
	
	int lenw1=strlen(w1);
	int lenw2=strlen(w2);

	int stat1[STRING_MAX];
	int stat2[STRING_MAX];

	char cmsg[STRING_MAX]={0};

	int i,j;

	strncpy(s1,w1,STRING_MAX);
	strncpy(s2,w2,STRING_MAX);

	strupr(trim(s1));
	strupr(trim(s2));

	if(strcasecmp(s1,s2)==0) {
		sprintf(msg,"Correct! The word is \"%s\".",w1);
		return 1;
	} else if(lenw1!=lenw2) {
		sprintf(msg,"The word must be %d letters.",lenw1);
	} else if(!find(WORDLIST_FILE,s2)) {
		sprintf(msg,"The word \"%s\" is not in the dictionary.",w2);
	} else {
		for(i=0;i<lenw1;i++) {
			stat1[i]=0;
			stat2[i]=0;
		}

		for(i=0;i<lenw1;i++) {
			if(s2[i]==s1[i]) {
				stat1[i]=1;
				stat2[i]=1;
			}
		}


		for(i=0;i<lenw2;i++) {
			if(stat2[i]!=0) continue;

			for(j=0;j<lenw1;j++) {
				if(stat1[j]==0 && s2[i]==s1[j]) {
					stat1[j]=2;
					stat2[i]=2;
					break;
				}
			}
		}

		for(i=0;i<lenw2;i++) {
			switch(stat2[i]) {
				case 0: cmsg[i]='?'; break;
				case 1: cmsg[i]=toupper(s2[i]); break;
				case 2: cmsg[i]=tolower(s2[i]); break;
			}
		}

		cmsg[lenw2]='\0';

		sprintf(msg,"Clue: %s.",cmsg);
	}

	return 0;
}




int find(char *path,char *w) {
	FILE *fin;
	char line[STRING_MAX];
	char *p=NULL;

	if((fin=fopen(path,"rt"))==NULL) {
		return 0;
	}

	while(fgets(line,STRING_MAX,fin)) {
		p=strchr(line,'\n');
		if(p) *p='\0';

		trim(line);

		if(strcasecmp(line,w)==0) {
			return 1;
		}
	}

	return 0;
}



