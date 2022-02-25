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

#define UTIL_IMPLEMENTATION
#include "util.h"

#define CONFIG_IMPLEMENTATION
#include "config.h"

#define TONGITS_IMPLEMENTATION
#include "tongits.h"



#define CONFIG_PATH "tongits.cfg"



static int prt = 6667;
static char *srv = "sakura.jp.as.dal.net";
static char *chn = "#pantasya";
static char *nck = "siesto";
static char *pss = NULL;

static char *cmd=NULL, *usr=NULL, *par=NULL, *txt=NULL;

bool isReg=false;

size_t ticks=0;

Config **configs=NULL;
size_t nconfigs=0;



static void sendf(dyad_Stream * s, char *fmt, ...);

static void onConnect(dyad_Event *e);
static void onError(dyad_Event *e);
static void onTick(dyad_Event *e);
static void onLine(dyad_Event *e);



int main(void) {
    dyad_Stream *stm;
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

    va_start(args,fmt);
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
    ticks++;
}



static void onLine(dyad_Event *e) {

    char *tmp=NULL;

    printf("--> %s\n", e->data);

    if(e->data==NULL) return;

    tmp = strdup(e->data);

    if (tmp==NULL) {
        return;
    }

    if (*tmp=='\0') {
        free(tmp);
        return;
    }

    cmd = tmp;
    usr = srv;

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

        sendf(e->stream, "PONG :%s", txt);

    } else if (strcmp(cmd, "001")==0) {

        printf("Connected.\n");
        sendf(e->stream, "JOIN %s", chn);
        isReg = true;

    } else if (strcmp(cmd, "PRIVMSG")==0) {

        printf("<%s> %s\n", usr, txt);

        if(strcasecmp(txt,PFX "JOIN")==0) {

            if(gamestate==GAME_STATE_INIT) {
                if(nplayers<PLAYER_MAX) {
                    int k=FindPlayer(players,nplayers,usr);
                    if(k==-1) {
                        AddPlayer(players,&nplayers,PLAYER_MAX,usr);
                        sendf(e->stream,"PRIVMSG %s :" GAME_TITLE " %s, joined the game.",chn,usr);

                        if(nplayers==3) {
                            sendf(e->stream,"PRIVMSG %s :" GAME_TITLE " start the game when ready.",chn,usr);
                        } else {
                            sendf(e->stream,"PRIVMSG %s :" GAME_TITLE " %s, we still need %d more player(s).",chn,usr,3-nplayers);
                        }

                    } else {
                        sendf(e->stream,"PRIVMSG %s :" GAME_TITLE " %s, you already joined.",chn,usr);
                    }
                } else {
                    sendf(e->stream,"PRIVMSG %s :" GAME_TITLE " %s, cannot join maximum of %d players only.",chn,usr,PLAYER_MAX);
                }
            } else if(gamestate==GAME_STATE_START) {
                sendf(e->stream,"PRIVMSG %s :" GAME_TITLE " %s, you cannot join game is already running.",chn,usr);
            }

        } else if(strcasecmp(txt,PFX "PLAYERS")==0) {

            sendf(e->stream,"PRIVMSG %s :" GAME_TITLE " %s, there are %d player(s) joined.",chn,usr,nplayers);

        } else if(strcasecmp(txt,PFX "START")==0) {

            if(gamestate==GAME_STATE_INIT) {
                int i,j,k,l;

                k=FindPlayer(players,nplayers,usr);
                if(k!=-1) {
                    if(nplayers<PLAYER_MAX) {
                        sendf(e->stream,"PRIVMSG %s :" GAME_TITLE " %s, we need %d more players.",chn,usr,PLAYER_MAX-nplayers);
                    } else {

                        char*errmsg=NULL;
                        char *msg=NULL;
                        int cnt=0;

                        sendf(e->stream,"PRIVMSG %s :" GAME_TITLE " %s, game is started.",chn,usr);

                        errmsg=InitDeck(deck,&ndeck);

                        if(errmsg!=NULL) {
                            sendf(e->stream,"PRIVMSG %s :" GAME_TITLE " %s",chn,errmsg);
                            free(errmsg);
                            errmsg=NULL;

                            return;
                        }

                        ShuffleDeck(deck,ndeck);

                        msg=CardsToString(deck,ndeck);

                        printf("DECK: %s\n",msg);

                        free(msg);
                        msg=NULL;


                        for(i=0; i<nplayers; i++) {
                            players[i].isDealer=false;
                            players[i].isOpen=false;
                            players[i].isPick=true;
                            players[i].canFight=false;
                            players[i].isFighting=false;
                            players[i].isBurned=false;
                        }

                        if(dealer!=-1) {
                            players[dealer].isDealer=false;
                        }

                        dealer=rand()%nplayers;
                        players[dealer].isDealer=true;
                        players[dealer].isPick=false;

                        current=dealer;

                        i=dealer;
                        append(&msg,"Player arrangement (counter clockwise): ");
                        do {
                            if(i!=dealer) append(&msg,", ");
                            if(i==dealer)
                                append(&msg,"DEALER: ");
                            else
                                append(&msg,"PLAYER%d: ",++cnt);
                            append(&msg,"%s",players[i].nick);

                            l=(i==dealer?DEALER_CARD_MAX:PLAYER_CARD_MAX);
                            for(j=0; j<l; j++) {
                                int card;
                                char *errmsg=NULL;

                                errmsg=GetCardAtTop(&card,deck,&ndeck);

                                if(errmsg!=NULL) {
                                    sendf(e->stream,"PRIVMSG %s :" GAME_TITLE " %s, %s",chn,usr,errmsg);
                                    free(errmsg);
                                    errmsg=NULL;

                                    return;
                                } else {
                                    errmsg=PutCardAtBottom(players[i].cards,&(players[i].ncards),l,card);
                                    if(errmsg!=NULL) {
                                        sendf(e->stream,"PRIVMSG %s :" GAME_TITLE " %s",errmsg);
                                        free(errmsg);
                                        errmsg=NULL;

                                        return;
                                    }
                                }
                            }
                            i=(i+1)%nplayers;
                        } while(i!=dealer);

                        sendf(e->stream,"PRIVMSG %s :" GAME_TITLE " %s",chn,msg);

                        free(msg);
                        msg=NULL;

                        for(i=0; i<nplayers; i++) {

                            msg=CardsToString(players[i].cards,players[i].ncards);
                            sendf(e->stream,"NOTICE %s :" GAME_TITLE " %s",players[i].nick,msg);

                            free(msg);
                            msg=NULL;
                        }

                        sendf(e->stream,"PRIVMSG %s :" GAME_TITLE " %s, it is your turn.",chn,players[current].nick);

                        ticks=0;

                        gamestate=GAME_STATE_START;

                    }
                } else {
                    sendf(e->stream,"PRIVMSG %s :" GAME_TITLE " %s, you are not joined.",chn,usr);
                }

            } else if(gamestate==GAME_STATE_START) {
                sendf(e->stream,"PRIVMSG %s :" GAME_TITLE " %s, game is already running.",chn,usr);
            }

        } else if(strcasecmp(txt,PFX "HAND")==0) {

            if(gamestate==GAME_STATE_START) {
                char *msg=NULL;
                int cardsScore=0;
                int k=FindPlayer(players,nplayers,usr);
                if(k!=-1) {
                    msg=CardsToString(players[k].cards,players[k].ncards);
                    cardsScore=CountCardsScore(players[k].cards,players[k].ncards);
                    sendf(e->stream,"NOTICE %s :" GAME_TITLE " (%d) %s.",usr,cardsScore,msg);

                    free(msg);
                    msg=NULL;
                } else {
                    sendf(e->stream,"PRIVMSG %s :" GAME_TITLE " %s, you are not joined.",chn,usr);
                }
            } else {
                sendf(e->stream,"NOTICE %s :" GAME_TITLE " %s, game is not started.",chn,usr);
            }

        } else if(strcasecmp(txt,PFX "MELDS")==0) {

            if(gamestate==GAME_STATE_START) {
                int i,j,k;
                char *msg=NULL;
                char *tmp=NULL;
                bool first;
                bool none;
                k=FindPlayer(players,nplayers,usr);
                if(k!=-1) {
                    i=dealer;
                    do {
                        if(i!=dealer) append(&msg," ");
                        append(&msg,"[%s] ",players[i].nick);
                        none=true;
                        first=true;
                        for(j=0; j<nmelds; j++) {
                            if(i==melds[j]->playerId) {
                                none=false;
                                if(first) {
                                    first=false;
                                } else {
                                    append(&msg,", ");
                                }
                                tmp=CardsToString(melds[j]->cards,melds[j]->ncards);
                                append(&msg,"(%d) %s",j+1,tmp);
                                free(tmp);
                                tmp=NULL;
                            }
                        }
                        if(none) {
                            append(&msg,"none");
                        }
                        i=(i+1)%nplayers;
                    } while(i!=dealer);

                    sendf(e->stream,"PRIVMSG %s :" GAME_TITLE " MELDS: %s",chn,msg);
                    free(msg);
                    msg=NULL;

                } else {
                    sendf(e->stream,"PRIVMSG %s :" GAME_TITLE " %s, you are not joined.",chn,usr);
                }

            } else {
                sendf(e->stream,"PRIVMSG %s :" GAME_TITLE " %s, game is not started.",chn,usr);
            }

        } else if(strncasecmp(txt,PFX "DROP ",6)==0) {

            if(gamestate==GAME_STATE_START) {
                int k=FindPlayer(players,nplayers,usr);
                if(k==current) {
                    if(strlen(txt)>6) {
                        Meld **xmelds=NULL;
                        int nxmelds=0;
                        char *errmsg=NULL;
                        int i;

                        errmsg=Drop(&xmelds,&nxmelds,players,nplayers,k,trim(txt+6));
                        if(errmsg==NULL) {
                            char *msg=NULL;

                            for(i=0; i<nxmelds; i++) {
                                if(i!=0) append(&msg,", ");
                                append(&msg,"%s",CardsToString(xmelds[i]->cards,xmelds[i]->ncards));
                            }

                            sendf(e->stream,"PRIVMSG %s :" GAME_TITLE " %s, %s.",chn,usr,msg);

                            free(msg);
                            msg=NULL;

                            if(players[current].ncards==0) {
                                sendf(e->stream,"PRIVMSG %s :" GAME_TITLE " %s, TONGITS.",chn,usr);
                                sendf(e->stream,"PRIVMSG %s :" GAME_TITLE " %s, wins.",chn,usr);
                                gamestate=GAME_STATE_INIT;
                            }
                        } else {
                            sendf(e->stream,"PRIVMSG %s :" GAME_TITLE " %s, %s",chn,usr,errmsg);
                        }
                        free(errmsg);
                        errmsg=NULL;
                    }
                } else if(k!=-1) {
                    sendf(e->stream,"PRIVMSG %s :" GAME_TITLE " %s, it is not your turn.",chn,usr);
                } else {
                    sendf(e->stream,"PRIVMSG %s :" GAME_TITLE " %s, you are not joined.",chn,usr);
                }
            } else {
                sendf(e->stream,"NOTICE %s :" GAME_TITLE " %s, game is not started.",chn,usr);
            }

        } else if(strncasecmp(txt,PFX "LAY ",5)==0) {

            if(gamestate==GAME_STATE_START) {
                int k=FindPlayer(players,nplayers,usr);
                if(k==current) {
                    if(strlen(txt)>5) {

                        int meldId=-1;
                        char *mld=NULL;
                        char *prm=NULL;

                        int xcards[MELD_CARD_MAX];
                        int nxcards=0;

                        mld=trim(txt+5);
                        prm=skip(mld,',');
                        trim(prm);
                        meldId=atoi(mld)-1;

                        if(meldId>=0 || meldId<nmelds) {
                            char *errmsg=Lay(xcards,&nxcards,players,nplayers,k,meldId,prm);
                            if(errmsg==NULL) {
                                sendf(e->stream,"PRIVMSG %s :" GAME_TITLE " %s, you layed '%s' at meld number %d.",chn,usr,CardsToString(xcards,nxcards),meldId+1);

                                players[melds[meldId]->playerId].canFight=false;
                                if(players[current].ncards==0) {
                                    sendf(e->stream,"PRIVMSG %s :" GAME_TITLE " %s, TONGITS.",chn,usr);
                                    sendf(e->stream,"PRIVMSG %s :" GAME_TITLE " %s, wins.",chn,usr);
                                    gamestate=GAME_STATE_INIT;
                                }

                            } else {
                                sendf(e->stream,"PRIVMSG %s :" GAME_TITLE " %s, %s.",chn,usr,errmsg);
                            }

                            free(errmsg);
                            errmsg=NULL;
                        } else {
                            sendf(e->stream,"PRIVMSG %s :" GAME_TITLE " %s, invalid meld number '%d'.",chn,usr,meldId);
                        }

                    }
                } else if(k!=-1) {
                    sendf(e->stream,"PRIVMSG %s :" GAME_TITLE " %s, it is not your turn.",chn,usr);
                } else {
                    sendf(e->stream,"PRIVMSG %s :" GAME_TITLE " %s, you are not joined.",chn,usr);
                }
            } else {
                sendf(e->stream,"NOTICE %s :" GAME_TITLE " %s, game is not started.",chn,usr);
            }

        } else if(strncasecmp(txt,PFX "DUMP ",6)==0) {

            if(gamestate==GAME_STATE_START) {
                int k=FindPlayer(players,nplayers,usr);
                if(k==current) {
                    if(strlen(txt)>6) {
                        int card;

                        char *errmsg=Dump(&card,players,nplayers,k,trim(txt+6));


                        if(errmsg==NULL) {

                            sendf(e->stream,"PRIVMSG %s :" GAME_TITLE " %s, you dump card '%s' to pile.",chn,usr,CardToString(card));
                            free(tmp);
                            tmp=NULL;

                            if(players[current].ncards==0) {
                                sendf(e->stream,"PRIVMSG %s :" GAME_TITLE " %s, TONGITS.",chn,usr);
                                sendf(e->stream,"PRIVMSG %s :" GAME_TITLE " %s, wins.",chn,usr);
                                gamestate=GAME_STATE_INIT;
                            } else if(ndeck==0) {

                                char *msg=NULL;

                                int i,j,k;

                                sendf(e->stream,"PRIVMSG %s :" GAME_TITLE " %s, there are no cards in the stock.",chn,usr);


                                i=dealer;
                                j=0;
                                do {
                                    if(i!=dealer) append(&msg,", ");

                                    players[i].cardsScore=CountCardsScore(players[i].cards,players[i].ncards);

                                    if(i==dealer) {
                                        k=i;
                                        append(&msg,"DEALER: ");
                                    } else {
                                        if(players[i].cardsScore<players[k].cardsScore) k=i;
                                        append(&msg,"PLAYER%d: ",++j);
                                    }

                                    append(&msg,"%s: %d",players[i].cardsScore);

                                    i=(i+1)%nplayers;

                                } while(i!=dealer);

                                sendf(e->stream,"PRIVMSG %s :" GAME_TITLE " %s, %s.",chn,usr,msg);


                                sendf(e->stream,"PRIVMSG %s :" GAME_TITLE " %s, wins.",chn,usr);

                                gamestate=GAME_STATE_INIT;
                            }

                            players[current].canFight=true;

                            NextPlayer();

                            sendf(e->stream,"PRIVMSG %s :" GAME_TITLE " %s, it is your turn.",chn,players[current].nick);

                        } else {
                            sendf(e->stream,"PRIVMSG %s :" GAME_TITLE " %s, invalid dump.",chn,usr);
                        }
                    }
                } else if(k!=-1) {
                    sendf(e->stream,"PRIVMSG %s :" GAME_TITLE " %s, it is not your turn.",chn,usr);
                } else {
                    sendf(e->stream,"PRIVMSG %s :" GAME_TITLE " %s, you are not joined.",chn,usr);
                }
            } else {
                sendf(e->stream,"PRIVMSG %s :" GAME_TITLE " %s, game is not started.",chn,usr);
            }

        } else if(strncasecmp(txt,PFX "PICK",5)==0) {

            if(gamestate==GAME_STATE_START) {
                int k=FindPlayer(players,nplayers,usr);

                if(k==current) {

                    if(players[current].isPick) {

                        char *ins=NULL;

                        ins=skip(txt,' ');
                        skip(ins,' ');
                        trim(ins);

                        if(strcasecmp(ins,"STOCK")==0) {

                            int card_max;
                            int card;

                            char *errmsg=NULL;
                            char *tmp=NULL;

                            card_max=players[current].isDealer?DEALER_CARD_MAX:PLAYER_CARD_MAX;
                            errmsg=GetCardAtTop(&card,deck,&ndeck);

                            if(errmsg!=NULL) {
                                sendf(e->stream,"PRIVMSG %s :" GAME_TITLE " %s",errmsg);
                                free(errmsg);
                                errmsg=NULL;
                            } else {

                                errmsg=PutCardAtBottom(players[current].cards,&players[current].ncards,PLAYER_CARD_MAX,card);

                                if(errmsg!=NULL) {
                                    sendf(e->stream,"PRIVMSG %s :" GAME_TITLE " %s",errmsg);
                                    free(errmsg);
                                    errmsg=NULL;
                                } else {
                                    tmp=CardToString(card);

                                    sendf(e->stream,"NOTICE %s :" GAME_TITLE " %s, you picked '%s' form stock.",usr,usr,tmp);

                                    free(tmp);
                                    tmp=NULL;

                                    players[current].isPick=false;
                                }
                            }
                        } else if(strcasecmp(ins,"PILE")==0) {

                            Meld **xmelds;
                            int nxmelds=0;
                            char *errmsg=NULL;
                            int card=-1;

                            if(npile>0) {

                                card=pile[0];

                                errmsg=CanPickToPile(&xmelds,&nxmelds,melds,nmelds,players[current].cards,players[current].ncards,current,card);

                                if(errmsg!=NULL) {
                                    sendf(e->stream,"PRIVMSG %s :" GAME_TITLE " %s",errmsg);
                                    errmsg=NULL;

                                } else if(nxmelds>0) {
                                    errmsg=GetCardAtTop(&card,pile,&npile);

                                    if(errmsg!=NULL) {
                                        sendf(e->stream,"PRIVMSG %s :" GAME_TITLE " %s",errmsg);
                                        errmsg=NULL;
                                    } else {
                                        errmsg=PutCardAtBottom(players[current].cards,&players[current].ncards,PLAYER_CARD_MAX,card);

                                        if(errmsg!=NULL) {
                                            sendf(e->stream,"PRIVMSG %s :" GAME_TITLE " %s",errmsg);
                                            free(errmsg);
                                            errmsg=NULL;
                                        } else {

                                            players[current].pickedFromPile=card;

                                            tmp=CardToString(card);

                                            sendf(e->stream,"PRIVMSG %s :" GAME_TITLE " %s, you picked '%s' form pile.",usr,usr,tmp);

                                            free(tmp);
                                            tmp=NULL;
                                        }
                                    }
                                }
                            } else {
                                sendf(e->stream,"PRIVMSG %s :" GAME_TITLE " %s, there are no cards in pile",chn,usr);
                            }
                        } else {
                            sendf(e->stream,"PRIVMSG %s :" GAME_TITLE " %s, you must specify where to pick from stock or pile.",chn,usr);
                        }

                    } else {
                        sendf(e->stream,"PRIVMSG %s :" GAME_TITLE " %s, you cannot picked.",chn,usr);
                    }
                } else {
                    sendf(e->stream,"PRIVMSG %s :" GAME_TITLE " %s, you are not joined.",chn,usr);
                }
            } else {
                sendf(e->stream,"PRIVMSG %s :" GAME_TITLE " %s, game is not started.",chn,usr);
            }

        } else if(strcasecmp(txt,PFX "INFO")==0) {

            if(gamestate==GAME_STATE_START) {

                int i,j,k;

                k=FindPlayer(players,nplayers,usr);

                if(k!=-1) {
                    char *msg=NULL;
                    if(npile==0) {
                        append(&msg,"STOCK (%d) PILE (%d) ",ndeck,npile);
                    } else {
                        append(&msg,"STOCK (%d) PILE (%d) %s ",ndeck,npile,CardToString(pile[0]));
                    }
                    i=dealer;
                    j=0;
                    do {
                        if(i!=dealer) append(&msg,", ",j++);
                        if(i==dealer)
                            append(&msg,"DEALER: ");
                        else
                            append(&msg,"PLAYER%d: ",++j);
                        append(&msg,"%s (%d)",players[i].nick,players[i].ncards);
                        i=(i+1)%nplayers;
                    } while(i!=dealer);
                    sendf(e->stream,"PRIVMSG %s :" GAME_TITLE " %s, %s.",chn,usr,msg);
                    free(msg);
                    msg=NULL;
                } else {
                    sendf(e->stream,"PRIVMSG %s :" GAME_TITLE " %s, you are not joined.",chn,usr);
                }

            } else {
                sendf(e->stream,"NOTICE %s :" GAME_TITLE " %s, game is not started.",chn,usr);
            }
        } else if(strcasecmp(txt,PFX "FIGHT")==0) {

            if(gamestate==GAME_STATE_START) {
                int k=FindPlayer(players,nplayers,usr);
                if(k==current) {

                } else if(k!=-1) {

                } else {
                    sendf(e->stream,"PRIVMSG %s :" GAME_TITLE " %s, you are not joined.",chn,usr);
                }
            } else {
                sendf(e->stream,"NOTICE %s :" GAME_TITLE " %s, game is not started.",chn,usr);
            }

        } else if(strcasecmp(txt,PFX "FOLD")==0) {

            if(gamestate==GAME_STATE_START) {
                int k=FindPlayer(players,nplayers,usr);
                if(k==current) {

                } else if(k!=-1) {

                } else {
                    sendf(e->stream,"PRIVMSG %s :" GAME_TITLE " %s, you are not joined.",chn,usr);
                }
            } else {
                sendf(e->stream,"NOTICE %s :" GAME_TITLE " %s, game is not started.",chn,usr);
            }

        } else if(strcasecmp(txt,PFX "TURN")==0) {
            if(gamestate==GAME_STATE_START) {
                int k=FindPlayer(players,nplayers,usr);
                if(k!=-1) {
                    sendf(e->stream,"PRIVMSG %s :" GAME_TITLE " %s, it is %s's turn.",chn,usr,players[current]);
                } else {
                    sendf(e->stream,"PRIVMSG %s :" GAME_TITLE " %s, you are not joined.",chn,usr);
                }
            } else {
                sendf(e->stream,"PRIVMSG %s :" GAME_TITLE " %s, game is not started.",chn,usr);
            }
        } else if(strcasecmp(txt,PFX "PART")==0) {

            if(gamestate==GAME_STATE_INIT) {
                int k=FindPlayer(players,nplayers,usr);
                if(k!=-1) {
                    char *errmsg=NULL;
                    errmsg=RemovePlayer(players,&nplayers,usr);
                    if(errmsg) {
                        sendf(e->stream,"PRIVMSG %s :" GAME_TITLE " %s, %s",chn,usr,errmsg);
                        free(errmsg);
                        errmsg=NULL;
                    }
                } else {
                }
            } else if(gamestate==GAME_STATE_START) {
                sendf(e->stream,"PRIVMSG %s :" GAME_TITLE " %s, cannot part game is running.",chn,usr);
            }
        } else if(strcasecmp(txt,PFX "ARRANGE")==0) {
            if(gamestate==GAME_STATE_START) {
                int k=FindPlayer(players,nplayers,usr);
                if(k!=-1) {
                    char*msg=NULL;
                    int cardsScore=0;

                    qsort(players[k].cards,players[k].ncards,sizeof(*players[k].cards),cmpsortcardsasc);

                    msg=CardsToString(players[k].cards,players[k].ncards);
                    cardsScore=CountCardsScore(players[k].cards,players[k].ncards);
                    sendf(e->stream,"NOTICE %s :" GAME_TITLE " (%d) %s.",usr,cardsScore,msg);

                    free(msg);
                    msg=NULL;


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


