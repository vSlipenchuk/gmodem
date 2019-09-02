#include "gmodem.h"
#include "vstrutil.h"
#include <stdio.h>

#include "gsm_sim.h"

#include "coders.h"
#include "vstrutil.h"



//DF_TELECOM 0x7f10
//EF_MSISDN (6F40).
//EF_SPN (6F46)

//http://www.slatedroid.com/topic/19741-information-and-test-with-3g-modems/page__st__80
//setcnum
//AT+CRSM=220,28480,1,4,26,FFFFFFFFFFFFFFFFFFFFFFFF07919343193254F6FFFFFFFFFFFF
//AT+CRSM=220,28480,1,4,28,FFFFFFFFFFFFFFFFFFFFFFFFFF07919343193254F6FFFFFFFFFFFFFF
//AT+CRSM=220,28400,1,4,28,FFFFFFFFFFFFFFFFFFFFFFFFFF07919343193254F6FFFFFFFFFFFFFF
  //AT+CRSM=220,28400,1,4,29,FFFFFFFFFFFFFFFFFFFFFFFFFFFF07919761206687F8FFFFFFFFFFFFFF
//AT+CRSM=220,28400,1,4,28,FFFFFFFFFFFFFFFFFFFFFFFFFF08919343193254F6FFFFFFFFFFFF
                      //  "FFFFFFFFFFFFFFFFFFFFFFFFFFFF07919761206687F8FFFFFFFFFFFF"
    //+CRSM=220,28400,1,4,28,FFFFFFFFFFFFFFFFFFFFFFFFFF07919343193254F6FFFFFFFFFFFFFF
//AT+CRSM=220,28480,1,4,28,FFFFFFFFFFFFFFFFFFFFFFFFFFFF03812143FFFFFFFFFFFFFFFFFFFF
    //+CRSM=220,28400,1,4,28,FFFFFFFFFFFFFFFFFFFFFFFFFF07919343193254F6FFFFFFFFFFFFFF
//read: AT+CRSM=178,28480,1,4,28
                        //    "FFFFFFFFFFFFFFFFFFFFFFFFFFFF07919761206687F8FFFFFFFFFFFF"
//AT+CRSM=178,28480,1,4,28
 //AT+CRSM=178,28480,1,4,28
//cnum_set +39349123456
//cnum_set +79101779668
//FFFFFFFFFFFFFFFFFFFFFFFFFFFF
//0791
//9761206687F8FFFFFFFFFFFF
//791602

//AT+CRSM=178,28480,1,4,28
//read  FFFFFFFFFFFFFFFFFFFFFFFFFFFF07919761206687F8FFFFFFFFFFFF
//write FFFFFFFFFFFFFFFFFFFFFFFFFFFF07919761206687F8FFFFFFFFFFFF


//8004100418041CFFFFFFFFFFFFFF
//07819861206687F8FFFFFFFFFFFF

int gmodem_crsm(gmodem *g,int oper, int file, int p1, int p2, int p3,char *r) {
char buf[256];
switch(oper) {
  case UPDATE_RECORD:
    sprintf(buf,"+CRSM=%d,%d,%d,%d,%d,\"%s\"",oper,file,p1,p2,p3,r);
    //printf("AT%s\n",buf);
    return gmodem_At(g,buf);
    break;
  case STATUS:
    sprintf(buf,"+CRSM=%d,%d",oper,file); // OR 30?
    break;
  case GET_RESPONSE:
    //
    //sprintf(buf,"+CRSM=%d,%d,0,0,30",oper,file); // OR 30?
    sprintf(buf,"+CRSM=%d,%d,0,0",oper,file); // OR 30?
    break;
  default:
    sprintf(buf,"+CRSM=%d,%d,%d,%d,%d",oper,file,p1,p2,p3);
    break;
}
//printf("BEGIN:%s\n",buf);
return  gmodem_At2bufFilter(g,buf,"+CRSM:",g->out,sizeof(g->out));
}

int gmodem_crsm_length(gmodem *g,int file) {
gmodem_crsm(g,STATUS,file,0,0,0,0);
//if (gmodem_crsm(g,GET_RESPONSE,file,0,0,0,0)<=0) return -1; // some error
char *p=g->out;
//printf("OUT=%s\n",p);
p = gmodem_par(&p,2);  int sl=strlen(p);
//printf("CRSM_LENGTH :<%s>, length:%d\n",p,strlen(p));
if (sl<30) return -8; // ??
int l=0;
//printf("p+28=%s\n",p+28);
if (sscanf(p+28,"%02x",&l)!=1) return -3; // last byte is length...
return l;
}

