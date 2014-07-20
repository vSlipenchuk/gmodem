/* whell - its a linux command line utility */
#include <stdio.h>
#include <stdlib.h>
#include "gmodem.h"

char szmodem[80]="/dev/modem"; // "/dev/gobi/modem"; //"/dev/modem"; // default modem name
char voice[80]; // /dev/ttyUSB1 (if first ppp-port is /dev/ttyUSB0)
char szmon[80]; // monitor port
int no_init = 0;
char exec_cmd[512]; // exec command
char on_in_call[512]; // on in call
int gmode = 0; // phoenix mode - default off

gmodem Modem,*m=&Modem; // my main modem

// some utils - defained in common.c
int kbhit2(); int gets_buf2(char *buf,int sz);

int g_on_data(gmodem *g,char *buf, int len) {
printf(">%*.*s",len,len,buf); // just print on a screen!!!
return 0;
}

int intmode=0; // interactive???

int g_on_line(gmodem *g,char *buf,int len,int code) { // when lines completes
    if (len>0 && ( intmode || g->logLevel>4)) printf("{%s}[%d]\n",buf,g->res);
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

#include <stdarg.h>

int systemf(char *fmt, ...) {
char buf[1024];
BUF_FMT(buf,fmt);
return system(buf);
}


int on_call_state(gmodem *g, int newstate) { // called before new state fires
printf("MODEM: newstate: <%d> num:%s time=%ld\n",newstate,g->o.num,newstate,g->o.modified);
switch(newstate) {
 case callPresent: // now we have a call?
  return systemf("%s %s",on_in_call,g->o.num);
 case callNone:
   printf("CALL DONE\n");
   break;
  }
return 0; // do nothing
}


#include <getopt.h>

char szVersion[20];

void usage(char *name) {
fprintf(stderr,"usage: %s version %s\n"
        "\t-h print this message\n"
        "\t-m <modem>    or ---modem=<modem>   (default: /dev/modem)\n"
        "\t-M <monitor>  or ---monitor=<modem> (default: empty)\n"
        "\t-V <port>     or --voice=<port>\n"
        "\t-o          or --no-init\n"
        "\t-X          or --phoenix phoenix card reader\n"
        "\t-e <cmd>    or --exec=<command>\n"
        "\t-i <runcmd> or --on_in_call=<shell_command>\n"
        ,name,szVersion);
}

#include "vstrutil.h"

void parse_options(int argc,char **argv) {
int optIdx = 0, c=0;
sprintf(szVersion,"%d.%d.%d.%d",gmodem_version);
while (1){
  static struct option long_opt[] = {
                    {"help",  0, 0, 'h'},
                    {"modem", 1, 0, 'm'},
                    {"monitor", 1, 0, 'M'},
                    {"no-init",0,0, 'o'},
                    {"exec",   1,0, 'e'},
                    {"on_in_call",   1,0, 'i'},
                    {"voice", 1,0, 'V'},
                    {"phoenix", 0,0, 'X'},
                    {0,0,0,0}
                   };
  if((c = getopt_long(argc, argv, "m:h:M:oe:i:V", long_opt, &optIdx)) == -1) {
     // printf("Done, index=%d optopt=%d\n",optIdx,optind);
   break;
  }
  //printf("ch=%c here\n",c);

  switch( c ){
     case 'h':
          usage(argv[0]);
          exit(-1);
     case 'm':
          //printf("option 'm' selected, filename: %s\n", optarg);
          strNcpy(szmodem,optarg);
          //return(1);
          break;
     case 'M':
           strNcpy(szmon,optarg);
           break;
     case 'o':
           no_init = 1; //atoi(optarg);
           break;
     case 'i':
           strNcpy(on_in_call,optarg);
           break;
     case 'e':
            strNcpy(exec_cmd,optarg);
            break;
     case 'V':
            //printf("HERE:%s\n",optarg);
            strNcpy(voice,optarg);
            break;
     case 'X':
            gmode = 1;
            no_init = 1;
            break;
     default:
          usage(argv[0]);
          exit(-1);
   }
 }
}

int in_pppd = 0;

int set_echo(int echo);
//int pa_thread(void *);

#include "voice_stream.h"

voice_stream VS;

int main(int argc, char **argv) {
//return sec_test();
    parse_options(argc,argv);
    printf("Done parse\n");

    //m->on_data = g_on_data; // print letters on a screen
    m->on_line = g_on_line; // when a line here
    if (gmodem_init(m,szmodem)<=0) {
       fprintf(stderr,"%s :: fail open %s. try --help for options\n",argv[0],szmodem);
       return 1;
       }
    if (szmon[0]) {
       m->mon = calloc( 1, sizeof(gmodem));
       m->mon->parent = m;
       if (gmodem_init(m->mon,szmon)<=0) {
         fprintf(stderr,"%s :: fail open %s as monitor\n",argv[0],szmon);
         }
       m->mon->on_line = g_on_line;
       }
    m->o.on_call_state = on_call_state;
//printf("VOICE=%s\n",voice);
if (voice[0]) {
       voice_stream *vs = &VS;
       vs->name = voice; // same name for pulseaudio channel
       vs->comName = voice ;// comPort name
       m->voice = vs; // just to be not
       //thread_create(pa_thread,voice) ; // ZUZUKA - test of voice thread
       printf(">>>Begin voice_init vs=%x\n",vs);
       if (voice_init(vs)<=0) {
          printf("Voice Init Failed\n");
          m->voice = 0;
          }
        printf("done voice_init\n");
       }
    fprintf(stderr,"gmodem %s opened ok\n",szmodem);
if (gmode == 1) m->mode = 1; // set phoenix mode
    if (! no_init) {
     gmodem_clear(m,1000);
     gmodem_echo_off(m);
     gmodem_info(m);
     gmodem_clip_on(m);
     gmodem_At(m,"+creg=2"); // register & report of changes
     }

if (gmode == 1) gmodem_atr(m); // call ATR

    int i; for(i=optind;i<argc;i++) { // process all commands one-by one
        char *cmd = argv[i];
        gmodem_echo_off(m);
        gmodem_run2(m);
        m->out[0]=0;
        if (strcmp(cmd,"exit")==0) exit(0); // OK
        int ok = gmodem_cmd(m,cmd);
        if (ok<=0) {
             fprintf(stderr,"gmodem fail[%d]{%s} exec '%s'. abort.\n",ok,m->out,cmd);
             exit(1); // fail
             }
        printf("%s\n",m->out); // auto-mode
        }
    //gmodem_info(m); // callit ???
    fprintf(stderr,"gmodem '%s' ready, usage: \n (info|console|+clac|balance|pppd|ussd<num>|sms<num><text>)\n",szmodem);
    while(1) {
       int cnt=0;
       if (m->f.eof && !in_pppd) {
           fprintf(stderr,"gmodem EOF reported, Abort.\n");
           break;
          }
       cnt= gmodem_run2(m); // And - check time-outs
       if (cnt==0) msleep(100); // not 100% CPU
       if (kbhit2()) { // run at command here
           char buf[256],*c=buf;
           if (m->f.console) {
               char ch = getc(stdin);
               //printf("ch=%d\n",ch);
               if (ch==27) {
                   fprintf(stderr,"done console mode\n");
                   set_echo(0);
                   m->f.console=0;
                   continue;
                  } else if (ch =='\n')
                     gmodem_put(m,gmodem_crlf(m),-1);
                  else   gmodem_put(m,&ch,1);

              } else {
                gets_buf2(buf,sizeof(buf));
           if (buf[0]==0 || 0==strcmp(buf,"exit") || 0==strcmp(buf,"quit")) break;

           int ok;
           m->out[0]=0;
           ok = gmodem_cmd(m,c);
           if (ok) {
              printf("{%d}{%s} result for {%s}\n",ok,m->out,buf);
              continue;
              }
           if (lcmp(&c,"pin")) {
                gmodem_pin(m,c);
                continue;
              }


           if (lcmp(&c,"console")) { // console mode start
               fprintf(stderr,"console mode here, ESC to return back\n");
               set_echo(1);
               m->f.console=1; //
               continue;
              }
             printf("unknown command, skip: '%s'\n",c);
             }
          }//int gmodem_At2buf(gmodem *g,char *cmd,char *out, int size) {
       }
// restore console???
set_echo(1);
return 0;
}
