#ifndef MYLIB_H
#define MYLIB_H



#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>



#define NEW0(x,y) calloc(y,sizeof(*(x)))
#define NEW1(x) malloc(sizeof(*(x)))
#define NEW2(x,y) malloc(sizeof(*(x))*(y))
#define FREE0(x) free((x))



bool strmatch(char str[], char pattern[], int n, int m);

char *mytrim(char *s);
char *mystrsub(char *s,int b,int l);
char *myskip(char *s,char *d);

void mytokadd(char ***t,int *l,char *s);
void mytokfree(char ***toks,int *ntoks);
int mystrtok(char *s,char *d,int n,char ***t,int *l);
char *mystrupr(char *s);
char *mystrlwr(char *s);

char *mystrrnl(char *s);
char *myappend(char **a,const char *fmt,...);



#ifdef MYLIB_IMPLEMENTATION



// Function that matches input str with
// given wildcard pattern
bool strmatch(char str[], char pattern[], int n, int m)
{
    // empty pattern can only match with
    // empty string
    if (m == 0)
        return (n == 0);
 
    // lookup table for storing results of
    // subproblems
    bool lookup[n + 1][m + 1];
 
    // initialize lookup table to false
    memset(lookup, false, sizeof(lookup));
 
    // empty pattern can match with empty string
    lookup[0][0] = true;
 
    // Only '*' can match with empty string
    for (int j = 1; j <= m; j++)
        if (pattern[j - 1] == '*')
            lookup[0][j] = lookup[0][j - 1];
 
    // fill the table in bottom-up fashion
    for (int i = 1; i <= n; i++) {
        for (int j = 1; j <= m; j++) {
            // Two cases if we see a '*'
            // a) We ignore ‘*’ character and move
            //    to next  character in the pattern,
            //     i.e., ‘*’ indicates an empty sequence.
            // b) '*' character matches with ith
            //     character in input
            if (pattern[j - 1] == '*')
                lookup[i][j]
                    = lookup[i][j - 1] || lookup[i - 1][j];
 
            // Current characters are considered as
            // matching in two cases
            // (a) current character of pattern is '?'
            // (b) characters actually match
            else if (pattern[j - 1] == '?'
                     || toupper(str[i - 1]) == toupper(pattern[j - 1]))
                lookup[i][j] = lookup[i - 1][j - 1];
 
            // If characters don't match
            else
                lookup[i][j] = false;
        }
    }
 
    return lookup[n][m];
}



char *mytrim(char *s) {
	char *p,*q;
	p=s; q=s;
	while(isspace(*q)) ++q;
	while(*q) *p++ = *q++;
	*p=0;
	while(p>s && isspace(*--p)) *p=0;
	return s;
}



char *mystrsub(char *s,int b,int l) {
	char *r,*pr,*ps;
	int i;

	r=malloc(sizeof(char)*(l+1));

	pr=r;
	ps=s+b;
	i=l;
	
	while(*ps && (l==0 || i--)) {
		*pr=*ps;
		pr++;
		ps++;
	}
	*pr=0;

	return r;
}




char *myskip(char *s,char *d) {
	char *pb,*ps;
	ps=0;
	pb=strstr(s,d);
	if(pb) {
		ps=pb+strlen(d);
		*pb=0;
	}
	return ps;
}



void mytokadd(char ***t,int *l,char *s) {
	(*t)=realloc((*t),sizeof(char*)*((*l)+1));
	(*t)[(*l)++]=strdup(s);
}



void mytokfree(char ***toks,int *ntoks) {
	int i;
	i=0;
	while(i<(*ntoks)) {
		free((*toks)[i]);
		(*toks)[i]=NULL;
		i++;
	}
	free(*toks);
	(*toks)=NULL;
	(*ntoks)=0;
}



int mystrtok(char *s,char *d,int n,char ***t,int *l) {
	char *ps,*tk;
	int quit;

	quit=0;

	ps=s;

	while(!quit) {
		if((n==-1 || *l<n) && ps) {
			tk=ps;
			ps=myskip(ps,d);
			mytokadd(t,l,tk);
		} else {
			if(ps) mytokadd(t,l,ps);
			quit=1;
		}
	}
		
	return *l;
}



char *mystrupr(char *s) {
	char *p=s;
	while(*p) {
		*p=toupper(*p);
		p++;
	}
	return s;
}



char *mystrlwr(char *s) {
	char *p=s;
	while(*p) {
		*p=tolower(*p);
		p++;
	}
	return s;
}



char *mystrrnl(char *s) {
	char *p=strchr(s,'\n');
	if(p) *p='\0';
	return s;	
}



char *myappend(char **a,const char *fmt,...) {

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





#endif /* MYLIB_IMPLEMENTATION */



#endif /* MYLIB_H */


