#include "gmodem.h"
#include "gsm_sim.h"

char szPoR[]="90,61";

int por_ok(int code) { //Положительные коды PoR
if (code==0x90) return 1;
if (code==0x61) return 1;
return 0;
}


int _gmodem_csim_(gmodem *g, char *cmd ) {
char buf[800];
sprintf(buf,"+CSIM=%d,\"%s\"",strlen(cmd),cmd);
if (gmodem_At2bufFilter(g,buf,"+CSIM", g->out,sizeof(g->out))<0) return -1; // error already reported
cmd = g->out;
int code = -1;
printf("RESP:<%s>\n",g->out);
char * p = gmodem_par(&cmd,1);
strNcpy(g->out,p); // copy here
return 1;
}

int _gmodem_csim0(gmodem *g, char *cmd ) { // deprecated
int code = _gmodem_csim_(g,cmd);
/*
char buf[800];
sprintf(buf,"+CSIM=%d,\"%s\"",strlen(cmd),cmd);
if (gmodem_At2bufFilter(g,buf,"+CSIM", g->out,sizeof(g->out))<0) return -1; // error already reported
*/
cmd = g->out;
printf("RESP:<%s>\n",g->out);
if (code>0) sscanf((void*) gmodem_par(&cmd,1),"%04x",&code);
//cmd = gmodem_par(&cmd,0);
//strNcpy(g->out,cmd);
if (code<=0) {
  sprintf(g->out,"CSIM result %d unexpected, wait: code,resp",code);
  return -2;
  }
printf("CSIM_RESP: <%d,0x%x>\n",code,code);
return code;
}

int gmodem_GetResponce(gmodem *g, int len) {
char buf[30];
sprintf(buf,"A0C00000%02x",len);
printf("Begin GetResponce len=%d\n",len);
g->out[0]=0; // _gmodem_csim(g,b);
return _gmodem_csim(g,buf); // get last resp
}


int _gmodem_csim(gmodem *g, char *cmd ) {
//int code = _gmodem_csim_(g,cmd);
if (g->mode == 0) return _gmodem_csim0(g,cmd); // CRSIM
/*
char buf[800];
sprintf(buf,"+CSIM=%d,\"%s\"",strlen(cmd),cmd);
if (gmodem_At2bufFilter(g,buf,"+CSIM", g->out,sizeof(g->out))<0) return -1; // error already reported
*/
int code;
uchar c[256];  int bl = hexstr2bin(c,cmd,-1); // convert to binary
uchar sw[2], out[256];
//hexdump("_gmodem_csim apdu", c,bl);
if (gmodem_apdu2(g,c, bl, sw, out)>=0)
    {
    code = sw[0];
    code = (code<<8) + (sw[1]);
    //printf("We Are Here!\n");
    }
    else {
        code = -3;
        printf("--- BAD APDU CSIM !!! there\n");
    }
//printf("RESP:<%s>\n",g->out);
//if (code>0) sscanf( gmodem_par(&cmd,1),"%04x",&code);
//cmd = gmodem_par(&cmd,0);
//strNcpy(g->out,cmd);
printf("SW:%04x ; %02x:%02x\n",code,sw[0],sw[1]);
if (code<=0) {
  sprintf(g->out,"CSIM result %d unexpected, wait: code,resp",code);
  return -2;
  }
printf("CSIM_RESP: <%d,0x%x>\n",code,code);
return code;
}

void hexdump2(char *msg, uchar *data,int len) {
printf("hexdump2 %s len=%d(0x%x)\n",msg,len,len);
int i; for(i=0;i<len;i++) printf("%02X",data[i]);
printf("\n");
}

int apdu_add_secure(uchar *apdu,
                       int spi, int kic, int kid, char tar[3], // profile
                       void *KIC,void *KID, int counter // personal
                    ) {
// add secure header, for now SPI=0001, KIC=KID=0  - 13 bytes
int l=apdu[0]; uchar *ud = apdu+1; // current  user_data & len
int  hlen=0xD,rc_sz=0;
//printf("SPI use=%d, %d\n",spi,(spi>>8));
if (spi>>8) { rc_sz=8; hlen+=rc_sz; } // HAVE security, SIGN 8 bytes?
int sz=l+hlen+2+1;  // hlen=13 bytes if no RC/CC
 //printf("SPI use=%d hlen=%x sz=%d rc_sz=%d counter=%d\n",spi,hlen,sz,rc_sz,counter);
uchar buf[sz], *p=buf; // new size
*(short*)p=htons( l+hlen+1 ); p+=2; *p=hlen; p++;
*(short*)p=htons( spi); p+=2; *p = kic; p++; *p=kid; p++; memcpy(p,tar,3); p+=3;
memset(p,0,5); p++; if (spi>>8) *(int*)p=htonl(counter); // setup counter
            p+=4;
             *p=0; p++; // counter + pcntr
if (rc_sz) { // место для имитовставки
  memset(p,0,rc_sz); p+=rc_sz;
  }
memcpy(p,ud,l);
memcpy(ud,buf,sz); apdu[0]=sz;
if (spi>>8) {
     //hexdump2("sign_KID",KID,8);
     cbc_sign_11(apdu+1,apdu[0],KID); // Sign with user key
     }
//hexdump2("signed_packet",apdu+1,apdu[0]);
return sz; // ok
}

