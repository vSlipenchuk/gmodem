/*
  common gsm modem functions -
  at command handling
  send & recv bytes
*/

#include "gmodem.h"
#include <string.h>
#include <stdio.h>


int gmodem_init(gmodem *g, char *name) {
vstream *s=&g->port;
  g->now=g->o.modified = os_ticks(); //
  strcpy(g->o.num,"-");
s->p=&vstream_com_procs;
s->handle = s->p->open(name);
if (!s->handle) return 0;
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
int code=0;
if (lcmp(&buf,"RING") && g->o.state == callNone ) { // we have ring indicator?
  gmodem_set_call_state(g,callRing);
  return 1;
  }
if (g->o.state == callRing && lcmp(&buf,"+CLIP:")) {
     buf=trim(buf); printf("TR:<%s>\n",buf);
     buf=get_till(&buf,","); printf("PAR:<%s>\n",buf);
     buf=str_unquote(buf);
     printf("NUM:<%s>\n",buf);
     strNcpy(g->o.num,buf);
  gmodem_set_call_state(g,callPresent); // report on it
  return 1;
  }
if (0==strcmp(buf,"OK")) code=g_ok;
 else if (0==strcmp(buf,"ERROR")) code=g_error;
  else if (0==strcmp(buf,"NO CARRIER")) code = g_no_carrier;

if (code && g->res==0 ) g->res = code; // set global flag
// now - call functions here ...

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
return 1;
}

int gmodem_At(gmodem *g,char *cmd) {
char buf[1024];
sprintf(buf,"at%s\r\n",cmd);
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
if (g->f.c1 || gmodem_At(g,"clip=1")>0) g->f.c1=1;
return g->f.c1;
}

int gmodem_At2buf(gmodem *g,char *cmd,char *out, int size) {
void *proc;
char buf[1024];
int lineno=0;
gmodem_echo_off(g); // check off echo
g->res = 0;
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
sprintf(buf,"at%s\r\n",cmd);
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
sprintf(buf,"at%s\r\n",cmd);
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


int gmodem_ussd(gmodem *g,char *str) { // call string
char buf[1024];
sprintf(buf,"+CUSD=0,\"%s\",15",str);
gmodem_At(g,buf);
// Now - wait for +CUSD:
return 1;
}


