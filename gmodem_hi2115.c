#include "gmodem.h"


int gmodem_cgatt(gmodem *g) { // attach & check status
int ok; char buf[256];
ok = gmodem_Atf(g,"+CGATT=1"); if (ok<0) return ok;
ok = gmodem_At2buf(g,"+CGATT?",buf,sizeof(buf)); if (ok<0) return ok;
char *p = strstr(buf,"+CGATT:");
 //printf("P=%s P+8=%s",p,p+7);
if (!p || sscanf(p+7,"%d",&ok)<1 || ok!=1) return gmodem_errorf(g,-2,"attach_failed");
return gmodem_errorf(g,1,"attached_ok");
}

int gmodem_hi2115_cmd(gmodem *g,char *cmd) {
if (lcmp(&cmd,"nb.init")) {
  int prev_log = g->logLevel;
  if (prev_log<3) g->logLevel = 3;
  if (!*cmd) cmd = "m2m.nidd"; // default m2m non-ip APN
  //gmodem_Atf(g,"+CIMI"); // !!! NEED PAUSE, and just for info
  //gmodem_Atf(g,"+CMEE=1");
  int ok = gmodem_Atf(g,"+CIMI")>0 &&  gmodem_Atf(g,"+CMEE=1")>0 && gmodem_Atf(g,"+CGDCONT=0,\"NONIP\",\"%s\"",cmd) &&
   gmodem_Atf(g,"+CFUN=1")>0 &&  // ON RADIO
   //gmodem_Atf(g,"+CGATT=1")>0 && gmodem_Atf(g,"+CGATT?")>0 &&  // Attach to PDP, cgatt MUST return 1
   gmodem_Atf(g,"+CGDCONT?")>0 &&  // get APN
   gmodem_Atf(g,"+cereg=2")>0 && // eps reg notifications
   gmodem_Atf(g,"+cscon=1")>0 && // connection notifications
   gmodem_Atf(g,"+NPTWEDRXS=3")>0 && // disable DRX
   gmodem_Atf(g,"+CPSMS=0")>0 && // disable PSM
   gmodem_Atf(g,"+CRTDCP=1")>0 && // allow receive: +CRTDCP:<cid>,<cpdata_length>,<cpdata>"
    gmodem_cgatt(g) >0;
  g->logLevel = prev_log;
  return ok?1:-1;
   }
if (lcmp(&cmd,"nb.send")) { // send NB data
  int prev_log = g->logLevel;
  if (prev_log<3) g->logLevel = 3;
  int len = strlen(cmd); if (len>128) len=128; cmd[len]=0; // zu - check len?
  char buf[512+2];
 // char b64[256+2];
  bin2hex(buf,cmd,len); gmodem_logf(g,4,"nb.sending:'%s',hex:'%s'\n",cmd,buf);
  int ok = gmodem_Atf(g,"+CSODCP=0,%d,%s",len,buf);
  g->logLevel = prev_log;
  return ok;
  }
return 0;
}
