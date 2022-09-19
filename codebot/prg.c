#!/usr/bin/env runc

#include <stdio.h>

int main(int argc,char **argv) {
  char line[1024];
  while(fgets(line,sizeof(line),stdin)) {
    printf("%s",line);
  }
  return 0;
}
