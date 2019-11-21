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
#include "wsSrv.h"
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
#ifdef P_AUDIO
 write_wav_header(h,100*1024*1024); // set 100Mb file by default
#endif //P_AUDIO
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


int onHttp1234(Socket *sock,vssHttp *req, SocketMap *map) { // call at command and send back results
char buf[1024]; gmodem *g = G;
//httpSrv *srv = (void*)sock->pool;
 sprintf(buf,"%*.*s", VSS(req->B));  //VSS(req->args)); // arguments = atCommand
printf("HERE 1234 command <%s>\n",buf);
int ok = buf2file(req->B.data,req->B.len,".mo.rep")>=0; // OK, save buf2file
SocketPrintHttp(sock,req,"{res:%d,out:'%s'}",ok,g->out); // Flash Results as http
return 1; // OK - generated
}


int port=80; int logLevel=3;
 int keepAlive=0;
char *rootDir = "./";
char *mimes=".htm,.html=text/html&.js=text/javascript";
httpSrv *srv;
wsSrv *ws; // webSocket Server

int gmodem_onidle(gmodem *g) { //
if (!srv) return 0;
  int cnt = SocketPoolRun(&srv->srv);
      cnt+=wsSrvStep(ws); // run web sockets
  //printf("CNT=%d\n",cnt);
  return cnt;

}

int _gmodem_broadcast_len(gmodem *g,char *m,int len) {
unsigned char *msg = (unsigned char *)m;
if (len<0) len=strlen((char*)msg);
while(len>0 && *msg<=32) { msg++; len--;}
while(len>0 &&  msg[len-1]<=32) len--; //rtrim
if (ws && len>0) wsBroadcast((void*)ws,msg,len);
return 1;
}

int gmodem_broadcast(gmodem *g,char *msg) { return _gmodem_broadcast_len(g,msg,-1);}



int onWebMessage(Socket *sock,char *data,int len) {
char buf[512];

sprintf(buf,"%s#%d: %s",sock->szip,sock->N,data); // echo it back
if (G) if ( (memcmp(data,"at",2)==0) || lcmp(&data,"com")) {
 int code = gmodem_cmd(G,data);
 snprintf(buf,sizeof(buf),"result:%d",code);
 wsPutStr(sock,buf,-1);
}
//wsBroadcast((void*)sock->pool,buf,-1);
return 1;
}

int onWebSock(Socket *sock, vssHttp *req, SocketMap *map) {
//return 1;
//char buf[1024];
httpSrv *srv = (void*)sock->pool;
strSetLength(&srv->buf,0); // ClearResulted
wsSrvUpgrade(ws,sock,req); // send upgrade request
//wsPutStr(sock,"+Hello new gmodem client!",-1);
return 1; // OK - generated
}

char basicUserPass[200],allowIP[200];
httpAuth gmodemAuth={.realm="Gmodem",.basicUserPass=basicUserPass,.allowIP=allowIP};


int httpStart(gmodem *g) {
int Limit=1000000;
  //printf("create0\n");
net_init();
TimeUpdate();

ws = wsSrvCreate();
ws->onMessage = (void*)onWebMessage;
 //printf("create1\n");
srv = httpSrvCreate(0); // New Instance, no ini
//srv->log =  srv->srv.log = logOpen("microHttp.log"); // Create a logger
srv->logLevel = srv->srv.logLevel = g->logLevel;

//srv->logLevel = 10;
//srv->keepAlive=keepAlive;
//srv->readLimit.Limit = Limit;
  //printf("create2\n");
IFLOG(srv,0,"...starting microHttp {port:%d,logLevel:%d,rootDir:'%s',keepAlive:%d,Limit:%d},\n   mimes:'%s'\n",
  port,logLevel,rootDir,keepAlive,Limit,
  mimes);
//printf("...Creating a http server\n");
srv->defmime= vssCreate("text/plain;charset=utf8",-1);
httpSrvAddMimes(srv,mimes);
//httpMime *m = httpSrvGetMime(srv,vssCreate("1.HHtm",-1));printf("Mime here %*.*s\n",VSS(m->mime));
//httpSrvAddFS(srv,"/c/","c:/",0); // Adding some FS mappings
if (basicUserPass[0]) srv->defauth = &gmodemAuth; // if defined user-pass
httpSrvAddFS(srv,"/",rootDir,0); // Adding some FS mappings
httpSrvAddMap(srv, strNew("/.stat",-1), onHttpStat, 0);
httpSrvAddMap(srv, strNew("/.at",-1), onHttpCmd, 0);
httpSrvAddMap(srv, strNew("/.audio",-1), onHttpAudio, 0);
httpSrvAddMap(srv, strNew("/1234",-1), onHttp1234, 0);
httpSrvAddMap(srv, strNew("/.chat",-1), onWebSock, 0);


if (httpSrvListen(srv,port)<=0) { // Starts listen port
   Logf("-FAIL start listener on port %d\n",port);
   return 0;
   }
Logf(".. listener is ready on port %d, Ctrl+C to abort\n",port);
//if (runTill) srv->runTill = TimeNow + runTill;
/*
httpSrvProcess(srv); // Run All messages till CtrlC...
TimeUpdate();
IFLOG(srv,0,"...stop microHttp, done:{connects:%d,requests:%d,runtime:%d}\n",
     srv->srv.connects,srv->srv.requests,TimeNow - srv->created);
*/
return 1;
}



int gmodem_http(gmodem *g,char *par) {
    G=g; // remoeber modem
if (lcmp(&par,"start")) {
  sscanf(get_word(&par),"%d",&port); // try change
  if (par[0]) strNcpy(basicUserPass,get_word(&par));
  if (par[0]) strNcpy(allowIP,get_word(&par));
  if (httpStart(g)<=0) return gmodem_errorf(g,-2,"fail start http on port %d",port);
  return  gmodem_errorf(g,1,"http_started:{port:%d,basicUserPass:'%s',allowIP:'%s'}",port,basicUserPass,allowIP);
  }
if (lcmp(&par,"stop")) {

  }
return gmodem_errorf(g,-2,"http unkonwn, usage: start|stop");
}
