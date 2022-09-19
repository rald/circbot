#!/usr/bin/env runc

#include <stdio.h>
#include <stdlib.h>


int main(int argc,char **argv) {

  double weight,height,bmi;

  weight=atof(argv[1]);
  height=atof(argv[2]);
  
  bmi=(weight/(height*height))*703;

  printf("bmi: %.2f\n",bmi);

  printf("result: ");

  if(bmi<18.5) printf("underweight");
  else if(bmi<24.9) printf("healthy");
  else if(bmi<29.9) printf("overweight");
  else printf("obese");

  printf("\n");

  return 0;
}
