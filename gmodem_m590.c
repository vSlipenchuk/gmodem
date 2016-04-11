#include "gmodem.h"


int gmodem_wait_status(gmodem *g) {
while (g->res == 0 ) {
   if (g->f.eof) g->res=g_eof;
   if (gmodem_run(g) == 0) msleep(100); // wait for answer
   }
//g->on_line=proc; // restore proc
return g->res; // anyway ^)
}

int gmodem_neoway_getip(gmodem *g,char ip[16]) {
char ips[200],*c=ips;
ips[200]=0;
    gmodem_At2bufFilter(g,"+XIIC?","+XIIC:",ips,sizeof(ips));
if (atoi(c) ==  1) { // Have IP address
  //printf("C1=%s\n",c);
  char *p = get_till(&c,",",1); c=trim(c);
  memset(ip,16,0);
  strncpy(ip,c,15);
  //printf("C2=%s\n",c);
  return gmodem_errorf(g,1,c);
  }
return gmodem_errorf(g,-2,"no IP yet");
}

int gmodem_neoway_ip_status(gmodem *g) { // IPSTATUS does not answer with OK
char buf[200];
int on_line(char *handle,char *line) {
 // g->res = -2;
  if (strstr(line,"DISCONNECT")) g->res=-2;
   else if (strstr(line,"CONNECT")) g->res=1;
 }
gmodem_At2Lines(g,"+IPSTATUS=0","+IPSTATUS:",on_line,0);
 /*
memset(buf,0,sizeof(buf));
gmodem_put(g,"AT+IPSTATUS=0\r",-1); // no answer, need normal AT
if (gmodem_At2buf(g,"e1",buf,sizeof(buf))<=0) return -1; // unknown
printf("IP_STATUS=<%s>\n",buf);
if (strstr(buf,"DISCONNECT")) return -2; // disconnected
if (strstr(buf,"CONNECT")) return 1; // connected OK
 */
return g->res; // unknown again
}

extern int gmodem_lines_trim, gmodem_ignore_stdresp;

int gmodem_neoway_dns(gmodem *g, char *host) {
char addr[80],cmd[80];
addr[0]=0;
//if (gmodem_Atf(g,"+dns=\"%s\"",host)<1) return -2; // syntax error
snprintf(cmd,sizeof(cmd),"+dns=\"%s\"",host);
// ok - wait for DNS now...
//printf("Now - wait for numbers\n");
int do_recv(void *h,char *line, int len ) {
char *c = line; int pos;
printf("Here Number line <%s>\n",c);
if (lcmp(&c,"+DNS:")) { // found DNS now
    if (addr[0]==0) { strNcpy(addr,c); } // copy first addr
    g->res = 1; // OK
    }
return 1;
}
gmodem_ignore_stdresp = 1;
gmodem_At2Lines(g,cmd,0, do_recv ,0 );
gmodem_ignore_stdresp = 0;
if (addr[0]) {
     if (strcmp(addr,"Error")==0) return gmodem_errorf(g,2,"-error-");
     return gmodem_errorf(g,1,addr); // ok - and addr here
     }
return gmodem_errorf(g,-3,"timeout");
}

