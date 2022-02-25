#ifndef MELD_H
#define MELD_H



#include "common.h"

#define UTIL_IMPLEMENTATION
#include "util.h"

#define CARD_IMPLEMENTATION
#include "card.h"



typedef enum MeldType MeldType;

enum MeldType {
    MELD_TYPE_RUN=0,
    MELD_TYPE_SET,
    MELD_TYPE_SECRET_SET,
    MELD_TYPE_INVALID,
    MELD_TYPE_MAX
};



typedef struct Meld Meld;

struct Meld {
    int playerId;
    MeldType type;
    int cards[MELD_CARD_MAX];
    int ncards;
};



const char *MeldTypeNames[]= {
    "run",
    "set",
    "secret_set",
    "invalid"
};


Meld *CreateMeld(int playerId,MeldType type,int cards[],int ncards);
void AddMeld(Meld ***melds,int *nmelds,Meld *meld);
MeldType GetMeldType(int ocards[],int nocards);
char *MeldToString(Meld *meld);


#ifdef MELD_IMPLEMENTATION



Meld *CreateMeld(int playerId,MeldType type,int cards[],int ncards) {
    Meld *meld=malloc(sizeof(*meld));
    int i;
    if(meld!=NULL) {
        meld->playerId=playerId;
        meld->type=type;
        for(i=0; i<ncards; i++) {
            meld->cards[i]=cards[i];
        }
        meld->ncards=ncards;
    }
    return meld;
}


void AddMeld(Meld ***melds,int *nmelds,Meld *meld) {
    (*melds)=realloc(*melds,sizeof(**melds)*((*nmelds)+1));
    (*melds)[(*nmelds)++]=meld;
}



MeldType GetMeldType(int ocards[],int nocards) {

    CardRank rank;
    CardSuit suit;
    bool valid;

    int cards[MELD_CARD_MAX];
    int ncards=0;

    int i;



    for(i=0; i<nocards; i++) {
        cards[i]=ocards[i];
    }
    ncards=nocards;


    qsort(cards,ncards,sizeof(*cards),cmpsortcardsasc);



    /* check for valid set */
    if(ncards>=3 && ncards<=4) {
        valid=true;
        rank=GetCardRank(cards[0]);
        for(i=1; i<ncards; i++) {
            if(rank!=GetCardRank(cards[i])) {
                valid=false;
                break;
            }
        }
        if(valid) {
            if(ncards==4) {
                return MELD_TYPE_SECRET_SET;
            } else {
                return MELD_TYPE_SET;
            }
        }
    }


    /* check for valid run */
    if(ncards>=3 && ncards<=13) {
        valid=true;
        suit=GetCardSuit(cards[0]);
        for(i=1; i<ncards; i++) {
            if(suit!=GetCardSuit(cards[i])) {
                valid=false;
                break;
            }
        }
        if(valid) {
            for(i=0; i<ncards-1; i++) {
                if(cards[i+1]-cards[i]!=1) {
                    valid=false;
                    break;
                }
            }
        }
        if(valid) {
            return MELD_TYPE_RUN;
        }
    }

    return MELD_TYPE_INVALID;

}


char *MeldToString(Meld *meld) {
    char *result=NULL;
    append(&result,"{ \"playerId\": %d, \"type\": \"%s\", \"cards\": %s, \"ncards\": %d }",meld->playerId,MeldTypeNames[meld->type],CardsToString(meld->cards,meld->ncards),meld->ncards);
    return result;
}



#endif /* MELD_IMPLEMENTATION  */



#endif /* MELD_H */


