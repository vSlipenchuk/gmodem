/* whell - its a linux command line utility */
#include <stdio.h>
#include <stdlib.h>
#include "gmodem.h"

char szmodem[80]="/dev/ttyUSB1"; // "/dev/gobi/modem"; //"/dev/modem"; // default modem name

gmodem Modem,*m=&Modem; // my main modem

// some utils - defained in common.c
int kbhit2(); int gets_buf2(char *buf,int sz);

int g_on_data(gmodem *g,char *buf, int len) {
printf(">%*.*s",len,len,buf); // just print on a screen!!!
return 0;
}

int g_on_line(gmodem *g,char *buf,int len,int code) { // when lines completes
    if (len>0) printf("{%s}[%d]\n",buf,g->res);
return 0; // do as ypu want
}

#include <unistd.h>
#include <readline/readline.h>
#include <readline/history.h>


//link: readline termcap
int gets_buf(char *buf,int sz) {
char *g = readline(""); // read, no prompt
if (!g)  { *buf=0;return 0; }  // eof
int l = strlen(g);
if (l>=sz-1) l = sz-1;
memcpy(buf,g,l); buf[l]=0;
add_history(buf);
free(g);
return 1; // OK
}

int gmodem_info(gmodem *g) { // collects and prointf info on a screen
char imsi[80],imei[80],iccid[80];
gmodem_At2buf(g,"+cimi",imsi,sizeof(imsi));
gmodem_At2buf(g,"+cgsn",imei,sizeof(imei));
gmodem_At2bufFilter(g,"+ICCID","ICCID",iccid,sizeof(iccid));
printf("info:{imei:'%s',imsi:'%s',iccid:'%s'}\n",imei,imsi,iccid);
return 1; //ok
}

int lcmp(char **s,char *cmd) {
int l=strlen(cmd);
if (memcmp(*s,cmd,l)!=0) return 0;
(*s)+=l;
while ( (*s)[0]==32 ) (*s)++;
return 1;
}

/*
 http://we.easyelectronics.ru/part/gsm-gprs-modul-sim900-chast-vtoraya.html

 +creg?
 +CREG: 2,1,6C03,11F4
 OK
 +CREG: 1,6c03,11F4 - при нотификациях - меньше полей? Ну ни фига себе?

 +csq
 +CSQ: 12,99
 OK

 +COPS? - current operator registred
 OK

 +COPS=? - scaning operatios
 OK

 AT+CCLK=«yy/mm/dd,hh:mm:ss+zz»

 AT+CCLK="12/08/11,12:23:46:00+04"
 gobi:+CCLK="12/08/11,12:23:46"
 +cclk?
 O$QCDMG
K

 at+clip=1
  +ctzu=1 +ctzr=1 time-zone update& report

 +cscb=? Broadcast SMS

telit: moni#,nitz# #snum #i2cwr #sd #ping easyscan-net
  stk: #stgi stgg
  #jdr #gpsp #rsen-remote sim
  opengate(m2m telephinica)
  #SPCM sound-read/write
  AT#NITZ=7,0 -sync realtime cock with network

+CLAC = Help = all-commands

*/


char on_incoming_call[100]="/home/bin/on_incoming_call";

int on_call_state(gmodem *g, int newstate) { // called before new state fires
printf("MODEM[%ld]: newstate: %d\n",g->now,newstate);
if (newstate == callPresent) {
  char buf[1024];
  snprintf(buf,sizeof(buf),"%s %s",on_incoming_call,g->o.num);
  system(buf); // call it!!!
  }
return 0; // do nothing
}

int main() {
    //m->on_data = g_on_data; // print letters on a screen
    m->on_line = g_on_line; // when a line here
    if (gmodem_init(m,szmodem)<=0) {
       fprintf(stderr,"fail open %s\n",szmodem);
       return 1;
       }
    m->o.on_call_state = on_call_state;
    fprintf(stderr,"gmodem %s opened ok\n",szmodem);
    gmodem_clear(m,1000);
    gmodem_echo_off(m);
    gmodem_clip_on(m);
    gmodem_At(m,"+creg=2"); // register & report of changes
    //gmodem_info(m); // callit ???
    fprintf(stderr,"gmodem '%s' cleared and ready (info|console)\n",szmodem);
    while(1) {
       int cnt=0;
       if (m->f.eof) {
           fprintf(stderr,"gmodem EOF reported, Abort.\n");
           break;
          }
       cnt= gmodem_run2(m); // And - check time-outs
       if (cnt==0) msleep(100); // not 100% CPU
       if (kbhit2()) { // run at command here
           char buf[80],*c=buf;
           if (m->f.console) {
               char ch = getc(stdin);
               //printf("ch=%d\n",ch);
               if (ch==27) {
                   fprintf(stderr,"done console mode\n");
                   m->f.console=0;
                   continue;
                  } else   gmodem_put(m,&ch,1);

              } else {
                gets_buf2(buf,sizeof(buf));
           if (buf[0]==0 || 0==strcmp(buf,"exit") || 0==strcmp(buf,"quit")) break;
           if (lcmp(&c,"info")) {
               gmodem_info(m);
               continue;
              }
           if (lcmp(&c,"console")) { // console mode start
               fprintf(stderr,"console mode here, ESC to return back\n");
               m->f.console=1; //
               continue;
              }
            if (lcmp(&c,"ussd")) {
               gmodem_ussd(m,c);
               continue;
              }
           gmodem_At(m,buf); // call At cmd
             }
          }//int gmodem_At2buf(gmodem *g,char *cmd,char *out, int size) {
       }
return 0;
}
