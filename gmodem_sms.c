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

enum {
 flText=4,
 flSmsc = 8,
 };

int gsm_get_smaddr(char *num, uchar *bin, int len) {
int nl = bin[0]; int r = nl+1;
if (nl>24 || (nl+1)>len ) return -1; // invalid addr
if (nl>0 && bin[1]==0x91) { *num='+'; num++;}
nl--;
for(bin+=2;nl>0;nl--,num+=2,bin++) {
   char n[3];
   sprintf(n,"%02X",*bin);
   num[0]=n[1]; num[1]=n[0]; if (num[1]=='F') num[1]=0;
   }
 *num=0;
return r; // converted length
}

int sms_decode_data(t_sms *sms, char *bin,int len,int flags) {
char BUF[256];
memset(sms,0,sizeof(*sms));
if (flags & flText) {
   int maxsz=2*sizeof(BUF)-1;
   if (len>maxsz) len=maxsz; // no more than buf
   len = hexstr2bin(BUF,bin,len); bin = BUF;
  }
if (flags & flSmsc) { // extract SMSC
  int nl = gsm_get_smaddr(sms->smsc,bin,len);
  //printf("SMSC:<%s>\n",sms.smsc);
  len-=nl; bin+=nl;
  }
//hexdump("todecode:",bin,len);
int l = sms_decode(sms,bin,len);
//printf("decodedsms %d err:%s\n",l,sms.error);
if (l<=0) return l;
/*
char *mti[4]={"DELIVER","SUBMIT","REPORT","RESERVED"}; // message type info
printf("MTI:'%s',SMSC:'%s',NUM:'%s',SENT:'%s',pid:0x%x,dcs:0x%x,udhi:%d\n",mti[sms.mti&3],
         sms.smsc,sms.da,sms.vp,sms.pid,sms.dcs,sms.udhi);
*/
if ( (sms->dcs & 0xF) == 8) { // we have UCS2?
  gsm2utf(sms->text,sms->ud,sms->udl);
  //char utf[512]; gsm2utf(utf,sms->ud,sms->udl);  printf("UTFtext:<%s>\n",utf);  hexdump("UTF1",sms->ud,sms->udl);
  } else if ( (sms->dcs==0xF) ==0) {
  //char txt[512];
  gsm7_decode(sms->text,sms->ud,sms->udl,0);
  //printf("7bit:%s\n",txt);
  } else {
  bin2hexstr(sms->text,sms->ud,sms->udl);
  //hexdump("UD",sms.ud,sms.udl);
  }
return 1;
}

void sms_print(t_sms *sms,int mode) {
switch(mode) {}
char *mti[4]={"DELIVER","SUBMIT","REPORT","RESERVED"}; // message type info
switch(mode) {
case 1:
  printf(" %02d>%s (%s,%s)\n",sms->pos,sms->text,sms->da,sms->vp); // normal print of sms in list
  break;
default: // shpow max of fields
printf("Mti:'%s',SMSC:'%s',NUM:'%s',SENT:'%s',pid:0x%x,dcs:0x%x,udhi:%d\n",mti[sms->mti&3],
         sms->smsc,sms->da,sms->vp,sms->pid,sms->dcs,sms->udhi);
printf("TEXT[UDL:%d]:%s\n",sms->udl,sms->text);
}
}

