#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#include <readline/readline.h>
#include <readline/history.h>


#include "common.h"


#define BIBLY_IMPLEMENTATION
#include "bibly.h"

#define LEXER_IMPLEMENTATION
#include "lexer.h"

#define BIBLE_PATH "kjv.csv"



static char *rlgets(const char *prompt);

static void PrintHelp(char *topic);
static void Search(const char *text);
static void Parse(Token **tokens,size_t ntokens);



int main(void) {

    char *line=NULL;

    char **tokens=NULL;
    size_t ntokens=0;

    size_t i;



    using_history();

    Bibly_PrintVersion();

    PrintHelp("intro");

    while((line=rlgets("> "))) {

        printf("\n");

        tokenize(&tokens,&ntokens,line," ",1);

        if(ntokens>0) {

            char *cmd=tokens[0];

            if(!strcmp(cmd,".quit")) {
                break;
            } else if(!strcmp(cmd,".help")) {
                if(ntokens==2) {
                    PrintHelp(tokens[1]);
                } else {
                    PrintHelp(NULL);
                }
            } else if(!strcmp(cmd,".ver")) {
                Bibly_PrintVersion();
            } else if(!strcmp(cmd,".search")) {
                if(ntokens==2) Search(tokens[1]);
            } else {

                Token **ltokens=NULL;
                size_t nltokens=0;

                lex(&ltokens,&nltokens,line);

                Parse(ltokens,nltokens);

                DestroyTokens(&ltokens,&nltokens);

            }

        }

        for(i=0; i<ntokens; i++) {
            free(tokens[i]);
            tokens[i]=NULL;
        }
        ntokens=0;

        free(line);
        line=NULL;
    }

    printf("Bye!\n");

    return 0;

}



static char *rlgets(const char *prompt) {
    char *line=NULL;

    line=readline(prompt);
    if(line && *line) {
        add_history(line);
    }

    return line;
}



static void PrintHelp(char *topic) {

    if(topic==NULL) {

        printf(
            "Command starts with a dot '.' prefix."
            "Commands: help, ver, search, quit.\n"
            "Type .help [command] for help on a command.\n"
            "Type [book] [chapter]:[verse]-[verse] to display a verse.\n"
        );

    } else if(!strcmp(topic,"intro")) {

        printf("\nType \".help\" for help.\n\n");

    } else if(!strcmp(topic,"ver")) {
        printf(
            "Syntax: .ver\n"
            "Print Bibly version.\n"
        );
    } else if(!strcmp(topic,"search")) {
        printf(
            "Syntax: .search [text]\n"
            "Search for text.\n"
        );
    } else if(!strcmp(topic,"quit")) {
        printf(
            "Syntax: .quit\n"
            "Exits the program.\n"
        );
    }
}



static void Search(const char *text) {
    char **lines=NULL;
    size_t nlines=0;
    size_t i;

    Bibly_Search(BIBLE_PATH,&lines,&nlines,text);

    printf("\n");

    for(i=0; i<nlines; i++) {
        printf("%s\n",lines[i]);
    }

    printf("found %zu occurences\n\n",nlines);

    for(i=0; i<nlines; i++) {
        free(lines[i]);
        lines[i]=NULL;
    }
    nlines=0;

}


