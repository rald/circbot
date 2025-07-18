#include <string.h>

#include "ezxml.h"

int main(void) {

	ezxml_t bible = ezxml_parse_file("adb.xml"), book,chap,vers;

	for(book = ezxml_child(bible,"BIBLEBOOK"); book; book = book->next) {
    for(chap = ezxml_child(book,"CHAPTER"); chap; chap = chap->next) {
	    for(vers = ezxml_child(chap,"VERS"); vers; vers = vers->next) {
		    const char *bname = ezxml_attr(book, "bname");
    		int bnum = atoi(ezxml_attr(book, "bnumber"));
		    int cnum = atoi(ezxml_attr(chap, "cnumber"));
		    int vnum = atoi(ezxml_attr(vers, "vnumber"));
	    	char *text = ezxml_txt(vers);

        char *pbname;
        int pbnum;
        int pcnum;
        int pvnum;
        char *ptext;

//				printf("%s|%d:%d|%s\n",bname,cnum,vnum,text);

        


        pbname=strdup(bname);
        pbnum=bnum;
        pcnum=cnum;
        pvnum=vnum;
        ptext=strdup(text);

	    }
    }
	}

	ezxml_free(bible);

}
