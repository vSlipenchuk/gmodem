/*
  common gsm modem functions -
  at command handling
  send &  bytes

*/

#include "gmodem.h"
#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#include "vstrutil.h"
#include "coders.h"


/*
         foreground background
black        30         40
red          31         41
green        32         42
yellow       33         43
blue         34         44
magenta      35         45
cyan         36         46
white        37         47

reset             0  (everything back to normal)
bold/bright       1  (often a brighter shade of the same colour)
underline         4
inverse           7  (swap foreground and background colours)
bold/bright off  21
underline off    24
inverse off      27

*/

char *gmodem_color_out="\033[21;37m"; // dark-white
char *gmodem_color_in ="\033[21;36m"; // dark-cyan
char *gmodem_color_ok ="\033[1;32m"; // green
char *gmodem_color_err ="\033[1;31m"; // red
char *gmodem_color_none="\033[0m";


char *cme_code_desc(int code) {
switch (code) {
case 0: return "Phone failure";
case 1: return "No connection to phone";
case 2: return "Phone adapter link reserved";
case 3: return "Operation not allowed";
case 4: return "Operation not supported";
case 5: return "PH_SIM PIN required";
case 6: return "PH_FSIM PIN required";
case 7: return "PH_FSIM PUK required";
case 10 : return "SIM not inserted";
case 11 : return "SIM PIN required";
case 12 : return "SIM PUK required";
case 13 : return "SIM failure";
case 14 : return "SIM busy";
case 15 : return "SIM wrong";
case 16 : return "Incorrect password";
case 17 : return "SIM PIN2 required";
case 18 : return "SIM PUK2 required";
case 20 : return "Memory full";
case 21 : return "Invalid index";
case 22 : return "Not found";
case 23 : return "Memory failure";
case 24 : return "Text string too long";
case 25 : return "Invalid characters in text string";
case 26 : return "Dial string too long";
case 27 : return "Invalid characters in dial string";
case 30 : return "No network service";
case 31 : return "Network timeout";
case 32 : return "Network not allowed, emergency calls only";
case 40 : return "Network personalization PIN required";
case 41 : return "Network personalization PUK required";
case 42 : return "Network subset personalization PIN required";
case 43 : return "Network subset personalization PUK required";
case 44 : return "Service provider personalization PIN required";
case 45 : return "Service provider personalization PUK required";
case 46 : return "Corporate personalization PIN required";
case 47 : return "Corporate personalization PUK required";
case 48 : return "PH-SIM PUK required";

case 100 : return "Unknown error";
case 103 : return "Illegal MS";
case 106 : return "Illegal ME";
case 107 : return "GPRS services not allowed";
case 111 : return "PLMN not allowed";
case 112 : return "Location area not allowed";
case 113 : return "Roaming not allowed in this location area";
case 126 : return "Operation temporary not allowed";
case 132 : return "Service operation not supported";
case 133 : return "Requested service option not subscribed";
case 134 : return "Service option temporary out of order";
case 148 : return "Unspecified GPRS error";
case 149 : return "PDP authentication failure";
case 150 : return "Invalid mobile class";
case 256 : return "Operation temporarily not allowed";
case 257 : return "Call barred";
case 258 : return "Phone is busy";
case 259 : return "User abort";
case 260 : return "Invalid dial string";
case 261 : return "SS not executed";
case 262 : return "SIM Blocked";
case 263 : return "Invalid block";
case 772 : return "SIM powered down";
}
return "";
}

