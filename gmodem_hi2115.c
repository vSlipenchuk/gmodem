#include "gmodem.h"


int gmodem_hi2115_cmd(gmodem *g,char *cmd) {
if (lcmp(&cmd,"nb.init")) {
  int prev_log = g->logLevel;
  if (prev_log<3) g->logLevel = 3;
  if (!*cmd) cmd = "m2m.nidd"; // default m2m non-ip APN
  gmodem_Atf(g,"+CIMI"); // !!! NEED PAUSE, and just for info
  int ok = gmodem_Atf(g,"+CGDCONT=0,\"NONIP\",\"%s\"",cmd) &&
   gmodem_Atf(g,"+CFUN=1") &&  // ON RADIO
   gmodem_Atf(g,"+CGATT=1") &&  // Attach to PDP
   gmodem_Atf(g,"+CGDCONT?") &&  // get APN
   gmodem_Atf(g,"+CRTDCP=1"); // allow receive: +CRTDCP:<cid>,<cpdata_length>,<cpdata>"
  g->logLevel = prev_log;
  return ok;
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
