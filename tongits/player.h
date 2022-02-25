#ifndef PLAYER_H
#define PLAYER_H

#include <stdio.h>
#include <stdbool.h>


#include "common.h"



typedef struct Player Player;

struct Player {
    char nick[NICK_MAX];
    int cards[DEALER_CARD_MAX];
    int ncards;
    int pickedFromPile;
    int score;
    int cardsScore;
    bool isDealer;
    bool isOpen;
    bool isPick;
    bool canFight;
    bool isFighting;
    bool isBurned;
};



char *AddPlayer(Player players[],int *nplayers,int player_max,char nick[]);
int FindPlayer(Player players[],int nplayers,char *nick);
char *RemovePlayer(Player players[],int *nplayers,char *nick);



#ifdef PLAYER_IMPLEMENTATION



char *AddPlayer(Player players[],int *nplayers,int player_max,char nick[]) {

    char *errmsg=NULL;

    if((*nplayers)>=player_max) {
        append(&errmsg,"Error: cannot add players full.");
        return errmsg;
    }

    strcpy(players[*nplayers].nick,nick);
    players[*nplayers].ncards=0;
    players[*nplayers].score=0;
    players[*nplayers].pickedFromPile=-1;
    players[*nplayers].isDealer=false;
    players[*nplayers].isOpen=false;
    players[*nplayers].isPick=false;
    players[*nplayers].canFight=false;
    players[*nplayers].isFighting=false;
    players[*nplayers].isBurned=false;

    (*nplayers)++;

    return NULL;
}



int FindPlayer(Player players[],int nplayers,char *nick) {
    int i;
    for(i=0; i<nplayers; i++) {
        if(strcasecmp(players[i].nick,nick)==0) {
            return i;
        }
    }
    return -1;
}



char *RemovePlayer(Player players[],int *nplayers,char *nick) {
    char *errmsg=NULL;
    int i,j;

    if((*nplayers)<=0) {
        append(&errmsg,"Error: there are no players.");
        return errmsg;
    }

    j=FindPlayer(players,*nplayers,nick);
    if(j!=-1) {
        for(i=j; i<(*nplayers)-1; i++) {
            players[i]=players[i+1];
        }
        (*nplayers)--;
    } else {
        append(&errmsg,"Error: player '%s' not found.",nick);
        return errmsg;
    }

    return NULL;
}


#endif /* PLAYER_IMPLEMENTATION */



#endif /* PLAYER_H */
