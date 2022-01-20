#ifndef BOGGLE_H
#define BOGGLE_H

#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdarg.h>
#include <stddef.h>
#include <string.h>
#include <ctype.h>
#include <time.h>

#define TRIE_IMPLEMENTATION
#include "trie.h"

#define BOX_SIZE 4
#define MIN_WORD_LEN 3

char letters[BOX_SIZE][BOX_SIZE];
char graph[BOX_SIZE][BOX_SIZE];
char *dice[BOX_SIZE*BOX_SIZE];
char choices[17];

char **words=NULL;
size_t nwords=0;


void AddWord(char *word);
ssize_t FindWord(char *word);

void dfs(int x,int y,int depth,const TRIE_NODE *trie);

void LoadWords();
void LoadDice();
void ShuffleDice();
void NewBoard();
void GetWords();



#ifdef BOGGLE_IMPLEMENTATION



static int cmpalphaasc(const void *a,const void *b);



void AddWord(char *word) {
	words=realloc(words,sizeof(*words)*(nwords+1));
	words[nwords++]=strdup(word);
}



ssize_t FindWord(char *word) {
	size_t i;
	ssize_t j=-1;
	for(i=0;i<nwords;i++) {
		if(strcasecmp(word,words[i])==0) {
			j=i;
			break;
		}
	}
	return j;
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

	if(trie->mark && depth>=MIN_WORD_LEN && FindWord(choices)==-1) {
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

		if(trie->mark && depth>=MIN_WORD_LEN && FindWord(choices)==-1) {
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



static int cmpalphaasc(const void *a,const void *b) {
	char *l=*(char**)a;
	char *r=*(char**)b;
	return strcmp(l,r);
}



void LoadWords() {
	FILE *fh;

	char *line=NULL;
	size_t llen=0;
	ssize_t rlen=0;

	size_t len=0;

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
}



void LoadDice() {
	FILE *fh;

	char *line=NULL;
	size_t llen=0;
	ssize_t rlen=0;

	size_t i;

	char *p;

	fh=fopen("dice.txt","rt");

	for(i=0;i<BOX_SIZE*BOX_SIZE;i++) {
		dice[i]=NULL;
		rlen=getline(&dice[i],&llen,fh);
		p=strchr(dice[i],'\n');
		if(p) *p='\0';
		llen=0;
	}
}



void ShuffleDice() {
	size_t i,j;
	char *tmp;
	for(i=BOX_SIZE*BOX_SIZE-1;i>0;i--) {
		j=(size_t)(rand()%(i+1));
		tmp=dice[i];
		dice[i]=dice[j];
		dice[j]=tmp;
	}
}



void NewBoard() {
	size_t i;
	for(i=0;i<BOX_SIZE*BOX_SIZE;i++) {
		letters[i/BOX_SIZE][i%BOX_SIZE]=dice[i][rand()%strlen(dice[i])];
	}
}



void GetWords() {
	size_t i,j;

	for(j=0;j<BOX_SIZE;j++) {
		for(i=0;i<BOX_SIZE;i++) {
			dfs(i,j,0,&trie_root);
		}
	}

	qsort(words,nwords,sizeof(*words),cmpalphaasc);
}


#endif /* BOGGLE_IMPLEMENTATION */

#endif /* BOGGLE_H */
