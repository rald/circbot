#ifndef TONGITS_H
#define TONGITS_H



#define _GNU_SOURCE



#include "common.h"

#define UTIL_IMPLEMENTATION
#include "util.h"

#define PLAYER_IMPLEMENTATION
#include "player.h"

#define CARD_IMPLEMENTATION
#include "card.h"

#define TOKEN_IMPLEMENTATION
#include "token.h"

#define LEXER_IMPLEMENTATION
#include "lexer.h"

#define PARSER_IMPLEMENTATION
#include "parser.h"

#define MELD_IMPLEMENTATION
#include "meld.h"



typedef enum GameState GameState;

enum GameState {
    GAME_STATE_INIT=0,
    GAME_STATE_START,
    GAME_STATE_MAX
};



char *InitDeck(int deck[],int *ndeck);
void ShuffleDeck(int deck[],int ndeck);

char *Drop(Meld ***xmelds,int *nxmelds,Player players[],int nplayers,int playerId,char *line);

char *Lay(int xcards[],int *nxcards,Player players[],int nplayers,int playerId,int meldId, char *line);

char *Dump(int *card,Player players[],int nplayers,int playerId,char *line);

void Arrange(int cards[],int ncards);

void NextPlayer(void);

char *CanPickToPile(Meld ***xmelds,int *nxmelds,Meld **melds,int nmelds,int cards[],int ncards,int playerId,int card);

char *combination(Meld ***xmelds,int *nxmelds,Meld **melds,int nmelds,int playerId,int cards[], int ncards, int r, int index, int data[], int i,int card);

GameState gamestate=GAME_STATE_INIT;

Player players[PLAYER_MAX];
int nplayers=0;

int dealer=-1;
int current=-1;
int fighter=-1;

int deck[DECK_MAX];
int ndeck=0;

int pile[PILE_MAX];
int npile=0;

Meld **melds=NULL;
int nmelds=0;

int turns=0;

bool askFight=false;



#ifdef TONGITS_IMPLEMENTATION



char *InitDeck(int deck[],int *ndeck) {
    char *errmsg=NULL;
    int i;
    for(i=CARD_VALUE_MIN; i<=CARD_VALUE_MAX; i++) {
        errmsg=PutCardAtBottom(deck,ndeck,DECK_MAX,i);
        if(errmsg) {
            return errmsg;
        }
    }

    return NULL;
}



void ShuffleDeck(int deck[],int ndeck) {
    int i,j,k;
    for(i=ndeck-1; i>0; i--) {
        j=rand()%(i+1);
        k=deck[i];
        deck[i]=deck[j];
        deck[j]=k;
    }
}



char *Drop(Meld ***xmelds,int *nxmelds,Player players[],int nplayers,int playerId,char *line) {

    Token **tokens=NULL;
    int ntokens=0;

    int i,j,k;

    char *errmsg=NULL;

    int card;

    Lex(&tokens,&ntokens,line);

    errmsg=ParseDrop(xmelds,nxmelds,players,nplayers,playerId,tokens,ntokens);
    if(errmsg==NULL) {
        for(i=0; i<(*nxmelds) && errmsg==NULL; i++) {
            AddMeld(&melds,&nmelds,(*xmelds)[i]);
            for(j=0; j<(*xmelds)[i]->ncards && errmsg==NULL; j++) {
                k=FindCard(players[playerId].cards,players[playerId].ncards,(*xmelds)[i]->cards[j]);
                errmsg=GetCardAt(&card,players[playerId].cards,&players[playerId].ncards,k);
                if(errmsg!=NULL) {
                    return errmsg;
                }
            }
        }
    }

    return errmsg;
}



char *Lay(int xcards[],int *nxcards,Player players[],int nplayers,int playerId,int meldId, char *line) {

    Token **tokens=NULL;
    int ntokens=0;

    char *errmsg=NULL;

    if(meldId<=0 || meldId>=nmelds) {
        append(&errmsg,"Error: invalid meld number '%d'.",meldId);
        return errmsg;
    }

    if(line==NULL || *line=='\0') {
        append(&errmsg,"Error: invalid lay.");
        return errmsg;
    }

    Lex(&tokens,&ntokens,line);

    errmsg=ParseLay(xcards,nxcards,&melds,&nmelds,players,nplayers,playerId,meldId,tokens,ntokens);

    return errmsg;
}



