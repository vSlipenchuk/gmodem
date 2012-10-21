/*
  common gsm modem functions -
  at command handling
  send & recv bytes
*/

#include "gmodem.h"
#include <string.h>
#include <stdio.h>
#include "vstrutil.h"


int gmodem_init(gmodem *g, char *name) {
g->name[0]=0;
vstream *s=&g->port;
  g->now=g->o.modified = os_ticks(); //
  strcpy(g->o.num,"-");
s->p=&vstream_com_procs;
s->handle = s->p->open(name);
if (!s->handle) return 0;
strNcpy(g->name,name);
return 1; //ok
}



//+CLIP: "+79151999003",145,,,"     ",0}

int gmodem_scan_str(char **buf,char *out,int sz) {
return 0;
}

int gmodem_scan(char *buf,char *fmt, ... ) {
while(*buf && *fmt) {
  //
  break;
  }
return (*buf==0) && (*fmt==0); // not match
}

#include "vstrutil.h"


int g_modem_do_line(gmodem *g,char *buf,int ll) { // call processing
int code=0; char *cmd = buf;
 if (g->logLevel>3) printf("L<%s>\n",buf);
if (lcmp(&cmd,"RING") && g->o.state == callNone ) { // we have ring indicator?
  gmodem_set_call_state(g,callRing);
  return 1;
  }
if (lcmp(&cmd,"+CUSD:")) { // CUSD responce ???
  strNcpy(g->cusd,cmd);
     // on_cusd ?
  return 1;
  }
if (g->o.state == callRing && lcmp(&cmd,"+CLIP:")) {
     cmd=trim(cmd); //printf("TR:<%s>\n",buf);
     cmd=get_till(&cmd,","); //printf("PAR:<%s>\n",buf);
     cmd=str_unquote(cmd);
     //printf("NUM:<%s>\n",buf);
     strNcpy(g->o.num,cmd);
  gmodem_set_call_state(g,callPresent); // report on it
  return 1;
  }

// status codes - changes flow
char *szCode[]={"OK","CONNECT","ERROR","COMMAND NOT SUPPORT","+CME ERROR",
     "BUSY","NO CARRIER","NO DIAL TONE",0};
int   g_codes[]={g_ok,g_connect,g_error,g_error,g_error,
      g_busy,g_no_carrier,g_no_dial_tone};
int i; for(i=0;szCode[i];i++) if (lcmp(&cmd,szCode[i])) { code=g_codes[i]; break ;}
if (code<0 && g->o.state) { // have active call and reports from a network found
    g->o.release_request=code;
    }
if ( code == g_ok && g->o.state == callSetup ) { // special case -- wait for adt...
     gmodem_set_call_state(g, callInit); // YES!
     return 1;
    }
if ( code == g_connect && (g->o.state == callInit || g->o.state == callSetup) ) {
     gmodem_set_call_state(g, callActive); // YES!
     return 1;
    }
if (code && g->res==0 ) g->res = code; // set global flag
if (g->on_line) g->on_line(g,buf,ll,code); // recollect
return 1; // anyway
}

int gmodem_do_lines(gmodem *g) {
int i;
unsigned char *s = (void*)g->in;
for(i=0;i<g->in_len;i++) if (s[i]=='\n' || s[i]=='\r') break;
if (i<g->in_len || (i==sizeof(g->in)-1))  { // Yes  - do a line !!!
  int ll=i; // now - line trim
  while(ll>0 && s[ll-1]<=32) ll--;
  char buf[sizeof(g->in)]; memcpy(buf,g->in,ll); buf[ll]=0; // gets a copy buffer
  if (i<g->in_len) i++;    memcpy(g->in,g->in+i,g->in_len-i); g->in_len-=i;   g->in[g->in_len]=0; // remove it freom a queue
  g_modem_do_line(g,buf,ll); // call processing
  return 1; // ok
  }
return 0; // not yet
}

