#include "gmodem.h"

/* gmodem cellbroadcast service 03.41 & 03.38

// "1082,52,16,15,50" - MTS channels

AT+CSMS? // supporting SMS
  +CSMS: 0,1,1,1  <code><MO><MT><CB>

AT+CSCB?
АT+CSCB= <mode>, [ <mids>, [ <dcss> ] ] - выбра 0(включить), мес-ид, и кодировка. AT+CSCB=0,”15-17,50,86”,””
AT+CSCB=0,”1-65535”,””

AT+CMNI?
+CNMI <mode>=1, <mt>=1, <bm>=0, <ds>=2, <bfr>=0.
+CNMI=1,0,2,0,1

// CellBroadcast looks like http://cellphoneforums.net/alt-cellular-nokia/t106118-gsm-cell-broadcast-info-pdu-formats.html
+CBM: 88
011000320111C2327BFC76BBCBEE46A3D168341A8D46A3D168341A8D46A3D168341A8
D46A3D168341A8D46A3D168341A8D46A3D168341A8D46A3D168341A8D46A3D168341A
8D46A3D168341A8D46A3D168341A8D46A3D100
+CBM: "123456","98/10/01,12 :3000+00",129,1,2,5<CR><LF>
Hello
*/

typedef struct {
  uchar sernum[2]; // 1-2
  uchar msgid[2];  // 3-4
  uchar dcs,page;  // 5,6
  uchar content[]; // CBS-Message-Information-Page as Sent from CBC to BSC
  } cb_msg_head;

typedef struct {
  int GS,MessageCode,UpdateNumber; // sernum fiels
  int emerAlert, Poupup;
  } cb_msg;

int cb_dump(uchar *cmd,int len) { // TS 23.041 pp 9.4.1.2
hexdump("CellBroadcast message dump",cmd,len); // SERNUM=272 MSGID=50 DCS=1 PAGEPARAMETER=17 SZ=88
cb_msg_head *h = (void*)cmd;
int sernum = htons(*(short*)h->sernum);
int msgid  = htons(*(short*)h->msgid);
int  dsc = h->dcs; int page=h->page;
 printf("SERNUM=%d MSGID=%d DCS=%d PAGEPARAMETER=%d\n",sernum,msgid,dsc,page);
cmd+=6; len-=6;
hexdump("CBC-Message-Information-Page",cmd,len); // SZ=82
//if (h->dsc == )
char buf[256];
 int l = gsm7_decode(buf,cmd,len,0);
 while(l>0 && buf[l-1]==0xD) l--; buf[l]=0; // trim OD bytes used as filler in CB
 printf("TEXT:<%s>\n",buf);
 hexdump("dumptext",buf,l);
return 0;
}

int gmodem_cb(gmodem *m,char *cmd) {
if (lcmp(&cmd,"info")) {
   gmodem_At(m,"+cscb?");
   gmodem_At(m,"+cnmi?");
   // gmodem_
   return 1;
   }
if (lcmp(&cmd,"dec")) {
    int l = hexstr2bin(cmd,cmd,-1);
   cb_dump(cmd,l);
   return 1;
   }
if (lcmp(&cmd,"start")) {
    gmodem_At(m,"+CSCB=0,\”1-65535\”,\”\”");
    //gmodem_At(m,"+CSCB=0");
    gmodem_At(m,"+CNMI=2,,2"); // ACCEPT INLINE
   return 1;
   }
printf("cb usage: <info|start> ,  '%s' not implemented\n",cmd);
return 1; // bad
}
