
#include "gmodem.h"

/*
  send some APDU thru at+csim=<len>,"string" command

csim 80aa00000aa9088100820101830107
gmodem_send: at+CSIM=30,"80aa00000aa9088100820101830107"<crlf:3>
>> +CSIM: 4, 9000
<+CSIM: 4, 9000>
{+CSIM: 4, 9000}[0]
>> OK
<OK>


E[ample:
 gmodem "logLevel2" "csim start <server-with-vpcd-started>"



*/

static gmodem *m;

static char atr[5]={0x3B,0x80,0x80,0x01,0x01}; //char *viccatr = "3B80800101";

static int sock = 0;  // socket for connetcing to vpc
static char rbuf[2024]; static int rlen;  // buffer recv

int pcsc_connect(char *host) { // def port = 0x8C7B
if (sock>0) {sock_close(sock); sock = 0;}
sock = sock_connect(host,0x8C7B);
return sock; // if ok
}

void pcsc_send_apdu(char *buf, int len) {
uint16_t size;
size = htons ( len );
send(sock, &size, sizeof(size),0);
send(sock, buf, len ,0);
}

void rbuf_peek(int size) {
size+= sizeof( uint16_t ); // remove from a buffer with size
memmove(rbuf,rbuf+size, rlen-size);
rlen-=size;
if (m->logLevel>5) printf("rest_buf:%d\n",rlen);
}


int gmodem_pcsc_run() { // ping & try restart
if (sock<=0) return 0; // nothing to do...
char buf[1000];
int l=sock_readable(sock);
if (l<0) {
  printf("vicc sock disconnected\n");
  sock=0;
  return 0;
  }
if (l==0) return 0; // not ready data yet
l = sock_read(sock,rbuf+rlen,sizeof(rbuf)-rlen); // read all in a socket
if (l<=0) {
  printf("vicc sock disconnected\n");
  sock=0;
  return 0;
  }
if (m->logLevel>5) printf("recv:%d bytes\n",l);
rlen+=l;


while(rlen>0) { // process all data
uint16_t size;
if (rlen<sizeof(size)) return 0; // not yet
size = ntohs ( *(uint16_t*)rbuf );
if (rlen<sizeof(size) + size ) return 0; // not yet...
char *cmd = rbuf + sizeof(size);
if (m->logLevel>5) hexdump("recv_cmd", cmd, size );
if (size == 1) { // spec commands
  switch(*cmd) {
  case 0:  case 1: case 2: break; // ignore OFF,ON,RESET commands, no responce
  case 4:
      if (m->logLevel>5) printf("Send ATR");
      pcsc_send_apdu(atr,sizeof(atr));
  }
  rbuf_peek(size);
  continue; // done..
  }
// Here APDU for exchange!
char out[1000];

int len = apdu_exchange(m,cmd,size,   out,sizeof(out) );

if (len>0) {
  pcsc_send_apdu(out,len);
  }
rbuf_peek(size);
}
// zu - if we have buffered commands???
return 1;
}



int apdu_exchange(gmodem *g, char *in, int ilen, char *out, int size) { // returns output size
char szin[900],szout[1000],cmd[1000];
bin2hex(szin,in,ilen);
if (g->logLevel>=2) {
  //hexdump("APDU",in,ilen);
  int i; for(i=0;i<ilen;i++) printf("%02X",(unsigned char)in[i]);
  printf("\n");
  }
sprintf(cmd,"+CSIM=%d,\"%s\"",strlen(szin),szin);
szout[0]=0;
int code = gmodem_At2buf(m,cmd,szout,sizeof(szout));
if (code < 0) return -1; // error
// now - parse result?
char *c = strstr(szout,"+CSIM");
if (!c) return -2;
c = strchr (c,','); if (!c) return -3;
int code2 = hexstr2bin(out,c+1,-1);
if (g->logLevel>=2) {
  //hexdump("RAPDU",out,code2);
  int i; for(i=0;i<code2;i++) printf("%02X",(unsigned char)out[i]);
  printf("\n\n");
  }
return code2;
}


int gmodem_csim_cmd(gmodem *g,char *cmd) {
//g->logLevel=40;
char out[1000]; int len=0,i;
cmd=trim(cmd);
m = g; // remember gmodem
if (lcmp(&cmd,"start")) { // connect socket & report state
    if (!*cmd) cmd = "192.168.199.230"; // my def debug host
    pcsc_connect(cmd);

    if (sock>0) return gmodem_errorf(g,1,"vicc connected to %s",cmd);
    return gmodem_errorf(g,-3,"fail connect to %s",cmd);
    }
if (lcmp(&cmd,"stop")) {
    if (sock>0) { sock_close(sock); sock=0; }
    return gmodem_errorf(g,1,"OK");
    }
char bIN[800], OUT[800]; int blen,code;
blen = hexstr2bin(bIN, cmd, -1);
 code = apdu_exchange(g,bIN,blen, out,sizeof(out ));
 //hexdump("OUT",out,code);
return code;
// validate input
/*for(i=0;cmd[i];i++) if (  (cmd[i]>='0' && cmd[i]<='9') || (cmd[i]>='a' && cmd[i]<='f') || (cmd[i]>='A' && cmd[i]<='F')) {
   out[len]=cmd[i]; len++;
   if (len>=sizeof(out)) return gmodem_errorf(g,-4,"command buffer overflow, max %d",sizeof(out));
   }

out[len]=0;
int ok = gmodem_Atf(g,"+CSIM=%d,\"%s\"",len,out);
*/
//return ok;
}
