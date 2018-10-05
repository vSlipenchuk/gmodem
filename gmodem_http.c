#include "gmodem.h"

/*
  start http_server and allow acces to it
*/


/* compile with all need sources --- */
#include "../vos/std_sock.c"
#include "../vos/logger.c"
#include "../vos/exe.c"
#include "../vos/sock.c"
#include "../vos/httpSrv.c"
//#include "vos.c"
#include "../vos/vs0.c"
#include "../vos/vss.c"

/*

int SocketSendHttpCode(Socket *sock, vssHttp *req, char *code, uchar *data, int len) {
char buf[1024];
vss reqID = {0,0};
if (req && req->reqID.len>0) reqID=req->reqID;
if (len<0) len = strlen(data);
sprintf(buf,"HTTP/1.1 %s\r\nConnection: %s\r\n%s: %*.*s\r\nContent-Length: %d\r\n\r\n",code,sock->dieOnSend?"close":"Keep-Alive",
    X_REQUEST_ID,VSS(reqID),len);
strCat(&sock->out,buf,-1); // Add a header
strCat(&sock->out,data,len); // Push it & Forget???
//printf("TOSEND:%s\n",sock->out);
sock->state = sockSend;
// Wait???
return 1;
}
*/

int SocketSendNow(Socket *sock,char *data,int len) {
if (len>0) SocketSend(sock,data,len);
len = strLength(sock->out);
int l = send(sock->sock,sock->out,len,MSG_NOSIGNAL);
 if (l>0) strDelete(&sock->out,0,l);
return l == len;
}

int SocketSendChank(Socket *sock,char *data,int len) {
char buf[30];
sprintf(buf,"%x\r\n",len);
SocketSend(sock,buf,-1);
SocketSend(sock,data,len);
SocketSend(sock,"\r\n",-1);
return SocketSendNow(sock,0,0);
}

Socket *asock = 0;

void SocketSendAudioCode(Socket *sock) {
char h[44];
SocketSend(sock,"HTTP/1.1 200 OK\r\nContent-Type: audio/wav\r\nConnection: close\r\n\r\n",-1);
 write_wav_header(h,100*1024*1024); // set 100Mb file by default
 SocketSend(sock,h,44); // Send a header
//SocketSendNow(sock,0,0);
printf("AudoSocket opened\n");
return ;

//Connection: close
//SocketSend(sock,"HTTP/1.1 200 OK\r\nContent-Type: audio/wav\r\nTransfer-Encoding: chunked\r\n\r\n",-1);
 //write_wav_header(h,100*1024*1024); // set 100Mb file by default
 //SocketSendChank(sock,h,44); // Send a header
//SocketSendNow(sock,0,0);
//printf("AudoSocket opened\n");

}

void gmodem_http_send_wav(char *data,int len) {
//printf("AudioSendok asock=%x\n",asock);
if (!asock) return;
if (!SocketSendNow(asock,data,len)) {
  printf("AudioSocket closed\n");
  SocketClear(&asock);
  }

}



int onHttpAudio(Socket *sock, vssHttp *req, SocketMap *map) { // add socket to audio listen
//httpSrvAddMap(srv, strNew("/.audio",-1), onHttpAudio, 0);
SocketSendAudioCode(sock);
if (asock) { SocketClear(&asock); } // remove prev
asock = objAddRef(sock); // set it here
printf("Here onHttpAudio!\n");
//sock->sock = a_socket;
return 1;
}

int onHttpStat(Socket *sock, vssHttp *req, SocketMap *map) { // Ãåíåðàöèÿ ñòàòèñòèêè ïî ñåðâåðó
char buf[1024];
httpSrv *srv = (void*)sock->pool;
strSetLength(&srv->buf,0); // ClearResulted
sprintf(buf,"{clients:%d,connects:%d,requests:%d,mem:%d,serverTime:'%s',pps:%d}",arrLength(srv->srv.sock)-1,
  srv->srv.connects,
  srv->srv.requests,
  os_mem_used(), szTimeNow,
  (srv->readLimit.pValue+srv->readLimit.ppValue)/2);
SocketPrintHttp(sock,req,"%s",buf); // Flash Results as http
return 1; // OK - generated
}

gmodem *G;

int onHttpCmd(Socket *sock,vssHttp *req, SocketMap *map) { // call at command and send back results
char buf[1024]; gmodem *g = G;
//httpSrv *srv = (void*)sock->pool;
 sprintf(buf,"%*.*s", VSS(req->B));  //VSS(req->args)); // arguments = atCommand
printf("Start a command <%s>\n",buf);
  int ok = gmodem_cmd(g,buf);
SocketPrintHttp(sock,req,"{res:%d,out:'%s'}",ok,g->out); // Flash Results as http
return 1; // OK - generated
}

int port=80; int logLevel=3;
 int keepAlive=0;
char *rootDir = "./";
char *mimes=".htm,.html=text/html&.js=text/javascript";
httpSrv *srv;

int gmodem_onidle(gmodem *g) { //
if (!srv) return 0;
  int cnt = SocketPoolRun(&srv->srv);
  //printf("CNT=%d\n",cnt);
  return cnt;

}

int httpStart(gmodem *g) {
int Limit=1000000;
  printf("create0\n");
net_init();
TimeUpdate();
 printf("create1\n");
srv = httpSrvCreate(0); // New Instance, no ini
//srv->log =  srv->srv.log = logOpen("microHttp.log"); // Create a logger
srv->logLevel = srv->srv.logLevel = g->logLevel;
//srv->keepAlive=keepAlive;
//srv->readLimit.Limit = Limit;
  printf("create2\n");
IFLOG(srv,0,"...starting microHttp {port:%d,logLevel:%d,rootDir:'%s',keepAlive:%d,Limit:%d},\n   mimes:'%s'\n",
  port,logLevel,rootDir,keepAlive,Limit,
  mimes);
//printf("...Creating a http server\n");
srv->defmime= vssCreate("text/plain;charset=windows-1251",-1);
httpSrvAddMimes(srv,mimes);
//httpMime *m = httpSrvGetMime(srv,vssCreate("1.HHtm",-1));printf("Mime here %*.*s\n",VSS(m->mime));
//httpSrvAddFS(srv,"/c/","c:/",0); // Adding some FS mappings
httpSrvAddFS(srv,"/",rootDir,0); // Adding some FS mappings
httpSrvAddMap(srv, strNew("/.stat",-1), onHttpStat, 0);
httpSrvAddMap(srv, strNew("/.at",-1), onHttpCmd, 0);
httpSrvAddMap(srv, strNew("/.audio",-1), onHttpAudio, 0);

if (httpSrvListen(srv,port)<=0) { // Starts listen port
   Logf("-FAIL start listener on port %d\n",port);
   return 0;
   }
Logf(".. listener is ready, Ctrl+C to abort\n");
//if (runTill) srv->runTill = TimeNow + runTill;
/*
httpSrvProcess(srv); // Run All messages till CtrlC...
TimeUpdate();
IFLOG(srv,0,"...stop microHttp, done:{connects:%d,requests:%d,runtime:%d}\n",
     srv->srv.connects,srv->srv.requests,TimeNow - srv->created);
*/
return 1;
}



int gmodem_http(gmodem *g,uchar *par) {
    G=g; // remoeber modem
if (lcmp(&par,"start")) {
  httpStart(g);
  return 1;
  }
if (lcmp(&par,"stop")) {

  }
return gmodem_errorf(g,-2,"http unkonwn, usage: start|stop");
}
