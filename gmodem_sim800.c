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
   if (gmodem_sim800_ip_attach(g,apn )<=0) return -1;
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

int gmodem_sim800_ls(gmodem *g,char *path) {
char buf[1024];
if (!path || !path[0]) path="C:\\";
snprintf(buf,sizeof(buf),"+FSLS=%s",path);
int ok = gmodem_At2buf(g,buf,buf,sizeof(buf));
if (ok<=0) return ok;
printf("%s\n",buf); // just print on screen
return ok;
}

int gmodem_sim800_rm(gmodem*g,char *file) {
return gmodem_Atf(g,"+FSDEL=%s",file);
}

int gmodem_sim800_file_rewrite(gmodem *g,char *file, char *data, int len) {
gmodem_sim800_rm(g,file); // delete any way
 gmodem_Atf(g,"+FSCREATE=%s",file);
 g->bin = data; g->bin_len = len;
int ok = gmodem_Atf(g,"+FSWRITE=%s,0,%d,10",file,len);
 g->bin = 0; g->bin_len = -1; // default
return ok;
}

char *tmpFile = 0;

int gmodem_sim800_put(gmodem *g,char *local,char *remote) {
strSetLength(&tmpFile,0);
if (!strCatFile(&tmpFile,local)) return gmodem_errorf(g,-3,"cant load local file %s",local);
return gmodem_sim800_file_rewrite(g,remote,tmpFile,strLength(tmpFile));
}

int szCollect = 0;


int gmodem_collect_out(gmodem *g, char *buf, int len) {
int r  = len; int pc = szCollect;
if  (r>szCollect) r = szCollect; // not more than
if ( strLength(tmpFile)==0 && r>=2 ) { // skip first '\r\n'
 strCat(&tmpFile,buf+2,r-2);  szCollect-=r-2;
 } else {  strCat(&tmpFile,buf,r);  szCollect-=r; }
 //printf("(%*.*s) rest %d",len,len,buf,szCollect);
gmodem_logf(g,5,"collect_gmodem_out: got %d rest %d, expect_at_begin %d  move:%d",len,szCollect,pc,r);
if (g->logLevel>=5) if ( szCollect<200) hex_dump("buf",buf,len);
memmove(g->in,g->in+r,g->in_len-r); g->in_len-=r; // clear gmodem buffer...
if (szCollect<=0)
     {
       hex_dump("do_lines",g->in,g->in_len);
       g->mode = 0; // return back to read
       gmodem_do_lines(g); // proceed rest of lines
     }
return 0;
}

int gmodem_sim800_CatFile(gmodem *g,char *remote,char **str) {
char buf[512],*p;
snprintf(buf,sizeof(buf),"+FSFLSIZE=%s",remote);
if ( gmodem_At2buf(g,buf,buf,sizeof(buf))<0) return gmodem_errorf(g,-2,"cant define size of %s",remote);
int sz; p = buf;
if ( !lcmp(&p,"+FSFLSIZE:") || sscanf(p,"%d",&sz)<1 || sz<0) return gmodem_errorf(g,-2,"wrong size %d of %s",remote);
g->on_data = gmodem_collect_out; g->mode = 2; szCollect = sz ; // collecting szBytes
gmodem_logf(g,1,"... start intrcept %d bytes for %s\n",sz,remote);
gmodem_Atf(g,"+FSREAD=%s,0,%d,10",remote,sz); // do not get result...
g->on_data = 0; g->mode = 0; // return back anyway
return gmodem_errorf(g,1,"OK");
}

int gmodem_sim800_get(gmodem *g,char *remote,char *local) {
strSetLength(&tmpFile,0);
if ( gmodem_sim800_CatFile(g,remote,&tmpFile)<0) return -1;
// now - save it or print it
if (!local || !local[0]) {
   printf("%s\n",tmpFile);
   return 1;
   }
if ( !strSave(tmpFile,local)) return gmodem_errorf(g,-2,"save file error");
return gmodem_errorf(g,1,"OK");
}

int gmodem_sim800_play(gmodem *g,char *file) {
char buf[80];
int n = atoi(file); if (n>0) { sprintf(buf,"C:\\%d.amr",n); file=buf;}
return gmodem_Atf(g,"+CREC=4,\"%s\",0,100",file);
}

char tbuf[300];

int gmodem_sim800(gmodem *g,char *cmd) {
char *p;
gmodem_logf(g,5,"sim800 cmd started: %s",cmd);
if (lcmp(&cmd,"wget"))  {

  return gmodem_sim800_wget(g,cmd, tbuf,sizeof(tbuf));
  }
if (lcmp(&cmd,"ls")) return gmodem_sim800_ls(g,cmd);
if (lcmp(&cmd,"put")) {
  p = get_word(&cmd);
  return gmodem_sim800_put(g,p,cmd);
  }
if (lcmp(&cmd,"get")) {
  p = get_word(&cmd);
  return gmodem_sim800_get(g,p,cmd);
  }
if (lcmp(&cmd,"play")) return gmodem_sim800_play(g,cmd);
return 1; // OK anyway
}


