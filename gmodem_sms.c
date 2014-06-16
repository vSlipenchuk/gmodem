#include "gmodem.h"
#include "coders.h"
#include <stdarg.h>
#include "vos.h"
/*
  do test sms, first - check a book?
*/

/*
int gmodem_BookWrite(gmodem *g,uchar *name,uchar *num) {
int slot=1;
char ucs_name[80],hxname[160];
int l = utf2gsm(ucs2_name,name,-1); // convert to UCS2 binary
bin2hex(hxname,ucs2_name,l); // change to printable hexstring 1A2F...
gmodemAtf(g,"at+cpbw=%d,\"%s\",%d,\"%s\"",slot,num,num[0]=='+'?145:129,hxname);
}
*/

int sms_dump(char *bin,int len,int flags) {
t_sms sms;
int l = sms_decode(&sms,bin,len);
printf("decodedsms %d\n",l);
if (l<=0) {
 printf("ERR:%s\n",sms.error);
 return 0;
 }
char *mti[4]={"DELIVER","SUBMIT","REPORT","RESERVED"}; // message type info
printf("MTI:'%s',NUM:'%s',pid:0x%x,dcs:0x%x,udhi:%d\n",mti[sms.mti&3],sms.da,sms.pid,sms.dcs,sms.udhi);
if ( (sms.dcs & 0xF) == 8) { // we have UCS2?
 char utf[512];
  gsm2utf(utf,sms.ud,sms.udl);
  printf("text:<%s>\n",utf);
  }
return 1;
}

int gmodem_Atf(gmodem *g,char *fmt, ... ) {
char buf[256];
BUF_FMT(buf,fmt);
return gmodem_At2buf(g,buf,0,0);
}

int gmodem_SendTextSms(gmodem *g,char *phone,char *utext) { // send it as SMS
t_sms sms;
int reqReport = 1, msgRef=1, dcs=0x8;
char text[256];
int dlen = utf2gsm(text,utext,-1);
int i= sms_submit(&sms, 0, reqReport ,msgRef,phone,0,dcs,0,msgRef,0,text,dlen); // No UHDI
if (i<=0) {
  printf("Some coding error=%s\n",sms.error);
  return 0;
  }
printf("OK, coded <%s> <%s>\n",phone,text);
int len; char data[200];
while((len=sms_fetch(&sms))>0) { // Send It to Phone
            sms_dump(sms.data,len,0);
            strcpy(data,"00");
            for(i=0;i<len;i++) sprintf(data+2+2*i,"%02x",sms.data[i]); // Create ComPort Commsnd
            hexdump("smsdata",data,strlen(data));
            printf("Ready to send at+cmgs=%d bin=%s",len,data);
            g->bin=data;

            //break; // do not yest send

            if (gmodem_Atf(g,"+cmgs=%d",len)<=0) {
              printf("Send data error\n");
              break;
              }
         //   CLOG(com,3,"SmsSending... %d/%d message (%d row bytes) DATA:'%s'\n",sms.segment,sms.total,len,data);
            // sms_dump(data,strlen(data),1+2);
    /*

              if (comCmd(com,"at+cmgs=%d",len)<=0) { msgRef=-4; break;}  // Это команда, которую надо послать - ошибка???

              comOut(com,data,strlen(data)); uchar ch = 26; comOut(com,&ch,1); // CtrlZ
               i = comCmd(com,0); // Flash data & wait for it...
               if (i<=0) {msgRef=-4; break;} // OtherError
             //  }
            //CLOG(com,2,"COM_SEND_OK:%d <%s> %s\n",msgRef,phone,data);
    */
            }
// ok - now we need fetch chain/by/chain
printf("Done OK");
return 1;
}

int gmodem_sms(gmodem *g,char *sms) {
uchar *phone=get_word((void*)&sms);
if (!phone[0]) {
  printf("Usage <phone> <text>\n");
  return 0;
  }
return gmodem_SendTextSms(g,phone,sms);
//return gmodem_SendTextSms(g,"+79151999003","Привет");
}
