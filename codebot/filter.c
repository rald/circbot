#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define LINE_MAX 256

int main() {

	char line[LINE_MAX];
	char *p=NULL;

	while(fgets(line,LINE_MAX,stdin)) {

		p=strchr(line,'\n');
		if(p) *p='\0';

		if(strlen(line)==5) printf("%s\n",line);

	}

	return 0;
}