int apdu_add_udh70(uchar *apdu) {
int l = apdu[0];
uchar buf[l+3]; buf[0]=02; buf[1]=0x70; buf[2]=0x00;
memcpy(buf+3,apdu+1,l); l+=3;
memcpy(apdu+1,buf,l);
apdu[0]=l;
return l;
}

int apdu_pack2delivr(uchar *apdu,char *_num) {
int l = apdu[0]; char num[24];
int nl = sms_put_addr2(num,_num,strlen(_num));
int sz = l+11+nl+2;
uchar buf[sz],*c=buf;
c[0]=0x8B;
 int rl = 11+nl+l;
 if (rl>=0x80) {
     c[1]=0x81;c[2]=rl; c+=3; sz++; // SMS_PDU_TAG + length followes
   } else {
   c[1]=11+nl+l; c+=2; // SMS_PDU_TAG + length followes
   }
c[0]=0x40; c++; memcpy(c,num,nl); c+=nl; *c = 0x7F; c++; *c=0xF6; c++; // MTI, OD, PID, DCS
char smsc_ts[7]={0x14,0x06,0x04,0x17,0x31,0x49,0x04 }; // YYMMDDHHNNSS offset
memcpy(c,smsc_ts,7); c+=7;
*c = l; c++; memcpy(c,apdu+1,l); //udlen & ud here
memcpy(apdu+1,buf,sz); apdu[0]=sz;
return sz;
}

int apdu_ins(uchar *apdu,uchar *head,int len) {
int l = apdu[0], sz = l+len; // now_len and new_size
uchar buf[sz];
memcpy(buf,head,len); memcpy(buf+len,apdu+1,l);
memcpy(apdu+1,buf,sz); apdu[0]=sz;
return sz; // new size
}

int apdu_pack2env(uchar *apdu) {
uchar data_id[4]={0x82,0x02,0x83,0x81};   apdu_ins(apdu,data_id,4); // add data identities
int l = apdu[0];
if (l<0x80) {
 uchar sms_download[2]={0xD1,apdu[0]};     apdu_ins(apdu,sms_download,2);
 uchar a[5]={0xA0,0xC2,0x00,0x00,apdu[0]}; apdu_ins(apdu,a,5);
} else {
uchar sms_download[3]={0xD1,0x81,apdu[0]};     apdu_ins(apdu,sms_download,3);
uchar a[5]={0xA0,0xC2,0x00,0x00,apdu[0]};   apdu_ins(apdu,a,5);
  }
return 1;
}

void gmodem_dump_out(gmodem *g, char *msg) {
if (g->logLevel>3) hexdump(msg ,g->out+1,g->out[0]);
}

extern uchar SW[2];



int gmodem_csim_apdu(gmodem *g, uchar *in, uchar *out ) {
char buf[800],cmd[512];
bin2hexstr(cmd,in+1,in[0]); // hexstr
sprintf(buf,"+CSIM=%d,\"%s\"",strlen(cmd),cmd);
if (gmodem_At2bufFilter(g,buf,"+CSIM", g->out,sizeof(g->out))<0) return -1;
if (!g->out[0]) gmodem_errorf(g,-2,"expected +CSIM output not found");
uchar *c = g->out;
uchar *p = gmodem_par(&c,1);
//printf("DEC1<%s>\n",p);
int l = hexstr2bin(out+1,p,strlen(p));
if (l<2) {
  printf("Invalid CSIM answer %d - expect at least 2 bytes\n",l);
  return -2;
  }
out[0]=l; // zu;
bin2hexstr(g->out,out+1,out[0]); // convert it back
//printf("DEC2<%s>\n",p);
return 1; // exchange OK
}

