#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <errno.h>


#define STRING_MAX 1024



void freeLines(char ***lines,size_t *nlines) {
  for(size_t i=0;i<(*nlines);i++) {
    free((*lines)[i]);
    (*lines)[i]=NULL;
  }
  free(*lines);
  (*lines)=NULL;
  (*nlines)=0;
}



int addLine(char ***lines,size_t *nlines,char *line) {
  char **tmp=realloc(*lines,sizeof(*lines)*((*nlines)+1));
  if(tmp==NULL) {
    printf("Error: allocating memory\n");
    return 1;
  }
  (*lines)=tmp;
  (*lines)[(*nlines)++]=strdup(line);
  return 0;
}



int load(char *path,char ***lines,size_t *nlines) {
  FILE *fin;
  char *line=NULL;
  size_t llen=0;
  ssize_t rlen=0;

  if((fin=fopen(path,"rt"))==NULL) {
    perror(path);
    return 1;
  }
  
  while((rlen=getline(&line,&llen,fin))>0) {
    char *p=strchr(line,'\n');
    if(p) *p='\0';
    if(addLine(lines,nlines,line)!=0) {
      return 2;
    }
  }
  fclose(fin);

  return 0;
}



int save(char *path,char **lines,size_t nlines) {
  FILE *fout;

  if((fout=fopen(path,"wt"))==NULL) {
    perror(path);
    return 1;
  }
  
  for(size_t i=0;i<nlines;i++) {
    fprintf(fout,"%s\n",lines[i]);
  }
  fclose(fout);

  return 0;
}



int main(int argc,char **argv) {

  bool quit=false;



  char **lines=NULL;
  size_t nlines=0;

  char *line=NULL;
  size_t llen=0;
  ssize_t rlen=0;



  char **clines=NULL;
  size_t cnlines=0;

  char *cline=NULL;
  size_t cllen=0;
  ssize_t crlen=0;



  char path[STRING_MAX];
  char filename[STRING_MAX]="noname.txt";

  size_t num;
  char str[STRING_MAX];
  
  size_t startLine=0;
  size_t endLine=0;



  if(argc==2) {
    load(argv[1],&lines,&nlines);
  }

  while(!quit) {

    printf("> ");

    if((rlen=getline(&line,&llen,stdin))<=0) {
      quit=true;
      continue;  
    }

    char *p=strchr(line,'\n');
    if(p) *p='\0';

//    if(*line) printf("%s\n",line);

    if(!strcmp(line,"q")) {
      quit=true;
      continue;
    } else if(sscanf(line,"n %[^\n]",path)==1) {
      strcpy(filename,path);
      printf("OK\n");
    } else if(!strcmp(line,"r")) {
      if(lines) {
        freeLines(&lines,&nlines);
      }
      if(load(filename,&lines,&nlines)==0) {
        printf("OK\n");
      }
    } else if(!strcmp(line,"w")) {
      if(lines) {
        if(save(filename,lines,nlines)==0) {
          printf("OK\n");
        }
      }
    } else if(sscanf(line,"a %[^\n]",str)==1) {
      addLine(&lines,&nlines,str);      
      printf("%4zu %s\n",nlines,lines[nlines-1]);
    } else if(!strcmp(line,"a")) {
      addLine(&lines,&nlines,"");      
      printf("%4zu %s\n",nlines,lines[nlines-1]);
    } else if(sscanf(line,"c %zu %[^\n]",&startLine,str)==2) {
      if(startLine>=1 && startLine<=nlines) {
        startLine--;
        free(lines[startLine]);
        lines[startLine]=strdup(str);
        printf("OK\n");
      }
    } else if(sscanf(line,"l %zu %zu",&startLine,&endLine)==2) {
      if(startLine>=1 && startLine<=nlines && endLine>=1 && endLine<=nlines && startLine<=endLine) {
        for(ssize_t i=(ssize_t)startLine;i<=(ssize_t)endLine;i++) {
          printf("%4zu %s\n",i,lines[i-1]);
        }
        printf("OK\n");
      }     
    } else if(!strcmp(line,"l")) {
      printf("LINES: %zu\n",nlines);
    } else if(sscanf(line,"d %zu %zu",&startLine,&endLine)==2) {
      if(startLine>=1 && startLine<=nlines && endLine>=1 && endLine<=nlines && startLine<=endLine) {
        size_t i,j;
        size_t numLines;

        startLine--;
        endLine--;        

        numLines=endLine-startLine+1;
        
        if(clines) freeLines(&clines,&cnlines);
        
        for(i=0;i<numLines;i++) {
          addLine(&clines,&cnlines,lines[i+startLine]);
          free(lines[i+startLine]);
          lines[i+startLine]=NULL;
        }

        i=startLine;
        j=startLine+numLines;
        while(j<nlines) {
          lines[i]=strdup(lines[j]);
          free(lines[j]);
          lines[j]=NULL;
          i++;
          j++;
        }
        
        char **tmp=realloc(lines,sizeof(*lines)*(nlines-numLines));
        if(tmp) {
          lines=tmp;
          nlines-=numLines;
          printf("OK\n");
        }
      }               
    } else if(sscanf(line,"y %zu %zu",&startLine,&endLine)==2) {

      if(startLine>=1 && startLine<=nlines && endLine>=1 && endLine<=nlines && startLine<=endLine) {

        size_t numLines;

        startLine--;
        endLine--;        

        numLines=endLine-startLine+1;
          
        if(clines) freeLines(&clines,&cnlines);
        
        for(size_t i=0;i<numLines;i++) {
          addLine(&clines,&cnlines,lines[i+startLine]);
        }

        printf("OK\n");
      }

    } else if(sscanf(line,"p %zu",&startLine)==1) {
      if(clines && startLine<=nlines) {
        size_t i,j;
        startLine--;
        char **tmp=realloc(lines,sizeof(*lines)*(nlines+cnlines));
        if(tmp) {
          lines=tmp;
          nlines+=cnlines;

          i=nlines-cnlines-1;
          j=nlines-1;
          while(i>=startLine) {
            lines[j]=strdup(lines[i]);
            free(lines[i]);
            lines[i]=NULL;
            i--;
            j--;
          }

          for(size_t i=0;i<cnlines;i++) {
            lines[i+startLine]=strdup(clines[i]);
          }

          printf("OK\n");
        }
      }        
    }
    
  }

  printf("Bye!\n");
    
  return 0;
}