int num_scan(char *num,char *sznum,int *ton_npi) {
 int r=0; int l;
if (*sznum=='+') { sznum++; *ton_npi=0x91;} // international
            else { *ton_npi  = 0x81 ; } // national
for(l=0;  sznum[l] && l<20 ;l++) { // scan it 2 digits at once
  char d = sznum[l];
  if (d>='0' && d<='9') { *num=d; num++; r++;}
  }
if (r%2) { *num='F'; num++; r++;}
return r;
}

int gmodem_cnum_set_on(gmodem *g,char *num) {
gmodem_At(g,"+CPBS=\"ON\""); // set proper sim-book
return gmodem_Atf(g,"+CPBW=1,\"%s\",%d",num,(num[0]=='+')?145:129);

}

int gmodem_cnum_set(gmodem *g, char *num) {
                  //return gmodem_crsm_cnum_set(g,num);
                  return gmodem_cnum_set_on(g,num);
int ton_npi;
char n[20]; // max 20 digits
int nl = num_scan(n,num,&ton_npi); //+CRSM=192,28480,0,0,30
int alpha = gmodem_crsm_length(g, EF_MSISDN ); //
printf("Alpha=%d\n",alpha);
if (alpha<14 || alpha>60) {
    printf("cnum alpha invalid\n");
    alpha=28;
  //return -12;
  }
printf("scanned: t=%x num:'%s'\n",ton_npi,num);
//AT+CRSM=220,28480,1,4,28,
char buf2[256];
memset(buf2,0,sizeof(buf2));
memset(buf2,'F',alpha*2);
//FFFFFFFFFFFFFFFFFFFFFFFFFF (26letter,13byte)
//07919343193254F6FFFFFFFFFFFFFF (30letter,15byte)
//printf("set!\n");
//char *b =  buf2+26; // begins here ...  (alpha-12)*2  28-12=16*2
char *b =  buf2+(alpha-14)*2; // begins here ...  (alpha-12)*2  28-12=16*2
if (ton_npi == 0x91 ) { // print international
   //sprintf(b,"%02x9193",2+nl/2);
   sprintf(b,"%02x91",1+nl/2);
   } else {
   sprintf(b,"%02x81",1+nl/2);
   }
//printf("printed b=%s!\n",b);
b+=strlen(b);
int i; for(i=0;i<nl/2;i++) { // swap bytes
  b[0+2*i]=n[2*i+1];
  b[1+2*i]=n[2*i];
  //printf("ii=%d\n",i);
  }
//printf("i=%d b:%s\n",i,b);
//printf("buf2=%s\n",buf2);
//printf("buf2=%s,len=%d",buf2,strlen(buf2));
//sprintf(buf,"+CRSM=220,28480,1,4,28,FFFFFFFFFFFFFFFFFFFFFFFFFF%s");
return gmodem_crsm(g,UPDATE_RECORD,EF_MSISDN,1,4,alpha,buf2);
}

void swap_bytes(char *d,char *s,int l) {
while(l>1) {
  char ch0,ch1;
  ch0=s[0]; ch1=s[1];
  d[0]=ch1; d[1]=ch0;
  d+=2,s+=2,l-=2;
  }
}

int gmodem_cnum_get(gmodem *g) {
return gmodem_crsm_cnum_get(g);
}

char *str_unquote(char *buf) ;

uchar *get_till_quoted(uchar **data,uchar *del,int dl) { // get_till but allows delimiters inside quotes '"'
uchar *str = *data; int sl,ipos;
if (dl<0) dl = strlen(del); sl = strlen(str);
int q=0; // quote
int i;
for(i=0,ipos=-1;i<=sl-dl;i++) {
     if ( (q==0) && (memcmp(str+i,del,dl)==0)) { ipos=i; break;}
     if (str[i]=='"') q=!q;
   }
//ipos = strnstr(str,sl,del,dl);
//printf("get_till ipos=%d in %s the %s\n",ipos,str,del);
if (ipos<0) {  *data=str+sl; return str; } // return all
str[ipos]=0; *data=str+ipos+dl; // remove delimiter
return str;
}

char *str_unquote(char *buf);

char *gmodem_par(uchar **cmd,int skip) {
char *p=0; char *del=",";
 //printf("PAR1_IN:%x VAL:%x\n",cmd,*cmd);
while(skip>=0) {
  //printf(">>>cmd:%s==SKIP:%d,ptr=%x\n",*cmd,skip,*cmd);
  p  = get_till_quoted((void*)cmd,del,-1);
  //printf("==p:%s,cmd:%s\n",p,*cmd);
  skip--;
  *cmd=trim(*cmd);
  }
//printf(">>>done\n");
p=trim(p);
//printf(">>>done2  ->%s\n",p);
p=str_unquote(p);
//printf(">>>done3   ->%s\n",p);

return p;
}

