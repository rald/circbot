#ifndef STRING_H
#define STRING_H

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <ctype.h>

char *trim(char *str);
char *strlwr(char *s);
char *strupr(char *s);
int tokenize(char *str,char ***toks,char *dels);
char *append(char **a,char *fmt,...);
char *skip(char *s, char c);

#endif
