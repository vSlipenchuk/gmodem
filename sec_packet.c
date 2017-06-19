/*

  Secure packet 03.48 helpers

*/

#include "gsm_sim.h"
// apt-get install libssl-dev
#include <openssl/des.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>




//KiC: 30 42 30 42 30 44 30 44 30 45 30 45 30 46 30 46 // 16 or 32 bytes
//KiD: 01 23 45 67 89 AB CD EF 10 02 76 FE DC BA 01 23

// TP-UD                TAR   CNTR5+PCNT1+     RS/CC/DS(8) SEC_DATA
//027000 0030150E192525000000 010E0A8A0E1BD80CABB2C3F3903D 80EF579BAEECBE6941A6DC0D437D553FE120026765CF497DEE5D

sec_info S,*s=&S;
char data[512]; int dl;

typedef unsigned char uchar;

int cbc3_decode2(char *dst,char *src, int len, uchar *key,uchar ival[8]) { // sign with ival
DES_key_schedule ks1,ks2,ks3;
DES_cblock cb1,cb2,cb3;
memcpy(&cb1,key,8); memcpy(&cb2,key+8,8); memcpy(&cb3,key+16,8);

DES_cblock cblock; //' = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
  if (DES_set_key(&cb1, &ks1) ||
        DES_set_key(&cb2, &ks2) ||
         DES_set_key(&cb3, &ks3)) {
      printf("Key error!!!, exiting ....\n");
      return -1;
   }

//memset(cblock,0,sizeof(DES_cblock));
printf("sizeof cblock=%d\n",sizeof(cblock));
memcpy(&cblock,ival,8); // init it
//DES_set_odd_parity(&cblock);
//int h = DES_cbc_cksum(src, (void*)key, len, &ks1, &cblock );
//printf("HEX=%x\n",h);

DES_ede3_cbc_encrypt
((const unsigned char*)src, // outer triple cbc des
                         (unsigned char*)dst,
                          len, &ks1, &ks2, &ks3,
                                     &cblock,DES_ENCRYPT);

return len;
}


int cbc3_decode(char *dst,char *src, int len, uchar *key) {
DES_key_schedule ks1,ks2,ks3;
DES_cblock cb1,cb2,cb3;
memcpy(&cb1,key,8); memcpy(&cb2,key+8,8); memcpy(&cb3,key+16,8);

DES_cblock cblock; //' = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
  if (DES_set_key(&cb1, &ks1) ||
        DES_set_key(&cb2, &ks2) ||
         DES_set_key(&cb3, &ks3)) {
      printf("Key error!!!, exiting ....\n");
      return -1;
   }

memset(cblock,0,sizeof(DES_cblock));
DES_set_odd_parity(&cblock);
DES_ede3_cbc_encrypt((const unsigned char*)src, // outer triple cbc des
                         (unsigned char*)dst,
                          len, &ks1, &ks2, &ks3,
                                     &cblock,DES_DECRYPT);

return len;
}

int cbc1_sign(char *keys,char *src,int len,char *kid) {
DES_cblock cb1,cblock;
DES_key_schedule k;


memcpy(&cb1,kid,8); DES_set_key(&cb1,&k);
memset(&cblock,0,sizeof(DES_cblock));
//DES_set_odd_parity(&cblock);
//void DES_cbc_encrypt(const unsigned char *input,unsigned char *output,
		  //   long length,DES_key_schedule *schedule,DES_cblock *ivec,
		  //   int enc);
char dst[512];
/*
DES_cbc_encrypt((const unsigned char*)src, // outer triple cbc des
                         (unsigned char*)dst,
                          len, &k, &cblock,DES_DECRYPT);
hexdump("sec_decrypt",dst,len);
*/
memcpy(&cb1,kid,8); DES_set_key(&cb1,&k);
memset(&cblock,0,sizeof(DES_cblock));
//DES_set_odd_parity(&cblock); // Что это? Я не знаю...

DES_cbc_encrypt((const unsigned char*)src, // outer triple cbc des
                         (unsigned char*)dst,
                          len, &k, &cblock,DES_ENCRYPT);
 //hexdump("sec_encrypt",dst,len);

memcpy(&cb1,kid,8); DES_set_key(&cb1,&k);
memset(&cblock,0,sizeof(DES_cblock));
//DES_set_odd_parity(&cblock);

int code =  DES_cbc_cksum(src, (void*)keys, len, &k, &cblock ); // -> last DES_ENCRYPT

  //printf("SIGN = %x \n",code);
  return 1;
}