char *cms_code_desc(int code) {
switch(code) {
case 1 : return "Unassigned number";
case 8 : return "Operator determined barring";
case 10 : return "Call bared";
case 21 : return "Short message transfer rejected";
case 27 : return "Destination out of service";
case 28 : return "Unindentified subscriber";
case 29 : return "Facility rejected";
case 30 : return "Unknown subscriber";
case 38 : return "Network out of order";
case 41 : return "Temporary failure";
case 42 : return "Congestion";
case 47 : return "Recources unavailable";
case 50 : return "Requested facility not subscribed";
case 69 : return "Requested facility not implemented";
case 81 : return "Invalid short message transfer reference value";
case 95 : return "Invalid message unspecified";
case 96 : return "Invalid mandatory information";
case 97 : return "Message type non existent or not implemented";
case 98 : return "Message not compatible with short message protocol";
case 99 : return "Information element non-existent or not implemente";
case 111 : return "Protocol error, unspecified";
case 127 : return "Internetworking , unspecified";
case 128 : return "Telematic internetworking not supported";
case 129 : return "Short message type 0 not supported";
case 130 : return "Cannot replace short message";
case 143 : return "Unspecified TP-PID error";
case 144 : return "Data code scheme not supported";
case 145 : return "Message class not supported";
case 159 : return "Unspecified TP-DCS error";
case 160 : return "Command cannot be actioned";
case 161 : return "Command unsupported";
case 175 : return "Unspecified TP-Command error";
case 176 : return "TPDU not supported";
case 192 : return "SC busy";
case 193 : return "No SC subscription";
case 194 : return "SC System failure";
case 195 : return "Invalid SME address";
case 196 : return "Destination SME barred";
case 197 : return "SM Rejected-Duplicate SM";
case 198 : return "TP-VPF not supported";
case 199 : return "TP-VP not supported";
case 208 : return "D0 SIM SMS Storage full";
case 209 : return "No SMS Storage capability in SIM";
case 210 : return "Error in MS";
case 211 : return "Memory capacity exceeded";
case 212 : return "Sim application toolkit busy";
case 213 : return "SIM data download error";
case 255 : return "Unspecified error cause";
case 300 : return "ME Failure";
case 301 : return "SMS service of ME reserved";
case 302 : return "Operation not allowed";
case 303 : return "Operation not supported";
case 304 : return "Invalid PDU mode parameter";
case 305 : return "Invalid Text mode parameter";
case 310 : return "SIM not inserted";
case 311 : return "SIM PIN required";
case 312 : return "PH-SIM PIN required";
case 313 : return "SIM failure";
case 314 : return "SIM busy";
case 315 : return "SIM wrong";
case 316 : return "SIM PUK required";
case 317 : return "SIM PIN2 required";
case 318 : return "SIM PUK2 required";
case 320 : return "Memory failure";
case 321 : return "Invalid memory index";
case 322 : return "Memory full";
case 330 : return "SMSC address unknown";
case 331 : return "No network service";
case 332 : return "Network timeout";
case 340 : return "No +CNMA expected";
case 500 : return "Unknown error";
case 512 : return "User abort";
case 513 : return "Unable to store";
case 514 : return "Invalid Status";
case 515 : return "Device busy or Invalid Character in string";
case 516 : return "Invalid length";
case 517 : return "Invalid character in PDU";
case 518 : return "Invalid parameter";
case 519 : return "Invalid length or character";
case 520 : return "Invalid character in text";
case 521 : return "Timer expired";
case 522 : return "Operation temporary not allowed";
case 532 : return "SIM not ready";
case 534 : return "Cell Broadcast error unknown";
case 535 : return "Protocol stack busy";
case 538 : return "Invalid parameter";
}
return "";
}


int gmodem_port_speed = 115200;
extern gsm_device gsm_devices[];

int gmodem_init(gmodem *g, char *name) {
g->name[0]=0;
vstream *s=&g->port;
  g->now=g->o.modified = os_ticks(); //
  strcpy(g->o.num,"-");
s->p=&vstream_com_procs;
if (!g->dev) g->dev = gsm_devices; // default - first device
// printf("Open %s\n",name);
s->handle = s->p->open(name,gmodem_port_speed);
// printf("Done handle %d\n",s->handle);
if (!s->handle) return 0;
strNcpy(g->name,name);
g->msgRef=rand()%100; //random()%200; // randoize first msgReference
g->bin_len = -1; // strlen (g->bin)
return 1; //ok
}

int  gmodem_outf(gmodem *g,int res, char *fmt,...) { BUF_FMT(g->out,fmt); return res;}

