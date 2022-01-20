#include "string.h"

char *trim(char *a) {
    char *p = a, *q = a;
    while (isspace(*q))            ++q;
    while (*q)                     *p++ = *q++;
    *p = '\0';
    while (p > a && isspace(*--p)) *p = '\0';
	return a;
}

char *strlwr(char *s) {
	size_t i;
	for(i=0;i<strlen(s);i++) {
		s[i]=tolower(s[i]);
	}
	return s;
}

char *strupr(char *s) {
	size_t i;
	for(i=0;i<strlen(s);i++) {
		s[i]=toupper(s[i]);
	}
	return s;
}

int tokenize(char *str,char ***toks,char *dels) {
	int n=0;
	char *tok=NULL;
	tok=strtok(str,dels);
	while(tok) {
		(*toks)=realloc((*toks),sizeof(**toks)*(n+1));
		(*toks)[n++]=strdup(tok);
		tok=strtok(NULL,dels);
	}
	return n;
}

char *append(char **a,char *fmt,...) {
	char *b=NULL;
	ssize_t lenb=0;

	va_list args;
	va_start(args,fmt);
	lenb=vsnprintf(NULL,0,fmt,args);
	b=malloc(sizeof(*b)*(lenb+1));
	vsprintf(b,fmt,args);
	va_end(args);

	if(*a) {
		(*a)=realloc(*a,sizeof(**a)*(strlen(*a)+lenb+1));
	} else {
		(*a)=realloc(*a,sizeof(**a)*(lenb+1));
		(*a)[0]='\0';
	}
	strcat(*a,b);
	return *a;
}

char *skip(char *s, char c)
{
	while (*s != c && *s != '\0')
		s++;
	if (*s != '\0')
		*s++ = '\0';
	return s;
}