int sec_test_good() { //D99E2304FB17158A -- Это врооде бы и есть подпись дл CBC3DES?
    printf("Start Sec test now\n");
hexstr2bin(s->kid, //"D99E2304FB17158A"//,
             "A01A037FDF76B575"
              ,-1); // 8bytes, one key
char *d =  "002D 15  1201 00 11 000000 0000000100 00   " //1A6C12A99D6E9C24
 "80E40000124F10FF434E525801011A00000000011A0000";
memset(data,0,sizeof(data));
int dl = hexstr2bin(data,d,-1); // data without 8 byte RS/CC/DS
hexdump("StartSignData",data,dl);
char keys[8]; memset(keys,0,sizeof(keys));
cbc1_sign(keys,data,dl,s->kid);
hexdump("EXPECT: 3239B94797592E44",keys,8);
return 0;
}

int cbc_sign_11(uchar *data,int len,char *kid) {
uchar buf[512],out[8];
memcpy(buf,data,16);memcpy(buf+16,data+16+8,len-(16+8)); // signer without MAC
if (cbc1_sign(out ,buf,len-8, kid ) <=0) return -1;
memcpy(data+16,out,8); // set sigh inPlace
return 1; // OK
}

int sec_test_nn() { //D99E2304FB17158A -- Это врооде бы и есть подпись дл CBC3DES?
    printf("Start Sec test now\n");
hexstr2bin(s->kid, //"D99E2304FB17158A"//,
             "A01A037FDF76B575" //A01A037FDF76B575
              ,-1); // 8bytes, one key

char *d = "002D151201001100000000000001D200 80E40000124F10FF434E525801011A00000000011A0000";
memset(data,0,sizeof(data));
int dl = hexstr2bin(data,d,-1); // data without 8 byte RS/CC/DS
hexdump("StartSignData",data,dl);
char keys[8]; memset(keys,0,sizeof(keys));
cbc1_sign(keys,data,dl,s->kid);
hexdump("EXPECT: 754F757117B285A8",keys,8);
return 0;
}

int sec_test() { //D99E2304FB17158A -- Это врооде бы и есть подпись дл CBC3DES?
    printf("Start Sec test now\n");
hexstr2bin(s->kid, //"D99E2304FB17158A"//,
             "A01A037FDF76B575" //A01A037FDF76B575
              ,-1); // 8bytes, one key

char *d = "002D151201001100000000000001D200000000000000000080E40000124F10FF434E525801011A00000000011A0000";
memset(data,0,sizeof(data));
int dl = hexstr2bin(data,d,-1); // data without 8 byte RS/CC/DS
hexdump("StartSignData",data,dl);
cbc_sign_11(data,dl,s->kid);

char buf[512];
bin2hexstr(buf,data,dl);
printf(">>>%s\n",buf);

return 0;
}

int sec_test0() {
    //return main1();
    hexstr2bin(s->kic,"30 42 30 42 30 44 30 44 30 45 30 45 30 46 30 46",-1);;
      // 0x25 - Triple DES in outer CBC-Mode using 2 different keys -> 16 bytes?
      //-	Keyset to be used is 0x02
    hexstr2bin(s->kid,"01 23 45 67 89 AB CD EF 10 02 76 FE DC BA 01 23",-1);;
    memset(data,0,sizeof(data));
    dl = hexstr2bin(data,"010E0A8A0E1BD80CABB2C3F3903D 80EF579BAEECBE6941A6DC0D437D553FE120026765CF497DEE5D",-1);
    char out[512];
printf("Hello!\n");
       memcpy(s->kic+16,s->kic,8); // copy first key to 3rd
     hexdump("KIC",s->kic,24);
     hexdump("KID",s->kid,16);
     hexdump("data",data,dl);
       cbc3_decode(out,data,dl,s->kic); //,s->kic+8,s->kic+16); // encode me
     hexdump("outd",out,dl);
     hexdump("secdata",out+14,dl-14);

//sec_test();

return 0;
}

