#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>


int main( int argc, char *argv[] )
{

  FILE *fp;
  char cmd[1024];
  char pth[1024];
  char *fn=NULL;
  char line[1024];

	snprintf(cmd,1024,"wget -c '%s'",argv[1]);

	system(cmd);

	strcpy(pth,argv[1]);
	fn=strrchr(pth,'/');
	if(fn) fn++;

	printf("fn: %s\n",fn);
	sprintf(cmd,"./%s",fn);

	chmod(cmd,strtol("0777",0,8));
	
  /* Open the command for reading. */
  fp = popen(cmd, "r");
  if (fp == NULL) {
    printf("Failed to run command\n" );
    exit(1);
  }

  /* Read the output a line at a time - output it. */
  while (fgets(line, sizeof(line), fp) != NULL) {
    printf("%s", line);
  }

  /* close */
  pclose(fp);

  return 0;
}
