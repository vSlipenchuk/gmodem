#include "gmodem.h"


/*
  extra functions for sim800 modem - wget http
   USSD - at+CUSD=1,"#100#",15 - работают (!)
*/
char *gmodem_get_default_apn(gmodem *g) {
if (g->oper && g->oper->apn) return g->oper->apn;
return "internet";
}

int gmodem_csq(gmodem *g,int *csq) { // retutrns CSQ answer
char buf[200],*p;
if (gmodem_At2buf(g,"+CSQ",buf,sizeof(buf))<=0) return 0;
p = strstr(buf,"+CSQ:"); if (p) { *csq = atoi(p+5);
    int rssi=(*csq)*2-113; // in dBm
    return gmodem_errorf(g,1,"RSSI:%d",rssi);
    }
*csq=0;
return 0; // failed read
}

int gmodem_sim800_getip(gmodem *g,char *ip) {
char buf[200],*p;
if (gmodem_At2buf(g,"+SAPBR=2,1",buf,sizeof(buf))<=0) return 0;
p=strstr(buf,"+SAPBR:"); if (p) { p=strstr(p,"\"");
   if (p) {
          p++; int i; for(i=0;i<16 && p[i] && p[i]!='\"';i++); p[i]=0;
          if (strcmp(p,"0.0.0.0")==0) return gmodem_errorf(g,-3,"no_ip_addr: 0.0.0.0");
          strcpy(ip,p);
          return gmodem_errorf(g,1,"%s",ip);
          }
   }
return 0; // not yet
}

int gmodem_sim800_ip_attach(gmodem *g,char *apn) {
char ip[20];
if (gmodem_At(g,"+SAPBR=3,1,\"Contype\",\"GPRS\"")<=0) return 0; // set GPRS
if (gmodem_Atf(g,"+SAPBR=3,1,\"APN\",\"%s\"",apn)<=0) return 0; // set APN
if (gmodem_sim800_getip(g,ip)>0) return 1; // ok, already good
if (gmodem_At(g,"+SAPBR=1,1")<=0) return 0; // request open bearer
if (gmodem_sim800_getip(g,ip)>0) return 1; // ok, already good
return gmodem_errorf(g,-2,"failed open bearer and get ip");
}

int gmodem_sim800_http_connect(gmodem *g,char *url,int ta) {
gmodem_At(g,"+HTTPINIT?");
gmodem_At(g,"+HTTPTERM"); // terminate any session if any
if (gmodem_At(g,"+HTTPINIT")<=0) return 0; // failed start
gmodem_logf(g,3,"http inited, now set URL");
if (gmodem_Atf(g,"+HTTPPARA=\"URL\",\"%s\"",url)<=0) return 0; // wait for OK
g->http_action=-1; // will set up by line callback
if (gmodem_Atf(g,"+HTTPACTION=0")<=0) return 0; // 0 - GET, 1 - POST
gmodem_logf(g,3,"http_action started, wait for connection establish");
int i;
for(i=0;ta>=0;ta--,i++) {
  gmodem_run(g);
  if (g->http_action!=-1) break; // get responce
  if (g->logLevel>=3) printf(".");
  sleep(1);
  }
if (g->http_action==-1) return gmodem_errorf(g,-2,"http_connect timeout %d sec",i);
if (g->http_action!=200) return gmodem_errorf(g,-3,"http cant connect status %d",g->http_action);
gmodem_logf(g,3,"http connected in %d seconds",i);
return 1;
}

int gmodem_sim800_http_read(gmodem *g, char *buf, int len) { // read in buffer HTTPREAD does not finished with OK/ERROR
 //gmodem_At(g,"+HTTPREAD=?");
 int sz=80,s=0; //len=249;
gmodem_logf(g,3,"http_read : actially read %d bytes",len);
for(s=0,sz=80;len>0;) {
 int b=sz; if (b>len) b=len;
 char cmd[200]; sprintf(cmd,"+HTTPREAD=%d,%d",s,b);
 gmodem_At2buf(g,cmd,buf,b+1); // надо читать маленькими порциями - а то глючит
   gmodem_logf(g,3,"here done read buf=<%s>\n",buf);
    len-=b; s+=b; buf+=b; *buf=0;
 }
return 1;
}

int gmodem_sim800_wget(gmodem *g,char *url,char *buf,int size) {
int csq = 0;
char ip[20]; buf[0]=0;
gmodem_logf(g,3,"sim800.wget:check GSM coverage");
if (gmodem_csq(g,&csq)<=0) return gmodem_errorf(g,-3,"GSM not connected");
gmodem_logf(g,3,"sim800.wget:check GPRS bearer");
if (gmodem_sim800_getip(g,ip)>0) {
   gmodem_logf(g,3,"sim800.wget: modem has IP address: %s",ip);
   } else {
   char *apn = gmodem_get_default_apn(g);
   if (gmodem_sim800_ip_attach(g,apn )<=0) return 0;
   }
//char buf[300]; int len=sizeof(buf);
gmodem_logf(g,3,"sim800.wget ready for http");
int code = gmodem_sim800_http_connect(g,url,60); if (code<=0) return code; // gmodem_errorf(g,-3,"wget http_connect failed");
int len = size-1;
  if( len > g->http_len) len=g->http_len;
int ok = gmodem_sim800_http_read(g,buf,len); // read one block
if (ok<=0) return -1;
strNcpy(g->out,buf); // copy to state
return 1;
}

char tbuf[300];

int gmodem_sim800(gmodem *g,char *cmd) {
gmodem_logf(g,5,"sim800 cmd started: %s",cmd);
if (lcmp(&cmd,"wget"))  {

  return gmodem_sim800_wget(g,cmd, tbuf,sizeof(tbuf));
  }
return 1; // OK anyway
}
