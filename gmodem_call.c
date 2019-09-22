/*
  call related functions
*/

#include "gmodem.h"
#include <stdio.h>
#include "vstrutil.h"

/* default handling call values */
int toPresent = 0; // wait ringing
int toAnswer  = 10;  // autoanswer (no yet)
int toActive =  40; // max conversation



//int lcmp(uchar **str,uchar *cmp); //@strutil.c

//extern int incall; // temporary garbage - voice calls debug


int gmodem_call_callback(gmodem *g, char *cmd) { // check - if it is a call notification?
if (lcmp(&cmd,"RING")) {
    if (g->o.state == callNone) gmodem_set_call_state(g,callRing); // till 3 ring - we wait for a CLI
    return 1;
   }
if (lcmp(&cmd,"+CLIP:")) { // check - if call report ed...
   char *num = gmodem_par((char**)&cmd,0); // text
   int code =  gmodem_parInt((char**)&cmd,0,0);
   if (num[0] && g->o.num[0]=='-' ) {
      char sz[100];
      if (code==145) sprintf(sz,"+%s",num);
                else strcpy(sz,num);
      strNcpy(g->o.num,sz);
      } // set new code
   gmodem_set_call_state(g,callPresent);
   return 1;
   }
if (lcmp(&cmd,"^CONN:")) { // E1550 connect notification
    //incall=1;
    //printf("---- in call now ----\n");
    gmodem_set_call_state(g,callActive);
    return 1;
   }
if (lcmp(&cmd,"^CEND:")) {
    gmodem_set_call_state(g,callNone); // or other?
    return 1;
    }
return 0; // not mine
}

//char ATH[10]="+CHUP"; // or H

int gmodem_kill(gmodem *g) { // kill a call

gmodem_set_call_state(g,callDisconnecting); // or other?
//gmodem_At(g,"+CHUP");
gmodem_put(g,"ATH\r\n",-1); // ZUZUKA - not every time !
//gmodem_put(g,"AT+CHUP\r\n",-1);
g->o.release_request=1;
//g->o.dur=0; // last duration
voice_stream *v = g->voice;
if (v) v->incall=0;
//incall=0;
return 1;
}

// 2move -> gmodem_call.c
int gmodem_set_call_state(gmodem *g,int newstate) {
int st = g->o.state;
if (g->o.state==newstate) return 1;
voice_stream *v = g->voice;
gmodem_logf(g,3,"set_call_state new=%d old=%d\n",newstate,g->o.state);
//if (newstate == g->o.state) return 1; // ok, already
switch(newstate) {
   case callRing:
      if (st == callPresent || st == callActive) return 0; // just ignore
      if (v) v->incall=1;
       // just caport on a new call
      break;
   case callPresent:
       // ok - data collected (CLI, response OK)
      break;
   case callActive:
      // it is - connect event
      gmodem_logf(g,3,"now call active");
      if (v) v->incall=1;
      g->o.connected = g->now;
      break;
   case callNone:
      strcpy(g->o.num,"-"); // on
      g->o.ring=0; g->o.dur = 0; // ring counter, call duration
      voice_stream *v = g->voice;
      if (v) v->incall=0;
      break;
   }
int oldstate = g->o.state; g->o.state=newstate; g->o.modified=g->now;
if (g->o.on_call_state && (newstate!=oldstate)) g->o.on_call_state(g,newstate,oldstate); // notify
return 1;
}

int gmodem_answer(gmodem *g) {
if ( (g->o.state != callPresent) && (g->o.state!=callRing) ) {
  sprintf(g->out,"state %d invalid for answer",g->o.state);
  return 0;
  }
if (gmodem_Atf(g,"a")>0) {
  gmodem_set_call_state(g,callActive);
  if (g->voice) {
        // need extra call
       gmodem_At(g,"^DDSETEX=2"); // ZU -
  }
  return 1;
  }
sprintf(g->out,"modem declined answer on 'ata'");
return 0;
}