void gmodem_logf(gmodem *g,int level, char *fmt,...) {
if (g->logLevel<level) return;
uchar buf[1024];
BUF_FMT(buf,fmt);
while(level>0) { printf(" "); level--;} // indents
 printf("%s\n",buf);
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

extern int incall;


int gmodem_spam_callback(gmodem *g,char *cmd) { // very annoing
if (*cmd==0) { // empty
    return 1;
   }
if (lcmp(&cmd,"^RSSI:")) { // e1550
  return 1;
  }
if (lcmp(&cmd,"^BOOT:")) {
  return 1;
  }
if (lcmp(&cmd,"+HTTPREAD:")) { // it is preamble of HTTPREAD <len>
    return 1;
  }
return 0;
}

int gmodem_ignore_stdresp = 0;
int gmodem_call_callback(gmodem *g, char *cmd);
int buf2file(uchar *data,int len,uchar *filename); // some utils...

int g_modem_do_line(gmodem *g,uchar *buf,int ll) { // call processing
int code=0; char *cmd = buf;
if (g->logLevel>2 && buf[0]) printf(">>%s %s%s\n",gmodem_color_in,buf,gmodem_color_none); // gmodem_recv

gmodem_broadcast(g,buf) ; // Broadcast ALL websockets

code = gmodem_spam_callback(g,buf);
if (code) {
   if (g->logLevel>10) printf("<%s>\n",buf);
   return 1; // anyway
   }
if (g->logLevel>5) printf("<%s>\n",buf); // rest
code = gmodem_call_callback(g,cmd);  if (code) return code; // call notify specific

if (lcmp(&cmd,"+CUSD:")) { // CUSD responce ???
  strNcpy(g->cusd,cmd);
  if (g->parent) strcpy(g->parent->cusd,g->cusd);
     // on_cusd ?
  return 1;
  }

if (lcmp(&cmd,"+HTTPACTION:")) { // CUSD responce ???
  int code,resp,len;
    //printf("CALLBACK FOUND: %s\n",cmd);
  if (sscanf(cmd,"%d,%d,%d",&code,&resp,&len)>=2)  { g->http_action = resp; g->http_len=len;}
  return 1;
  }

if (lcmp(&cmd,"+CMTI:")) { // EW77 Huawei sms notification
  g->cmt++;
  }
if (lcmp(&cmd,"+CDSI:")) { // treat delivery reports as a new messages
  g->cmt++;
  }
if (lcmp(&cmd,"+CRTDCP:")) { // non-ip trafic notifications
    char msg[256];
    //REST: 0,4,31313132
    int ok,pdp,len;
    ok = gmodem_scan2(cmd,"%u,%u,%S",&pdp,&len,msg);
    if (ok ==3) {
            hexstr2bin(msg,msg,-1);
            printf("[[%s%s%s]]\n",gmodem_color_ok,msg,gmodem_color_none);
            buf2file(msg,strlen(msg),".mt.rep");
            }
           else if (ok !=1)  // if not just reporting to AT+CRTDSP? it is a fail parsing
              printf("%s%s%s ; FAIL scan data from +CRTDCP ok=%d\n",gmodem_color_err,cmd,gmodem_color_none,ok);
  }
if (lcmp(&cmd,"+CSONMI:")) { // simcom sim7020E udp incomnng notifications
  char msg[256];
  int ok,pdp,len;
    ok = gmodem_scan2(cmd,"%u,%u,%S",&pdp,&len,msg);
    if (ok ==3) {
            hexstr2bin(msg,msg,-1);
            printf("[[%s%s%s]]\n",gmodem_color_ok,msg,gmodem_color_none);
            buf2file(msg,strlen(msg),".mt.udp");
            }
           else if (ok !=1)  // if not just reporting to AT+CRTDSP? it is a fail parsing
              printf("%s%s%s ; FAIL scan data from +CSONMI ok=%d\n",gmodem_color_err,cmd,gmodem_color_none,ok);
  }
// status codes - changes flow
char *szCode[]={"OK","CONNECT","ERROR","COMMAND NOT SUPPORT","+CME ERROR","+CMS ERROR",
     "BUSY","NO CARRIER","NO DIAL TONE",0};
int   g_codes[]={g_ok,g_connect,g_error,g_error,g_error,g_error,
      g_busy,g_no_carrier,g_no_dial_tone};
char *pcode = cmd;
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
if (code && g->res==0 && gmodem_ignore_stdresp == 0) {
    g->res = code; // set global flag
    if (code == g_error) { strNcpy(g->out,pcode); } // if we have error -> fix out descriptions
    }
if (g->on_line) g->on_line(g,buf,ll,code); // recollect
return 1; // anyway
}


int gmodem_lines_trim = 1;


int gmodem_do_lines(gmodem *g) {
static char buf[GMODEM_READ_BUF+2]; // ZU - overflow of buffer?
unsigned char *s = (void*)g->in;
//hex_dump("gmodem_do_lines,begin",g->in,g->in_len);
int i,ll=0,rlen=0; // define nonzero and full length
for(i=0;i<g->in_len;i++) {
     if ( i<g->in_len-1 && s[i]=='\r' && s[i+1]=='\n') { rlen=i+2; ll=i; break;}
     if ( s[i]=='\r' || s[i]=='\n') { rlen=i+1; ll=i; break;}
     }
if (rlen == 0) {
     if (i<sizeof(g->in)) return 0; // not yet line
     rlen = ll= sizeof(g->in); // overflow - get it all, split a line
     }
//printf("ll=%d rlen=%d in_len=%d\n",ll,rlen,g->in_len);
memmove(buf,g->in,ll); buf[ll]=0; // gets a copy buffer
//hex_dump("gmodem_copy_buf",buf,ll);
//hex_dump("gmodem_do_lines,before",g->in,g->in_len);
memmove(g->in,g->in+rlen,g->in_len-rlen); g->in_len-=rlen; g->in[g->in_len]=0; // remove from a buffer
//hex_dump("call_do_line",buf,ll);
//hex_dump("rest_line",g->in,g->in_len);
g_modem_do_line(g,buf,ll); // call processing
return 1; // ok
}

int gmodem_do_lines_full(gmodem *g) {
int i;
unsigned char *s = (void*)g->in;
for(i=0;i<g->in_len;i++) if (s[i]=='\n' || s[i]=='\r') break;
if (i<g->in_len || (i==sizeof(g->in)-1))  { // Yes  - do a line !!!
  int ll=i; // now - line trim
  //while(ll>0 && s[ll-1]<=32) ll--; --! no trim!
  char buf[sizeof(g->in)]; memmove(buf,g->in,ll); buf[ll]=0; // gets a copy buffer
  if (i<g->in_len) i++;    memmove(g->in,g->in+i,g->in_len-i); g->in_len-=i;   g->in[g->in_len]=0; // remove it freom a queue
  g_modem_do_line(g,buf,ll); // call processing
  return 1; // ok
  }
return 0; // not yet
}


//int hexdump(char *msg,uchar *s,int len);
int gmodem_onidle(gmodem *g);

int gmodem_run_do_lines(gmodem *g,int (*run_do_lines)()) {
int cnt=0;
cnt = gmodem_onidle(g); // starts with it
//char buf[1024]; // need collect it !!!
vstream *s=&g->port;
int sz = sizeof(g->in) - g->in_len -1; // free space
int r = 0;
if (g->f.eof) return 0; // NO!
g->now = os_ticks();
if (g->in_len>=0 && sz>0) r = s->p->peek(s->handle,g->in+g->in_len,sz); // if can read - do it
if (r>0) { // yes, read !
  char *dat = g->in+g->in_len; // data_starts
  if (g->logLevel>10) hex_dump("modem_peek",dat,r);
  g->in_len+=r;  g->in[g->in_len]=0; // correct collected buffer
  if (g->bin && g->mode == 0 ) { // we have binary data ready to send, need '>' in a stream
       int i; for(i=0;i<r;i++) if (dat[i]=='>') break;
       if (dat[i]=='>') { // ok  - have to flash binary data
          char chZ=26,*bin=g->bin;  int bin_len = g->bin_len;
          g->bin=0; g->bin_len = -1; // return to def
          //printf("ZUZKA : FOUND '>' flash a binary %s\n",bin);
          if (bin_len<0) bin_len = strlen(bin);
          gmodem_put(g,bin,bin_len);
          gmodem_put(g,&chZ,1);
          }
      }
  if (g->f.console) printf("%*.*s",r,r, g->in+g->in_len-r);
  if (g->on_data) {
       g->on_data(g,g->in+g->in_len-r,r); // raport
       }
  if (g->logLevel>10) hex_dump("MODEM_LINE",g->in,g->in_len);
  if (g->mode == 0) while (run_do_lines(g)); // process line by line
  //printf("*%*.*s",r,r,buf); // just print on a screen!!!
  if (g->logLevel>10) hex_dump("REST_LINE",g->in,g->in_len);
  cnt++; g->f.idle = 0;
  } else {
  if (r<0) g->f.eof=1; // EOF!
  g->f.idle++;
  }
if (g->mon)  cnt+=gmodem_run_do_lines(g->mon,run_do_lines); // run monitor as well
return cnt;
}

int gmodem_run(gmodem *g) { return gmodem_run_do_lines(g,gmodem_do_lines);}


int gmodem_setLogLevel(gmodem *g,int logLevel) {
g->logLevel=logLevel;
if (g->mon) g->mon->logLevel = logLevel;
sprintf(g->out,"logLevel=%d now",g->logLevel);
return 1;
}


/* send command and wait for result */

int gmodem_put(gmodem *g, char *out,int len) {
vstream *s=&g->port;
if (len<0) len=strlen(out);
if (g->f.eof) return 0;
if (len<0) len=strlen(out);
 if (g->logLevel>10) hexdump("serial_write",out,len);
int r = s->p->write(s->handle,out,len);
if (r<=0) { g->f.eof=1; return 0; } ; //error!
_gmodem_broadcast_len(g,out,len);
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

int gmodem_Atf(gmodem *g,char *fmt, ... ) {
char buf[256];
BUF_FMT(buf,fmt);
return gmodem_At2buf(g,buf,0,0);
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
if (size>0 && out) out[0]=0; // for empty responses
g->out[0]=0; // clear output at begin
if (g->mode == 1) {
     sprintf(g->out,"modem in phoenix mode");
     return -1;
     }
//gmodem_echo_off(g); // check off echo
g->res = 0;
 int on_line(gmodem *g,char *line,int len,int code) {
     //printf("ON LINE HERE %s\n",line);
     if (proc) proc(g,line,len,code); // call prev
     if (len<=0) return 0; // ignore
     if (code==0 && size>0) { // my line, have a buffer
         if (lineno>0) { *out='\n'; out++; size--;};
         lineno++;
         int r = strlen(line); // first line ???
         if (r>size) { r=size; g->f.over++;} // over
         memmove(out,line,r); size-=r; out+=r;
         //memcpy()
         //printf("Line:<%s>\n",line);
         }
     // here - we are
 return 0; // continue
 }
//int crlf=3; if (g->dev) crlf=g->dev->crlf;
sprintf(buf,"at%s%s",cmd,gmodem_crlf(g));
 memset(out,0,size); size--; // out and cmd - can be a same buffer
if (g->logLevel>3) printf("gmodem_send: at%s<crlf:%d>\n",cmd,g->dev->crlf);
 else if (g->logLevel>2) printf(">>%s at%s%s\n",gmodem_color_out,cmd,gmodem_color_none);
if (gmodem_put(g,buf,-1)<=0) return g_eof;
proc=g->on_line; g->on_line = on_line;
while (g->res == 0 ) {
   if (g->f.eof) g->res=g_eof;
   if (gmodem_run(g) == 0) {
      msleep(100); // wait for answer
      //printf("RES=0, wait <%s>\n",g->in);
      }
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
     //printf("LINE:%s FILTER:%s\n",line,filter);
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
if (cmd) { // Allow cmd = NULL for just check lines till g->res set up
sprintf(buf,"at%s%s",cmd,gmodem_crlf(g));
if (g->logLevel>3) printf("gmodem_send: at%s<crlf>\n",cmd);
 else if (g->logLevel>2) printf(">>%s at%s%s\n",gmodem_color_out,cmd,gmodem_color_none);
if (gmodem_put(g,buf,-1)<=0) return g_eof;
 }
proc=g->on_line; g->on_line = on_line;
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
         memmove(out,line,r); size-=r; out+=r;
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

//int gsm7_code(unsigned int offset, int max_out, unsigned char *input,
	//	   unsigned char *output, unsigned int *get_len, int maxLen);
int bin2hex(uchar *d,uchar *s,int len) {
*d=0; // len<=0 case
int i;for(i=0;i<len;i++,s++,d+=2) sprintf(d,"%02X",*s);
return len*2;
}


int utf8_poke ( char *utf8char, int wchar, size_t count );

// move to -> coders
int uni2utf(char *out,char *ucs2,int len) { // converts UCS2 -> utf8, len in bytes
//hexdump("gsm2utf:",ucs2,len);
int tlen=0;
    while(len>0) {
      int wch  = ( ucs2[1]<<8 | ucs2[0] ); // first go page
      int rlen = utf8_poke(out,wch,4);
      //printf("rlen=%d wch=%d\n",rlen,wch);
      //hexdump("U",out,rlen); hexdump("rest",ucs2,len);
      len-=2; ucs2+=2;  tlen+=rlen;
    if (out) { out+=rlen;   }
    }
if (out) *out=0;
return tlen;

}

char *str_unquote(char *buf);

int gsm7_code(unsigned int offset, int max_out, unsigned char *input,
		   unsigned char *output, unsigned int *get_len, int maxLen); // coded below in this file

		   char *gmodem_par(char **cmd,int skip);
int gsm2utf(char *out,char *ucs2,int len) ;

int gmodem_ussd(gmodem *g,char *str) { // call string
char buf[1024]; int dcs=15; // or 15 for
int ussd = 0;
if (g->dev) ussd = g->dev->ussd;
if (*str=='.') { // strict
   str++;
   ussd = atoi (get_word(&str));
   }
if (ussd == 7) { // 7bit mode for string - E1550
  char numbin[100],num[100]; int len=0;
  //int l =
  gsm7_code(0,sizeof(num),str,numbin,&len,strlen(str));
  //if (!g->mon) {
    // sprintf(g->out,"No monitor interface (while USSD %s)",str);
    // return -1;
    // }
  //printf("gsm_code=%d len=%d\n",l,len);
  //hexdump("1",numbin,len);
  //hexdump("2",numbin,l);
  bin2hex(num,numbin,len);
  sprintf(buf,"+CUSD=1,%s,%d",num,dcs);
  } else if (ussd ==-1)  { // via ATD command - neoway
       sprintf(buf,"d%s",str);
  }
  else {
 sprintf(buf,"+CUSD=1,\"%s\",%d",str,dcs);
  }
g->cusd[0]=0;
if (gmodem_At(g,buf)<=0) {
     return -2; // error already reported
     //return gmodem_errorf(g,-2,"fail_launch %s",buf);
     }
int i;
for(i=0;i<30000;i+=100) {
  gmodem_run(g);
  if (g->cusd[0]) {
           // ALCATEL: +CUSD: 2,"Balance:259,17r,Limit:0,01r ", 15
           // E1550  :
        char *cmd = g->cusd,*txt; int dcs;
        char buf[512],buf2[200];;
        //CLOG(g,2,"USSD RESP HERE>%s\n",cmd);
        txt = str_unquote(gmodem_par(&cmd,1)); // skip presentation code (normally=0)
        dcs = atoi((void*)gmodem_par(&cmd,0)); // 15 - 7bit, 72 - UCS2, optionally HEX-coded
        int l;
        //dcs=72;

        gmodem_logf(g,3,"USSD for decode dcs=%d , data: '%s'",dcs,txt);
        switch (dcs) {
        case 72: // decode UCS2
          l = hexstr2bin(txt,txt,-1);
          gsm2utf(buf,txt,l);
          //printf("TEXT:%s\n",buf);
          break;
        case 15: // GSM-default-alphabet. A LOT of modems have a BUG here ...
          // Sometimes here not HEX data, but just normal text...
          {
            int bad=0,i ;
            for(i=0;txt[i];i++) if (!strchr("0123456789ABCDEFabcdef",txt[i])) bad++;
            if (bad == 0 && ussd == -1) { // !!! BAG - neoway returns in UCS2 anyway
                l = hexstr2bin(buf2,txt,-1);
                gsm2utf(buf,buf2,l);
                }
            else if (bad == 0) { // It is normally here, but
                l = hexstr2bin(buf2,txt,-1);
                gsm7_decode(buf,buf2,l,0);
                } else { // leave as is - it can be HEX anyway
                strNcpy(buf,txt);
                }

          }
          break;
        default:
          strNcpy(buf,txt);
          }
        gmodem_logf(g,3,"USSD DCS:%d TEXT:%s\n",dcs,buf);
        strNcpy(g->out,buf);
        return 1; // OK
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
return strlen(g->out);
}

int gmodem_device_detect(gmodem *g, char *imei, char *ati) {
gsm_device *dev;
 // first - check by ati?
 //printf("ATI:%s\n",ati);
if (ati) for(dev=gsm_devices+1;dev->name;dev++) if (dev->ati && strstr(ati,dev->ati)) {g->dev=dev; break; }

if (imei) if (!g->dev) // if not set yet
 for(dev=gsm_devices+1;dev->name;dev++) if ( dev->imei && memcmp(dev->imei,g->imei,strlen(dev->imei))==0 ) {g->dev=dev; break; }

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
if (!g->dev || g->dev == gsm_devices) { // not set or default
 char ati[512];
 gmodem_At2buf(g,"i",ati,sizeof(ati));
 gmodem_device_detect(g,g->imei,ati);
 if (!g->dev) g->dev = gsm_devices; // default
 }
//printf("Found dev:%s for imei:%s crlf:%d\n",g->dev->name,g->imei,g->dev->crlf);
return 1;
}

// sends pin & waits for MSISDN
int gmodem_pin(gmodem *g,char *pin) {
char cpin[80];
int ok;
strcpy(cpin,"EMPTY");
ok = gmodem_At2bufFilter(g,"+CPIN?","+CPIN",cpin,sizeof(cpin));
 //printf("cpin state: <%s>\n",cpin);
int ready = (ok>0) &&  ((strcmp(cpin,"READY")==0)||(cpin[0]==0));
if (!pin || !pin[0]) { // just report
   if (ready) return gmodem_errorf(g,1,"pin_ready");
   return gmodem_errorf(g,-2,"no pin yet");
   }
 // now - 'READY' is OK
 if (!ready) {
   gmodem_logf(g,1,"pin not ready, try enter : '%s'",pin);
   ok = gmodem_Atf(g,"+CPIN=\"%s\"",pin);
   if (ok<=0) return gmodem_errorf(g,-2,"pin enter failed"); // fail
   }
gmodem_imsi(g);
gmodem_logf(g,1,"pin entered, wait for imsi 100 sec");
int i;
gmodem_At(g,""); gmodem_clear(g,1000); // dump OK
for(i=0;i<100;i++) { // 100 sec
  //int k=1;
  if ( gmodem_imsi(g) >0 )  {
     return gmodem_errorf(g,1,"pin_ok,imsi_here:%s",g->imsi);
     }
  sleep(1);
  //if (gmodem_)
  }
return gmodem_errorf(g,-3,"time_out wait on imsi");
}

int gmodem_balance(gmodem *g) {
   // printf("run balance\n");
char *num=0; // default USSD balance number
if (!g->imsi[0]) gmodem_imsi(g); // update imsi of not yet
gsm_operator *o = g->oper;
if (o && o->ussd_balance) num=o->ussd_balance;
if (!num) {
    printf("balance undefined or operator unknown\n");
    return 0;
  }
if (g->logLevel>2) printf("begin ussd %s\n",num);
return gmodem_ussd(g,num);
}

extern int in_pppd ;

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
 in_pppd = 1;

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

//extern int system(char *cmd);

int code = system(buf);
//system("pppd  /dev/ttyACM0 nodetach debug user \"beeline\" password \"beeline\""); /// Ну почти заработало?
// whell - need to do it on other thread...
sprintf(g->out,"pppd:%d",code);
 in_pppd=0;
//printf("PPPD DONE code=%d\n",code);

// now - wait for that... and run system(pppd)???
//return gmodem_ussd(g,num);
return 1;
}

int gmodem_creg(gmodem *g) { // >0 means registred
char buf[200];
int c1,c2;
if (gmodem_At2bufFilter(g,"+CREG?","+CREG",buf,sizeof(buf))<0) return gmodem_errorf(g,-1,"command failed");
if ( sscanf(buf,"%d,%d",&c1,&c2)!=2) return gmodem_errorf(g,-3,"invalid creg resp: '%s'",buf);
if ( (c1 == 1) || (c1 == 2) ) { // has register code
  if (c2 == 1) return gmodem_errorf(g,1,"home registred");
  if (c2 == 5) return gmodem_errorf(g,2,"roaming reistred");
  if (c2 == 3) return gmodem_errorf(g,-2,"forbiden registration");
  if (c2 == 0) return gmodem_errorf(g,-3,"not registred");
  if (c2 == 2) return gmodem_errorf(g,-1,"searching");
  }
return gmodem_errorf(g,-1,"creg unknown status %d,%d",c1,c2);
}

int gmodem_wait_ok(gmodem *g,int sec) {
int ms = 0; int rep =0;
//g->logLevel=10;
g->res=0; sec=sec*10; // in msec
while(sec>0 && g->res==0) {
  if (ms>2000 && g->logLevel>=2 && rep==0) { fprintf(stderr,"[wait 'AT' responce %d sec more...]",sec/10); rep=1;}
  //printf("req=%d\n",g->res);
  gmodem_put(g,"AT",2); gmodem_put(g,gmodem_crlf(g),-1);
  gmodem_run2(g);
  msleep(1000); sec--; ms+=1000;
  }
if (rep) fprintf(stderr,"[AT resp=%d in %d sec]\n",g->res,ms/1000);
return g->res>0;
}



