#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdarg.h>
#include <stddef.h>
#include <string.h>
#include <ctype.h>
#include <time.h>



#include "dyad.h"

#define TRIE_IMPLEMENTATION
#include "trie.h"



#define BOX_SIZE 4
#define MIN_WORD_LEN 3

char letters[BOX_SIZE][BOX_SIZE];
char graph[BOX_SIZE][BOX_SIZE];
char choices[17];

char **words=NULL;
size_t nwords=0;



void AddWord(char *word) {
	words=realloc(words,sizeof(*words)*(nwords+1));
	words[nwords++]=strdup(word);
}



bool find(char *word) {
	size_t i;
	for(i=0;i<nwords;i++) {
		if(strcasecmp(word,words[i])==0) return true;
	}
	return false;
}



void dfs(int x,int y,int depth,const TRIE_NODE *trie) {

	int i,j;
	int numlet;

	if(x<0 || x>=BOX_SIZE || y<0 || y>=BOX_SIZE) return;

	if(graph[y][x]) return;

	numlet=letters[y][x]-'a';

	trie=trie->next[numlet];

	if(trie==NULL) return;

	choices[depth++]=letters[y][x];
	choices[depth]='\0';

	if(trie->mark && depth>=MIN_WORD_LEN && !find(choices)) {
		AddWord(choices);
	}

	graph[y][x]=true;

	for(j=-1;j<=1;j++) {
		for(i=-1;i<=1;i++) {
			if (i || j) {
				dfs(x+i,y+j,depth,trie);
			}
		}
	}

	graph[y][x]=false;



	if(letters[y][x]=='q') {

		numlet='u'-'a';

		trie=trie->next[numlet];

		if(trie==NULL) return;

		choices[depth++]='u';
		choices[depth]='\0';

		if(trie->mark && depth>=MIN_WORD_LEN && !find(choices)) {
			AddWord(choices);
		}

		graph[y][x]=true;

		for(j=-1;j<=1;j++) {
			for(i=-1;i<=1;i++) {
				if (i || j) {
					dfs(x+i,y+j,depth,trie);
				}
			}
		}

		graph[y][x]=false;

	}

}



int cmpalphaasc(const void *a,const void *b) {
	char *l=*(char**)a;
	char *r=*(char**)b;
	return strcmp(l,r);
}



int main(void) {

	FILE *fh;

	char *line=NULL;
	size_t llen=0;
	ssize_t rlen=0;

	size_t len=0;

	char *dice[BOX_SIZE*BOX_SIZE];

	size_t i,j;
	char *tmp;

	char *p;

	srand((unsigned int)time(NULL));

	trie_init();

	fh=fopen("enable.txt","rt");

	while((rlen=getline(&line,&llen,fh))!=-1) {
		char *p=strchr(line,'\n');
		if(p) *p='\0';

		len=strlen(line);

		if(len>=3 && len<=16) {
			trie_add_word(line);
		}

		free(line);
		line=NULL;
		llen=0;
	}

	fclose(fh);


	fh=fopen("dice.txt","rt");

	for(i=0;i<BOX_SIZE*BOX_SIZE;i++) {
		dice[i]=NULL;
		rlen=getline(&dice[i],&llen,fh);
		p=strchr(dice[i],'\n');
		if(p) *p='\0';
		llen=0;
	}

	for(i=15;i>0;i--) {
		j=(size_t)(rand()%(i+1));
		tmp=dice[i];
		dice[i]=dice[j];
		dice[j]=tmp;
	}

	for(i=0;i<BOX_SIZE*BOX_SIZE;i++) {
		letters[i/BOX_SIZE][i%BOX_SIZE]=dice[i][rand()%strlen(dice[i])];
	}

	for(j=0;j<4;j++) {
		for(i=0;i<4;i++) {
			printf("%-2s ",letters[j][i]=='q'?"Qu":(char[]){toupper(letters[j][i]),0});
		}
		printf("\n");
	}
	printf("\n");


	for(j=0;j<BOX_SIZE;j++) {
		for(i=0;i<BOX_SIZE;i++) {
			dfs(i,j,0,&trie_root);
		}
	}

	qsort(words,nwords,sizeof(*words),cmpalphaasc);

	for(i=0;i<nwords;i++) {
		if(i!=0) printf(", ");
		printf("%s",words[i]);
	}
	printf("\n");


	return 0;
}