char *Dump(int *card,Player players[],int nplayers,int playerId,char *line) {
    char *errmsg=NULL;

    int i;

    (*card)=CardStringToInt(line);

    if((*card)==-1) {
        append(&errmsg,"Error: invalid card.");
        return errmsg;
    }

    i=FindCard(players[playerId].cards,players[playerId].ncards,*card);

    if(i!=-1) {
        errmsg=GetCardAt(card,players[playerId].cards,&players[playerId].ncards,i);
        if(errmsg) return errmsg;

        errmsg=PutCardAtTop(pile,&npile,PILE_MAX,*card);
        if(errmsg) return errmsg;
    } else {
        char *tmp=CardToString(*card);
        append(&errmsg,"Error: card '%s' not in hand.",tmp);
        free(tmp);
        tmp=NULL;
        return errmsg;
    }

    return NULL;
}



void Arrange(int cards[],int ncards) {
    qsort(cards,ncards,sizeof(*cards),cmpsortcardsasc);
}



void NextPlayer(void) {
    turns++;
    current=(current+1)%nplayers;
}


int CountCardsScore(int cards[],int ncards) {
    int result=0;
    int i;
    for(i=0; i<ncards; i++) {
        result+=CardValues[GetCardRank(cards[i])];
    }
    return result;
}



char *CanPickToPile(Meld ***xmelds,int *nxmelds,Meld **melds,int nmelds,int cards[],int ncards,int playerId,int card) {

    int data[MELD_CARD_MAX];

    char *errmsg=NULL;

    int r;

    for(r=1; r<=ncards; r++) {
        errmsg=combination(xmelds,nxmelds,melds,nmelds,playerId,cards,ncards,r,0,data,0,card);
        if(errmsg!=NULL) {
            return errmsg;
        }
    }

    return NULL;
}



char *combination(Meld ***xmelds,int *nxmelds,Meld **melds,int nmelds,int playerId,int cards[], int ncards, int r, int index, int data[], int i,int card) {

    char *errmsg=NULL;

    if (index == r) {
        Meld *meld1=malloc(sizeof(*meld1));
        Meld *meld2=malloc(sizeof(*meld2));
        int j,k;

        meld1->playerId=playerId;
        for (j=0; j<r; j++) {
            errmsg=PutCardAtBottom(meld1->cards,&meld1->ncards,PLAYER_CARD_MAX,data[j]);
            if(errmsg!=NULL) {
                return errmsg;
            }
        }

        errmsg=PutCardAtBottom(meld1->cards,&meld1->ncards,PLAYER_CARD_MAX,card);
        if(errmsg!=NULL) {
            return errmsg;
        }

        meld1->type=GetMeldType(meld1->cards,meld1->ncards);

        if(meld1->type!=MELD_TYPE_INVALID) {
            AddMeld(xmelds,nxmelds,meld1);
        }

        for(j=0; j<nmelds; j++) {

            meld2->playerId=melds[j]->playerId;
            for(k=0; k<melds[j]->ncards; k++) {
                errmsg=PutCardAtBottom(meld2->cards,&meld2->ncards,MELD_CARD_MAX,melds[j]->cards[k]);
                if(errmsg!=NULL) {
                    return errmsg;
                }
            }

            for(k=0; k<meld1->ncards; k++) {
                errmsg=PutCardAtBottom(meld2->cards,&meld2->ncards,MELD_CARD_MAX,meld1->cards[k]);
                if(errmsg!=NULL) {
                    return errmsg;
                }
            }

            meld2->type=GetMeldType(meld2->cards,meld2->ncards);

            if(meld2->type!=MELD_TYPE_INVALID) {
                AddMeld(xmelds,nxmelds,meld2);
            } else {
                free(meld2);
                meld2=NULL;
            }

        }

        if(meld1->type==MELD_TYPE_INVALID) {
            free(meld1);
            meld1=NULL;
        }

        return NULL;
    }

    if (i >= ncards)
        return NULL;

    data[index] = cards[i];
    errmsg=combination(xmelds, nxmelds, melds, nmelds, playerId, cards, ncards, r, index+1, data, i+1, card);

    if(errmsg!=NULL) return errmsg;

    errmsg=combination(xmelds, nxmelds, melds, nmelds, playerId, cards, ncards, r, index, data, i+1, card);

    if(errmsg!=NULL) return errmsg;

    return NULL;

}



#endif /* TONGITS_IMPLEMENTATION */



#endif /* TONGITS_H */