int sms_dump(uchar *bin,int len,int flags) {
t_sms sms;
if (sms_decode_data(&sms,bin,len,flags)<=0) {
   printf (">>> FAIL_DECODE_SMS: %s\n",sms.error);
   hexdump(">>> OFFENDING_DATA:",bin,len);
   return 0;
   }
sms_print(&sms,0);
return 1;
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

int gmodem_SendOtaSms(gmodem *g,char *phone,char *utext,int dlen) { // send it as SMS
t_sms sms;
int reqReport = 1, msgRef=1, dcs=0xf6;
hexdump("ota2send",utext,dlen);
int i= sms_submit(&sms, 0, reqReport ,msgRef,phone, 0x7f, dcs, 0,msgRef,utext ,utext,dlen); // HAS UDH
if (i<=0) {
  printf("Some coding error=%s\n",sms.error);
  return 0;
  }
//printf("OK, coded <%s> <%s>\n",phone,text);
int len; char data[200];
while((len=sms_fetch(&sms))>0) { // Send It to Phone
            sms_dump(sms.data,len,0);
            strcpy(data,"00");
            for(i=0;i<len;i++) sprintf(data+2+2*i,"%02x",sms.data[i]); // Create ComPort Commsnd
            //hexdump("smsdata",data,strlen(data));
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

int _gmodem_sms_print_ls(gmodem *g, t_sms *sm) { // do print on a screen
          sms_print(sm, g->logLevel>2? 255 : 1 );
return 1;
}

int gmodem_sms_enum(gmodem *g,int (*callback)(), void *handle) {
 t_sms sm;
 int sms_slot=-1;// no slot ready
if (!callback) callback=_gmodem_sms_print_ls; // dafault->print
int on_sms(gmodem *g,uchar *line, int len, int code ) { // called on each SMS
   gmodem_logf(g,5,"ON_SMS_LIST:<%*.*s>",len,len,line);
   if (sms_slot>=0) {
       //       sms_dump(line,len,flSmsc | flText);
       if (sms_decode_data(&sm,line,len, flSmsc | flText )>0) {
          sm.pos = sms_slot;
          callback(handle,&sm,g); // call callback
          //          sms_print(&sm, g->logLevel>2? 255 : 1 );
          }
       sms_slot=-1;
       return 0;
       }
   sms_slot=-1; // not yet
   if (lcmp(&line,"+CMGL:")) {
       sscanf(line,"%d",&sms_slot);
      //sms_ready=1; // next is SMS for decode
      }
   return 0;
   }
 gmodem_At2Lines(g,"+CMGL=4","",on_sms,g);
return 1;
}

int gmodem_sms_delete(gmodem *g, int slot) {
if (gmodem_Atf(g,"+cmgd=%d",slot)<=0) return gmodem_errorf(g,-3,"delete sms#d slot failed",slot);
return gmodem_errorf(g,1,"sms#%d deleted OK",slot);
}

int _gmodem_sms_del(int mode, t_sms *sms,gmodem *g) { // callback function for DELETE_ALL
if (mode !=4 ) return 0; // do nothing
if (gmodem_sms_delete(g,sms->pos))
   gmodem_logf(g,2,"OK DELETE SMS#%d TEXT:%s\n",sms->pos,sms->text);
else
  gmodem_logf(g,0,"FAIL DELETE SMS#%d TEXT:%s\n",sms->pos,sms->text);
return 1;
}

int gmodem_sms(gmodem *g,uchar *sms) {
if (lcmp(&sms,"dump")) {
    uchar out[512];
    int l = hexstr2bin(out,sms,-1);
    sms_dump(out,l,flSmsc | flText);
  return 1;
  }
if (lcmp(&sms,"rm") || lcmp(&sms,"del")) {
  // delete sms from a slot...
  int slot=-2;
  if (lcmp(&sms,"all")) { // remove all messages
      gmodem_sms_enum(g,_gmodem_sms_del, 4); // DELETE ALL
      return 1; // OK - anyway
      }
  if (lcmp(&sms,"rep")) { // delete reports
      gmodem_sms_enum(g,_gmodem_sms_del, 1); // reports
      }
  if ( (sscanf(sms,"%d",&slot)<=0) || (slot<0) ) return gmodem_errorf(g,-2,"sms sim_slot >=0 expected");
  return gmodem_sms_delete(g,slot);
  }
if (lcmp(&sms,"list") || lcmp(&sms,"ls") ) {
  gmodem_sms_enum(g, _gmodem_sms_print_ls, g ); // just print it
  return 1;
  }
uchar *phone=get_word((void*)&sms);
if (!phone[0]) return gmodem_errorf(g,-2,"usage: <phone> <text>");
return gmodem_SendTextSms(g,phone,sms);
//return gmodem_SendTextSms(g,"+79151999003","Привет");
}



int gmodem_ota(gmodem *g,uchar *sms) {
//sms = trim(sms);
uchar *phone=get_word((void*)(&sms));
if (!phone[0]) {
  printf("Usage <phone> <bin_data: 027000... >\n");
  return 0;
  }
char data[512];
printf("OTA_SMS:<%s><%s>\n",phone,sms);
int bl = hexstr2bin(data,sms,-1);
hexdump("ota_data",data,bl);
return gmodem_SendOtaSms(g,phone,data,bl);
//return gmodem_SendTextSms(g,"+79151999003","Привет");
}

#include "../smppTest/im.c"


int gmodem_im(gmodem *g,uchar *cmd) {
if (lcmp(&cmd,"init")) {
  uchar *phone=get_word(&cmd);
  uchar *key = get_word(&cmd);
  uchar *tar = get_word(&cmd);
  uchar *master = get_word(&cmd);
  if (!phone[0] || !key[0] || !tar[0]) return gmodem_errorf(g,-2,"usage: im init phone key tar");
     //im_init(phone,key,tar,master); ZUZUKA!
  sprintf(g->out,"im inited for phone %s",phone);
  return 1;
  }
  /*
if (lcmp(&cmd,"live")) {
  char out[256];
  int l = im_code_IdleModeText(out+19,cmd);
  hexdump("live_packet",out+19,l);
  l+=codeCommandPacket(out,im_tar,l);
  //hexdump("live_packet",out,l);
  return gmodem_SendOtaSms(g,im_phone,out,l);
  }
  */
return gmodem_errorf(g,-2,"usage: im <init|live>");
}
