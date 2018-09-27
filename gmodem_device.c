#include "gmodem.h"

 //case 1: return "\r"; // E1550 by default!!!
 //case 2: return "\n";

 /*
   dmode=1 - no command can be send after ATD till OK accepted, which means OK,Connected or any negative answer
 */

gsm_device gsm_devices[] = {
    {
        .name="default", .crlf=3 ,/*\r\n*/ // GSM-0707
    },
    {
        .name="SIMCOM", .imei="0132260", .crlf=1, .ussd=8,
    },
    {
        .name="m590", .imei="8681060", .crlf=1, .ussd=-1, // neoway m 590
    },
    //355735003810571
    {
        .name="SIM600", .imei="3557350", .crlf=1, .ussd=8, // Novacom
    },


{
        .name="HOJY", .imei="3530360", .crlf=1, .ussd=8, .dmode=1,
    },
    {   /*
          Qualcomm based - can support SD Card
          AT^U2DIAG=0 (девайс в режиме "только модем")
          AT^U2DIAG=256 // Serial + SDCard
          AT^U2DIAG=255 // Serial + SD + CDROM

        */
        .name="E1550", .imei="3534430",.crlf=1, .ussd=7   /*\n*/
        //  bug:    +CUSD: <mode>,<text>,<dcs> always report dcs=15 (even in CSCS=UCS2 mode)
        //  example:+CREG: 1, "0EE3", "2CE3"
    },
    {   .name="E3533",  .imei="8671990", .crlf=1, .ussd=7 }, // ipad revision
    {   .name="E3533",  .imei="8671990", .crlf=1, .ussd=7 }, // ipad revision
    {   .name="E171",   .imei="356356", .crlf=1, .ussd=7}, // r1
    {   .name="E171",   .imei="3548070",.crlf=1, .ussd=7 }, // r2
    {   .name="HUA",    .imei="867157",  .crlf=1, .ussd=7}, // all other hua


  {
        /*
         Mediatek based phone,
            CRSM for CNUM broked (in responce while get length)
        */
        .name="OT-808",.imei="3563890",.crlf=1   /*\n*/   // !!! ussd_coder !!! and other space!!!
    },




    {
        .name="Telit", .imei="3550940",.crlf=3
    },


{
 .name="EM770W", // Huawei
 .imei="3570300", //"25840295
 .ussd=7
},


    {
        .name=0
    }

    };


/*

Read:
 AT+CRSM=178,28480,1,4,28

Telit:
BEGIN:+CRSM=192,28480,0,0,30
RESP:<0000001C6F4004001100440102011C>, length:30

E171:
BEGIN:+CRSM=192,28480,0,0,30
RESP:<621E82054221001C0183026F40A5038001718A01058B036F06048002001C>, length:60



*/
#include <string.h>
#include <stdlib.h>
#include <openssl/md5.h>

#include "huawei_calc.c"

/*
void encrypt_v1(char* imei, char* resbuf, char* hstr) {
    unsigned char xbytes[17];
    char ybytes[100];
    char hash[100];
    unsigned int rh[30];
    unsigned char res[4];
    int i;

    memset(xbytes,0,17);
    MD5((unsigned char*)hstr, strlen(hstr), xbytes);

    for(i = 0; i < 16; i++)
        sprintf(ybytes + (i * 2), "%02x", xbytes[i] & 0xff);

    strcpy(hash, imei);
    strncat(hash, ybytes + 8, 16);
    hash[31] = 0;
    MD5((unsigned char*)hash, 31, xbytes);

    for (i = 0; i < 16; i++)
        rh[i] = xbytes[i] & 0xff;

    for(i = 0; i < 4; i++)
        res[3-i] = rh[i] ^ rh[i+4] ^ rh[i+8] ^ rh[i+12];

    i=*((unsigned int*)&res);
    i |= 0x2000000;
    i &= 0x3FFFFFF;

    sprintf(resbuf, "%i", i);
}
*/

