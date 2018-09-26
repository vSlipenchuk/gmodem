#include "gmodem.h"
#include "coders.h"
#include <stdarg.h>
#include "vos.h"
/*
  +CMTI: - sms notifications require AT+CMTI=2,1("+CMTI"),0,2("+CDSI")
     gmodem_Atf(g,"+CNMI=2,1,0,2"); - needs for mode,mt_indication,,ds_indication

     AT+CPMS="MT" - read both "SM" and "ME" sms storages
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

int sms_submit(t_sms *sms,int rejDup, int repRequest,
  int mRef, uchar *phone, int pid,int dcs, int vp,
      int chainsTR, uchar *udh, uchar *text, int len) ;
int sms_fetch(t_sms *sms) ;

int str_to_unicode(u_char *wbuf,size_t wsize,u_char *buf,size_t size);
int utf8_poke ( char *utf8char, int wchar, size_t count );

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

int gsm2utf(char *out,char *ucs2,int len);


int sms_decode_data(t_sms *sms, char *bin,int len,int flags) {
char BUF[256];
if (len<0) len=strlen(bin);
memset(sms,0,sizeof(*sms));
if (flags & flText) {
   int maxsz=2*sizeof(BUF)-1;
   if (len>maxsz) len=maxsz; // no more than buf
   len = hexstr2bin(BUF,bin,len); bin = BUF;
  }
if (flags & flSmsc) { // extract SMSC
  int nl = gsm_get_smaddr(sms->smsc,bin,len);
  //printf("SMSC:<%s>\n",sms->smsc);
  len-=nl; bin+=nl;
  }
//hexdump("todecode:",bin,len);
int l = sms_decode(sms,bin,len);
//printf("decodedsms %d err:%s\n",l,sms->error);
if (l<=0) return l;
/*
char *mti[4]={"DELIVER","SUBMIT","REPORT","RESERVED"}; // message type info
printf("MTI:'%s',SMSC:'%s',NUM:'%s',SENT:'%s',pid:0x%x,dcs:0x%x,udhi:%d\n",mti[sms.mti&3],
         sms.smsc,sms.da,sms.vp,sms.pid,sms.dcs,sms.udhi);
*/
//printf("SMS->DCS & 0XF=%d\n",sms->dcs&0xF);
if ( (sms->dcs & 0xF) == 8) { // we have UCS2?
  gsm2utf(sms->text,sms->ud,sms->udl);
  //char utf[512]; gsm2utf(utf,sms->ud,sms->udl);  printf("UTFtext:<%s>\n",utf);  hexdump("UTF1",sms->ud,sms->udl);
  } else if ( (sms->dcs&0xF) ==0) {
  //char txt[512];
  gsm7_decode(sms->text,sms->ud,sms->udl,0);
  //printf("7bit:%s\n",txt);
  } else {
  //    printf("BINHERE!\n");
  bin2hexstr(sms->text,sms->ud,sms->udl);
    //   printf("SMSTEXT=%s\n",sms->text);
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
  if (sms->udhi && sms->udh) {
      printf("UDH[%d] %02X",sms->udh[0]+1,sms->udh[0]);
      int i; for(i=0;i<sms->udh[0];i++) printf("%02X",sms->udh[i+1]);
  printf("\n");
      }
 printf("TEXT[UDL:%d]:%s\n",sms->udl,sms->text);
}
}

int hexdump(char *msg,uchar *s,int len);

int sms_dump(uchar *bin,int len,int flags) {
t_sms sms;
hexdump("sms_dump",bin,len);
if (sms_decode_data(&sms,bin,len,flags)<=0) {
   printf (">>> FAIL_DECODE_SMS: %s\n",sms.error);
   hexdump(">>> OFFENDING_DATA:",bin,len);
   return 0;
   }
sms_print(&sms,0);
return 1;
}

