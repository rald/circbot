#ifndef PARSER_H
#define PARSER_H


#define PLAYER_IMPLEMENTATION
#include "player.h"

#define TOKEN_IMPLEMENTATION
#include "token.h"

#define CARX_IMPLEMENTATION
#include "card.h"

#define MELD_IMPLEMENTATION
#include "meld.h"



char *ParseDrop(Meld ***melds,int *nmelds,Player players[],int nplayers,int playerId,Token **tokens,int ntokens);

char *ParseLay(int cards[],int *ncards,Meld ***melds,int *nmelds,Player players[],int nplayers,int playerId,int meldId,Token **tokens,int ntokens);


#ifdef PARSER_IMPLEMENTATION



char *ParseDrop(Meld ***melds,int *nmelds,Player players[],int nplayers,int playerId,Token **tokens,int ntokens) {

    bool flags[MELD_CARD_MAX];
    int nflags=0;

    int ocards[MELD_CARD_MAX];
    int nocards=0;

    int cards[MELD_CARD_MAX];
    int ncards=0;

    MeldType type;

    int card;

    char *msg=NULL;

    char *errmsg=NULL;

    int i,j;



    for(i=0; i<players[playerId].ncards; i++) {
        ocards[i]=players[playerId].cards[i];
        flags[i]=false;
    }
    nflags=nocards=players[playerId].ncards;

    i=0;
    for(;;) {

        if(		tokens[i]->type==TOKEN_TYPE_COMMA ||
                tokens[i]->type==TOKEN_TYPE_EOF ||
                i>=ntokens) {

            if(ncards>=3) {
                type=GetMeldType(cards,ncards);
                if(type!=MELD_TYPE_INVALID) {
                    AddMeld(melds,nmelds,CreateMeld(playerId,type,cards,ncards));
                    ncards=0;
                } else {
                    msg=CardsToString(cards,ncards);
                    append(&errmsg,"Error: invalid meld '%s'",msg);
                    free(msg);
                    msg=NULL;
                    return errmsg;
                }
            }

            if(tokens[i]->type==TOKEN_TYPE_EOF || i>=ntokens) {
                break;
            }
        }

        if(tokens[i]->type==TOKEN_TYPE_CARD) {
            card=CardStringToInt(tokens[i]->text);
            j=FindCard(ocards,nocards,card);
            if(j!=-1) {
                if(!flags[j]) {
                    flags[j]=true;
                    errmsg=PutCardAtBottom(cards,&ncards,MELD_CARD_MAX,card);
                    if(errmsg!=NULL) {
                        return errmsg;
                    }
                } else {
                    append(&errmsg,"Error: card '%s' is used.",CardToString(card));
                    return errmsg;
                }
            } else {
                append(&errmsg,"Error: card '%s' is not in hand.",CardToString(card));
                return errmsg;
            }
        }

        i++;
    }

    return NULL;
}


char *ParseLay(int cards[],int *ncards,Meld ***melds,int *nmelds,Player players[],int nplayers,int playerId,int meldId,Token **tokens,int ntokens) {

    bool flags[MELD_CARD_MAX];
    int nflags=0;

    int ocards[MELD_CARD_MAX];
    int nocards=0;

    MeldType type;

    int card;

    char *msg=NULL;

    char *errmsg=NULL;

    int i,j;

    for(i=0; i<players[playerId].ncards; i++) {
        ocards[i]=players[playerId].cards[i];
        flags[i]=false;
    }
    nflags=nocards=players[playerId].ncards;


    i=0;
    for(;;) {

        if(		tokens[i]->type==TOKEN_TYPE_EOF ||
                i>=ntokens) {

            for(j=0; j<(*melds)[meldId]->ncards; j++) {
                errmsg=PutCardAtBottom(cards,ncards,MELD_CARD_MAX,(*melds)[meldId]->cards[j]);
                if(errmsg) return errmsg;
            }

            type=GetMeldType(cards,*ncards);
            if(type!=MELD_TYPE_INVALID) {

                for(j=0; j<(*ncards); j++) {
                    (*melds)[meldId]->cards[j]=cards[j];
                }
                (*melds)[meldId]->ncards=(*ncards);

                break;

            } else {
                msg=CardsToString(cards,*ncards);
                append(&errmsg,"Error: invalid meld '%s'",msg);
                free(msg);
                msg=NULL;
                return errmsg;
            }

        } else if(tokens[i]->type==TOKEN_TYPE_CARD) {
            card=CardStringToInt(tokens[i]->text);
            j=FindCard(ocards,nocards,card);
            if(j!=-1) {
                if(!flags[j]) {
                    flags[j]=true;
                    errmsg=PutCardAtBottom(cards,ncards,MELD_CARD_MAX,card);
                    if(errmsg) return errmsg;
                } else {
                    append(&errmsg,"Error: card '%s' is used.",CardToString(card));
                    return errmsg;
                }
            } else {
                append(&errmsg,"Error: card '%s' is not in hand.",CardToString(card));
                return errmsg;
            }
        } else if(	tokens[i]->type==TOKEN_TYPE_COMMA ||
                    tokens[i]->type==TOKEN_TYPE_UNKNOWN) {
            append(&errmsg,"Error: invalid character(s) '%s'.",tokens[i]->text);
            return errmsg;
        }

        i++;
    }

    return NULL;

}

#endif /* PARSER_IMPLEMENTATION */



#endif /* PARSER_H */