static void Parse(Token **tokens,size_t ntokens) {

    char cite[STRING_MAX];

    char **verses=NULL;
    size_t nverses=0;

    char book[STRING_MAX]="";
    unsigned int cnum=0;
    unsigned int svnum=0;
    unsigned int evnum=0;

    char **vtokens=NULL;
    size_t nvtokens=0;

    char *xbook=NULL;
    char *xnum=NULL;
    char *xvers=NULL;
    unsigned int xcnum=0;
    unsigned int xvnum=0;

    size_t i;

    if(
        ntokens==5 &&
        tokens[0]->type==TOKEN_TYPE_STRING &&
        tokens[1]->type==TOKEN_TYPE_INTEGER &&
        tokens[2]->type==TOKEN_TYPE_COLON &&
        tokens[3]->type==TOKEN_TYPE_INTEGER
    ) {
        sprintf(cite,"%s %s:%s",
                tokens[0]->text,
                tokens[1]->text,
                tokens[3]->text
               );

        strcpy(book,tokens[0]->text);
        cnum=atoi(tokens[1]->text);
        svnum=atoi(tokens[3]->text);
        evnum=svnum;

    } else if(
        ntokens==6 &&
        tokens[0]->type==TOKEN_TYPE_INTEGER &&
        tokens[1]->type==TOKEN_TYPE_STRING &&
        tokens[2]->type==TOKEN_TYPE_INTEGER &&
        tokens[3]->type==TOKEN_TYPE_COLON &&
        tokens[4]->type==TOKEN_TYPE_INTEGER
    ) {
        sprintf(cite,"%s %s %s:%s",
                tokens[0]->text,
                tokens[1]->text,
                tokens[2]->text,
                tokens[4]->text
               );

        sprintf(book,"%s %s",tokens[0]->text,tokens[1]->text);
        cnum=atoi(tokens[2]->text);
        svnum=atoi(tokens[4]->text);
        evnum=svnum;

    } else if(
        ntokens==7 &&
        tokens[0]->type==TOKEN_TYPE_STRING &&
        tokens[1]->type==TOKEN_TYPE_INTEGER &&
        tokens[2]->type==TOKEN_TYPE_COLON &&
        tokens[3]->type==TOKEN_TYPE_INTEGER &&
        tokens[4]->type==TOKEN_TYPE_DASH &&
        tokens[5]->type==TOKEN_TYPE_INTEGER
    ) {
        sprintf(cite,"%s %s:%s-%s",
                tokens[0]->text,
                tokens[1]->text,
                tokens[3]->text,
                tokens[5]->text
               );

        strcpy(book,tokens[0]->text);
        cnum=atoi(tokens[1]->text);
        svnum=atoi(tokens[3]->text);
        evnum=atoi(tokens[5]->text);

    } else if(
        ntokens==8 &&
        tokens[0]->type==TOKEN_TYPE_INTEGER &&
        tokens[1]->type==TOKEN_TYPE_STRING &&
        tokens[2]->type==TOKEN_TYPE_INTEGER &&
        tokens[3]->type==TOKEN_TYPE_COLON &&
        tokens[4]->type==TOKEN_TYPE_INTEGER &&
        tokens[5]->type==TOKEN_TYPE_DASH &&
        tokens[6]->type==TOKEN_TYPE_INTEGER
    ) {
        sprintf(cite,"%s %s %s:%s-%s",
                tokens[0]->text,
                tokens[1]->text,
                tokens[2]->text,
                tokens[4]->text,
                tokens[6]->text
               );

        sprintf(book,"%s %s",tokens[0]->text,tokens[1]->text);
        cnum=atoi(tokens[2]->text);
        svnum=atoi(tokens[4]->text);
        evnum=atoi(tokens[6]->text);
    }

    Bibly_GetVerses(BIBLE_PATH,&verses,&nverses,book,cnum,svnum,evnum);

    for(i=0; i<nverses; i++) {

        tokenize(&vtokens,&nvtokens,verses[i],"|",2);

        xbook=strdup(vtokens[0]);
        xnum=strdup(vtokens[1]);
        xvers=strdup(vtokens[2]);

        freetok(&vtokens,&nvtokens);

        tokenize(&vtokens,&nvtokens,xnum,":",1);

        xcnum=atoi(vtokens[0]);
        xvnum=atoi(vtokens[1]);

        freetok(&vtokens,&nvtokens);

        printf("%s %u:%u -> %s\n",xbook,xcnum,xvnum,xvers);

        xvnum=0;
        xcnum=0;
        free(xvers);
        xvers=NULL;
        free(xnum);
        xnum=NULL;
        free(xbook);
        xbook=NULL;

    }

    for(i=0; i<nverses; i++) {
        free(verses[i]);
        verses[i]=NULL;
    }
    free(verses);
    verses=NULL;
    nverses=0;

}


