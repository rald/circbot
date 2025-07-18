#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdarg.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>

#include "dyad.h"

#include "common.h"

#define UTIL_IMPLEMENTATION
#include "util.h"

#define BIBLY_IMPLEMENTATION
#include "bibly.h"

#define LEXER_IMPLEMENTATION
#include "lexer.h"

#define BIBLE_PATH "kjv.csv"
#define BIBLE_INFO "kjv.inf"
#define ID_FILE "id.txt"
#define DIARY_FILE "diary.txt"



static char    *server = "irc.undernet.org";
static char    *channels = "#pantasya";
static char    *nick = "frio";
static char    *pass = NULL;
static int 	isRegistered = 0;

static BiblyInfo **binfos = NULL;
static size_t 	nbinfos = 0;

static void 	send(dyad_Stream * stream, char *dest, char *fmt,...);
static void 	onConnect(dyad_Event * e);
static void 	onError(dyad_Event * e);
static void 	onLine(dyad_Event * e);



int
main(void)
{
	dyad_Stream    *s;
	dyad_init();

	s = dyad_newStream();
	dyad_addListener(s, DYAD_EVENT_CONNECT, onConnect, NULL);
	dyad_addListener(s, DYAD_EVENT_ERROR, onError, NULL);
	dyad_addListener(s, DYAD_EVENT_LINE, onLine, NULL);
	dyad_connect(s, server, 6667);

	while (dyad_getStreamCount() > 0) {
		dyad_update();
	}

	dyad_shutdown();
	return 0;
}



static void
send(dyad_Stream * stream, char *dest, char *fmt,...)
{
	int 		len = 0;
	char           *buf1 = NULL;
	char 		buf2     [256];
	size_t 		i = 0, j = 0;


	va_list 	args;

	va_start(args, fmt);
	len = vsnprintf(NULL, 0, fmt, args);
	va_end(args);

	va_start(args, fmt);
	buf1 = calloc(len + 1, sizeof(*buf1));
	vsprintf(buf1, fmt, args);
	va_end(args);

	buf2[0] = '\0';
	while (buf1[i]) {
		buf2[j++] = buf1[i++];
		buf2[j] = '\0';
		if (j >= 255) {
			//sleep(60);
			dyad_writef(stream, "PRIVMSG %s :%s\r\n", dest, buf2);
			buf2[0] = '\0';
			j = 0;
		}
	}
	if (j > 0) {
		//sleep(60);
		dyad_writef(stream, "PRIVMSG %s :%s\r\n", dest, buf2);
		buf2[0] = '\0';
		j = 0;
	}
}



static void
onConnect(dyad_Event * e)
{
	if (pass)
		dyad_writef(e->stream, "PASS %s\r\n", pass);
	dyad_writef(e->stream, "NICK %s\r\n", nick);
	dyad_writef(e->stream, "USER %s %s %s :%s\r\n", nick, nick, nick, nick);
}



static void
onError(dyad_Event * e)
{
	printf("error: %s\n", e->msg);
}



