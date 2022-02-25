#ifndef CARD_H
#define CARD_H



typedef enum CardSuit CardSuit;

enum CardSuit {
    CARD_SUIT_CLUB=0,
    CARD_SUIT_SPADE,
    CARD_SUIT_HEART,
    CARD_SUIT_DIAMOND,
    CARD_SUIT_MAX
};



typedef enum CardRank CardRank;

enum CardRank {
    CARD_RANK_ACE=0,
    CARD_RANK_TWO,
    CARD_RANK_THREE,
    CARD_RANK_FOUR,
    CARD_RANK_FIVE,
    CARD_RANK_SIX,
    CARD_RANK_SEVEN,
    CARD_RANK_EIGHT,
    CARD_RANK_NINE,
    CARD_RANK_TEN,
    CARD_RANK_JACK,
    CARD_RANK_QUEEN,
    CARD_RANK_KING,
    CARD_RANK_MAX
};



const int CardValues[]= {1,2,3,4,5,6,7,8,9,10,10,10,10};



CardSuit GetCardSuit(int card);
CardRank GetCardRank(int card);
char *CardToString(int card);
char *CardsToString(int cards[],int ncards);
int CardStringToInt(const char *str);
char *PutCardAt(int cards[],int *ncards,int card_max,int index,int card);
char *GetCardAt(int *card,int cards[],int *ncards,int index);
char *PutCardAtTop(int cards[],int *ncards,int card_max,int card);
char *PutCardAtBottom(int cards[],int *ncards,int card_max,int card);
char *GetCardAtTop(int *card,int cards[],int *ncards);
char *GetCardAtBottom(int *card,int cards[],int *ncards);
int FindCard(int cards[],int ncards,int card);

int cmpsortcardsasc(const void *lhs,const void *rhs);



#ifdef CARD_IMPLEMENTATION



CardSuit GetCardSuit(int card) {
    CardSuit suit;
    switch(card/CARD_RANK_MAX) {
    case 0:
        suit=CARD_SUIT_CLUB;
        break;
    case 1:
        suit=CARD_SUIT_SPADE;
        break;
    case 2:
        suit=CARD_SUIT_HEART;
        break;
    case 3:
        suit=CARD_SUIT_DIAMOND;
        break;
    default:
        break;
    }
    return suit;
}



CardRank GetCardRank(int card) {
    CardRank rank;
    switch(card%CARD_RANK_MAX) {
    case 0:
        rank=CARD_RANK_ACE;
        break;
    case 1:
        rank=CARD_RANK_TWO;
        break;
    case 2:
        rank=CARD_RANK_THREE;
        break;
    case 3:
        rank=CARD_RANK_FOUR;
        break;
    case 4:
        rank=CARD_RANK_FIVE;
        break;
    case 5:
        rank=CARD_RANK_SIX;
        break;
    case 6:
        rank=CARD_RANK_SEVEN;
        break;
    case 7:
        rank=CARD_RANK_EIGHT;
        break;
    case 8:
        rank=CARD_RANK_NINE;
        break;
    case 9:
        rank=CARD_RANK_TEN;
        break;
    case 10:
        rank=CARD_RANK_JACK;
        break;
    case 11:
        rank=CARD_RANK_QUEEN;
        break;
    case 12:
        rank=CARD_RANK_KING;
        break;
    default:
        break;
    }
    return rank;
}



char *CardToString(int card) {
    char *result=NULL;
    char *rank=NULL;
    char *suit=NULL;

    switch(GetCardRank(card)) {
    case CARD_RANK_ACE:
        rank="A";
        break;
    case CARD_RANK_TWO:
        rank="2";
        break;
    case CARD_RANK_THREE:
        rank="3";
        break;
    case CARD_RANK_FOUR:
        rank="4";
        break;
    case CARD_RANK_FIVE:
        rank="5";
        break;
    case CARD_RANK_SIX:
        rank="6";
        break;
    case CARD_RANK_SEVEN:
        rank="7";
        break;
    case CARD_RANK_EIGHT:
        rank="8";
        break;
    case CARD_RANK_NINE:
        rank="9";
        break;
    case CARD_RANK_TEN:
        rank="10";
        break;
    case CARD_RANK_JACK:
        rank="J";
        break;
    case CARD_RANK_QUEEN:
        rank="Q";
        break;
    case CARD_RANK_KING:
        rank="K";
        break;
    default:
        break;
    }

    switch(GetCardSuit(card)) {
    case CARD_SUIT_CLUB:
        suit="C";
        break;
    case CARD_SUIT_SPADE:
        suit="S";
        break;
    case CARD_SUIT_HEART:
        suit="H";
        break;
    case CARD_SUIT_DIAMOND:
        suit="D";
        break;
    default:
        break;
    }

    append(&result,"%s%s",rank,suit);

    return result;

}



