#ifndef LEXER_H
#define LEXER_H



#include <string.h>
#include <ctype.h>



#define UTIL_IMPLEMENTATION
#include "util.h"

#define TOKEN_IMPLEMENTATION
#include "token.h"



typedef enum LexerState LexerState;

enum LexerState {
    LEXER_STATE_DEFAULT=0,
    LEXER_STATE_CARD,
    LEXER_STATE_MAX
};



int Lex(Token ***tokens,int *ntokens,const char *line);



#ifdef LEXER_IMPLEMENTATION



int Lex(Token ***tokens,int *ntokens,const char *line) {
    LexerState state=LEXER_STATE_DEFAULT;
    int i=0;
    char ch;
    int part=0;
    char *text=NULL;

    for(;;) {
        ch=line[i];

        switch(state) {
        case LEXER_STATE_DEFAULT:
            if(strchr("A234567891JQK",toupper(ch))!=NULL) {
                i--;
                state=LEXER_STATE_CARD;
            } else if(ch==' ') {
                AddToken(tokens,ntokens,TOKEN_TYPE_SPACE," ");
            } else if(ch==',') {
                AddToken(tokens,ntokens,TOKEN_TYPE_COMMA,",");
            } else {
                AddToken(tokens,ntokens,TOKEN_TYPE_UNKNOWN,(char[]) {
                    ch,'\0'
                });
            }
            break;
        case LEXER_STATE_CARD:
            if(strchr("A234567891JQK",toupper(ch))!=NULL && part==0) {
                append(&text,"%c",toupper(ch));
                if(ch=='1') part=1;
                else part=2;
            } else if(ch=='0' && part==1) {
                append(&text,"%c",toupper(ch));
                part=2;
            } else if(strchr("CSHD",toupper(ch))!=NULL && part==2) {
                append(&text,"%c",toupper(ch));
                AddToken(tokens,ntokens,TOKEN_TYPE_CARD,text);
                free(text);
                text=NULL;
                part=0;
                i--;
                state=LEXER_STATE_DEFAULT;
            } else {
                if(text!=NULL) {
                    free(text);
                    text=NULL;
                }
                return i;
            }
            break;
        default:
            break;
        }
        if(ch=='\0') break;
        i++;
    }

    AddToken(tokens,ntokens,TOKEN_TYPE_EOF,NULL);

    return -1;
}



#endif /* LEXER_IMPLEMENTATION */



#endif /* LEXER_H */