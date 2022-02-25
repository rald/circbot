#ifndef TOKEN_H
#define TOKEN_H


#include <string.h>


#define UTIL_IMPLEMENTATION
#include "util.h"



typedef enum TokenType TokenType;

enum TokenType {
    TOKEN_TYPE_NONE=0,
    TOKEN_TYPE_CARD,
    TOKEN_TYPE_SPACE,
    TOKEN_TYPE_COMMA,
    TOKEN_TYPE_EOF,
    TOKEN_TYPE_UNKNOWN,
    TOKEN_TYPE_MAX
};



typedef struct Token Token;

struct Token {
    TokenType type;
    const char *text;
};



const char *TokenTypeNames[]= {
    "none",
    "card",
    "space",
    "comma",
    "eof",
    "unknown"
};



Token *CreateToken(TokenType type,const char *text);
void AddToken(Token ***tokens,int *ntokens,TokenType type,const char *text);
char *TokenToString(Token *token);


#ifdef TOKEN_IMPLEMENTATION



Token *CreateToken(TokenType type,const char *text) {
    Token *token=malloc(sizeof(*token));
    if(token!=NULL) {
        token->type=type;
        token->text=(text==NULL?NULL:strdup(text));
    }
    return token;
}



void AddToken(Token ***tokens,int *ntokens,TokenType type,const char *text) {
    (*tokens)=realloc(*tokens,sizeof(**tokens)*((*ntokens)+1));
    (*tokens)[(*ntokens)++]=CreateToken(type,text);
}



char *TokenToString(Token *token) {
    char *result=NULL;
    append(&result,"{ \"type\" = \"%s\", \"text\" = \"%s\" }",TokenTypeNames[token->type],token->text);
    return result;
}



#endif /* TOKEN_IMPLEMENTATION */



#endif /* TOKEN_H */