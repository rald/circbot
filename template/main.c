#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdarg.h>
#include <stddef.h>
#include <string.h>
#include <ctype.h>
#include <time.h>

#include "dyad.h"



static int prt = 6667;
static char *srv = "sakura.jp.as.dal.net";
static char *chn = "#pantasya";
static char *nck = "siesti";
static char *pss = NULL;



static char *cmd=NULL, *usr=NULL, *par=NULL, *txt=NULL;

bool isReg=false;

size_t ticks=0;



static void sendf(dyad_Stream * s, char *fmt, ...);
static char *trim(char *a);
static char *skip(char *s, char c);

static void onConnect(dyad_Event *e);
static void onError(dyad_Event *e);
static void onTick(dyad_Event *e);
static void onLine(dyad_Event *e);



int main(void) {

	dyad_Stream *stm;

	srand((unsigned int)time(NULL));

	dyad_init();

	stm = dyad_newStream();

	dyad_addListener(stm, DYAD_EVENT_CONNECT, onConnect, NULL);
	dyad_addListener(stm, DYAD_EVENT_ERROR, onError, NULL);
	dyad_addListener(stm, DYAD_EVENT_LINE, onLine, NULL);
	dyad_addListener(stm, DYAD_EVENT_TICK, onTick, NULL);

	printf("Connecting...\n");

	dyad_connect(stm, srv, prt);

	while (dyad_getStreamCount() > 0) {
		dyad_update();
	}

	dyad_shutdown();

	return 0;
}



static void sendf(dyad_Stream * s, char *fmt, ...) {

	char *buf1=NULL;
	char *buf2=NULL;
	ssize_t buflen=0;

	va_list args;

	va_start(args, fmt);
	buflen=vsnprintf( NULL, 0, fmt, args);
	va_end(args);

	buf1=calloc(buflen+5,sizeof(*buf1));
	buf2=calloc(buflen+5,sizeof(*buf2));

	va_start(args, fmt);
	vsprintf(buf1,fmt,args);
	va_end(args);

	sprintf(buf2,"<-- %s",buf1);

	printf("%s\n",buf2);

	dyad_writef(s, "%s\r\n", buf1);

	free(buf2);
	buf2=NULL;
	free(buf1);
	buf1=NULL;
}



static char *trim(char *a) {
    char *p = a, *q = a;
    while (isspace(*q))            ++q;
    while (*q)                     *p++ = *q++;
    *p = '\0';
    while (p > a && isspace(*--p)) *p = '\0';
	return a;
}



static char *skip(char *s, char c) {
	while (*s != c && *s != '\0')
		s++;
	if (*s != '\0')
		*s++ = '\0';
	return s;
}



static void onConnect(dyad_Event *e) {
	if (pss) {
		sendf(e->stream, "PASS %s", pss);
	}
	sendf(e->stream, "NICK %s", nck);
	sendf(e->stream, "USER %s %s %s :%s", nck, nck, nck, nck);
}



static void onError(dyad_Event *e) {
	printf("error: %s\n", e->msg);
}



static void onTick(dyad_Event *e) {
	ticks++;
}



static void onLine(dyad_Event *e) {

	size_t i,j;

	char *tmp=NULL;

	printf("--> %s\n", e->data);

	tmp = strdup(e->data);

	cmd = tmp;

	usr = srv;

	if (cmd==NULL) {
		return;
	}

	if (*cmd=='\0') {
		free(cmd);
		return;
	}

	if (cmd[0] == ':') {
		usr = cmd + 1;
		cmd = skip(usr, ' ');
		if (cmd[0] == '\0')
			return;
		skip(usr, '!');
	}
	skip(cmd, '\r');
	par = skip(cmd, ' ');
	txt = skip(par, ':');

	trim(par);
	trim(txt);

	if (strcmp(cmd, "PING")==0) {
		sendf(e->stream, "PONG %s\r\n", txt);
	} else if (strcmp(cmd, "001")==0) {
		printf("Connected.\n");
		sendf(e->stream, "JOIN %s", chn);
		isReg = true;
	} else if (strcmp(cmd, "PRIVMSG")==0) {
		printf("<%s> %s\n", usr, txt);
	}
}