int gmodem_run2(gmodem *g) { // calls
int cnt = gmodem_run(g);
 //printf("run2:%d call:%d\n",cnt,g->o.state); usleep(100);
int ta = g->now-g->o.modified; // msec in a state ^)
// printf("ta=%d state %d\n",ta,g->o.state); sleep(1);
    int dur = ta/1000;
    static int dur0 = -1;
// now - check a call states
switch(g->o.state) {
 case callRing:
    if (ta>3*1000) { // no clip anyway open  it
       if (g->o.num[0]==0) strcpy(g->o.num,"-"); // unknown number
       printf("run2 -> callRing changed to callpresent by timeoyut 3 sec (real %d)\n",ta);
       gmodem_set_call_state(g,callPresent);
       cnt++;
       }
    break;
 case callInit: // when AT send but no OK in responce
     //
    break;
 case callSetup: // OK here, but connect is not yet...
    //
    break;
 case callPresent:
    if (dur0!=dur) { printf("Present %d sec\n",dur);   dur0=dur; g->o.dur=dur0; }
    if ((toPresent>0) && (ta>1000*toPresent)) { // 3sec - enough to define kill or accept???
        printf("more %d sec preseting - set call to kill have ta=%d ====== \n",dur,toPresent);
        gmodem_kill(g); // request to kill
        cnt++;
       } else if (ta>1000*toAnswer) {
        printf("Need answer!\n");
        g->out[0]=0;
        int ok  = gmodem_answer(g);
        printf("ANSWER=%d out=%s\n",ok,g->out);
        }
    break;
 case callActive:
     if (dur0!=dur) {
            voice_stream *v = g->voice;
            printf("Active %d sec read=%d mic=%d\n",dur,v?v->push_bytes:-1,v?v->rec_bytes:-1);   dur0=dur;}
    if (ta>1000*toActive) { // 3sec - enough to define kill or accept???
        printf("more toActive %d - set call to kill have ta=%d ====== \n",toActive,ta);
        gmodem_kill(g); // request to kill
        cnt++;
       }
    break;
 case callDisconnecting:
    if (ta>3*1000) { // thats a clear interval
       printf("Think its cleared ok\n");
       gmodem_set_call_state(g,callNone);
       cnt++;
       }
    break;
 case callNone:
    break;
 }
if (g->o.release_request) {
  printf("kill by release request %d\n",g->o.release_request);
  //gmodem_At(g,"h0");
  g->o.release_request=0;
  gmodem_set_call_state(g,callDisconnecting);
  //char buf[1024];
    //sprintf(buf,"atd%s;\r\n",num);
  gmodem_put(g,"at+chup\r\n",-1); // if we are in callback...???

  //gmodem_At(g,"+chup");
  }
if (g->cmt && g->on_mt) g->on_mt(g);
return cnt;
}

int _gmodem_dial(gmodem *g,char *num,int voice) {
voice_stream *v = g->voice;
if (g->o.state != callNone) {
  printf("not null call state, skip dial\n");
  return 0;
  }
char buf[1024];
sprintf(buf,"atd%s%s\r\n",num,voice?";":"");
if (gmodem_put(g,buf,-1)<=0) return g_eof;
gmodem_set_call_state(g,callSetup);
strNcpy(g->o.num,num);
printf("Start call setup\n");
if (v) v->incall=1;
return 1;
}

int gmodem_dial(gmodem *g,char *num) {
    if (!*num)
    num="0611"; // temp beeline callcentre
     //num="88002004064"; // rostelec conf
     if (g->o.state != callNone) {
        printf("not null call state, skip dial\n");
        return 0;
        }
    int dmode = g->dev?g->dev->dmode:0; // dialmode
    int ok;
    if (dmode==0) {ok = gmodem_Atf(g,"d%s;",num);
                    if (ok>0) gmodem_set_call_state(g,callSetup);
                   }
               else ok = _gmodem_dial(g,num,1);

    if (ok<=0) return -1;

    if (g->voice) {
        // need extra call
        gmodem_put(g,"AT^DDSETEX=2\r\n",-1);
         //ok = gmodem_At(g,"^DDSETEX=2"); // ZU -
       }
    return 1; //OK
}

int gmodem_dtmf(gmodem *g, char *c) {
for(;*c;c++) if (gmodem_Atf(g,"+vts=%c",*c)<=0) return -2;
return 1; // all ok
}

/*
int gmodem_kill(gmodem *g,char *num) {
    if (!*num)
    //num="0611"; // temp beeline callcentre
     //num="88002004064"; // rostelec conf
    gmodem_Atf(g,"AT+CHUP",num);
    return 1; //OK
}
*/