int gmodem_imei_set312s(gmodem *g,char *newimei) { // huawei 312s can support imei change
char codebuf[100];
  calc2(g->imei,codebuf);  //printf("  Calc2        = %s\n", codebuf);
int ok = gmodem_Atf(g,"^DATALOCK=\"%s\"",codebuf) >0 &&
         gmodem_Atf(g,"^cimei=\"%s\"",newimei) > 0;
if (ok) {
   gmodem_logf(g,0,"imei changed to %s, reboot modem",newimei);
   gmodem_Atf(g,"^RESET");
   return ok;
   }
return gmodem_errorf(g,-2,"imei.set 312s failed");
}

int gmodem_imei_set3533(gmodem *g,char *newimei) { // huawei 312s can support imei change
char codebuf[100];
  calc201(g->imei,codebuf);  //printf("  Calc2        = %s\n", codebuf);
int ok = gmodem_Atf(g,"^DATALOCK=\"%s\"",codebuf) >0 &&
         gmodem_Atf(g,"^cimei=\"%s\"",newimei) > 0;
if (ok) {
   gmodem_logf(g,0,"imei changed to %s, reboot modem",newimei);
   gmodem_Atf(g,"^RESET");
   return ok;
   }
return gmodem_errorf(g,-2,"imei.set 312s failed");
}

int gmodem_imei_set(gmodem *g,char *str) { // set new imei on based one
char codebuf[100],ati[500],*imei;
if (g->imei[0]==0) gmodem_imei(g); // try it
gmodem_At2buf(g,"i",ati,sizeof(ati));
gmodem_logf(g,2,"ATI responce: %s\n",ati);

if (strstr(ati,"Model: E3121") && strstr(ati,"Revision: 21.158")) return gmodem_imei_set312s(g,str);
if (strstr(ati,"Model: E3533") && strstr(ati,"Revision: 21.318")) return gmodem_imei_set3533(g,str);

// known to be failed
if (strstr(ati,"Model: E3121") && strstr(ati,"Revision: 11.102")) return gmodem_errorf(g,-1,"huawei 320s cant support imei.set");

printf("Unknown algo, will try...\n");
//return -1; // fail
imei = g->imei;
     printf("  IMEI              = %s change to new=%s\n", imei,str);

    encrypt_v1(imei, codebuf, "hwe620datacard");  printf("  Unlock code v1code     = %s\n", codebuf);

  //  encrypt_v1(imei, codebuf, "e630upgrade");   printf("  Flash code        = %s\n", codebuf);

  //  calc2(imei,codebuf);  printf("  Calc2     v2code   = %s\n", codebuf);
  //  calc201(imei,codebuf);  printf("  Calc201        = %s\n", codebuf);
   //encrypt_5_v2(imei,codebuf); printf(" v2=%s\n",codebuf);

g->logLevel=10;

//nvrex - not support onn 312s, support on 320s
//gmodem_Atf(g,
//gmodem_put(g,"AT^NVWREX=50502,0,128,8F 29 FF 8E A8 CA 34 89 78 73 18 BA 9E F5 9C 64 0B A4 DB 81 DC 03 45 6E 72 DA EC 6A 0C 7C 90 65 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 8B 8C F4 B5 AF 0C F2 2C FE E0 F4 46 9C CF 47 95 36 71 1F 1C BF 05 7F 84 AB A9 F2 92 89 33 3C 12 01 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00\r",-1);
//gmodem_run2(g);

//char *newimei = "867157011212248";
//gmodem_Atf(g,"^CARDLOCK=\"%s\"",codebuf); //v2
//gmodem_Atf(g,"^DATALOCK=\"%s\"","15364022"); //v2
gmodem_Atf(g,"^DATALOCK=\"%s\"",codebuf);
//gmodem_Atf(g,"^DATALOCK=\"%s\"",codebuf);
gmodem_Atf(g,"^cimei=\"%s\"",imei); // let it be the same
//gmodem_Atf(g,"^RESET");

return 1; // OK
}

