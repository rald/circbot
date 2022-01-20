#include "PhotoData.h"

PhotoData *PhotoData_New(int id,int poolId,char *solution) {
	PhotoData *photoData=malloc(sizeof(PhotoData));
	if(photoData) {
		photoData->id=id;
		photoData->poolId=poolId;
		photoData->solution=solution;
	}
	return photoData;
}

