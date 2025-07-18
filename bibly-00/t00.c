#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void tokenize(char ***tk,size_t *nt,char *s,char *d) {
	char *tok=strtok(s,d);
	while(tok) {
		(*tk)=realloc(*tk,sizeof(*tk)*(*nt+1));
		(*tk)[(*nt)++]=strdup(tok);
		tok=strtok(NULL,d);
	}
}

int main() {

	char s[]="Genesis|1|1|In the beginning God created the heaven and the earth.";
	char d[]="|";

	char **tk=NULL;
	size_t nt=0;

	tokenize(&tk,&nt,s,d);

	for(int i=0;i<nt;i++) {
		printf("%s\n",tk[i]);
	}

	return 0;
}