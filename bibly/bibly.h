#ifndef BIBLY_H
#define BIBLY_H

#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "common.h"

#define UTIL_IMPLEMENTATION
#include "util.h"



#define BIBLY_VERSION "bibly v1.0.0"


typedef struct BiblyInfo BiblyInfo;

struct BiblyInfo {
	char *bname;
	char **snames;
	size_t nsnames;
	size_t bnum;
	size_t nchaps;
	size_t *nvers;
};


void Bibly_PrintVersion(void);
int Bibly_Search(const char *path,char*** lines,size_t *nlines,const char *line);
int Bibly_GetVerses(char *path,char ***verses,size_t *nverses,char *book,unsigned int cnum,unsigned int svnum,unsigned int evnum);
int Bibly_GetInfo(const char *path,BiblyInfo ***binfos,size_t *nbinfos);



#ifdef BIBLY_IMPLEMENTATION



void Bibly_PrintVersion(void) {
  printf(BIBLY_VERSION "\n");
}



int Bibly_Search(const char *path,char*** lines,size_t *nlines,const char *line) {
  FILE *fin;

  char *sline=NULL;
  size_t slinelen=0;
  ssize_t sreadlen=0;

  char *upr1=NULL;
  char *upr2=NULL;

  if((fin=fopen(path,"rt"))==NULL) {
    printf("Error: cannot open file %s\n",path);
    return 1;
  }

  upr1=strupr(line);

  while((sreadlen=getline(&sline,&slinelen,fin))!=-1) {

    char **tokens=NULL;
    size_t ntokens=0;

		char *slinecopy=strdup(sline);

    tokenize(&tokens,&ntokens,slinecopy,"|");

    upr2=strupr(tokens[3]);

    if(strstr(upr2,upr1)!=NULL) {
      (*lines)=realloc(*lines,sizeof(*lines)*((*nlines)+1));
      (*lines)[(*nlines)++]=strdup(sline);
    }

    free(upr2);

		freetokens(&tokens,&ntokens);

    free(slinecopy);
    slinecopy=NULL;

    free(sline);
    sline=NULL;
    slinelen=0;
    sreadlen=0;

  }

  free(upr1);

  return 0;

}



int Bibly_GetVerses(char *path,char ***verses,size_t *nverses,char *book,unsigned int cnum,unsigned int svnum,unsigned int evnum) {

	FILE *fh;

	char *line=NULL;
	size_t llen=0;
	ssize_t rlen=0;

	char **tokens=NULL;
	size_t ntokens=0;

	char *xbook=NULL;
	unsigned int xcnum=0;
	unsigned int xvnum=0;
	char *xvers=NULL;


	if((fh=fopen(path,"rt"))==NULL) {
		printf("Error: opening file %s.\n",path);
		return 1;
	}

	while((rlen=getline(&line,&llen,fh))!=-1) {

		tokenize(&tokens,&ntokens,line,"|");

		xbook=strdup(tokens[0]);
		xcnum=atoi(tokens[1]);
		xvnum=atoi(tokens[2]);
		xvers=strdup(tokens[3]);

		if(
				!strcasecmp(book,xbook) &&
				cnum==xcnum &&
				(svnum<=xvnum && evnum>=xvnum)
		) {
			(*verses)=realloc((*verses),sizeof(*verses)*((*nverses)+1));
			(*verses)[(*nverses)++]=strdup(line);
		}

		freetokens(&tokens,&ntokens);

		xvnum=0;
		xcnum=0;
		free(xvers);
		xvers=NULL;
		free(xbook);
		xbook=NULL;

		free(line);
		line=NULL;
		llen=0;
		rlen=0;

	}

	return 0;
}


int Bibly_GetInfo(const char *path,BiblyInfo ***binfos,size_t *nbinfos) {

	FILE *fh;

	char *line=NULL;
	size_t llen=0;
	ssize_t rlen=0;

	char **tokens1=NULL;
	size_t ntokens1=0;

	char **tokens2=NULL;
	size_t ntokens2=0;

	char **tokens3=NULL;
	size_t ntokens3=0;

	size_t i;

	if((fh=fopen(path,"rt"))==NULL) {
		printf("Error: cannot open %s.\n",path);
		return 1;
	}

	while((rlen=getline(&line,&llen,fh))!=-1) {
		char *p=strchr(line,'\n');
		if(p!=NULL) *p='\0';

		(*binfos)=realloc(*binfos,sizeof(**binfos)*((*nbinfos)+1));
		(*binfos)[(*nbinfos)]=malloc(sizeof(*((*binfos)[(*nbinfos)])));

		tokenize(&tokens1,&ntokens1,line,"|");
		tokenize(&tokens2,&ntokens2,tokens1[1],"/");
		tokenize(&tokens3,&ntokens3,tokens1[4],",");

		(*binfos)[(*nbinfos)]->bname=strdup(tokens1[0]);
		(*binfos)[(*nbinfos)]->bnum=atoi(tokens1[2]);
		(*binfos)[(*nbinfos)]->nchaps=atoi(tokens1[3]);

		(*binfos)[(*nbinfos)]->snames=malloc(sizeof(*((*binfos)[(*nbinfos)]->snames))*ntokens2);

		for(i=0;i<ntokens2;i++) {
			(*binfos)[(*nbinfos)]->snames[i]=strdup(tokens2[i]);
		}

		(*binfos)[(*nbinfos)]->nsnames=ntokens2;


		(*binfos)[(*nbinfos)]->nvers=malloc(sizeof(*((*binfos)[(*nbinfos)]->nvers))*ntokens3);

		for(i=0;i<ntokens3;i++) {
			(*binfos)[(*nbinfos)]->nvers[i]=atoi(tokens3[i]);
		}

		(*nbinfos)++;

		freetokens(&tokens3,&ntokens3);
		freetokens(&tokens2,&ntokens2);
		freetokens(&tokens1,&ntokens1);

		free(line);
		line=NULL;
		llen=0;
	}

	fclose(fh);

	return 0;
}

void Bibly_FreeVerses(char ***verses,size_t *nverses) {
	size_t i;
	for (i = 0; i < (*nverses); i++) {
		free((*verses)[i]);
		(*verses)[i] = NULL;
	}
	free(*verses);
	(*verses) = NULL;
	(*nverses) = 0;
}

char *Bibly_GetRandomVerse(char *path) {
	char *line=NULL;
	size_t llen=0;
	ssize_t rlen=0;

	char *sel=NULL;

	FILE *fin=fopen(path,"r");

  size_t n=0;

	while((rlen=getline(&line,&llen,fin))!=-1) {

		if(drand()<1.0/++n) {
			if(sel!=NULL) free(sel);
			sel=strdup(line);
		}

		free(line);
		line=NULL;
		llen=0;
		rlen=0;
	}

	return sel;
}


#endif /* BIBLY_IMPLEMENTATON */

#endif /* BIBLY_H */
