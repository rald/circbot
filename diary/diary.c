#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdarg.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>


#include "dyad.h"

#define ID_FILE "id.txt"
#define DIARY_FILE "diary.txt"

#define UTIL_IMPLEMENTATION
#include "util.h"



static char *server  = "sakura.jp.as.dal.net";
static char *channels = "#pantasya";
static char *nick = "siesto";
static char *pass = NULL;
static int  isRegistered = 0;



static void send(dyad_Stream *stream,char *fmt, ...) {
    va_list args;
    va_start(args,fmt);
    dyad_vwritef(stream, fmt, args);
    va_end(args);
    sleep(1);
}



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
        dyad_writef(e->stream, "JOIN %s\r\n", channels);
        isRegistered = 1;
    }

    if(sscanf(e->data,":%31[^!]!~%31[^@]@%256s PRIVMSG %31s :%512[^\n]",nick,user,serv,chan,body)==5) {
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
            send(e->stream, "PRIVMSG %s :%s:msg id %d\r\n", chan, nick,id);
            fh=fopen(ID_FILE,"w");
            fprintf(fh,"%zu\n",++id);
            fclose(fh);
        } else if(sscanf(body,".diary read %zu",&id)==1) {
            fh=fopen(DIARY_FILE,"r");
            while((rlen=getline(&line,&llen,fh))!=-1) {
                cnt++;
                if(cnt==id) {
                    send(e->stream, "PRIVMSG %s :%s:%d:%s\r\n", chan, nick, id, line);
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
                        send(e->stream, "PRIVMSG %s :%s:%s\r\n", chan, nick, tmp);
                        tmp[0]='\0';
                    }
                }
                free(upr2);
                upr2=NULL;
                free(upr1);
                upr1=NULL;
            }
            if(*tmp) {
                send(e->stream, "PRIVMSG %s :%s:%s\r\n", chan, nick, tmp);
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