int gmodem_apdu_exchange(gmodem *g,uchar *in,uchar *out) { // returns (SW[0]<<8)+SW[1] or ZERO on any ERROR
SW[0]=SW[1]=0; uchar dummy[258];
if (!out) out=dummy+2;
int code;

if (g->logLevel>1) {
 uchar buf[514];
 bin2hexstr(buf,in+1,in[0]);
 printf(">>%s\n",buf);
 }

if (g->mode == 0) code = gmodem_csim_apdu(g,in,out);
        else      code = phoenix_apdu2(g,in+1,in[0],dummy,out);
//printf("Code=%d\n",code);
if (code<0) { // transport fail, error already reported
    return code;
    }
int l = out[0];
//printf("l=%d\n",l);
if (l<2) return gmodem_errorf(g,-2,"APDU_EXCHANGE error, expect at least 2bytes out");

bin2hexstr(g->out,out+1,out[0]); // copy text out here...
if (g->logLevel>1) printf("<<%s\n",g->out);

memcpy(SW,out+1+l-2,2); // Last Status Bytes
//printf("SW=%02x %02x\n",SW[0],SW[1]);
return (SW[0]<<8)+SW[1];
}

int gmodem_apdu_GetResponce(gmodem *g,int len,uchar *out) {
uchar cmdGetResponce[]={ 5, 0xA0,0xC0,00,00, len };
int code = gmodem_apdu_exchange(g,cmdGetResponce,out);
if (code<=0) {
    printf("GetResponceFailed\n");
    return -2;
   }
if  ( ( SW[0]>>4) !=0x9 ) { // Valid Answers 9X XX
    printf("GetResponce: Unexpected result of getResp %04X\n",code);
    return -3; //
   }
return code;
}

char *porStatusName( int code ) {
char *err[]={"PoR OK","RC/CC/DS failed","CNTR low","CNTR high","CNTR Blocked","Ciphering error",
   "Unidentified security error","Insufficient memory","Need more time","TAR Unknown","Insufficient security"};
if (code>=0 && code<=10) return err[code];
return "undescribed";
}

int porDecode(porPacket *pack,uchar *d,int len) {
memcpy(pack->data,d,len); d=pack->data; // copy data inside
int l = d[0]; l++; d+=l; len-=l; // remove UDH -> 02 71 00
//printf("Skip UDH %d bytes\n",l);
// now - dump rest
int CPL = htons(*(short*)d);
if (CPL != (len-2) ) {
   printf(pack->data,"WrongCPL expect %d found %d",len-2,CPL);
   return -1;
   }
d+=2; len-=2;
// now - SEQPACK, POR, SW1SW2
//hexdump("por_nohead",d,len);
l = d[0];
if ( (l!=0xA ) && (l!=0xD)) {
  sprintf(pack->data,"Unxpected User Header need: A,D found %x",l);
  return -1;
  }
{ // print seq_packet
  uchar *s=d+1;
  //int rcode = s[l-1]; // responce status code, 0 = OK, 23.048, Table 5: Response Status Codes
  pack->status = s[l-1];
  //printf("POR code: %d, '%s'\n",rcode,porResultName(rcode));
  }
l++; d+=l; len-=l;
pack->ex_data = d; pack->ex_data_len = len;
if (l>=3) { // extra - coding unknowns
  pack->command=d[0];
  pack->sw[0]=d[1];
  pack->sw[1]=d[2];
  }
//hexdump("AdditionalData",d,len);
return 1;
}



int gmodem_apdu_POR(gmodem *g,porPacket *por) { // returns POR status code
uchar out[256];
int code = gmodem_apdu_GetResponce(g, SW[1],out); // GetLastResponce result
if (code<=0) return code;
if (porDecode(por,out+1,out[0]-2)<=0) {
   sprintf(g->out,"POR_decode_failed:%s",por->data);
   printf("FAIL: %s\n",g->out);
   return -2; // decode error
   }
if (g->logLevel>2) printf("gmodem_apdu_POR ok get status:%d\n",por->status);
// now - decode it
//out[0]-=2; // remoce status word
//hexdump("POR:",out+1,out[0]);
//hexdump("POR:",out+1,out[0]-2);
//dumpPOR(out+1,out[0]-2); // without SW0,SW1
return code;
}