int _gmodem_sendSmsChain(gmodem *g,t_sms *sms) { // send prepared sms
int i,len; char data[140*2+10]; int msgRef=sms->data[0];
gmodem_logf(g,3,"SmsChain started to %s total:%d",sms->da,sms->total);
while((len=sms_fetch(sms))>0) { // len max 140 bytes.
            //sms_dump(sms.data,len,0);
            strcpy(data,"00");
            for(i=0;i<len;i++) sprintf(data+2+2*i,"%02x",sms->data[i]); // Create ComPort Commsnd
//          printf("Ready to send at+cmgs=%d bin=%s",len,data);
            gmodem_logf(g,4,"Sms[%d/%d] try send %d gsm_octets hex:'%s'",sms->segment,sms->total,len,data);
            if (g->logLevel>=5)hexdump("smsHexData",data,strlen(data));
            g->bin=data;
            if (gmodem_Atf(g,"+cmgs=%d",len)<=0) return gmodem_errorf(g,1,"modem rejects at+cmgs, sms send network error");
            }
gmodem_logf(g,3,"SmsChain sent to '%s' OK, msgRef:%d, total: %d ",sms->da,msgRef,sms->total);
return gmodem_errorf(g,1,"smsOk for '%s' msgRef=[%d..%d]",sms->da,msgRef,msgRef+sms->total-1);
}


int gmodem_SendTextSms(gmodem *g,char *phone,char *utext,int vp) { // utf8 as UCS2-unicode sms
t_sms sms;
int rejDuplicate=0, reqReport = 1, msgRef=g->msgRef, pid=0, dcs=0x8; // always unicode ^)
int l = strlen(utext);
char text[l*4+1]; // UCS2 buffer
int r = gmodem_creg(g); if (r<0) return gmodem_errorf(g,-3,"no network yet"); // not registred
int dlen = utf2gsm(text,utext,-1); // buffer in UCS2 string
int i= sms_submit(&sms, rejDuplicate, reqReport ,msgRef,phone,pid,dcs,vp,msgRef,0,text,dlen); // No UHDI=NULL
if (i<=0) gmodem_errorf(g,-3,"internal sms_submit coding error code:%d",i);
g->msgRef+=sms.total; // inc on message counter
return _gmodem_sendSmsChain(g,&sms);
}

int gmodem_SendSpecSms(gmodem *g,char *phone,int specCode,int vp) { // send it as SMS
t_sms sms;
int reqReport = 1, msgRef=g->msgRef, dcs= specCode;
int r = gmodem_creg(g); if (r<0) return gmodem_errorf(g,-3,"no network yet"); // not registred
gmodem_logf(g,1,"SendSpecSms to %s dcs=0x%x msgRef=%d",phone,dcs,msgRef);
int i= sms_submit(&sms, 0, reqReport ,msgRef,phone, 0x0, dcs, vp,     0,0,0,0);  //  msgRef, no UDHI, no Text,Len
if (i<=0) gmodem_errorf(g,-3,"internal sms_submit coding error code:%d",i);
g->msgRef+=sms.total;
return _gmodem_sendSmsChain(g,&sms);
}

int gmodem_SendOtaSms(gmodem *g,char *phone,char *utext,int dlen,int vp) { // send it as SMS
t_sms sms;
int reqReport = 1, msgRef=g->msgRef, pid=0x7f, dcs=0xf6;
if (g->logLevel>=4) hexdump("ota2send",utext,dlen);
int i= sms_submit(&sms, 0, reqReport ,msgRef,phone, pid, dcs, vp,
                  msgRef,0,  //  msgRef,utext  -- No rtansRef & UDHI and
                  utext,dlen); // HAS UDH
if (i<=0) gmodem_errorf(g,-3,"internal sms_submit coding error code:%d",i);
g->msgRef+=sms.total;
return _gmodem_sendSmsChain(g,&sms);
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
if (mode == 1 && sms->mti == 2) {
   if (gmodem_sms_delete(g,sms->pos))
   gmodem_logf(g,2,"OK DELETE DELIVERY_REPORT SMS#%d TEXT:%s\n",sms->pos,sms->text);
   return 0;
   }
if (mode !=4 ) return 0; // do nothing

if (gmodem_sms_delete(g,sms->pos))
   gmodem_logf(g,2,"OK DELETE SMS#%d TEXT:%s\n",sms->pos,sms->text);
else
  gmodem_logf(g,0,"FAIL DELETE SMS#%d TEXT:%s\n",sms->pos,sms->text);
return 0; // process next
}