int gmodem_neoway_wget(gmodem *g,char *url) {
char ip[16];
g->logLevel=10; // debug
printf("Try get URL:%s\n",url);
if (gmodem_neoway_getip(g,ip)>0) {
 printf("Already have IP: %s\n",ip);
 } else { // try to get
 gmodem_At(g,"+CGATT=0"); // close ppp if any
 gmodem_At(g,"+CGDCONT=1,\"IP\",\"internet.mts.ru\""); // setup APN

msleep(1000); // simple wait

 if (gmodem_At(g,"+CGATT=1")!=1) {
     return gmodem_errorf(g,-3,"cant open the link");
    }
 gmodem_At(g,"+XISP=0"); // use internal stack
 gmodem_At(g,"+XIIC=1"); // request establish PPP link

int ok =0, i;
for(i=0;i<10;i++) {
  if (gmodem_neoway_getip(g,ip)<0) {
        msleep(1000);
        continue;
       }
 printf("New IP: %s\n",ip); ok = 1; break;
 }
if (!ok) return gmodem_errorf(g,-2,"no IP in PPP connection");
 }
//int  gmodem_neoway_getip(g);
// ip?

int status=gmodem_neoway_ip_status(g);

if (status == 1) {
     gmodem_At(g,"+tcpclose");
     msleep(100);
     status=gmodem_neoway_ip_status(g);
     }

if (status == -2 ) { // disconnected
     gmodem_At(g,"+tcpsetup=0,95.31.11.199,80"); //it can pushed
     msleep(100);
     //gmodem_At(g,"");
     }

int ok=0;
int i; for(i=0;i<100;i++) {
 int res=gmodem_neoway_ip_status(g);
 printf("Here RES=%d\n",res);
 if (res  == 1) { ok = 1; break;}
 msleep(1000);
}
printf("Connected, now - send a fata\n");

if (!ok) return gmodem_errorf(g,-3,"failed connectd in timeout");

char *data="GET / HTTP/1.0\r\nHost: www.onepamop.com\r\n\r\n";
g->bin=data;
gmodem_Atf(g,"+tcpsend=0,%d",strlen(data)); // ">" equal OK
printf("SENT OK, push data\n");

int recv_len = 0, total_recv=0;;

int do_recv(void *h,char *line, int len ) {
char *c = line; int pos;
if (recv_len>0) { // not recv yet
    int r = len;
    if (r>recv_len) r=recv_len;
    recv_len-=r; total_recv+=r;
    //printf("Now - more data [%d,rest:%d]: <%*.*s>\n",r,recv_len,r,r,line);
    hexdump("recv",line,r);
    return 1;
    }
if (lcmp(&c,"+TCPRECV:")) { // its a recv conuter here !!!
     char *p = strchr(c,','); // first - socket number
     if (p) {
       p++; sscanf(p,"%d",&recv_len); p =strchr(p,',');
       if (p) {
         p++; pos = p-line; len-=pos; total_recv+=len;
         printf("Start read %d bytes, already here %d <%*.*s>\n",recv_len,len,len,len,p);
         recv_len-=len;
         return 1; // do it again
         }
       } // the rest
    }
if (lcmp(&c,"+TCPCLOSE")) {
   g->res = 1; // DONE
   printf("OK, link closed\n");
   }
return 1;
}

gmodem_lines_trim = 0;
gmodem_At2Lines(g,0,0, do_recv ,0 ); // just realines till g->res setup
gmodem_lines_trim = 1; // return back

printf("Done, total_recv %d bytes\n", total_recv);

//gmodem_put(g,data,strlen(data));
 /*//char *data="GET / HTTP/1.0\r\nHost: www.onepamop.com\r\n\r\n";
 char *data="0123456789";

 char buf[200]; sprintf(buf,"at+tcpsend=0,%d\r",strlen(data));
 printf("SEND<%s>\n",buf);
for(i=0;i<100;i++) {
 gmodem_run(g);
 msleep(1);

  }
 gmodem_put(g,buf,strlen(buf));
 */
 //gmodem_put(g,buf,strlen(data));
 //..gmodem_put(g,"\r\n",2); //  need by commmand in a description
 //gmodem_wait_status(g);
 // now - wait for OK
 //gmodem_Atf(g,"+tcpsend=0,%d",strlen(data));

return 1; // OK
}

int gmodem_neoway(gmodem *g,char *cmd) { // spec neoway commands
if (lcmp(&cmd,"wget")) return gmodem_neoway_wget(g,cmd);
if (lcmp(&cmd,"dns"))  return gmodem_neoway_dns(g,cmd);
 return gmodem_errorf(g,-1,"neoway unknown");
}