int gmodem_apdu_wrapSend(gmodem *g,uchar *data) { // ZU - dump & data
struct _cardJob *j = &cardJob;
char tar[3]={0,0,0};
  apdu_add_secure(data, j->spi, j->kic, j->kid , tar,
                  j->KIC,j->KID, j->counter );
gmodem_dump_out(g,"sec_packet");
  apdu_add_udh70(data);
gmodem_dump_out(g,"sms_UD");
  apdu_pack2delivr(data,"556677");
//gmodem_dump_out(g,"sms_pdu");
  apdu_pack2env(data);
// -- now - we may be want to sign it?
//  sec_packet?

//gmodem_dump_out(g,"ENVELOPE");
porPacket *p=&j->por;
if (gmodem_apdu_exchange(g,data,0)<=0) return -2;
 j->counter++;  if (j->saveCounter) j->saveCounter(j); // updateCounterValue
if (gmodem_apdu_POR(g,p)<=0) return -2;
// now - check POR.status field, 0 - is a good responce
if (p->status!=0) {
    sprintf(g->out,"POR_err[%d]:%s, {new_counter:%d}",p->status,porStatusName(p->status),j->counter);
    printf("ERROR:%s\n",g->out);
    return -3;
    }
if (g->logLevel>1) printf(" ---  PoR:%d (%s) Command:%d SW:%02x%02x nextCounter:%d--- \n",
                            p->status,porStatusName(p->status),p->command,p->sw[0],p->sw[1],j->counter);
if (!por_ok( p->sw[0])) {
   return gmodem_errorf(g,-3, "PoR return error status, expect %s found %02x",szPoR,p->sw[0]);
   }
return 1;
}

char *szaid(uchar *aid) {
static uchar szbuf[100];
bin2hexstr(szbuf,aid+1,aid[0]);
return szbuf;
}

/*
int gmodem_apdu_delete(gmodem *g,uchar *aid,int l) {
char buf[256];
memcpy(g->out+1,aid,l); g->out[0]=l; // set aid as begin of g->out
uchar del[7]={0x80,0xE4,0x00,0x00,l+2,0x4F,l}; apdu_ins(g->out,del,7);
if (g->logLevel>1) {
  printf("cmd: DELETE %s\n",szaid(aid));
  }
gmodem_dump_out(g,"delete command");
int code = gmodem_apdu_wrapSend(g,g->out);
if (g->logLevel>1) {
  printf("res: %04x DELETE %s\n",code,szaid(aid));
  }
return code;
}
*/



int gmodem_apdu_delete_aid(gmodem *g,uchar *aid) {
char buf[256];
int l = aid[0];
memcpy(g->out,aid,l+1); // copy len+aid to g->out
uchar del[7]={0x80,0xE4,0x00,0x00,l+2,0x4F,l}; apdu_ins(g->out,del,7);
gmodem_logf(g,2,"==== cmd: DELETE %s",szaid(aid));
if (g->logLevel>4) gmodem_dump_out(g,"delete command");
int code = gmodem_apdu_wrapSend(g,g->out);
if (code>0) {
  uchar *sw = cardJob.por.sw;
  if (!por_ok(sw[0])) {
      code = -3; // unwanted result
      gmodem_logf(g,1,"  delete PoR.SW=%02X%02X (expect %s)",sw[0],sw[1],szPoR);
     }
  }
gmodem_logf(g,2,"==== res: DELETE %04x %s\n",code,szaid(aid));
if (code>0) gmodem_outf(g,code,"deleted:   %s",szaid(aid)); // correct responce
return code;
}





/*#ifndef WIN32
int filelength(int file) {
struct stat st;
if (fstat(file,&st)!=0) return -1;
return st.st_size;
}
#endif
*/