static void
onLine(dyad_Event * e)
{

	char 		nick     [32], user[32], serv[256], chan[32], body[512];

	printf("%s\n", e->data);

	/* Handle PING */
	if (!memcmp(e->data, "PING", 4)) {
		dyad_writef(e->stream, "PONG%s\r\n", e->data + 4);
	}
	/* Handle RPL_WELCOME */
	if (!isRegistered && strstr(e->data, "001")) {
		/* Join channel */
		dyad_writef(e->stream, "JOIN %s\r\n", channels);
		isRegistered = 1;

		Bibly_GetInfo(BIBLE_INFO, &binfos, &nbinfos);
	}
	if (sscanf(e->data, ":%31[^!]!~%31[^@]@%255s PRIVMSG %31s :%511[^\n]", nick, user, serv, chan, body) == 5) {
		char 		msg [STRING_MAX];
		size_t 	id;
		FILE    *fh;
		char    *line = NULL;
		size_t 	llen = 0;
		ssize_t rlen = 0;
		size_t 	cnt = 0;
		size_t 	page=1;

		if (sscanf(body, ".diary write %[^\n]", msg) == 1) {
			fh = fopen(ID_FILE, "r");
			fscanf(fh, "%zu\n", &id);
			fclose(fh);
			fh = fopen(DIARY_FILE, "a");
			fprintf(fh, "%s\n", msg);
			fclose(fh);
			send(e->stream, chan, "%s:msg id %d", nick, id);
			fh = fopen(ID_FILE, "w");
			fprintf(fh, "%zu\n", ++id);
			fclose(fh);
		} else if (sscanf(body, ".diary read %zu", &id) == 1) {
			fh = fopen(DIARY_FILE, "r");
			while ((rlen = getline(&line, &llen, fh)) != -1) {
				cnt++;
				if (cnt == id) {
					send(e->stream, chan, "%s:%d:%s", nick, id, line);
					break;
				}
			}
			fclose(fh);
		} else if (sscanf(body, ".diary find %[^\n]", msg) == 1) {
			char 		tmp      [512] = "";
			char 		scnt     [8] = "";
			bool 		first = true;
			fh = fopen(DIARY_FILE, "r");
			while ((rlen = getline(&line, &llen, fh)) != -1) {
				cnt++;
				char           *upr1 = strupr(trim(line));
				char           *upr2 = strupr(trim(msg));
				if (strstr(line, msg)) {
					sprintf(scnt, "%zu", cnt);
					if (strlen(nick) + strlen(tmp) + strlen(scnt) + 3 < 256) {
						if (first)
							first = false;
						else
							strcat(tmp, ", ");
						strcat(tmp, scnt);
					} else {
						send(e->stream, chan, "%s: %s", nick, tmp);
						tmp[0] = '\0';
					}
				}
				free(upr2);
				upr2 = NULL;
				free(upr1);
				upr1 = NULL;
			}
			if (*tmp) {
				send(e->stream, chan, "%s: %s", nick, tmp);
				tmp[0] = '\0';
			}
			fclose(fh);

		} else if ((sscanf(body, ".kjv find %[^\n]", msg) == 1) || (sscanf(body, ".kjv %zd find %[^\n]",&page,msg) == 2)) {

			char    **lines = NULL;
			size_t 	nlines = 0;

			char    **vtokens = NULL;
			size_t 	nvtokens = 0;

			char    *xbook = NULL;
			char    *xvers = NULL;
			size_t 	xcnum = 0;
			size_t 	xvnum = 0;

			size_t 		i;

			size_t startLine=0;
			size_t endLine=0;

			Bibly_Search(BIBLE_PATH, &lines, &nlines, msg);

			if(page>=1) {

				send(e->stream, chan, "found %zd occurences. page %zd of %zd", nlines,page,nlines/4+(nlines%4>1?1:0));

				startLine=(page-1)*4;
				endLine=(page-1)*4+4;
				if(endLine>=nlines) endLine=nlines-1;

				for (i = startLine; i < endLine; i++) {

					tokenize(&vtokens, &nvtokens, lines[i], "|");

					xbook = strdup(vtokens[0]);
					xcnum = atoi(vtokens[1]);
					xvnum = atoi(vtokens[2]);
					xvers = strdup(vtokens[3]);

					freetokens(&vtokens, &nvtokens);
					send(e->stream, chan, "(%zu) %s %zd:%zd -> %s",i+1,xbook,xcnum,xvnum,xvers);

					xvnum = 0;
					xcnum = 0;
					free(xvers);
					xvers = NULL;
					free(xbook);
					xbook = NULL;

				}

				for (i = 0; i < nlines; i++) {
					free(lines[i]);
					lines[i] = NULL;
				}
				nlines = 0;

			}

		} else if (sscanf(body, ".kjv %[^\n]", msg) == 1) {

			Token     **ltokens = NULL;
			size_t 		nltokens = 0;

			char          **verses = NULL;
			size_t 		nverses = 0;

			char 		book     [STRING_MAX] = "";
			unsigned int 	cnum = 0;
			unsigned int 	svnum = 0;
			unsigned int 	evnum = 0;

			char          **vtokens = NULL;
			size_t 		nvtokens = 0;

			char           *xbook = NULL;
			char           *xvers = NULL;
			unsigned int 	xcnum = 0;
			unsigned int 	xvnum = 0;

			size_t 		i, j, k;

			lex(&ltokens, &nltokens, msg);

			if (
			    nltokens == 5 &&
			    ltokens[0]->type == TOKEN_TYPE_STRING &&
			    ltokens[1]->type == TOKEN_TYPE_INTEGER &&
			    ltokens[2]->type == TOKEN_TYPE_COLON &&
			    ltokens[3]->type == TOKEN_TYPE_INTEGER
				) {

				strcpy(book, ltokens[0]->text);
				cnum = atoi(ltokens[1]->text);
				svnum = atoi(ltokens[3]->text);
				evnum = svnum;

			} else if (
				   nltokens == 6 &&
				   ltokens[0]->type == TOKEN_TYPE_INTEGER &&
				   ltokens[1]->type == TOKEN_TYPE_STRING &&
				   ltokens[2]->type == TOKEN_TYPE_INTEGER &&
				   ltokens[3]->type == TOKEN_TYPE_COLON &&
				   ltokens[4]->type == TOKEN_TYPE_INTEGER
				) {

				sprintf(book, "%s %s", ltokens[0]->text, ltokens[1]->text);
				cnum = atoi(ltokens[2]->text);
				svnum = atoi(ltokens[4]->text);
				evnum = svnum;

			} else if (
				   nltokens == 7 &&
				   ltokens[0]->type == TOKEN_TYPE_STRING &&
				   ltokens[1]->type == TOKEN_TYPE_INTEGER &&
				   ltokens[2]->type == TOKEN_TYPE_COLON &&
				   ltokens[3]->type == TOKEN_TYPE_INTEGER &&
				   ltokens[4]->type == TOKEN_TYPE_DASH &&
				   ltokens[5]->type == TOKEN_TYPE_INTEGER
				) {

				strcpy(book, ltokens[0]->text);
				cnum = atoi(ltokens[1]->text);
				svnum = atoi(ltokens[3]->text);
				evnum = atoi(ltokens[5]->text);

			} else if (
				   nltokens == 8 &&
				   ltokens[0]->type == TOKEN_TYPE_INTEGER &&
				   ltokens[1]->type == TOKEN_TYPE_STRING &&
				   ltokens[2]->type == TOKEN_TYPE_INTEGER &&
				   ltokens[3]->type == TOKEN_TYPE_COLON &&
				   ltokens[4]->type == TOKEN_TYPE_INTEGER &&
				   ltokens[5]->type == TOKEN_TYPE_DASH &&
				   ltokens[6]->type == TOKEN_TYPE_INTEGER
				) {

				sprintf(book, "%s %s", ltokens[0]->text, ltokens[1]->text);
				cnum = atoi(ltokens[2]->text);
				svnum = atoi(ltokens[4]->text);
				evnum = atoi(ltokens[6]->text);
			}
			k = 0;
			for (i = 0; i < nbinfos && k == 0; i++) {
				for (j = 0; j < binfos[i]->nsnames && k == 0; j++) {
					if (strcasecmp(book, binfos[i]->snames[j]) == 0) {
						strcpy(book, binfos[i]->bname);
						k = 1;
					}
				}
			}

			Bibly_GetVerses(BIBLE_PATH, &verses, &nverses, book, cnum, svnum, evnum);

			for (i = 0; i < (nverses > 4 ? 4 : nverses); i++) {

				tokenize(&vtokens, &nvtokens, verses[i], "|");

				xbook = strdup(vtokens[0]);
				xcnum = atoi(vtokens[1]);
				xvnum = atoi(vtokens[2]);
				xvers = strdup(vtokens[3]);

				freetokens(&vtokens, &nvtokens);

				send(e->stream, chan, "%s %d:%d -> %s", xbook, (int) xcnum, (int) xvnum, xvers);

				xvnum = 0;
				xcnum = 0;
				free(xvers);
				xvers = NULL;
				free(xbook);
				xbook = NULL;

			}

			Bibly_FreeVerses(&verses,&nverses);

			DestroyTokens(&ltokens, &nltokens);

		}
	}
}
