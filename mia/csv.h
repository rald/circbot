#ifndef CSV_H
#define CSV_H



#include <stdio.h>
#include <stdlib.h>
#include <string.h>



typedef enum ParserState {
  PARSERSTATE_DEFAULT=0,
  PARSERSTATE_STRING,
  PARSERSTATE_MAX  
} ParserState;



#ifdef CSV_IMPLEMENTATION



void CSV_AddLine(char ***lines,size_t *nlines,char *line) {
  (*lines)=realloc(*lines,sizeof(**lines)*(*nlines+1));  
  (*lines)[(*nlines)++]=strdup(line);
}
 


void CSV_Parse(char ***lines,size_t *nlines,char *line) {
  ParserState parserState=PARSERSTATE_DEFAULT;
  char *p=line;
  char text[STRING_MAX];
  text[0]='\0';
  while(*p) {
    switch(parserState) {
      case PARSERSTATE_DEFAULT: 
        if(*p=='\"') {
          parserState=PARSERSTATE_STRING;
        } 
      break;
      case PARSERSTATE_STRING: 
        while(*p!='\"') {
          if(*p=='\\') {
            p++;
            switch(*p) {
              case 'a': strcat(text,(char[2]){'\a','\0'}); break;
              case 'b': strcat(text,(char[2]){'\b','\0'}); break;
              case 'f': strcat(text,(char[2]){'\f','\0'}); break;
              case 'n': strcat(text,(char[2]){'\n','\0'}); break;
              case 'r': strcat(text,(char[2]){'\r','\0'}); break;
              case 't': strcat(text,(char[2]){'\t','\0'}); break;
              case 'v': strcat(text,(char[2]){'\v','\0'}); break;
              case '\'': strcat(text,(char[2]){'\'','\0'}); break;
              case '\"': strcat(text,(char[2]){'\"','\0'}); break;
              case '\?': strcat(text,(char[2]){'\?','\0'}); break;
              case '\\': strcat(text,(char[2]){'\\','\0'}); break;
              default: strcat(text,(char[2]){*p,'\0'});
            }
          } else strcat(text,(char[2]){*p,'\0'});
          p++;
        }
        CSV_AddLine(lines,nlines,text);
        text[0]='\0';
        parserState=PARSERSTATE_DEFAULT;
      break;
      default: break;
    }
    p++;
  }
}



void CSV_Print(char **lines,size_t nlines) {
  for(size_t i=0;i<nlines;i++) {
    printf("%s\n",lines[i]);
  }
  printf("\n");
}



void CSV_Free(char ***lines,size_t *nlines) {
  for(size_t i=0;i<(*nlines);i++) {
    free((*lines)[i]);
    (*lines)[i]=NULL;
  }
  free(*lines);
  (*lines)=NULL;
  (*nlines)=0;
}



#endif



#endif