int gmodem_apdu_load(gmodem *g,uchar *aid, int l, char *filename) { // open file and loads its
char buf[256];
int file,fileLen;

file = open(filename,O_RDONLY);
if (file<0) return gmodem_errorf(g,-2,"apdu_load:  \"%s\"  fail open file",filename);
fileLen = filelength(file);
gmodem_logf(g,2,"start load %d bytes from '%s'",fileLen,filename);
//uchar cmd_end[10]={0x00,0x00,0x06,0x06,0xEF04,0xC6,0x02,0x00,0x00,0x00};
uchar cmd_end[10]={0x00,0x00,0x06,0xEF,0x04,0xC6,0x02,0x00,0x00,0x00};
g->out[0]=0;
  apdu_ins(g->out,cmd_end,10); // starts from end
  uchar len=l;
  apdu_ins(g->out,aid,l); apdu_ins(g->out,&len,1); // AID len+data
  len = g->out[0]; // update len
uchar cmd[5]={0x80,0xE6,0x02,0x00,len}; apdu_ins(g->out,cmd,5); // attach head

//80E6 0200 14 ; install <02=for_load> p1,p2 len
//09 FF434E525801011AFF - len and aid
//00 len&sec domain
//00 len&blockhash
//06 EF04 C6020000 ; len & loadparameters (min non-volatile code 00 bytes) ???
//00
// ; length & load token

//gmodem_dump_out(g,"install_for_load command");
int code;

code = gmodem_apdu_wrapSend(g,g->out);
int porSW = cardJob.por.sw[0];

if ((porSW!=0x61 ) && (porSW!=0x90) ) { // ZUZUKA - can we relay on that?
  return gmodem_errorf(g,-99,"EXPECT por.sw in (61,90) found %x\n",cardJob.por.sw[0]);
  }

int sz = 0x40, SZ=fileLen + 4; // send by 64 bytes, SZ = app.bin + 4 bytes head
int count = SZ/sz; if (SZ%sz) count++; // count of blocks
gmodem_logf(g,2,"needs %d blocks with size %d (0x%x)",count,sz,sz);
int b;
time_t started,now; int dur,pdur;
time(&started);
for(b=0;b<count;b++) {
  int r;
  g->out[0]=0;
  uchar data[sz]; // block to write
  if (b==0) { // first block starts with header
     data[0]=0xC4; data[1]=0x82; *(short*)(data+2)=htons(fileLen); // set chunk data
     r = read(file,data+4,sz-4); // read rest
     if (r>0) r+=4;
     }  else {
     r = read(file,data,sz); // read data
     }
  if (r<=0) {
      gmodem_errorf(g,-2,"file read error :%d",r);
      return -2;
    }
  // ok - compoze it
  int last = count-1 == b;
  char head[5]={0x80,0xE8,last?0x80:00,b,r};
  g->out[0]=5+r; memcpy(g->out+1,head,5); memcpy(g->out+1+5,data,r); // copy data
  if (g->logLevel>3) gmodem_dump_out(g,"load command");
  //if (b>1) exit(2);
  //printf("logLevel = %d\n",g->logLevel);
  time(&now);
  dur=now-started, pdur = 0;
  if ( b>0 ) {
     pdur = (count*dur)/b;
     }
  if (g->logLevel == 1) { // normal operation - show progress
         fprintf(stderr,"\r LOADING %s (%d%c): %d/%d  dur:%d/%d  eta:%d      ",filename, (b*100)/count,'%',b+1,count,
                  dur,pdur, (pdur-dur) );
         fflush(stderr);
         } else { // push log info
         gmodem_logf(g,2," LOADING (%d%c): %d/%d",(b*100)/count,'%',b+1,count);
         }
  code = gmodem_apdu_wrapSend(g,g->out);
  if (g->logLevel>2) printf(" res: %d for load\n",code);
  if (code<=0) break;
  }
if (g->logLevel == 1) fprintf(stderr,"                                                                  \r"); // clear out
gmodem_logf(g,2,"LOAD DONE with code: %d\n",code,code);
close(file);
if (code>0) return gmodem_outf(g,code,"loaded:    {bytes:%d,blocks:%d,dur:%d}",fileLen,count,dur);
return code;
}

/*
80E6 0C00 6A ; install selectable_for_install
09 FF434E525801011AFF exe ID
10 FF434E525801011A00000000011A0000; modID
10 FF434E525801011A00000000011A0000; appID
0100 privs
3A - len of install_parameters
EF15C8020000C7020000CA0B0100FF07
14010A0A000000C921010F0F0F0F0004
44014010140252F09352F01000000211
000801000481031300F6
00 - installtocken
*/

void apdu_push(uchar *apdu,uchar *data,int len) { // push to end
memcpy(apdu+1,data,len); apdu[0]+=len;
}

#define pushTAG(a) { apdu_ins(g->out,a,a[0]+1);} // tag with len

extern uchar SW[2]; // laster resp codes

int gmodem_resp(gmodem *g) { // GetResponce from a last command
int p1,p2;
//sscanf(g->out,"%02x%02x",&p1,&p2); //  Based On Last Result in g->out
return gmodem_GetResponce(g,SW[1]); // result in g->out
}

int gmodem_apdu_install(gmodem *g,uchar *exeAID,uchar *modAID,uchar *appID,uchar *instPar) { // open file and loads its
char buf[256];
//uchar cmd_end[10]={0x00,0x00,0x06,0x06,0xEF04,0xC6,0x02,0x00,0x00,0x00};
  uchar cmd_end[10]={0x00,0x00,0x06,0xEF,0x04,0xC6,0x02,0x00,0x00,0x00};
g->out[0]=0; uchar ch=0; apdu_push(g->out,&ch,1) ; // instTockem
 //gmodem_dump_out(g,"ZERO");
 pushTAG(instPar);

 uchar priv[]={0x1,00}; pushTAG(priv);

 //gmodem_dump_out(g,"inst");
 pushTAG(appID);
 //gmodem_dump_out(g,"inst+app");

 pushTAG(modAID);
 pushTAG(exeAID);
 //gmodem_dump_out(g,"allAID");
uchar cmd_install[5]={0x80,0xE6,0x0C,0x00,g->out[0]};
 apdu_ins(g->out,cmd_install,5);
gmodem_dump_out(g,"install install+make_selectable");
int code = gmodem_apdu_wrapSend(g,g->out);
if (code>0) return gmodem_outf(g,code,"installed: %s",szaid(appID));
 //gmodem_resp(g);
return code;
}

