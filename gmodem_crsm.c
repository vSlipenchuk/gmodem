#include "gmodem.h"
#include "gsm_sim.h"

int _gmodem_crsm(gmodem *g,int oper, int file, int p1, int p2, int p3,char *r) {
char buf[256],*fmt;
switch(oper) {
  case UPDATE_RECORD: fmt="+CRSM=%d,%d,%d,%d,%d,\"%s\""; break;
  case STATUS:        fmt="+CRSM=%d,%d"; break;
  case GET_RESPONSE:  fmt="+CRSM=%d,%d,0,0"; break;
  default:            fmt="+CRSM=%d,%d,%d,%d,%d"; break;
}
g->out[0]=0; sprintf(buf,fmt,oper,file,p1,p2,p3,r);
int ok = gmodem_At2bufFilter(g,buf,"+CRSM:",g->out,sizeof(g->out)); if (ok<=0) return ok; // fail
if (g->out[0]==0) { sprintf(g->out,"empty CRSM result"); return -1; }
char *p = g->out;
 p1 = atoi((void*)gmodem_par(&p,0));
 p2 = atoi((void*)gmodem_par(&p,0));
 strcpy(g->out, (void*)gmodem_par(&p,0)); // move result here
return ((p1<<8) | p2); // code
}

int gmodem_crsm_cnum_get(gmodem *g) {
 // FIRST - we need define actuall size of CNUM record. Try read as more as we can.
int sz = 128; // normally 24,26,28 bytes ?
int ok = _gmodem_crsm(g,READ_RECORD,EF_MSISDN,1,4,128,0); // try read more
if (ok==0x9000) {
   if (g->logLevel>2) printf("CRSM: %04x %s LEN:%d OUT='%s'\n",ok,g->out,strlen(g->out),g->out);
   sz = strlen(g->out)/2;
   } else {
   sprintf(g->out,"invalid GSRM RESP %04x, EXPECT 9000 READ_RECORD EF_MSISDN",ok);
   return -1;
   }
if (sz<14 || sz>40) { // normally - 28 bytes, but can vary
  sprintf(g->out,"unexpected SIZE %d in CNUM EF_MSISDN, expext 14..40",sz);
  return -1;
  }
char *p = g->out+2*(sz-14); // rest 14 bytes or 28 hex
//printf("PNUM_BEGINS: %s shift=%d sz=%d\n",p,(sz-14)*2,sz);
int nl = 0,tn=0;
sscanf(p,"%02x%02x",&nl,&tn); p+=4;
nl--; if (nl>10) nl=10; //???
nl=2*nl;// printf("nl=%d\n",nl);
swap_bytes(p,p,nl); p[nl]=0;
if (nl>0 && p[nl-1]=='F') nl--; p[nl]=0;
char buf[20];
sprintf(buf,"%s%s",(tn==0x91?"+":""),p);
strcpy(g->out,buf);
strNcpy(g->cnum,buf);
return sz; // returns size of alpha...
}

int gmodem_crsm_cnum_set(gmodem *g,char *num) {
int sz = gmodem_crsm_cnum_get(g); // first - read and define a size
if (sz<=0) return sz; // some errors already reported
int ton_npi;  char n[20]; int nl = num_scan(n,num,&ton_npi); // sscanf a number
char buf2[256]; memset(buf2,'F',sizeof(buf2)); buf2[sz*2]=0;// prepare coded buffer
char *b =  buf2+(sz-14)*2; // begins here ...  (alpha-12)*2  28-12=16*2
if (ton_npi == 0x91 ) { // print international
   sprintf(b,"%02x91",1+nl/2);
   } else {
   sprintf(b,"%02x81",1+nl/2);
   }
b+=strlen(b);
int i; for(i=0;i<nl/2;i++) { // swap bytes
  b[0+2*i]=n[2*i+1];
  b[1+2*i]=n[2*i];
  }
int ok = _gmodem_crsm(g,UPDATE_RECORD,EF_MSISDN,1,4,sz,buf2);
//printf("PNUM_STARTS: %s, sz=%d, shift=%d\n",buf2+(sz-14)*2,sz,(sz-14)*2);
//printf("OK=%x\n",ok);
if (ok==0x9000) { // expect it
   if (g->logLevel>2) printf("CRSM CNUM UPDATED OK\n");
    } else {
    sprintf(g->out,"invalid UPDATE_RECORD RESP %04x, EXPECT 9000 READ_RECORD EF_MSISDN",ok);
    return -2;
    }
return sz; //
}


int gmodem_crsm_iccid(gmodem *g) {
if (_gmodem_crsm(g,READ_BINARY,EF_ICCID, 0,0,10,0)!=0x9000) {
    sprintf(g->out,"error READ EF_ICCID");
    return -1;
    }
int len = strlen(g->out);
//printf("<%s> len=%d before swap\n",g->out,strlen(g->out));
swap_bytes(g->out,g->out,len);
if (len>0) g->out[len-1]=0; // remove LUN
return len; // OK
}
