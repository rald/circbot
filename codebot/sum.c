#!/usr/bin/env runc

#include <stdio.h>
#include <stdlib.h>


int main(int argc,char **argv) {

  double sum=0;

  for(int i=1;i<argc;i++) {
    sum+=atof(argv[i]);
  }

  printf("sum: %.2lf\n",sum);

  return 0;

}
