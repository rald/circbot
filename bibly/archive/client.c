#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <sys/types.h>

#include "dyad.h"

#define ID_FILE "id.txt"
#define DIARY_FILE "diary.txt"

#define UTIL_IMPLEMENTATION
#include "util.h"

/* A simple IRC bot. Connects to an IRC network, joins a channel then sits
 * idle, responding to the server's PING messges and printing everything the
 * server sends it. */

static char *server  = "irc.dal.net";
static char *channel = "#pantasya";
static char *nick = "siesto";
static char *pass = "paanoanggagawinko";
static int  isRegistered = 0;

char **tokens=NULL;
size_t ntokens=0;



static void onConnect(dyad_Event *e) {
    dyad_writef(e->stream, "PASS %s\r\n", pass);
    dyad_writef(e->stream, "NICK %s\r\n", nick);
    dyad_writef(e->stream, "USER %s %s %s :%s\r\n", nick, nick, nick, nick);
}



static void onError(dyad_Event *e) {
    printf("error: %s\n", e->msg);
}



static void onLine(dyad_Event *e) {

    char nick[32], user[32], serv[256], chan[32], body[512];

    printf("%s\n", e->data);

    /* Handle PING */
    if (!memcmp(e->data, "PING", 4)) {
        dyad_writef(e->stream, "PONG%s\r\n", e->data + 4);
    }

    /* Handle RPL_WELCOME */
    if (!isRegistered && strstr(e->data, "001")) {
        /* Join channel */
        dyad_writef(e->stream, "JOIN %s\r\n", channel);
        isRegistered = 1;
    }

    if(sscanf(e->data,":%31[^!]!~%31[^@]@%256s PRIVMSG #%31s :%512[^\n]",nick,user,serv,chan,body)==5) {
        char msg[512];
        size_t id;
        FILE *fh;
        char *line=NULL;
        size_t llen=0;
        ssize_t rlen=0;
        size_t cnt=0;

        if(sscanf(body,".diary write %[^\n]",msg)==1) {
            fh=fopen(ID_FILE,"r");
            fscanf(fh,"%zu\n",&id);
            fclose(fh);
            fh=fopen(DIARY_FILE,"a");
            fprintf(fh,"%s\n",msg);
            fclose(fh);
            dyad_writef(e->stream, "PRIVMSG %s :%s:msg id %d\r\n", channel, nick,id);
            fh=fopen(ID_FILE,"w");
            fprintf(fh,"%zu\n",++id);
            fclose(fh);
        } else if(sscanf(body,".diary read %zu",&id)==1) {
            fh=fopen(DIARY_FILE,"r");
            while((rlen=getline(&line,&llen,fh))!=-1) {
                cnt++;
                if(cnt==id) {
                    dyad_writef(e->stream, "PRIVMSG %s :%s:%d:%s\r\n", channel, nick, id, line);
                    break;
                }
            }
            fclose(fh);
        } else if(sscanf(body,".diary find %[^\n]",msg)==1) {
            char tmp[512]="";
            char scnt[8]="";
            bool first=true;
            fh=fopen(DIARY_FILE,"r");
            while((rlen=getline(&line,&llen,fh))!=-1) {
                cnt++;
                char *upr1=strupr(trim(line));
                char *upr2=strupr(trim(msg));
                if(strstr(line,msg)) {
                    sprintf(scnt,"%zu",cnt);
                    if(strlen(nick)+strlen(tmp)+strlen(scnt)+2<256) {
                        if(first) first=false;
                        else strcat(tmp,",");
                        strcat(tmp,scnt);
                    } else {
                        dyad_writef(e->stream, "PRIVMSG %s :%s:%s\r\n", channel, nick, tmp);
                        tmp[0]='\0';
                    }
                }
                free(upr2);
                upr2=NULL;
                free(upr1);
                upr1=NULL;
            }
            if(*tmp) {
                dyad_writef(e->stream, "PRIVMSG %s :%s:%s\r\n", channel, nick, tmp);
                tmp[0]='\0';
            }

            fclose(fh);
        }
    }
}



int main(void) {
    dyad_Stream *s;
    dyad_init();

    s = dyad_newStream();
    dyad_addListener(s, DYAD_EVENT_CONNECT, onConnect, NULL);
    dyad_addListener(s, DYAD_EVENT_ERROR,   onError,   NULL);
    dyad_addListener(s, DYAD_EVENT_LINE,    onLine,    NULL);
    dyad_connect(s, server, 6667);

    while (dyad_getStreamCount() > 0) {
        dyad_update();
    }

    dyad_shutdown();
    return 0;
}