int gmodem_run(gmodem *g) {
int cnt=0;
//char buf[1024]; // need collect it !!!
vstream *s=&g->port;
int sz = sizeof(g->in) - g->in_len -1; // free space
int r = 0;
if (g->f.eof) return 0; // NO!
g->now = os_ticks();
if (g->in_len>=0 && sz>0) r = s->p->peek(s->handle,g->in+g->in_len,sz); // if can read - do it
if (r>0) { // yes, read !
  g->in_len+=r;  g->in[g->in_len]=0; // correct it
  if (g->on_data) g->on_data(g,g->in+g->in_len-r,r); // raport
  while (gmodem_do_lines(g)); // process line by line
  //printf("*%*.*s",r,r,buf); // just print on a screen!!!
  cnt++; g->f.idle = 0;
  } else {
  if (r<0) g->f.eof=1; // EOF!
  g->f.idle++;
  }

return cnt;
}


/* send command and wait for result */

int gmodem_put(gmodem *g, char *out,int len) {
vstream *s=&g->port;
if (g->f.eof) return 0;
if (len<0) len=strlen(out);
int r = s->p->write(s->handle,out,len);
if (r<=0) { g->f.eof=1; return 0; } ; //error!
//printf("PUT:{%d}",r);
return 1;
}

int gmodem_At(gmodem *g,char *cmd) {
//char buf[120];
char buf[10];
return gmodem_At2buf(g,cmd,0,0);



//int crlf=3;
//if (g->dev) crlf=g->dev->crlf; // redefine crlf
//sprintf(buf,"at%s\n",cmd);
//crlf=1;
//printf("at crlf:%d\n",crlf);
sprintf(buf,"AT%s%s",cmd,gmodem_crlf(g)); //(crlf==3?"\r\n":"\n"));
if (gmodem_put(g,buf,-1)<=0) return g_eof;
g->res = 0;
while (g->res == 0) {
   if (g->f.eof) return g_eof;
   if (gmodem_run(g) == 0) msleep(100); // wait for answer
   }
return g->res; // anyway ^)
}



// normal main commands
int gmodem_echo_off(gmodem *g) {
if (g->f.e1 || gmodem_At(g,"e0")>0) g->f.e1=1;
return g->f.e1;
}

int gmodem_clip_on(gmodem *g) {
if (g->f.c1 || gmodem_At(g,"+clip=1")>0) g->f.c1=1;
return g->f.c1;
}

int gmodem_At2buf(gmodem *g,char *cmd,char *out, int size) {
int (*proc)();
char buf[256];
int lineno=0;
//gmodem_echo_off(g); // check off echo
g->res = 0;
 memset(out,0,size); size--;
 int on_line(gmodem *g,char *line,int len,int code) {
     if (proc) proc(g,line,len,code); // call prev
     if (len<=0) return 0; // ignore
     if (code==0 && size>0) { // my line, have a buffer
         if (lineno>0) { *out='\n'; out++; size--;};
         lineno++;
         int r = strlen(line); // first line ???
         if (r>size) { r=size; g->f.over++;} // over
         memcpy(out,line,r); size-=r; out+=r;
         //memcpy()
         //printf("Line:<%s>\n",line);
         }
     // here - we are
 return 0; // continue
 }
//int crlf=3; if (g->dev) crlf=g->dev->crlf;
sprintf(buf,"at%s%s",cmd,gmodem_crlf(g));
if (gmodem_put(g,buf,-1)<=0) return g_eof;
proc=g->on_line;
g->on_line = on_line;
while (g->res == 0 ) {
   if (g->f.eof) g->res=g_eof;
   if (gmodem_run(g) == 0) msleep(100); // wait for answer
   }
g->on_line=proc; // restore proc
return g->res; // anyway ^)
}

char *gmodem_crlf(gmodem *g) {
int crlf=3;
if (g->dev) crlf=g->dev->crlf;
switch(crlf) {
 case 1: return "\r"; // E1550 by default!!!
 case 2: return "\n";
}
return "\r\n";
}