int sec_test2() {
    //return main1();
    //hexstr2bin(s->kic,"30 42 30 42 30 44 30 44 30 45 30 45 30 46 30 46",-1);;
      // 0x25 - Triple DES in outer CBC-Mode using 2 different keys -> 16 bytes?
      //-	Keyset to be used is 0x02
    hexstr2bin(s->kid,"70B0E09C5B8B7057B53CD7C49B5C99ED",-1);;
    memset(data,0,sizeof(data));
    dl = hexstr2bin(data,"0100003B00008000",-1);
    char out[512];
printf("Hello!\n");
      memcpy(s->kid+16,s->kid,8); // copy first key to 3rd
     //hexdump("KIC",s->kic,24);
     hexdump("KID",s->kid,24);
     hexdump("data",data,dl);
     uchar ival[8]={0x31,0x49,0x07,0x00,0x75,0x69,0x16,0x90};
       int code = cbc3_decode2(out,data,dl,s->kid,ival); //,s->kic+8,s->kic+16); // encode me
     hexdump(">> cbc2_decode2_outd",out,4); // first 4bytes is MAC
     printf("EXPECT:  D1EF4F1C, rescode=%x\n",code);
     //hexdump("secdata",out+14,dl-14);

//sec_test();

return 0;
}


#include <openssl/des.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main1() {
   DES_cblock cb1 = { 0xAE, 0xAE, 0xAE, 0xAE, 0xAE, 0xAE, 0xAE, 0xAE };
   DES_cblock cb2 = { 0xAE, 0xAE, 0xAE, 0xAE, 0xAE, 0xAE, 0xAE, 0xAE };
   DES_cblock cb3 = { 0xAE, 0xAE, 0xAE, 0xAE, 0xAE, 0xAE, 0xAE, 0xAE };

   DES_key_schedule ks1,ks2,ks3;

   DES_cblock cblock = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
   int sz = 128;
   char string[sz];
   strcpy(string,"I am a software developer and it is very good for now");
   // ---------------------------------------------
   // I use sizeof instead of strlen because I want
   // to count the '\0' at the end, strlen would
   // not count it
   int stringLen = sz; //sizeof(string);

   printf("PlainText : %s\n",string);
   hexdump("P",string,sizeof(string));

   char cipher[sz];
   char text[sz];

   memset(cipher,0,sizeof(cipher));
   memset(text,0,stringLen);

memset(cblock,0,sizeof(DES_cblock));
   DES_set_odd_parity(&cblock);

   if (DES_set_key_checked(&cb1, &ks1) ||
        DES_set_key_checked(&cb2, &ks2) ||
         DES_set_key_checked(&cb3, &ks3)) {
      printf("Key error, exiting ....\n");
      return 1;
   }

int i;
   //for(i=0;i<sz;i+=8)
   DES_ede3_cbc_encrypt((const unsigned char*)string,
                         (unsigned char*)cipher,
                          stringLen, &ks1, &ks2, &ks3,
                                  &cblock, DES_ENCRYPT);

   printf("Encrypted : %32.32s\n",cipher);

   //-----------------------------------------------
   // You need to start with the same cblock value
   memset(cblock,0,sizeof(DES_cblock));
   DES_set_odd_parity(&cblock);

   //-----------------------------------------------
   // I think you need to use 32 for the cipher len.
   // You can't use strlen(cipher) because if there
   // is a 0x00 in the middle of the cipher strlen
   // will stop there and the length would be short
   //for(i=0;i<sz;i+=8)
   DES_ede3_cbc_encrypt((const unsigned char*)cipher,
                         (unsigned char*)text,
                          sz, &ks1, &ks2, &ks3,
                                     &cblock,DES_DECRYPT);
   printf("Decrypted : %s\n",text);
   hexdump("P",text,sizeof(text));
}
