#include "gmodem.h"
#include "vss.h"

/*

   gprs - attach/detach
          httpget queries to files/screen

          to  do:
          1. transform gprs ping to normal ping
          2. save wget to file ?

*/

int http_action_to = 30; // 30 secbalan

int gmodem_httpstat(gmodem *g,int *status) { // buggy yet
char out[128]; int r;
out[0]=0;
r = gmodem_At2bufFilter(g,"+HTTPSTATUS?","+HTTPSTATUS:",out,sizeof(out));
if (r<=0) return r;
  int mode=-1; *status=-1;
  vss op={0,0};
  gmodem_scan2(out,"%s,%d,%d",&op,&mode,status);
return gmodem_errorf(g,1,"+httpstat:'%*.*s' %d %d",VSS(op), mode, *status);
}

int gmodem_gprs_detach(gmodem *g) {
return gmodem_At(g,"+SAPBR=0,1");
}

int gmodem_gprs_attach(gmodem *g,char *apn) {

    gmodem_gprs_detach(g);

if ( (!apn) || (!apn[0])) { // auto define from settings.
    gsm_operator *o = g->oper;
    char *num = 0;
    if (o && o->gprs_num) {
           num=o->apn;
           //printf("GetFrom Num %s\n",num);
           num = strchr(num,'@');
           if(num) num++;
           apn=num;
    }
}
if ( (!apn) || (!apn[0])) apn="internet"; // default
//printf("Attaching to APN %s\n",apn);
 gmodem_At(g,"+CGATT=1"); // attach
   gmodem_At(g,"+SAPBR=3,1,\"CONTYPE\",\"GPRS\"");
   gmodem_Atf(g,"+SAPBR=3,1,\"APN\",\"%s\"",apn); //test
if (gmodem_At(g,"+SAPBR=1,1")>0) return gmodem_errorf(g,1,"attached:%s",apn);
return -2; //error attaching
}

int gmodem_gprs_http_get(gmodem *g,char *url, int ta,char *buf, int size) {
int ok;
 gmodem_At(g, "+HTTPINIT");
   gmodem_At(g, "+HTTPPARA=\"CID\",1");  // bearer
   gmodem_Atf(g,"+HTTPPARA=\"URL\",\"%s\"",url); // URL // "m2m.onepamop.com/index.htm"

    g->http_action = -1; // wait for response
   gmodem_At(g, "+HTTPACTION=0"); // get
     //
 int i;
  for(i=0;i<http_action_to*10;i++) { // 10 sec timeout
    if (g->http_action != -1) break;
    int status ;
    gmodem_run2(g);
    msleep(100);
    }
   ok = g->http_action;
//   printf("HERE ACTION CODE = %d\n", g->http_action);
   //gmodem_At(g, "+HTTPREAD=?"); // do we have
   buf[0]=0;
   if (ok==200) {
   gmodem_At2buf(g, "+HTTPREAD=0,1024",buf,size); // just read on a screen ...
      buf[size-1]=0;
       }
   gmodem_At(g, "+HTTPTERM"); // and terminate
if (ok == 200 ) return gmodem_errorf(g,1,"%s",buf);
  else return gmodem_errorf(g,-2,"http_error %d",ok);
}


int gmodem_gprs(gmodem *g,char *cmd) {
if (lcmp(&cmd,"init") || lcmp(&cmd,"attach")) { //setup and attach
   //g->logLevel=10;
   return gmodem_gprs_attach(g,cmd);
   }
if (lcmp(&cmd,"status")) {
    int status;
   gmodem_At(g,"+CGATT?"); // gprs status
   gmodem_httpstat(g,&status);
   return 1;
   }
if (lcmp(&cmd,"done") || lcmp(&cmd,"detach")) {
    return gmodem_gprs_detach(g);
   }
if (lcmp(&cmd,"ping")) {
    int l = g->logLevel;
    //gmodem_At(g,"+CGATT?");
    //gmodem_At(g,"+CSTT=\"CMNET\""); // APN
    gmodem_At(g,"+CIICR");
    gmodem_At(g,"+CIFSR;AT"); //  -- hangs on SIM800L without second AT
    //AT+CIPPING="8.8.8.8" AT+CIPPING="ya.ru"
    g->logLevel=10; // toShow Results
    int ok = gmodem_Atf(g,"+CIPPING=\"%s\"",cmd);
    g->logLevel = l; // back it
    return ok;
   }
if (lcmp(&cmd,"wget")) {
     char buf[256]; // small internal buffer
     return gmodem_gprs_http_get(g,cmd,30,buf,sizeof(buf));
   return 1;
   }
return gmodem_errorf(g,-1,"gprs: init|done|wget");
}