int gmodem_At2Lines(gmodem *g,char *cmd,char *filter,int (*on_line_call)(),void *handle) {
void *proc;
char buf[1024];
//int lineno=0;
gmodem_echo_off(g); // check off echo
g->res = 0;
//memset(out,0,size); size--;
 int on_line(gmodem *g,unsigned char *line,int len,int code) {
     if (filter) {
         int fl=strlen(filter);
         if (memcmp(filter,line,fl)!=0) return 0; // not mine
         line+=fl; len-=fl; // remove it
         while(len>0 && line[0]<=32) {line++;len--;}
         if (len>0 && line[0]==':') { line++; len--;}
         while(len>0 && line[0]<=32) {line++;len--;}
         }
     on_line_call(handle,line,len,code); // call it!!!
     /*if (len<=0) return 0; // ignore
     if (code==0 && size>0) { // my line, have a buffer
         if (lineno>0) { *out='\n'; out++; size--;};
         lineno++;
         int r = strlen(line); // first line ???
         if (r>size) { r=size; g->f.over++;} // over
         memcpy(out,line,r); size-=r; out+=r;
         //memcpy()
         //printf("Line:<%s>\n",line);
         }
      */
     // here - we are
 return 0; // continue
 }
sprintf(buf,"at%s%s",cmd,gmodem_crlf(g));
if (gmodem_put(g,buf,-1)<=0) return g_eof;
proc=g->on_line;
g->on_line = on_line;
while (g->res == 0 ) {
   if (g->f.eof) g->res=g_eof;
   if (gmodem_run(g) == 0) msleep(100); // wait for answer
   }
g->on_line=proc; // restore proc
return g->res; // anyway ^)
}

int gmodem_At2bufFilter(gmodem *g,char *cmd,char *filter,char *out, int size) {
//void *proc;
int lineno=0;
memset(out,0,size); size--;
int on_line(gmodem *g,char *line,int len,int code) {
     if (len<=0) return 0; // ignore
     if (code==0 && size>0) { // my line, have a buffer
         if (lineno>0) { *out='\n'; out++; size--;};
         lineno++;
         int r = strlen(line); // first line ???
         if (r>size) { r=size; g->f.over++;} // over
         memcpy(out,line,r); size-=r; out+=r;
         //memcpy()
         //printf("Line:<%s>\n",line);
         }
     // here - we are
 return 0; // continue
 }
int r = gmodem_At2Lines(g,cmd,filter,on_line,g);
return r; // anyway ^)
}


int gmodem_clear(gmodem *g, int ms) { // wait for silecnce for ms... ZU !
int i=0;
gmodem_run(g); // first loop
while(g->f.idle<ms/100) {
  if (gmodem_run(g)<=0) { msleep(100); i+=100;}
  if (i>10*1000) return 0; // too long = 10sec!
  }
return 1; // ok ? anyway?
}


// 0,"normal text",15  -- MTS answer
// 0,"normal text",64  -- Beeline answer
#include "../vos/vos.h"

int gmodem_ussd(gmodem *g,char *str) { // call string
char buf[1024];
sprintf(buf,"+CUSD=1,\"%s\",15",str);
if (gmodem_At(g,buf)<=0) return 0; // fail
int i;
g->cusd[0]=0;
for(i=0;i<30000;i+=100) {
  gmodem_run(g);
  if (g->cusd[0]) {
        char *cmd = g->cusd,*txt;
        get_till(&cmd,",");
          txt=get_till(&cmd,",");
           txt=str_unquote(txt);
        printf("CUSD result: %s\n",txt);
        return 1; // found
        }
  usleep(100*1000);
  }
printf("TimeOut on wait CUSD\n");
// Now - wait for +CUSD:
return -1;
}

int gmodem_imsi(gmodem *g) { // when define imsi - we can start to definde network
char imsi[80];
imsi[0]=0;
//printf("off echo1\n");
gmodem_echo_off(g);
//printf("off echo2, check a pin...\n");
if (gmodem_pin(g,0)!=1) {
    sprintf(g->out,"[pin-not-ready]");
    return -1;
   }
gmodem_At2buf(g,"+cimi",imsi,sizeof(imsi));
//printf("strlen(imsi)=%d IMSI:<%s>\n",strlen(imsi),imsi);
if (strlen(imsi)!=15) return 0; // its is not stupid-style ^), some garbage really returns by SIM-EMU
strNcpy(g->imsi,imsi);
//printf("IMSI:%s\n",g->imsi);
// and now - reset operator
g->oper = gsm_operators; // first is default
gsm_operator *op;
for(op=gsm_operators+1;op->name;op++)
  if (memcmp(op->imsi,g->imsi,strlen(op->imsi))==0) { g->oper=op; break; }
//printf("Found oper:%s for imsi: %s\n",g->oper->name,g->imsi);
strcpy(g->out,g->imsi);
return 1;
}