typedef uchar aid[256];

int get_aid(aid out,uchar **cmd) {
uchar *p = get_word(cmd);
out[0]=hexstr2bin(out+1,p,-1);
return out[0];
}

/* small bin-string char [0]=len, char 1 - starts data (up 255 bytes) */
int _s_ins(uchar *buf,int pos, void *d, int l) {
int len = buf[0]; uchar *data = buf+1; // current length & buffer starts
if (len+l > 255 ) { // overflow - attach zeros to end and returns 0 - false
   memset(data+len,0,255-len);
   buf[0]=255;
   return 0; // ERROR
   }
if (pos==-1) pos = len; // add to end flag
if (pos<0 || pos>len) return 0; // invalid - out of data
memcpy(data+pos,data+pos+l,len-pos); // move data
memcpy(data+pos,d,l); // setcurrent
buf[0]+=l;
return 1; // OK
}

//int _s_cat(uchar *buf,int pos ,uchar *s2) { }

int gmodem_read(gmodem *g,uchar *buf,int len) {
int to; int i;
for(i=0;i<1000;i++) {
  gmodem_run(g);
  if (g->in_len>=len) {
      memcpy(buf,g->in,len);
      memcpy(g->in , g->in+len, g->in_len-len); // move reset
      g->in_len-=len;
      return 1; // OK
     }
  msleep(100);
  }
printf("TimeOut wait !!! \n");
return -2; // timeout
}

int phoenix_write_echo(gmodem *g, uchar *cmd, int len) {
uchar buf[256];
g->in_len = 0;
while(len>0) {
  int l = len;
  gmodem_put(g,cmd,l);
 // g->in_len = 0;
  gmodem_read(g,buf,l);
  if (buf[0]!=cmd[0]) {
        printf("Fail read get %x expect %x\n",buf[0],cmd[0]);
        exit(999);
        return -2;
        }
  cmd+=l; len-=l;
  }
return 1; // ok
}

int _gmodem_csim_XXX(gmodem *g, char *cmd ) {
char buf[800];
sprintf(buf,"+CSIM=%d,\"%s\"",strlen(cmd),cmd);
if (gmodem_At2bufFilter(g,buf,"+CSIM", g->out,sizeof(g->out))<0) return -1; // error already reported
cmd = g->out;
int code = -1;
printf("RESP:<%s>\n",g->out);
char * p = gmodem_par(&cmd,1);
strNcpy(g->out,p); // copy here
return 1;
}


int gmodem_apdu2(gmodem *g,uchar *cmd,int len,uchar *sw,uchar *out) {
uchar cbuf[512],buf[800],*p; int i;
if (g->mode == 1) return phoenix_apdu2(g,cmd,len,sw,out);
for(i=0;i<len;i++) sprintf(cbuf+2*i,"%02X",cmd[i]);
sprintf(buf,"+CSIM=%d,\"%s\"",strlen(cbuf),cbuf);
if (gmodem_At2bufFilter(g,buf,"+CSIM", g->out,sizeof(g->out))<0) return -1; // error already reported
uchar *pcmd = g->out;
int code = -1;
printf("apdu2 CSIM RESP:<%s>\n",g->out);
 p = gmodem_par(&pcmd,1);
 printf("apdu2 VALUE - 0:<%s>\n",p);
// we must have at least 2 bytes - SW?
 //strNcpy(g->out,p); // copy here
printf("apdu2 VALUE:<%s>\n",p);
int bl = hexstr2bin(buf,p,-1);
hexdump("binary",buf,bl);
if (bl<2) return -3; // ???
memcpy(sw,buf,2); out[0]=bl-2; memcpy(out+1,buf+2,bl-2); // copy result here
return 1;
}

int bin2hexstr(uchar *out,uchar *bin,int len) {
int i;
for(i=0;i<len;i++) sprintf(out+2*i,"%02X",bin[i]);
return 2*len;
}


int phoenix_apdu(gmodem *g,uchar *cmd, int  len) {
uchar sw[2],out[256];
int code = phoenix_apdu2(g,cmd,len,sw,out);
printf("SW: %02x %02x\n",sw[0],sw[1]);
  hexdump("OUT",out+1,out[0]);
return code;
}


int gmodem_apdu_exchange_sw(gmodem *g,uchar *in,uchar *out,uchar *sw) { // remove SW from ok-out
int code = gmodem_apdu_exchange(g,in,out); if (code<=0) return code;
int l = out[0];
//hexdump("ex_sw out",out+1,out[0]);
if (sw) memcpy(sw,out+1+l-2,2); out[0]-=2; // move bytes
return code;
}

