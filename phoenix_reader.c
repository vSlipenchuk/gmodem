/*
  Phoenix card reader -

woronscan ATR>3B 9F 95 80  ... 1F C3 80 31 A0 73 BE 21 13 67 03 0D
01 01 51 .. 03 01 82

AID CardManager: A0000000030000 00 может быть и меньше!
                 A000000003000000



*/

#include "gmodem.h"
#include "gsm_sim.h"
#include <termios.h>

/*
void init_personal() {
static int inited = 0;
if (inited ) return ;
 cardJobSetup( 0, 0x1201, 0x00, 0x11,  "8970139470005860934 4", "", "A01A037FDF76B575", 1403); // UTEL
struct _cardJob *j = &cardJob;
  printf("PERSONAL_SETUP:  ICCID: %s counter %d\n",j->ICCID,j->counter);
inited = 1;
}
*/

unsigned char SW[2]; // lastStatusWord

void  prt_cfg3(int ttyDev,int baudrate,int hwcontrol,int parity,int stop) {
struct termios term_attr;
printf("hwcontrol:%d\n",hwcontrol);
tcgetattr(ttyDev, &term_attr); //backup current settings

term_attr.c_iflag = 0;  ;;// IXON | IXOFF;
	term_attr.c_oflag = 0;

	term_attr.c_cflag = baudrate | CS8 | CREAD | parity;
 	  if (hwcontrol) term_attr.c_cflag|=CRTSCTS;
 	  if (stop==2) term_attr.c_cflag|=CSTOPB; // 2 stop bits
	  //if (parity) term_attr.c_iflag|=parity;
	  //if (parity) term_attr.c_oflag|=parity;


	//term_attr.c_iflag = 0;  ;;// IXON | IXOFF;
	//term_attr.c_oflag = 0;
      //if (parity) term_attr.c_oflag|=parity;
	term_attr.c_lflag = 0;

	memset(&term_attr.c_cc,0,sizeof(term_attr.c_cc));
	term_attr.c_cc[VMIN] = 1;
	term_attr.c_cc[VTIME] = 10;

	term_attr.c_cc[VSTART] = 11; term_attr.c_cc[VSTOP] = 13;

	cfsetispeed(&term_attr, baudrate);
    cfsetospeed(&term_attr, baudrate);

/*
newtio.c_cflag = baudrate | CS8 | CLOCAL | CREAD;
newtio.c_cflag &= ~CRTSCTS; //disable hw flow control
newtio.c_iflag &= ~(IXON | IXOFF | IXANY); //disable flow control
newtio.c_iflag |= IGNPAR; //ignore parity
newtio.c_oflag = 0;
newtio.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG); //raw mode

newtio.c_cc[VMIN] = 1;
newtio.c_cc[VTIME] = 0;
*/

//cfsetspeed(&term_attr, getBaud(baudrate));
//tcflush(ttyDev, TCIFLUSH);
//tcsetattr(ttyDev, TCSANOW, &term_attr);
tcsetattr(ttyDev, TCSAFLUSH, &term_attr);


//clear RTS
//ioctl(ttyDev, TIOCMBIC, &mcs);
}

void  prt_config_phoenix(int fd) {
  //prt_cfg2(fd, B9600, 0, PARENB, 2 ); // no flow, even parity, 2 stop bit
  prt_cfg3(fd, B9600, 0, PARENB, 2 ); // no flow, even parity, 2 stop bit
  prt_reset(fd); // need for phoenix ATR
 //exit(1);
}

int gmodem_atr(gmodem *g) { // wait Answer to Reset
int i;
prt_config_phoenix((int)g->port.handle);
for(i=0;i<10;i++) {
  gmodem_run(g);
  msleep(100);
  //hexdump("ATR",g->in,g->in_len);
  }
hexdump("ATR",g->in,g->in_len);
msleep(100);
return 1; // OK
}

//
//

//    '61 XX': The implementation MUST send GET RESPONSE to the card to get the response data before any other command is sent.
  //  '6C XX': The implementation MUST resend the command after setting Le equal to XX received from the card before any other command is sent.

// main proc

int phoenix_apdu2(gmodem *g,uchar *cmd, int  len,uchar *sw,uchar *out) {
uchar buf[512];
g->in_len = 0;
if (len<5) return -1; // invalid cmd

//hexdump("---- BEGIN SEND APDU ----",cmd,len);

uchar ins = cmd[1],clen=cmd[4]; // ins and cmdlength
   phoenix_write_echo(g,cmd,5);
    //gmodem_put(g,cmd,5);  gmodem_read(g,buf,5);

  len-=5; cmd+=5;

 //buf[0]=0;

   while(1) {
   int l = gmodem_read(g,buf,1); // INS = cmd[2] ???
   if (l >0 && buf[0] == 0x60) { // card working... wait
       if (g->logLevel>3) printf("<*>");
       continue;
      }
   if  ( (l<=0) || (buf[0]!=ins )) {
     printf("!!!! ACK FAILED ins=%x get %x\n",ins,buf[0]);
     return -2;
     }
    break; // OK - done read
   }

  if (g->logLevel>5) hexdump("--->ACK -- ->",buf,1);
  out[0]=0;


  g->out[0]=0;

    if (len>0) {
         out[0]=0;
         phoenix_write_echo(g,cmd,len);
         //gmodem_put(g,cmd,len); // push cmd
         //gmodem_read(g,buf,len); // INS = cmd[2] ???
         if (g->logLevel>5) printf("done echo exchange, wait status bytes\n");
        //gmodem_read(g,buf,1);  printf("STRANGE ZUZUKA BYTE %x\n",buf[0]);
//        if (g->in_len>0) {
  //          g->in_len=0;
    //        }
         } else { // do read
         //out[0]=clen;
         //gmodem_read(g,out+1,clen);
         //
         }
//hexdump("g->in_len\n",g->in,g->in_len);
   int l;
   out[0]=0;
   while(1) {
    uchar ch;
   l = gmodem_read(g,&ch,1);
   //printf("[%x] l:%d\n",ch,out[0]);
   if (l<0) {
        printf("Error gmodem_read while 2 bytes of resp!\n");
        return 0;
       }
    if (ch==0x60) continue;
    l = out[0];
    out[l+1]=ch; out[0]++; // add char
    //printf("Now l=%d\n",l);
    if (out[0]>=2) break;
   }
printf("OK, status here\n");
   {    msleep(10);    gmodem_run(g); // read again
        l = g->in_len; if (l>255) l =255;
        //out[0]=l;
        memcpy(out+1+2,g->in,l); out[0]+=l;// copy result
        g->in_len = 0;
        if (g->logLevel>5) hexdump("DONE read",out+1,out[0]);
    }
  //buf[0]=0x02; // ready to get 255 bytes
  //gmodem_put(g,buf,1);   gmodem_read(g,buf,1);
 //sw[0]=sw[1]=0;
 // gmodem_read(g,sw,2); // StatusBytes
  l = out[0];
  bin2hexstr(g->out, out+1,out[0]);
  memcpy(sw,out+1+l-2,2); // copy it
  memcpy(SW,sw,2); // LastStatus Upate
 //sprintf(g->out,"%02x,%02x",sw[0],sw[1]); // ZUZUKA - compatibility
 //if (g->logLevel>1) printf("<<%s\n",g->out);
 return 1; // OK
}