int gmodem_imei(gmodem *g) {
char imei[20];
imei[0]=0;
gmodem_At2buf(g,"+cgsn",imei,sizeof(imei));
//if (strlen(imsi)!=15) return 0; // its is not stupid-style ^), some garbage really returns by SIM-EMU
strNcpy(g->imei,imei);
strNcpy(g->out,g->imei);
//printf("IMEI:%s\n",g->imei);
extern gsm_device gsm_devices[];
g->dev = gsm_devices; // default = first
gsm_device *dev;
for(dev=gsm_devices+1;dev->name;dev++) if (memcmp(dev->imei,g->imei,strlen(dev->imei))==0 ) {g->dev=dev; break; }
//printf("Found dev:%s for imei:%s crlf:%d\n",g->dev->name,g->imei,g->dev->crlf);
return 1;
}

// sends pin & waits for MSISDN
int gmodem_pin(gmodem *g,char *pin) {
char cpin[80];
int ok;
 cpin[0]=0;
 gmodem_At2bufFilter(g,"+CPIN?","+CPIN",cpin,sizeof(cpin));
 //printf("cpin state: <%s>\n",cpin);
int ready =  strcmp(cpin,"READY")==0;
if (!pin) return ready; // just report it
 // now - 'READY' is OK
 if (!ready) {
   printf("not ready, try enter %s\n",pin);
   char buf[80]; sprintf(buf,"+CPIN=\"%s\"",pin);
   ok = gmodem_At(g,buf);
   if (ok<=0) return -1; // fail
   }
gmodem_imsi(g);
printf("--pin ok, wait for imsi\n");
int i;
gmodem_At(g,""); gmodem_clear(g,1000); // dump OK
for(i=0;i<100;i++) { // 100 sec
  //int k=1;
  if ( gmodem_imsi(g) >0 )  return 1; // here!
  sleep(1);
  //if (gmodem_)
  }
printf("Timeout wait imsi\n");
return -3;
}

int gmodem_balance(gmodem *g) {
char *num=0; // default USSD balance number
if (!g->imsi[0]) gmodem_imsi(g); // update imsi of not yet
gsm_operator *o = g->oper;
if (o && o->ussd_balance) num=o->ussd_balance;
if (!num) {
    printf("balance undefined or operator unknown\n");
    return 0;
  }
return gmodem_ussd(g,num);
}

int gmodem_pppd(gmodem *g,char *opt) { // dial pppd & wait for it...
char *num="*99#"; // default USSD balance number
if (!g->imsi[0]) gmodem_imsi(g); // update imsi of not yet
gsm_operator *o = g->oper;
if (o && o->gprs_num) num=o->gprs_num;
if (!num) {
    printf("balance undefined or operator unknown\n");
    return 0;
  }
char apn[80]; apn[0]=0;
if (o && o->apn) { strNcpy(apn,o->apn);}
char *a=apn,*u=0;
a=strchr(apn,'@'); if (a) { *a=0; a++; u=apn;};
     printf("setup APN %s\n",a);
     char buf[1000];
     sprintf(buf,"+CGDCONT=1,\"IP\", \"%s\"",a);
     gmodem_At(g,buf);

_gmodem_dial(g,num,0);
while(1) { // wile
  if (gmodem_run2(g)<=0) sleep(1);
  if (g->o.state == 0 ) {
      printf("Call Disconnected\n");
      return 0;
      }
  if (g->o.state == callActive ) break;
  }
printf("CALL CONNECTED OK!\n");

sprintf(buf,"pppd %s nodetach  ",g->name);

if (u) {
  char *p = strchr(u,'/'); if (p) { *p=0; p++;};
  if (*u) sprintf(buf+strlen(buf)," user \"%s\" ",u);
  if (p && *p) sprintf(buf+strlen(buf)," password \"%s\" ",p);
  }

if (opt && *opt) {
  sprintf(buf+strlen(buf)," %s",opt);
  }
printf("Start exec: %s\n",buf);
extern int system(char *cmd);

int code = system(buf);
//system("pppd  /dev/ttyACM0 nodetach debug user \"beeline\" password \"beeline\""); /// Ну почти заработало?
// whell - need to do it on other thread...
sprintf(g->out,"pppd:%d",code);
//printf("PPPD DONE code=%d\n",code);

// now - wait for that... and run system(pppd)???
//return gmodem_ussd(g,num);
return 1;
}



