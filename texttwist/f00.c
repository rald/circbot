#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define LINE_MAX 256

int main(void) {

	FILE *fin;
	char line[LINE_MAX];

	while(fgets(line,LINE_MAX,stdin)) {
		char *p=strchr(line,'\n');
		if(p) *p='\0';
		int len=strlen(line);
		if(len>=6 && len<=8) {
			printf("%s\n",line);
		}		
	}

	return 0;
}
