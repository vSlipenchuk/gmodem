#include "gmodem.h"


/*
  SimComm sim7020E -> documents: https://simcom.ee/documents/?dir=SIM7020
      and sim7000  ->

*/

int gmodem_sim_udpsend(gmodem *g,char *host,int port,char *message) {
 // first
 //gmodem_Atf(g,"+cfun=0"); // off
 int ok = gmodem_Atf(g,"+cfun=1")>0 && // radio ON
 gmodem_Atf(g,"*MCGDEFCONT=\"IP\",\"%s\"","internet")>0 && // set APN manually
 gmodem_Atf(g,"+CGCONTRDP")>0 &&  // attach PS and got IP automatically
 gmodem_Atf(g,"+CPSMS=1")>0 && // enable PSM
 gmodem_Atf(g,"+CSOC=1,2,1")>-999 &&  // open UDP socket -> ignore error (if already opened)
 gmodem_Atf(g,"+CSOCON=0,%d,\"%s\"",port,host)>0 && // need socket from +CSOC: <sock>

 gmodem_Atf(g,"+CGREG?")>0 &&  // show register?

 gmodem_Atf(g,"+CSOSEND=0,0,\"%s\"",message);
 if (ok) return gmodem_errorf(g,1,"ok.udpsend");
 else return -1;
 //return gmodem_errorf(g,1,"JustTest");
}

int gmodem_sim_init(gmodem *g,char *cmd) {
 int ok = gmodem_Atf(g,"+csq")>0 && // check rf signal
 gmodem_Atf(g,"+cgreg?")>0 && // check PS server
 gmodem_Atf(g,"+cgact?")>0 && // +CGACT: 1,1 = Activated automatically
 gmodem_Atf(g,"+cops?") >0 &&  // +COPS:0,2,"25001",9 ; 9=NbIot network
 gmodem_Atf(g,"+cgcontrdp"); //1,2,"apn,"ipaddr.ip_mask" Atatched to PS and get adress automatically
if (ok) return gmodem_errorf(g,1,"ok.init");
   else return -1;
}

/*
  Other example -> sim800 like TCP application AT-commands (failed on sim7020E).
*/
int gmodem_sim_tcpsend(gmodem *g,char *host,int port,char *message) {
 //gmodem_Atf(g,"+CFUN=0");
 gmodem_Atf(g,"+CFUN=1");
 gmodem_Atf(g,"+cstt"); // start tasks and set APM
 gmodem_Atf(g,"+CIICR"); // bringUP wireless connaction
   //gmodem_Atf(g,"+CIFSR"); // get local ip address
 gmodem_Atf(g,"+CIPSTART=\"TCP\",\"%s\",\"%d\"",host,port); // CONNECT OK
 g->bin = message;
 gmodem_Atf(g,"+CIPSEND"); // CtrlZ
   // Closed
 g->bin=0;
return gmodem_errorf(g,1,"Test-TCP");
}

void host2port(char *host,int *port) {
 char *   p = strchr(host,':'); if (p) { *p=0; sscanf(p+1,"%d",port);};
}

//+CSONMI: 0,LEN,HEXDATA  ; when incoming UDP

int gmodem_sim_iot(gmodem *g,char *cmd) {
if (lcmp(&cmd,"init")) return gmodem_sim_init(g,cmd);
if (lcmp(&cmd,"udpsend")) { // <server:port> MESSAGE
    int port = 30000; // default
    char *host=get_word(&cmd);
    host2port(host,&port);
    return gmodem_sim_udpsend(g,host,port,cmd);
  }
if (lcmp(&cmd,"tcpsend")) {
  int port = 30000; // default
  char *host=get_word(&cmd);
  host2port(host,&port);
  return gmodem_sim_tcpsend(g,host,port,cmd);
  }
return 0;
}
