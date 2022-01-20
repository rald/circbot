#ifndef PHOTODATA_H
#define PHOTODATA_H

#include <stdlib.h>

typedef struct PhotoData PhotoData;

struct PhotoData {
	int id;
	int poolId;
	char *solution;
};

PhotoData *PhotoData_New(int id,int poolId,char *solution);

#endif
