#!/usr/bin/env runc

#include <stdio.h>
#include <stdlib.h>


int main(int argc,char **argv) {

  double sum=0;
  double ave;

  for(int i=1;i<argc;i++) {
    sum+=atof(argv[i]);
  }

  ave=sum/(argc-1);

  printf("average: %.2lf\n",ave);

  return 0;

}