void gmodem_sms_delete_reports(gmodem *g) {
gmodem_sms_enum(g,_gmodem_sms_del, (void*)1); // reports
}

char buf[1024];
unsigned char  t[256],text2[256];


int encode_utf8_(char *d,char *s,int sl) { // Results in UTF8 ...
int len = 0; //wchar_t w;
if (sl<0) sl = strlen(s);
while (sl>0) {
	int ulen; short w; char buf[8];
	str_to_unicode((char*)&w,4,s,1); // Convert 1 char ...
	//printf("%4x\n",w);
	ulen = utf8_poke(d?d:buf,w,8); // Enough space ...
	len+=ulen;
	if (d) d+=ulen; // Move out buf ...
	sl--; s++;
	}
if (d) *d=0; // Zero terminated ...
return len;
}


int _gmodem_sms_enum(char *mode, t_sms *sms,gmodem *g) { // callback function for DELETE_ALL
char *text=sms->text; char *phone=sms->da;

int i;
 decode_utf8(t,text,strlen(text)); // to win1251
for(i=0;t[i];i++) if ( (t[i]=='\"') || (t[i]=='\'') || (t[i]<=32))  t[i]=32 ; // no bad symbols
 encode_utf8_(text2,t,-1);

sprintf(buf,"%s \"%s\" \"%s\"",mode,text2,phone);
printf("CALL: %s\n",buf);
int res = system(buf);
if (res == 0) {
   if (gmodem_sms_delete(g,sms->pos))
   gmodem_logf(g,2,"OK DELETE SMS#%d TEXT:%s\n",sms->pos,sms->text);
   }
return 1;
}


int gmodem_sms_copy(gmodem *g,t_sms *sms, int size) { // enum sms-> array of size, reurn real size
int r = 0; // real size
int add_sm(void *h,t_sms *s) {
  if (r+1<size)  { memmove(sms+r,s,sizeof(*s)); r++;}
  return 1; // do it again
}
gmodem_sms_enum(g,add_sm,0);
return r;
}

int gmodem_sms_system(gmodem *g,char *mode) { // callback function for DELETE_ALL
static t_sms s[100]; int k;
int r = gmodem_sms_copy(g,s,100);
//printf("Found %d sms, process them\n",r);
for(k=0;k<r;k++) {
  t_sms *sms = s+k;
  char *text=sms->text; char *phone=sms->da;
int i;
 decode_utf8(t,text,strlen(text)); // to win1251
for(i=0;t[i];i++) if ( (t[i]=='\"') || (t[i]=='\'') || (t[i]<=32))  t[i]=32 ; // no bad symbols
 encode_utf8_(text2,t,-1); // back to UTF

sprintf(buf,"export NUM_A='%s'; export NUM_B='%s'; export TXT='%s'; %s ",g->cnum,phone,text2,mode);

printf("CALL on_in_sms: %s\n",buf);

int res = system(buf); // must return 0 for delete message
if (res == 0) {
 if (gmodem_sms_delete(g,sms->pos))
   gmodem_logf(g,2,"OK DELETE SMS#%d TEXT:%s\n",sms->pos,sms->text);
   }
   }
g->cmt=0; // clear flag
return 1;
}

char expect_deliv_report[100]; // number to check deliver on
int  delivered_status=0;

