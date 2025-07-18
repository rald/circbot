#ifndef UTIL_H
#define UTIL_H

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>

double drand();
char *trim(char *str);
char *strupr(const char *str);
char *strsub(const char *str,size_t start,size_t length);
void tokenize(char ***tk,size_t *nt,char *s,char *d);
void freetokens(char ***tk,size_t *nt);

#ifdef UTIL_IMPLEMENTATION

double drand() {
	return rand()/(RAND_MAX+1.0);
}

char *trim(char *a) {
    char *p = a, *q = a;
    while (isspace(*q))            ++q;
    while (*q)                     *p++ = *q++;
    *p = '\0';
    while (p > a && isspace(*--p)) *p = '\0';
	return a;
}

char *strupr(const char *str) {
  char *result=strdup(str);
  size_t i=0;
  while(result[i]) {
    result[i]=toupper(result[i]);
    i++;
  }
  return result;
}

void tokenize(char ***tk,size_t *nt,char *s,char *d) {
	char *lc=strdup(s);
	char *tok=strtok(lc,d);
	while(tok) {
		(*tk)=realloc(*tk,sizeof(*tk)*(*nt+1));
		(*tk)[(*nt)++]=strdup(tok);
		tok=strtok(NULL,d);
	}
	free(lc);
	lc=NULL;
}

void freetokens(char ***tk,size_t *nt) {
	size_t i;
	for(i=0;i<*nt;i++) {
		free((*tk)[i]);
		(*tk)[i]=NULL;
	}
	free(*tk);
	(*tk)=NULL;
	(*nt)=0;
}

#endif /* UTIL_IMPLEMENTATION */

#endif /* UTIL_H */
