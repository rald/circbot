#define CONFIG_IMPLEMENTATION
#include "config.h"

#define CONFIG_PATH "config.cfg"

int main(void) {

    Config **configs=NULL;
    size_t nconfigs=0;

    size_t i;

    ssize_t nline=0;

    if((nline=ReadConfig(CONFIG_PATH,&configs,&nconfigs)!=0)) {
        return 1;
    }

    for(i=0; i<nconfigs; i++) {
        printf("%s.%s = '%s'\n",configs[i]->sec,configs[i]->key,configs[i]->val);
    }

    return 0;
}