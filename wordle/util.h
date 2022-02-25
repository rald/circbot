#ifndef UTIL_H
#define UTIL_H

#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <ctype.h>


double drand(void);

char *strupr(char *s);
char *strlwr(char *s);

char *trim(char *a);
char *skip(char *s, char c);
char *append(char **a,const char *fmt,...);



#ifdef UTIL_IMPLEMENTATION



double drand(void) {
	return rand()/(RAND_MAX+1.0);
}




char *strupr(char *s) {
    char *p=s;
    while(*p) {
        *p=toupper(*p);
        p++;
    }
    return s;
}



char *strlwr(char *s) {
    char *p=s;
    while(*p) {
        *p=tolower(*p);
        p++;
    }
    return s;
}



char *trim(char *a) {
    char *p = a, *q = a;
    while (isspace(*q)) ++q;
    while (*q) *p++ = *q++;
    *p = '\0';
    while (p > a && isspace(*--p)) *p = '\0';
    return a;
}



char *skip(char *s, char c) {
    while (*s != c && *s != '\0')
        s++;
    if (*s != '\0')
        *s++ = '\0';
    return s;
}



char *append(char **a,const char *fmt,...) {

    char *b=NULL;
    ssize_t lenb=0;

    va_list args;

    va_start(args,fmt);
    lenb=vsnprintf(NULL,0,fmt,args);
    va_end(args);

    va_start(args,fmt);
    b=calloc(lenb+1,sizeof(*b));
    (void)vsnprintf(b,lenb+1,fmt,args);
    va_end(args);

    if(*a) {
        (*a)=realloc(*a,sizeof(**a)*(strlen(*a)+lenb+1));
    } else {
        (*a)=calloc(lenb+1,sizeof(**a));
    }

    strcat(*a,b);

    free(b);

    return *a;
}



#endif /* UTIL_IMPLEMENTATION */



#endif /* UTIL_H */


