#include "gmodem.h"
#include "vstrutil.h"
#include <stdio.h>

#include "gsm_sim.h"

char *gmodem_par(char **cmd,int skip);


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
    sprintf(buf,"+CRSM=%d,%d,%d,%d,%d,%s",oper,file,p1,p2,p3,r);
    printf("AT%s\n",buf);
    return gmodem_At(g,buf);
    break;
  case GET_RESPONSE:
    sprintf(buf,"+CRSM=%d,%d,0,0,30 ",oper,file);
    break;
  default:
    sprintf(buf,"+CRSM=%d,%d,%d,%d,%d",oper,file,p1,p2,p3);
    break;
}
printf("BEGIN:%s\n",buf);
return  gmodem_At2bufFilter(g,buf,"+CRSM:",g->out,sizeof(g->out));
}

int gmodem_crsm_length(gmodem *g,int file) {
if (gmodem_crsm(g,GET_RESPONSE,file,0,0,0,0)<=0) return -1; // some error
char *p=g->out;
p = gmodem_par(&p,2);  int sl=strlen(p);
printf("RESP:<%s>, length:%d\n",p,strlen(p));
if (sl<30) return -8; // ??
int l=0;
printf("p+28=%s\n",p+28);
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

int gmodem_cnum_set(gmodem *g, char *num) {
int ton_npi;
char n[20]; // max 20 digits
int nl = num_scan(n,num,&ton_npi);
int alpha = gmodem_crsm_length(g, EF_MSISDN );
printf("Alpha=%d\n",alpha);
if (alpha<14 || alpha>60) {
    printf("cnum alpha invalid\n");
  return -12;
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
while(l>0) {
  char ch0,ch1;
  ch0=s[0]; ch1=s[1];
  d[0]=ch1; d[1]=ch0;
  d+=2,s+=2,l-=2;
  }
}

int gmodem_cnum_get(gmodem *g) {
//int ton_npi;
//char n[20]; // max 20 digits
//int nl = num_scan(n,num,&ton_npi);
int alpha = gmodem_crsm_length(g, EF_MSISDN );

//printf("Reading EF %d and %d\n",28480,EF_MSISDN);

//printf("Alpha=%d\n",alpha);
if (alpha<14 || alpha>60) {
  printf("cnum alpha %d invalid\n",alpha);
  return -12;
  }
//printf("scanned: t=%x num:'%s'\n",ton_npi,num);
// ok - now - request all data?
int ok;
ok = gmodem_crsm(g,READ_RECORD  ,EF_MSISDN,1,4,alpha, 0);
if (ok<=0) {
  printf("read record failed\n");
  return -1;
  }
char *p = g->out;
printf("OUT:<%s>\n",p);
p = gmodem_par(&p,2);
if (strlen(p)!=alpha*2) {
  printf("Wrong resp\n");
  return 0;
  }
p+=2*(alpha-14); // bytecode begins
int nl = 0,tn=0;
sscanf(p,"%02x%02x",&nl,&tn); p+=4;
nl--; if (nl>10) nl=10; //???
nl=2*nl; printf("nl=%d\n",nl);

 swap_bytes(p,p,nl); p[nl]=0;
printf("NUM:%s\n",p);
if (nl>0 && p[nl-1]=='F') nl--; p[nl]=0;
sprintf(g->out,"%s%s",(tn==0x91?"+":""),p);

return 1;//+79101779668

}


int gmodem_iccid_by_crsm(gmodem *g,char *iccid) {
char *p,*cmd;
if (gmodem_crsm(g,READ_BINARY,EF_ICCID, 0,0,10,0)<=0) return -1;  // no ? ^)
cmd = g->out; p = gmodem_par(&cmd,2); p=str_unquote(p);
swap_bytes(iccid,p,22); iccid[22]=0;
return 1;
}

#include "vstrutil.h"

char *gmodem_par(char **cmd,int skip) {
char *p=0;
while(skip>=0) {
  p  = get_till(cmd,",");
  skip--;
  }
p=str_unquote(p);
return p;
}

int gmodem_cnum(gmodem *g) { // try read cnum from a rsim?
char cnum[200]; int ok;
ok = gmodem_At2bufFilter(g,"+CNUM","+CNUM:",cnum,sizeof(cnum))>0;
if (ok<=0) return -1; // failed
 /// need to get 1 param...
char *p=cnum;
//printf("res:%s\n",cnum);
 p = gmodem_par(&p,1);
if (strlen(p)>0) {
   strNcpy(g->cnum,p);
   strNcpy(g->out,p);
   return 1;
   }
return -2;
}

int gmodem_iccid(gmodem *g) {
char iccid[24];
iccid[0]=0;
int ok = gmodem_At2bufFilter(g,"+ICCID","ICCID",iccid,23)>0
    || gmodem_At2bufFilter(g,"^ICCID?","^ICCID",iccid,23)>0
    || gmodem_iccid_by_crsm(g,iccid);
if (ok) { // remove '"' if any
        //printf("HERE<%s>\n",iccid);
         strcpy(iccid,str_unquote(iccid));
        //printf("HERE2<%s> %d\n",iccid,strlen(iccid));
        }
if (!ok) return -1;
//printf("iccid=?%s\n",iccid);
strcpy(g->iccid,iccid);
strcpy(g->out,iccid);
return ok;
}

// CNUM???
int gmodem_info(gmodem *g) { // collects and prointf info on a screen
gmodem_imei(g);
int pin = gmodem_pin(g,0); // just get Ready-state
gmodem_imsi(g);
gmodem_iccid(g);
gsm_device *d = g->dev;
printf("Modem:{name:'%s',imei:'%s',file:'%s'},\n",d->name,g->imei,g->name);
gsm_operator *o = gsm_operators;
if (g->oper) o=g->oper;
printf("HPLMN:{name:'%s',imsi:'%s',gprs:'%s',apn:'%s'}\n",o->name,o->imsi,o->gprs_num,o->apn);
gmodem_cnum(g);
printf("SimCard:{pin_ready:%d,iccid:'%s',imsi:'%s',cnum:'%s'}\n",pin==1?1:0,g->iccid,g->imsi,g->cnum);
return 1; //ok
}

//AT+CRSM=220,28480,1,4,26,FFFFFFFFFFFFFFFFFFFFFF07819861206687F8FFFFFFFFFFFFFF

int gmodem_cmd(gmodem *m,char *c) {
if (lcmp(&c,"balance")) return gmodem_balance(m);
if (lcmp(&c,"pppd"))    return gmodem_pppd(m,c);
if (lcmp(&c,"ussd"))    return gmodem_ussd(m,c);
if (lcmp(&c,"info"))    return  gmodem_info(m);
if (lcmp(&c,"imei"))    return gmodem_imei(m);
if (lcmp(&c,"iccid"))   return gmodem_iccid(m);
if (lcmp(&c,"imsi"))   return gmodem_imsi(m);
if (lcmp(&c,"cnum_set"))   return gmodem_cnum_set(m,c);
if (lcmp(&c,"cnum"))   return gmodem_cnum_get(m);
   //return gmodem_cnum(m);
// other
if (lcmp(&c,"at") || lcmp(&c,"AT") || strchr("+$#^",c[0])) return gmodem_At2buf(m,c,m->out,sizeof(m->out));
return 0;
}