int p_freeEEPROM, p_freeRAM = -1;

int apdu_GetData0(gmodem *g) {
uchar aidCardManager[]={8,0xA0,00,00,00,03,00,00,00};
//uchar aidCardManager[]={7,0xA0,00,00,00,03,00,00};
uchar getData[]={ 5, 0x80,0xCA,0xFF,0x21,0x10 };
uchar out[256],sw[2];
if (g->logLevel>1) printf("==getData begin\n");

int code = gmodem_apdu_select(g,aidCardManager);
if ( (code >> 8) != 0x61 ) return gmodem_errorf(g,-3,"getData failed select cardManager (code:%04X,expect  6147)",code);

if (gmodem_apdu_exchange_sw(g,getData,out,sw)<=0) {
     //gmodem_atr(g); // reset modem - ZUZUKA
    return -2;
    }
if (sw[0]!=0x90) return gmodem_errorf(g,-3,"getData expect 90XX, found %0X",sw[0]);
g->out[0]=0; // clear text
//printf("SW: %02x %02x\n",sw[0],sw[1]);
if (out[0]>=14) {
   int clen = out[0]-1-3; // tags len
   uchar *c = out+1+3; // skip header
   int freeEEPROM=-1,freeRAM=-1;
   while ( clen >0 ) {
       int t = c[0]; int  l = c[1];
       c+=2; clen--;
       if (t == 0x82 && l==4 ) freeEEPROM=htonl( *(int*)(c));
       if (t == 0x83 && l==2)  freeRAM=htons( *(short*)(c));
       c+=l; clen-=l;
       }
   char delta[256]={0};
   if (p_freeRAM>=0) {
       sprintf(delta," delta:{EERPOM:%d,RAM:%d,TOTAL:%d}",p_freeEEPROM-freeEEPROM,p_freeRAM-freeRAM,
                 (p_freeEEPROM-freeEEPROM)+(p_freeRAM-freeRAM)
                  );
       }
   p_freeEEPROM =freeEEPROM; p_freeRAM = freeRAM; // remember
   sprintf(g->out,"free EEPROM:%d(hex:%x) RAM:%d(hex:%x) %s",freeEEPROM,freeEEPROM,freeRAM,freeRAM,delta);
   if (g->logLevel>1) printf("==getData OK: %s\n",g->out);
   gmodem_atr(g); // reset modem - ZUZUKA
   return 1; // OK
   }
hexdump("INVALID_getData answer",out+1,out[0]);
return -2;
}

int apdu_GetData(gmodem *g) {
int code = apdu_GetData0(g);
gmodem_atr(g);
return code;
}

int _gmodem_csim_s(gmodem *g, uchar *bin) { // binary send
uchar sw[2],out[256];
return gmodem_apdu2(g,bin+1,bin[0],sw,out);
}

int gmodem_apdu_read_binary(gmodem *g, int file, int len, char *out) {
//uchar cmd[]={0xA0, READ_BIANRY, (file)>>8, file & 0xFF , len };

}


int gmodem_apdu_send(gmodem *g, uchar *apdu) {
//uchar bin[255]={ 4, 0x00,0xA4,04,00}; // //00A40400 <LEN> AID
//_s_ins(bin,-1, aid, aid[0]+1); // cat with len
return _gmodem_csim_s(g,apdu);
//return gmodem_
}

int gmodem_apdu_select(gmodem *g, uchar *aid) {
uchar bin[255]={ 4, 0x00,0xA4,04,00}; // //00A40400 <LEN> AID
_s_ins(bin,-1, aid, aid[0]+1); // cat with len
//return _gmodem_csim_s(g,bin);
return gmodem_apdu_exchange(g,bin,0);
}

int gmodem_apdu_select_file(gmodem *g, int FILE) { // FILE-2bytes
uchar bin[8]={ 7, 0xA0,0xA4,00,00, 02, (FILE>>8)&0xFF, FILE&0xFF }; // //00A40400 <LEN> AID
int code = gmodem_apdu_exchange(g,bin,0);
if ( (code <0) || ((code>>8)!=0x9F) ) return 0;
return 1; // seleted
}



int gmodem_updateJobCard(gmodem *g, uchar *iccid) {
      struct _cardJob *j = &cardJob;
      if (cardJobLoadCfg( 0 , iccid, 0 )<=0) {  // load defailt card info
          gmodem_logf(g,1,"UNKNOWN ICCID:%s, default no-security applied");
          return -1;
           } else {
           gmodem_logf(g,1,"ICCID:%s SPI:%04X COUNTER:%d MSISDN:%s IMSI:%s",j->ICCID,j->spi,j->counter,j->MSISDN,j->IMSI);
           // definitions!!! SPI & KIC & KID
           return 1; // OK
           }

}