int gmodem_parInt(uchar **cmd,int skip,int def) {
char *p = gmodem_par(cmd,skip);
if (!p) return def;
sscanf(p,"%d",&def);
return def;
}


int gmodem_cnum(gmodem *g) { // try read cnum from a rsim?
 int ok;
char cnum[180];//,*p0=cnum;
ok = gmodem_At2bufFilter(g,"+CNUM","+CNUM:",cnum,sizeof(cnum))>0;
//printf("CNUM:<%s>\n",cnum);
if (ok<=0) return -1; // failed
 /// need to get 1 param...
 //p0=cnum;
//printf("res:%s, ptr=%x PAR1=%x ptrVAL=%x\n",cnum,cnum,&p0,p0);
char *p = cnum;
p = gmodem_par(&p,1);
//printf("P=%p\n",p);
if (strlen(p)>0) {
   strNcpy(g->cnum,p);
   strNcpy(g->out,p);
   //printf("CNUM1:<%s>\n",cnum);
   return 1;
   }
//printf("CNUM2:<%s> p:%s\n",cnum,p);
return -2;
}

int gmodem_iccid(gmodem *g) {
char iccid[24]; iccid[0]=0;
int ok = gmodem_crsm_iccid(g); if (ok>0) {

    strNcpy(g->iccid,g->out);
    return ok;
    }
ok = (gmodem_At2bufFilter(g,"+ICCID","ICCID",iccid,23)>0)
    || (gmodem_At2bufFilter(g,"^ICCID?","^ICCID",iccid,23)>0);
if (ok>0) { // remove '"' if any
         strcpy(iccid,(void*)str_unquote(iccid));
         } else {
        sprintf(g->out,"ICCID - fail all: CRCM, +ICCIC, ^ICCID");
        return -1;
        }
if (iccid[0]=='9' && iccid[1]=='8') { // some modems do not swap bytes
    swap_bytes(iccid,iccid,strlen(iccid));
   }
int l = strlen(iccid); if (l>0) iccid[l-1]=0; // remove lun number
strcpy(g->iccid,iccid);
strcpy(g->out,iccid);
return ok;
}


// CNUM???
int gmodem_info(gmodem *g) { // collects and prointf info on a screen
//g->logLevel=10;
gmodem_imei(g);
int pin = gmodem_pin(g,0); // just get Ready-state
gmodem_imsi(g);
gmodem_iccid(g);
gsm_device *d = g->dev;
printf("Modem:{name:'%s',imei:'%s',file:'%s'},\n",d->name,g->imei,g->name);
gsm_operator *o = gsm_operators;
if (g->oper) o=g->oper;
//printf("DONE0\n");
if (o) { // known network
 printf("HPLMN:{name:'%s',imsi:'%s',gprs:'%s',apn:'%s'}\n",o->name,o->imsi,o->gprs_num,o->apn);
 }
//printf("DONE-0\n");
(gmodem_cnum(g)>0) ;
  //  || (gmodem_cnum_get(g)>0); // try any of AT+CNUM or AT+CRSM -- some modems NOT safe
  //printf("DONE1\n");
printf("SIM:{pin_ready:%d,iccid:'%s',imsi:'%s',cnum:'%s'}\n",pin==1?1:0,g->iccid,g->imsi,g->cnum);
  //printf("DONE2\n");
return 1; //ok
}

int gmodem_smsc(gmodem *g) {
char smsc[100],*p=smsc;
gmodem_At2bufFilter(g,"+CSCA?","+CSCA",smsc,23);
p = gmodem_par(&p,0);
if (!p[0]) { sprintf(g->out,"error in resp CSCA"); return -1; }
strNcpy(g->out,p);
return strlen(g->out);
}

int gmodem_smsc_set(gmodem *g,char *num) {
int  code=num[0]=='+'?145:129;
return gmodem_Atf(g,"+CSCA=\"%s\",%d",num,code);
}

//AT+CRSM=220,28480,1,4,26,FFFFFFFFFFFFFFFFFFFFFF07819861206687F8FFFFFFFFFFFFFF

int gmodem_cmd_file(gmodem *m,char *file) {
/*
char *txt = strLoad(file);
gmodem_logf(m,2,"process file: %s",file);
if (!txt) return gmodem_errorf(m,-2,"file %s not found",file);
char *b = txt, *e;
while( ! m->f.eof ) { // while we can ...
  e  = strchr(b,'\n');
  if (e) { *e=0; e++; }
    gmodem_cmd(m, b); // process this line
  if (!e) break;
  b=e; // next line
  }
strClear(&txt); // free text
return 1; // OK
*/
}