int gmodem_sms_check_dl(gmodem *g) { // callback function for DELETE_ALL
static t_sms s[100]; int k;
int r = gmodem_sms_copy(g,s,100);
//printf("Found %d sms, process them\n",r);
for(k=0;k<r;k++) {
  t_sms *sms = s+k;
  int res=0;
  if (sms->mti != SMS_REPORT) continue;
  //printf("deli")
  if (g->logLevel>=5) sms_print(sms,0);
  int ok = strncmp(sms->text,"<status:0,",10)==0;
  gmodem_logf(g,3," sms_check_dl: Delivered to :%s ok=%d <%s> <my_phone=%s>",sms->da,ok,sms->text,expect_deliv_report);
  int lda=strlen(sms->da);
  int lphone=strlen(expect_deliv_report);
  if (lda>=7 && lphone>=7 && strncmp(expect_deliv_report+lphone-7,sms->da+lda-7,7)==0) {
     delivered_status = atoi(sms->text+8); // code
     gmodem_logf(g,2," sms_check_dl: detect delivery for %s OK=%d, status_code=%d\n",sms->da,ok,delivered_status);
     expect_deliv_report[0]=0; // clear flag
     }
  if (res == 0) {
    if (gmodem_sms_delete(g,sms->pos))
   gmodem_logf(g,2,"OK DELETE SMS#%d TEXT:%s\n",sms->pos,sms->text);
   }
   }
g->cmt=0; // clear flag
return 1;
}

char on_in_sms[512]; // script to call

int on_mt_sms(gmodem *g) { // called when incoming sms here
 if (expect_deliv_report[0]) gmodem_sms_check_dl(g); // check delivery reports and delete it
 if (on_in_sms[0]) gmodem_sms_system(g,on_in_sms); // on_system called
return 1; // ok
}

int gmodem_dl_prepare(gmodem *g,char *phone) { // configuring
  gmodem_Atf(g,"+CPMS=\"MT\""); // read sms from sim & phone
  gmodem_Atf(g,"+CNMI=2,1,0,2"); // store mt and delReports and reports only indexes
  g->on_mt = on_mt_sms; // if not yet
   // char *phone = get_word(&sms);
  strNcpy(expect_deliv_report,phone); // copy delivery report here
  gmodem_sms_check_dl(g); // clear if was old one
  strNcpy(expect_deliv_report,phone); // copy again
   delivered_status = -1; // clear delivery status
return 0;
}

int gmodem_dl_exec(gmodem *g,int ta) { // wait timeout seconds for delivery report
int ms=ta*1000;
for(ms=0;ms<ta*1000;) { // wait 10 sec...
     if (delivered_status>=0) break; // done processing
     while (gmodem_run2(g)>0) ; // process all messages
     msleep(100); ms+=100;
     if (0==ms%2000) {
        gmodem_logf(g,4,"no sms mt %d msec, but check anyway",ms);
        on_mt_sms(g); // check anyway every 10 second
       }
     }
if (delivered_status == -1) on_mt_sms(g); // last appempt, check again
if (delivered_status == -1) return gmodem_errorf(g,1,"sent, undelivered in %d sec",ta);
if (delivered_status == 0)  return gmodem_errorf(g,2,"OK, delivered");
return gmodem_errorf(g,1," sent, but deliver failure smsc code:%d",delivered_status); // ok - found res.
}


