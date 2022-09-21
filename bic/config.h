#ifndef CONFIG_H
#define CONFIG_H



#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>


#define MYLIB_IMPLEMENTATION
#include "mylib.h"



typedef struct Config Config;

struct Config {
  char *sec;
  char *key;
  char *val;
};



ssize_t ReadConfig(const char *path, Config *** configs, size_t * nconfigs);
Config *CreateConfig(char *sec, char *key, char *val);
ssize_t FindConfigSecKey(Config ** configs, size_t nconfigs, const char *sec, const char *key);



#ifdef CONFIG_IMPLEMENTATION



ssize_t ReadConfig(const char *path, Config *** configs, size_t * nconfigs) {
  FILE *fh;

  char *line = NULL;
  size_t llen = 0;
  ssize_t rlen = 0;

  char *tmp = NULL;

  char *sec = NULL;
  char *key = NULL;
  char *val = NULL;

  size_t nline = 0;

  ssize_t k;

  if ((fh = fopen(path, "rt")) == NULL) {
    fprintf(stderr, "Error: cannot open %s.\n", path);
    return -1;
  }
  while ((rlen = getline(&line, &llen, fh)) != -1) {

    nline++;

    tmp = strdup(line);
    trim(tmp);

    if (*tmp != '\0') {

      if (tmp[0] != '#') {

	if (tmp[0] == '[') {

	  if (sec != NULL) {
	    free(sec);
	    sec = NULL;
	  }
	  sec = strdup(tmp + 1);
	  skip(sec, ']');
	  trim(sec);

	} else {

	  key = tmp;
	  val = skip(key, '=');
	  if (val == NULL) {
	    fprintf(stderr, "Error: error reading config file %s at line number %zd.\n", path, nline);
	    return nline;
	  }
	  trim(key);
	  trim(val);

	  k = FindConfigSecKey(*configs, *nconfigs, sec, key);

	  if (k == -1) {
	    (*configs) = realloc(*configs, sizeof(**configs) * ((*nconfigs) + 1));
	    if (configs != NULL) {
	      (*configs)[(*nconfigs)++] = CreateConfig(sec, key, val);
	    } else {
	      fprintf(stderr, "Error: allocating memory.\n");
	      return -2;
	    }
	  } else {
	    free((*configs)[k]->val);
	    (*configs)[k]->val = strdup(val);
	  }
	}
      }
    }
    free(tmp);
    tmp = NULL;

    free(line);
    line = NULL;
    llen = 0;

  }

  if (sec != NULL) {
    free(sec);
    sec = NULL;
  }
  return 0;

}



Config *CreateConfig(char *sec, char *key, char *val) {
  Config *config = malloc(sizeof(*config));
  if (config != NULL) {
    if (sec != NULL) {
      config->sec = strdup(sec);
    }
    config->key = strdup(key);
    config->val = strdup(val);
  }
  return config;
}



ssize_t FindConfigSecKey(Config ** configs, size_t nconfigs, const char *sec, const char *key) {
  size_t i;
  for (i = 0; i < nconfigs; i++) {
    if ((sec != NULL && strcmp(sec, configs[i]->sec) == 0) || strcmp(key, configs[i]->key) == 0) {
      return i;
    }
  }
  return -1;
}



#endif				/* CONFIG_IMPLEMENTATION */

#endif				/* CONFIG_H */