int gmodem_neoway(gmodem *g,char *cmd); // spec neoway commands

int gmodem_cmd(gmodem *m,char *c0) {
uchar *c=c0;
if (lcmp(&c,"@")) { // run file
   return gmodem_cmd_file(m,c);
   }
if (lcmp(&c,"try"))   {  int code = gmodem_cmd(m,c); if (code>0) return code; // OK
                         char buf[180];
                         strNcpy(buf,m->out);
                         sprintf(m->out," ignore: %s",buf);
                         return 1;
                       } // always OK

if (lcmp(&c,"pin")) return gmodem_pin(m,c);

if (lcmp(&c,"neoway")) return gmodem_neoway(m,c);
if (lcmp(&c,"sim800")) {
   if (c[0]=='.') c++;
   return gmodem_sim800(m,c);
   }

if (lcmp(&c,"http"))    return gmodem_http(m,c);
if (lcmp(&c,"balance")) return gmodem_balance(m);
if (lcmp(&c,"pppd"))    return gmodem_pppd(m,c);
if (lcmp(&c,"ussd"))    return gmodem_ussd(m,c);
if (lcmp(&c,"info"))    return  gmodem_info(m);
if (lcmp(&c,"imei.set")) return gmodem_imei_set(m,c);
if (lcmp(&c,"imei"))    return gmodem_imei(m);
if (lcmp(&c,"iccid"))   return gmodem_iccid(m);
if (lcmp(&c,"imsi"))   return gmodem_imsi(m);
if (lcmp(&c,"cnum_set"))   return gmodem_cnum_set(m,c);
if (lcmp(&c,"cnum.set"))   return gmodem_cnum_set(m,c);
if (lcmp(&c,"cnum_get"))   return gmodem_cnum_get(m); // or cnum_get for CRSM
if (lcmp(&c,"cnum"))       return gmodem_cnum(m); // or cnum_get for CRSM
if (lcmp(&c,"cb")) return gmodem_cb(m,c);
if (lcmp(&c,"smsc_set")) return gmodem_smsc_set(m,c);
if (lcmp(&c,"smsc")) return gmodem_smsc(m);

if (lcmp(&c,"sms")) return gmodem_sms(m,c);
if (lcmp(&c,"ota")) return gmodem_ota(m,c);
if (lcmp(&c,"im")) return gmodem_im(m,c); // instant manager

if (lcmp(&c,"dial")) return gmodem_dial(m,c); // start dial???
if ( lcmp(&c,"answer")|| lcmp(&c,"hold")) return gmodem_answer(m); //
if (lcmp(&c,"kill")) return gmodem_kill(m); // start dial???
if (lcmp(&c,"dtmf")) return gmodem_dtmf(m,c);

if (lcmp(&c,"creg")) return gmodem_creg(m);
if (lcmp(&c,"reg")) return gmodem_creg(m);

//if (lcmp(&c,"gprs")) return gmodem_gprs(m,c);

// APDU
if (lcmp(&c,"apdu")) return gmodem_apdu_cmd(m,c);

// phonebook

if (lcmp(&c,"book")) return gmodem_book_cmd(m,(uchar*)c);

if (lcmp(&c,"crlf")) {
    if (*c=='=') c++; int val=0;
    sscanf(c,"%d",&val);
    m->dev->crlf=val;
    printf("crlf now:%d\n",val);
    return 1;
    //return gmodem_setLogLevel(m,logLevel);
    }

if (lcmp(&c,"3g")) return gmodem_At(m,"^SYSCFG=14,2,3fffffff,0,1"); // huawei 3g only
if (lcmp(&c,"2g")) return gmodem_At(m,"^SYSCFG=13,1,3fffffff,0,0"); // huawei 2g only
if (lcmp(&c,"3g2g")) return gmodem_At(m,"^SYSCFG=2,2,3FFFFFFF,2,4"); // huawei 3g and after 2g

if (lcmp(&c,"no-cd")) return gmodem_At(m,"^SETPORT=\"A1,A2;1,2,3\""); // e171 mode

if (lcmp(&c,"hi")) return gmodem_hi2115_cmd(m,c); // hi_silicon



//if (lcmp(&c,"0"")
if (lcmp(&c,"logLevel")) {
    if (*c=='=') c++; int logLevel;
    sscanf(c,"%d",&logLevel);
    return gmodem_setLogLevel(m,logLevel);
    }
   //return gmodem_cnum(m);
// other
if (lcmp(&c,"at") || lcmp(&c,"AT") || strchr("+$#^",c[0])) return gmodem_At2buf(m,c,m->out,sizeof(m->out));
return 0;
}