int  gmodem_apdu_iccid(gmodem *g) {
uchar readBinary[]={ 5 , 0xA0,0xB0, 00, 00, 0xA };
uchar out[256],sw[2],iccid[24];
 gmodem_apdu_select_file(g,0x3F00); // MasterFile
 gmodem_apdu_select_file(g,EF_ICCID);
 out[0]=0; memset(iccid,0,sizeof(iccid));
 gmodem_apdu_exchange_sw(g,readBinary,out,sw);
 int i; for(i=0;(i<12) && (i<out[0]);i++) {
    char hex[3]; sprintf(hex,"%02X",out[i+1]);
    iccid[2*i]=hex[1]; iccid[2*i+1]=hex[0];
    }
 i = strlen(iccid); iccid[i-1]=0; // remove last digit - LUN number
sprintf(g->out,"%s",iccid);
if (cardJob.ICCID[0]==0)  gmodem_updateJobCard(g,iccid); // if not inited yet
return 1;
}



int gmodem_apdu_cmd(gmodem *g, uchar *cmd) {

//init_personal(); // ICCID, KID & counter
if ( cardJob.ICCID[0]==0 ) { // not inited
  if (g->mode == 0) { // E1550 cant read files EF_ICCID ?
      if (gmodem_iccid(g)>0)
         gmodem_updateJobCard(g,g->iccid); // try define it
      printf("JobUpdated for %s\n",g->iccid);
    } else {
    gmodem_apdu_iccid(g);
    }
  };
if (lcmp(&cmd,"iccid")) { // personalize me
   return gmodem_apdu_iccid(g);
   }
if (lcmp(&cmd,"delete")) { // do delete AID
    //uchar *aid =get_word(&cmd);
  //int l = hexstr2bin(aid,aid,-1);
  //return gmodem_apdu_delete_aid(g,aid,l);
  uchar aid[256];
  if (! get_aid(aid,&cmd)) {
     printf("DELETE <aid>\n");
     return -1; // syntax
    }
  return gmodem_apdu_delete_aid(g,aid);
  }
if (lcmp(&cmd,"load")) { // do delete AID
  uchar *aid =get_word(&cmd);
  int l = hexstr2bin(aid,aid,-1);
  return gmodem_apdu_load(g,aid,l,cmd); // aid, filename
  }
if (lcmp(&cmd,"install")) {
  uchar exe[256],mod[256],app[256],inst[256];
  if ( get_aid(exe,&cmd) && get_aid(mod,&cmd) && get_aid(app,&cmd) && get_aid(inst,&cmd)) {
      //      hexdump("EXE AID",exe+1,exe[0]);
      //hexdump("MOD AID",mod+1,mod[0]);
      //hexdump("APP AID",app+1,app[0]);
      //hexdump("INST PAR",inst+1,inst[0]);
      return gmodem_apdu_install(g,exe,mod,app,inst);
      }
  printf("USAGE install <exeAID> <modAID> <appAID> <instParams>\n");
  return -2;
  }
if (lcmp(&cmd,"select")) {
   uchar aid[256];
   if ( get_aid(aid,&cmd) ) {
        hexdump("AID",aid+1,aid[0]);
        return gmodem_apdu_select(g,aid);
      } else
      {
          printf("USAGE select <aid>\n");
          return -2;
      }
  }
if (lcmp(&cmd,"send")) {
   uchar aid[256];
   if ( get_aid(aid,&cmd)<=0 ) {
       printf("Need bin data");
       return -3;
      }
   return gmodem_apdu_send(g,aid);
  }
if (lcmp(&cmd,"put")) {
  uchar bin[256];
  g->in_len = 0;
  int bl = hexstr2bin(bin,cmd,-1);
  gmodem_put(g,bin,bl);
  return 1;
  }
if (lcmp(&cmd,"getData")) {
  return apdu_GetData(g);
  }
if (lcmp(&cmd,"console")) {
  while(1) {
     uchar buf[256],aid[256],*c=buf;
     if (kbhit2()) {
      //printf(">");
      gets(buf);
      if (get_aid(aid,&c)<=0) break;
      g->in_len = 0;
      gmodem_put(g,aid+1,aid[0]); // just write data
      }
     msleep(100);
     gmodem_run(g);
     }
  printf("done apdu console\n");
  }
sprintf(g->out,"apdu usage: delete|load|install|send");
return -1;
}