char *CardsToString(int cards[],int ncards) {
    char *result=NULL;
    char *card=NULL;
    int i;
    for(i=0; i<ncards; i++) {
        if(i!=0) append(&result," ");
        card=CardToString(cards[i]);
        append(&result,"%s",card);
        free(card);
        card=NULL;
    }
    return result;
}



int CardStringToInt(const char *str) {
    int result=-1;
    int p=-1;
    char *tmp=NULL;

    CardRank rank;
    CardSuit suit;

    tmp=strdup(str);
    trim(tmp);
    strupr(tmp);

    if(strlen(tmp)==3 && tmp[0]=='1' && tmp[1]=='0') {
        p=2;
        rank=CARD_RANK_TEN;
    } else if(strlen(tmp)==2) {
        p=1;
        switch(tmp[0]) {
        case 'A':
            rank=CARD_RANK_ACE;
            break;
        case '2':
            rank=CARD_RANK_TWO;
            break;
        case '3':
            rank=CARD_RANK_THREE;
            break;
        case '4':
            rank=CARD_RANK_FOUR;
            break;
        case '5':
            rank=CARD_RANK_FIVE;
            break;
        case '6':
            rank=CARD_RANK_SIX;
            break;
        case '7':
            rank=CARD_RANK_SEVEN;
            break;
        case '8':
            rank=CARD_RANK_EIGHT;
            break;
        case '9':
            rank=CARD_RANK_NINE;
            break;
        case 'J':
            rank=CARD_RANK_JACK;
            break;
        case 'Q':
            rank=CARD_RANK_QUEEN;
            break;
        case 'K':
            rank=CARD_RANK_KING;
            break;
        default:
            p=-1;
            break;
        }
    }

    if(p!=-1) {
        switch(tmp[p]) {
        case 'C':
            suit=CARD_SUIT_CLUB;
            break;
        case 'S':
            suit=CARD_SUIT_SPADE;
            break;
        case 'H':
            suit=CARD_SUIT_HEART;
            break;
        case 'D':
            suit=CARD_SUIT_DIAMOND;
            break;
        default:
            p=-1;
            break;
        }
    }

    if(p!=-1) result=suit*CARD_RANK_MAX+rank;

    free(tmp);
    tmp=NULL;

    return result;
}



char *PutCardAt(int cards[],int *ncards,int card_max,int index,int card) {
    char *errmsg=NULL;
    int i;

    if(card<CARD_VALUE_MIN || card>CARD_VALUE_MAX) {
        append(&errmsg,"Error: cannot put card value out of range.");
        return errmsg;
    }

    if((*ncards)>=card_max) {
        append(&errmsg,"Error: canot put too many cards.");
        return errmsg;
    }

    if(index<0 || index>(*ncards)) {
        append(&errmsg,"Error: cannot put invalid index.");
        return errmsg;
    }

    for(i=(*ncards); i>index-1; i--) {
        cards[i]=cards[i-1];
    }
    cards[index]=card;
    (*ncards)++;

    return NULL;
}



char *GetCardAt(int *card,int cards[],int *ncards,int index) {
    char *errmsg=NULL;
    int i;

    if((*ncards)<=0) {
        append(&errmsg,"Error: cannot get there are no cards.");
        return errmsg;
    }

    if(index<0 || index>=(*ncards)) {
        append(&errmsg,"Error: cannot get invalid index.");
        return errmsg;
    }

    (*card)=cards[index];
    for(i=index; i<(*ncards)-1; i++) {
        cards[i]=cards[i+1];
    }
    (*ncards)--;

    return NULL;
}



char *PutCardAtTop(int cards[],int *ncards,int card_max,int card) {
    return PutCardAt(cards,ncards,card_max,0,card);
}



char *PutCardAtBottom(int cards[],int *ncards,int card_max,int card) {
    return PutCardAt(cards,ncards,card_max,*ncards,card);
}



char *GetCardAtTop(int *card,int cards[],int *ncards) {
    return GetCardAt(card,cards,ncards,0);
}



char *GetCardAtBottom(int *card,int cards[],int *ncards) {
    return GetCardAt(card,cards,ncards,(*ncards)-1);
}



int FindCard(int cards[],int ncards,int card) {
    int i;
    for(i=0; i<ncards; i++) {
        if(cards[i]==card) {
            return i;
        }
    }
    return -1;
}



int cmpsortcardsasc(const void *lhs,const void *rhs) {
    int a=*(int*)lhs;
    int b=*(int*)rhs;
    if(a<b) return -1;
    else if(a>b) return 1;
    return 0;
}



#endif /* CARD_IMPLEMENTATION */



#endif /* CARD_H */


