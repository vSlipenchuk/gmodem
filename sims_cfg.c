#include "gmodem.h"
#include "gsm_sim.h"
#include <stdio.h>
#include "coders.h"
/*

  Загружаем персональные настройки для указанной СИМ
  в конфиг-файле должны быть строчки ...

  ICCID SPI KiC KiD TAR KIC KID Counter

*/

struct _cardJob cardJob = { .num={"556677"}, .tar={0,0,0} }; // some global var

uchar *maxl(uchar *s,int ml) {
int l = strlen(s); if (l>ml) l=ml; s[ml]=0;
return s;
}

void cardJobSetup(struct _cardJob *j,
                     int spi, int kic, int kid,  // profile data
                     uchar *ICCID,
                     uchar *KIC, uchar *KID, // personal data
                     int counter) {
if (j==0) j = &cardJob; /// default global
//printf("j=%x\n",j);
j->spi = spi; j->kic = kic; j->kid = kid;
strNcpy(j->ICCID,ICCID);
hexstr2bin(j->KID,KID,-1); // copy pair?
hexstr2bin(j->KIC,KIC,-1);
j->counter = counter;
}


uchar *get_wordl(uchar **cmd, int ml) { return maxl( get_word(cmd), ml); }

int cardJobSaveCounter(struct _cardJob *j) {
char buf[512];
sprintf(buf,"/root/.telco/counter/%s",j->ICCID);
FILE *f = fopen(buf,"w");
if (!f) return 0;
fprintf(f,"%x",j->counter);
fclose(f);
return 1;
}
#include <sys/stat.h>

int cardJobLoadCfg(struct _cardJob *j,char *_iccid,char *filename) { // Loads Info from a file
if (!j) j = &cardJob;
if (!filename) filename="/root/.telco/sims.cfg";
FILE *f = fopen(filename,"r");
if (!f) {  printf("Load File %s failed\n",filename); return 1;   } // ZUZUK!
char buf[1024];
while( fgets(buf,sizeof(buf),f) ) {
  uchar *c=buf;
  uchar *iccid = get_word(&c);
  if (strcmp(iccid,_iccid)==0) { // found !
    //printf("FOUND! <%s>\n",c);
   strNcpy( j->ICCID, iccid );
   sscanf( get_wordl(&c,4) ,"%04X", &j->spi); sscanf( get_wordl(&c,2) ,"%02X", &j->kic); sscanf( get_wordl(&c,2) ,"%04X", &j->kid);
   //printf("Line2\n");
   hexstr2bin( j->tar, get_wordl(&c,3),-1);  hexstr2bin( j->KIC, get_wordl(&c,24*2),-1); hexstr2bin( j->KID, get_wordl(&c,24*2),-1);
   //printf("Line3\n");
   uchar *s = get_wordl(&c,24); strNcpy(j->MSISDN,s);
          s = get_wordl(&c,24); strNcpy(j->IMSI,s);
   sscanf( get_wordl(&c,4) ,"%04X", &j->counter);
   fclose(f);
   //printf("NOW - update counter\n");
   mkdir("/root/.telco",S_IRWXU); mkdir("/root/.telco/counter",S_IRWXU); // ensure dir exists
  // Now - update counter - if any
   sprintf(buf,"/root/.telco/counter/%s",j->ICCID);
   f = fopen(buf,"r");
   if (f) {
      if (fgets(buf,sizeof(buf),f)) if (sscanf(buf,"%x",&j->counter)) {
           //printf("CounterUpdated: %d, hex:%x\n",j->counter,j->counter);
           }
      fclose(f);
       }
    j->saveCounter = cardJobSaveCounter;
    //exit(1222);
    return 1; // OK - config loaded
    }
  }
fclose(f);
printf("DataFor ICCID %s not found\n",_iccid);
return 1;
}