int gmodem_sms(gmodem *g,char *sms) {
char phone[24]; //
if (lcmp(&sms,"dump")) {
    uchar out[512],*o=out;
    int l = hexstr2bin(out,sms,-1);
    if (l>0 && (out[0]==0 ))  { o++; l--;}; // if starts from "default SMSC"
    sms_dump(o,l,0) ; //flSmsc );
  return 1;
  }
if (lcmp(&sms,"rm") || lcmp(&sms,"del")) {
  // delete sms from a slot...
  int slot=-2;
  if (lcmp(&sms,"all")) { // remove all messages
      gmodem_sms_enum(g,_gmodem_sms_del, (void*)4); // DELETE ALL
      return 1; // OK - anyway
      }
  if (lcmp(&sms,"rep")) { // delete reports
      gmodem_sms_delete_reports(g);
      }
  if ( (sscanf(sms,"%d",&slot)<=0) || (slot<0) ) return gmodem_errorf(g,-2,"sms sim_slot >=0 expected");
  return gmodem_sms_delete(g,slot);
  }
if (lcmp(&sms,"list") || lcmp(&sms,"ls") ) {
  gmodem_sms_enum(g, _gmodem_sms_print_ls, g ); // just print it
  return 1;
  }
if (lcmp(&sms,"system") ) {
  char *sys = get_word(&sms);
  if (!sys || !sys[0]) sys="echo";
  gmodem_sms_system(g,sys ); // just print it
  return 1;
  }
if (lcmp(&sms,"on_mt") || lcmp(&sms,"on_incoming")) {
  char *sys = get_word(&sms);
  if (!sys || !sys[0]) sys="echo";
  strNcpy(on_in_sms,sys);
  gmodem_dl_prepare(g,"");
  return 1;
  }
if (lcmp(&sms,".send")) { /// sms.send <phone> text ; send text sms and wait for an answer
  if (gmodem_get_phone(g,&sms,phone)<0) return -2;
  gmodem_dl_prepare(g,phone);
  int ok = gmodem_SendTextSms(g,phone,sms,1); // need set TTL = 5min
  if (ok<=0) return ok; // failed
  return gmodem_dl_exec(g,20);
  }
if (lcmp(&sms,".fax+")) {
  if (gmodem_get_phone(g,&sms,phone)<0) return -2;
  return gmodem_SendSpecSms(g,phone,smsInd|indOn|indFax,0);
 }
if (lcmp(&sms,".fax-")) {
  if (gmodem_get_phone(g,&sms,phone)<0) return -2;
  return gmodem_SendSpecSms(g,phone,smsInd|indFax,0);
 }
 if (lcmp(&sms,".vms+")) {
  if (gmodem_get_phone(g,&sms,phone)<0) return -2;
  return gmodem_SendSpecSms(g,phone,smsInd|indOn|indVms,0);
 }
if (lcmp(&sms,".vms-")) {
  if (gmodem_get_phone(g,&sms,phone)<0) return -2;
  return gmodem_SendSpecSms(g,phone,smsInd|indVms,0);
 }
if (lcmp(&sms,".mail+")) {
  if (gmodem_get_phone(g,&sms,phone)<0) return -2;
  return gmodem_SendSpecSms(g,phone,smsInd|indOn|indEmail,0);
 }
if (lcmp(&sms,".mail-")) {
  if (gmodem_get_phone(g,&sms,phone)<0) return -2;
  return gmodem_SendSpecSms(g,phone,smsInd|indEmail,0);
 }
if (lcmp(&sms,".ping")) { /// sms.send <phone> text ; send text sms and wait for an answer
 if (gmodem_get_phone(g,&sms,phone)<0) return -2;
  gmodem_dl_prepare(g,phone);
  int ok = gmodem_SendSpecSms(g,phone,smsInd|indOther,10); // need set TTL = 50min
  //int ok = gmodem_SendOtaSms(g,phone,"",0,10); // need set TTL = 50min
  if (ok<=0) return ok; // failed
  int wtime = 15;
  return gmodem_dl_exec(g,wtime); // wait and expect result
  }
if (gmodem_get_phone(g,&sms,phone)<0)  return gmodem_errorf(g,-2,"usage: <phone> <text>");
return gmodem_SendTextSms(g,phone,sms,0); // default VP
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
int r = gmodem_creg(g); if (r<0) return r; // not registred

return gmodem_SendOtaSms(g,phone,data,bl,0);
//return gmodem_SendTextSms(g,"+79151999003","Привет");
}

//#include "../smppTest/im.c" ZU


int gmodem_im(gmodem *g,uchar *cmd) {
if (lcmp(&cmd,"init")) {
  uchar *phone=get_word(&cmd);
  uchar *key = get_word(&cmd);
  uchar *tar = get_word(&cmd);
//  uchar *master = get_word(&cmd);
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
