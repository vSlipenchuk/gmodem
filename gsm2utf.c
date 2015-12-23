//gsm2utf(sms->text,sms->ud,sms->udl);

int gsm2utf(char *out,char *ucs2,int len) { // converts UCS2 -> utf8, len in bytes
//hexdump("gsm2utf:",ucs2,len);
int tlen=0;
    while(len>0) {
      int wch  = ( ucs2[0]<<8 | ucs2[1] ); // first go page
      int rlen = utf8_poke(out,wch,4);
      //printf("rlen=%d wch=%d\n",rlen,wch);
      //hexdump("U",out,rlen); hexdump("rest",ucs2,len);
      len-=2; ucs2+=2;  tlen+=rlen;
    if (out) { out+=rlen;   }
    }
if (out) *out=0;
return tlen;

}
