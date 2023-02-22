#ifndef BRAIN_H
#define BRAIN_H



#define CSV_IMPLEMENTATION
#include "csv.h"



typedef struct Brain Brain;

struct Brain {
  char **lines;
  size_t nlines;
};

Brain *Brain_New(char **lines,size_t nlines);

void Brain_Add(Brain ***brains,size_t *nbrains,Brain *brain);

void Brain_Load(Brain ***brains,size_t *nbrains,char *filename);

void Brain_Print(Brain *brain);

void Brains_Print(Brain **brains,size_t nbrains);

void Brain_Free(Brain **brain);

void Brains_Free(Brain ***brains,size_t *nbrains);



#ifdef BRAIN_IMPLEMENTATION

Brain *Brain_New(char **lines,size_t nlines) {
  Brain *brain=malloc(sizeof(*brain));
  if(brain) {
    brain->lines=lines;
    brain->nlines=nlines;
  }
  return brain;
}



void Brain_Add(Brain ***brains,size_t *nbrains,Brain *brain) {
  (*brains)=realloc(*brains,sizeof(**brains)*((*nbrains)+1));
  (*brains)[(*nbrains)++]=brain;
}



void Brain_Load(Brain ***brains,size_t *nbrains,char *filename) {

  char *line=NULL;
  size_t llen=0;
  ssize_t rlen=0;

  char **lines=NULL;
  size_t nlines=0;
  
	FILE *fin=fopen(filename,"r");

  while((rlen=getline(&line,&llen,fin))>0) {

    CSV_Parse(&lines,&nlines,line);

    Brain_Add(brains,nbrains,Brain_New(lines,nlines));

    lines=NULL;
    nlines=0;

    free(line);
    line=NULL;
    llen=0;
    rlen=0;
  }

  free(line);
  line=NULL;
  llen=0;
  rlen=0;

	fclose(fin);

}


void Brain_Print(Brain *brain) {
  CSV_Print(brain->lines,brain->nlines);
}



void Brains_Print(Brain **brains,size_t nbrains) {
  for(size_t i=0;i<nbrains;i++) {
    Brain_Print(brains[i]);
  }
}



void Brain_Free(Brain **brain) {
  CSV_Free(&(*brain)->lines,&(*brain)->nlines);
  free(*brain);
  (*brain)=NULL;
}



void Brains_Free(Brain ***brains,size_t *nbrains) {
  for(size_t i=0;i<(*nbrains);i++) {
    Brain_Free(&(*brains)[i]);
  } 
  free(*brains);
  (*brains)=NULL;
  (*nbrains)=0;
}



#endif


#endif


