/*
  call related functions
*/

#include "gmodem.h"
#include <stdio.h>
#include <string.h>

int gmodem_kill(gmodem *g) { // kill a call
g->o.release_request=1;
return 1;
}

// 2move -> gmodem_call.c
int gmodem_set_call_state(gmodem *g,int newstate) {
if (g->o.on_call_state && newstate != g->o.state) g->o.on_call_state(g,newstate); // notify
g->o.state=newstate;
g->o.modified=g->now;
if (newstate==0) {
   strcpy(g->o.num,"-"); // unknown
   }
return 0;
}



int gmodem_run2(gmodem *g) { // calls
int cnt = gmodem_run(g);
 //printf("run2:%d call:%d\n",cnt,g->o.state); usleep(100);
int ta = g->now-g->o.modified; // msec in a state ^)
// printf("ta=%d state %d\n",ta,g->o.state); sleep(1);

// now - check a call states
switch(g->o.state) {
 case callRing:

    break;
 case callPresent:
    if (ta>1*1000) { // 3sec - enough to define kill or accept???
        printf("more 3 sec - set call to kill\n");
        gmodem_kill(g); // request to kill
        cnt++;
       }
    break;
 case callActive:
    break;
 case callDisconnecting:
    if (ta>3*1000) {
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
  gmodem_At(g,"+chup");
  g->o.release_request=0;
  gmodem_set_call_state(g,callDisconnecting);
  }
return cnt;
}
