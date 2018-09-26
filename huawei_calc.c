//  origin source:: https://4pda.ru/forum/index.php?showtopic=582284&st=0#entry35773253

//  Калькулятор NCK-кодов для модемов Huawei с новым алгоритмом v201
//
//  Автор - forth32
//  2014 год
//
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

int calc201(char* imei, char* resbuf);
int calc2(char* imei, char* resbuf);
void encrypt_1(char* imei,char* resbuf,int version);
void encrypt_2(char* imei,char* resbuf,int version);
void encrypt_3(char* imei,char* resbuf,int version);
void encrypt_4(char* imei,char* resbuf,int version);
void encrypt_5_v2(char* imei,char* resbuf);
void encrypt_6(char* imei,char* resbuf,int mode);
void encrypt_7(char* imei,char* resbuf,int version);

void encrypt_v1(char* imei,char* resbuf,char* pass);

unsigned int  rotr32(unsigned int val, int n);


//
//  Калькулятор nlock-кодов для модемов Huawei с новым алгоритмом v201
//
//  Автор - forth32
//  2014 год
//
//  Ветка для вычисления кодов с индексом 0
//


void encrypt_1(char* imei,char* resbuf,int version) {

int i;
unsigned int r0,r1,r2,r3,r4,r5,r6,r7,r12,lr;
unsigned long long cx;
unsigned int ibuf128[128];
int srcbuf_201[]={0x6E9C2A,0x3CA2B3C,0x1080DC,0x30855EE,0x3D3283A,0x2F4F85A,0x1F8808E,0x3147D10,
          0x34BBBB5,0x29EEADD,0x2318616,0x50F3ADC,0xD11F38,0x2123BD2,0x4276C86,0x355CAAD};

int srcbuf_2[]={0x1966A9,0x21058F,0x2AEDA9,0x37CE91,0x488C9F,0x5E507D,0x7A9BE5,0x9F644B,0xCF35A1,0x10D5F55,0x15E2F25,
       0x1C73D6B,0x24FCFDD,0x3015B47,0x3E829E9,0x5143685};

unsigned int buf512[128];
int zvar[8];
int* srcbuf;

if (version == 201) srcbuf=srcbuf_201;
else srcbuf=srcbuf_2;


memset(resbuf,0,9);
memset(buf512,0,512);
for(i=0;i<strlen(imei);i++) buf512[i]=(int)imei[i];

for(i=0;i<16;i++) ibuf128[i]=srcbuf[i];
for(i=16;i<128;i++) ibuf128[i]=i;  // остаток буфер заполняем увеличивающимися числами
r6=0;

for (i=0;i<15;i++)  r6+=buf512[i]*ibuf128[i];

r2=r6;
r7=0xCCCCCCCD;

r3=r2&0xf0;
r1=r2&0xf00;
r0=r2&0xf000;
r12=r2&0xf0000;
lr=r2&0xf00000;
r4=r2&0xf000000;

r3>>=4;
r1>>=8;
zvar[1]=r3;
r0>>=12;
r3=r2>>28;
r12>>=16;
lr>>=20;
r4>>=24;
r2&=0xf;
zvar[2]=r1;
zvar[3]=r0;
zvar[4]=r12;
zvar[5]=lr;
zvar[6]=r4;
zvar[7]=r3;
zvar[0]=r2;

for (i=0;i<8; i++) {
  r3=zvar[i];
  cx=(unsigned long long)r7*(unsigned long long)r3;
  r1=cx&0xffffffff;
  r2=(cx>>32)&0xffffffff;
  r2>>=3;
  r1=r2<<3;
  r2<<=1;
  r2+=r1;
  r3=r3-r2;
  zvar[i]=r3;
}


r3=zvar[0];
if (r3 == 0) {
   r3++;
   zvar[0]=r3;
}

for(i=0;i<8;i++) {
  r3=zvar[i];
  r3+=0x30;
  resbuf[i]=r3;
}

}

//
//  Калькулятор nlock-кодов для модемов Huawei с новым алгоритмом v201
//
//  Автор - forth32
//  2014 год
//
//  Ветка для вычисления кодов с индексом 1
//



void encrypt_2(char* imei,char* resbuf,int version) {

char imei_buf[30];
char buf20[30];
char buf1016[30];

int i;
int r0,r1,r2,r3;
unsigned int u1;
int hash1;
unsigned long long cx;

unsigned int thash1_201[]={
 0,0x77073096,0xEE0E612C,0x990951BA,0x76DC419,0x196C3671,0x6E6B06E7,0xFED41B76,0x89D32BE0,0x10DA7A5A,0xFBD44C65,0x4DB26158,0x3AB551CE,0xA3BC0074,0xD4BB30E2,0x4ADFA541,0x3DD895D7,0xA4D1C46D,0xD3D6F4FB,0x4369E96A,0xD6D6A3E8,0xA1D1937E,0x38D8C2C4,0x4FDFF252,0xD1BB67F1,0xA6BC5767,0x3FB506DD,0x48B2364B,0xD80D2BDA,0xAF0A1B4C,0x36034AF6,0x41047A60,0xDF60EFC3,0xA867DF55,0x316E8EEF,0x90BF1D91,0x1DB71064,0x6AB020F2,0xF3B97148,0x84BE41DE,
 0x1ADAD47D,0x6DDDE4EB,0xF4D4B551,0x83D385C7,0x136C9856,0xFA0F3D63,0x8D080DF5,0x3B6E20C8,0x4C69105E,0xD56041E4,0xA2677172,0x3C03E4D1,0x4B04D447,0xD20D85FD,0xA50AB56B,0x646BA8C0,0xFD62F97A,0x8A65C9EC,0x14015C4F,0x63066CD9,0x45DF5C75,0xDCD60DCF,0xABD13D59,0x26D930AC,0x51DE003A,0xC8D75180,0xBFD06116,0x21B4F4B5,0x56B3C423,0xCFBA9599,0x706AF48F,0xE963A535,0x9E6495A3,0xEDB8832,0x79DCB8A4,0xE0D5E91E,0x97D2D988,0x9B64C2B,0x7EB17CBD,0xE7B82D07,
 0x35B5A8FA,0x42B2986C,0xDBBBC9D6,0xACBCF940,0x32D86CE3,0xB8BDA50F,0x2802B89E,0x5F058808,0xC60CD9B2,0xB10BE924,0x2F6F7C87,0x58684C11,0xC1611DAB,0xB6662D3D,0x76DC4190,0x4969474D,0x3E6E77DB,0xAED16A4A,0xD9D65ADC,0x40DF0B66,0x37D83BF0,0xA9BCAE53,0xDEBB9EC5,0x47B2CF7F,0x30B5FFE9,0xBDBDF21C,0xCABAC28A,0x53B39330,0x24B4A3A6,0xBAD03605,0x3B6E20C,0x74B1D29A,0xEAD54739,0x9DD277AF,0x4DB2615,0xE10E9818,0x7F6A0DBB,0x86D3D2D,0x91646C97,0xE6635C01,
 0x6B6B51F4,0x1C6C6162,0x856530D8,0xF262004E,0x6C0695ED,0x1B01A57B,0x8208F4C1,0xF50FC457,0x65B0D9C6,0x12B7E950,0x8BBEB8EA,0xFCB9887C,0x62DD1DDF,0x15DA2D49,0x8CD37CF3,0xE40ECF0B,0x9309FF9D,0xA00AE27,0x7D079EB1,0xF00F9344,0x4669BE79,0xCB61B38C,0xBC66831A,0x256FD2A0,0x5268E236,0xCC0C7795,0xBB0B4703,0x220216B9,0x5505262F,0xC5BA3BBE,0x68DDB3F8,0x1FDA836E,0x81BE16CD,0xF6B9265B,0x6FB077E1,0x18B74777,0x88085AE6,0xFF0F6A70,0x66063BCA,0x11010B5C,
 0x8F659EFF,0xF862AE69,0x616BFFD3,0x166CCF45,0xA00AE278,0xB2BD0B28,0x2BB45A92,0x5CB36A04,0xC2D7FFA7,0xB5D0CF31,0x2CD99E8B,0x5BDEAE1D,0x9B64C2B0,0xEC63F226,0x756AA39C,0x26D930A,0x9C0906A9,0xEB0E363F,0x72076785,0x5005713,0x346ED9FC,0xAD678846,0xDA60B8D0,0x44042D73,0x33031DE5,0xAA0A4C5F,0xDD0D7CC9,0x5005713C,0x270241AA,0xBE0B1010,0x1DB7106,0x98D220BC,0xEFD5102A,0x71B18589,0x6B6B51F,0x9FBFE4A5,0xE8B8D433,0x7807C9A2,0xF00F934,0x9609A88E,
 0xC90C2086,0x5768B525,0x206F85B3,0xB966D409,0xCE61E49F,0x5EDEF90E,0x29D9C998,0xB0D09822,0xC7D7A8B4,0x59B33D17,0xCDD70693,0x54DE5729,0x23D967BF,0xB3667A2E,0xC4614AB8,0x5D681B02,0x2A6F2B94,0xB40BBE37,0xC30C8EA1,0x5A05DF1B,0x2EB40D81,0xB7BD5C3B,0xC0BA6CAD,0xEDB88320,0x9ABFB3B6,0x73DC1683,0xE3630B12,0x94643B84,0xD6D6A3E,0x7A6A5AA8,0x67DD4ACC,0xF9B9DF6F,0x8EBEEFF9,0x17B7BE43,0x60B08ED5,0x8708A3D2,0x1E01F268,0x6906C2FE,0xF762575D,0x806567CB,
 0x95BF4A82,0xE2B87A14,0x7BB12BAE,0xCB61B38,0x92D28E9B,0xE5D5BE0D,0x7CDCEFB7,0xBDBDF21,0x86D3D2D4,0xF1D4E242,0xD70DD2EE,0x4E048354,0x3903B3C2,0xA7672661,0xD06016F7,0x2D02EF8D
};


unsigned int thash1_2[]={
 0,0x77073096,0xEE0E612C,0x990951BA,0x76DC419,0x706AF48F,0xE963A535,0x9E6495A3,0xEDB8832,0x79DCB8A4,0xE0D5E91E,0x97D2D988,0x9B64C2B,0x7EB17CBD,0xE7B82D07,0x90BF1D91,0x1DB71064,0x6AB020F2,0xF3B97148,0x84BE41DE,0x1ADAD47D,0x6DDDE4EB,0xF4D4B551,0x83D385C7,0x136C9856,0x646BA8C0,0xFD62F97A,0x8A65C9EC,0x14015C4F,0x63066CD9,0xFA0F3D63,0x8D080DF5,0x3B6E20C8,0x4C69105E,0xD56041E4,0xA2677172,0x3C03E4D1,0x4B04D447,0xD20D85FD,0xA50AB56B,
 0x35B5A8FA,0x42B2986C,0xDBBBC9D6,0xACBCF940,0x32D86CE3,0x45DF5C75,0xDCD60DCF,0xABD13D59,0x26D930AC,0x51DE003A,0xC8D75180,0xBFD06116,0x21B4F4B5,0x56B3C423,0xCFBA9599,0xB8BDA50F,0x2802B89E,0x5F058808,0xC60CD9B2,0xB10BE924,0x2F6F7C87,0x58684C11,0xC1611DAB,0xB6662D3D,0x76DC4190,0x1DB7106,0x98D220BC,0xEFD5102A,0x71B18589,0x6B6B51F,0x9FBFE4A5,0xE8B8D433,0x7807C9A2,0xF00F934,0x9609A88E,0xE10E9818,0x7F6A0DBB,0x86D3D2D,0x91646C97,0xE6635C01,
 0x6B6B51F4,0x1C6C6162,0x856530D8,0xF262004E,0x6C0695ED,0x1B01A57B,0x8208F4C1,0xF50FC457,0x65B0D9C6,0x12B7E950,0x8BBEB8EA,0xFCB9887C,0x62DD1DDF,0x15DA2D49,0x8CD37CF3,0xFBD44C65,0x4DB26158,0x3AB551CE,0xA3BC0074,0xD4BB30E2,0x4ADFA541,0x3DD895D7,0xA4D1C46D,0xD3D6F4FB,0x4369E96A,0x346ED9FC,0xAD678846,0xDA60B8D0,0x44042D73,0x33031DE5,0xAA0A4C5F,0xDD0D7CC9,0x5005713C,0x270241AA,0xBE0B1010,0xC90C2086,0x5768B525,0x206F85B3,0xB966D409,0xCE61E49F,
 0x5EDEF90E,0x29D9C998,0xB0D09822,0xC7D7A8B4,0x59B33D17,0x2EB40D81,0xB7BD5C3B,0xC0BA6CAD,0xEDB88320,0x9ABFB3B6,0x3B6E20C,0x74B1D29A,0xEAD54739,0x9DD277AF,0x4DB2615,0x73DC1683,0xE3630B12,0x94643B84,0xD6D6A3E,0x7A6A5AA8,0xE40ECF0B,0x9309FF9D,0xA00AE27,0x7D079EB1,0xF00F9344,0x8708A3D2,0x1E01F268,0x6906C2FE,0xF762575D,0x806567CB,0x196C3671,0x6E6B06E7,0xFED41B76,0x89D32BE0,0x10DA7A5A,0x67DD4ACC,0xF9B9DF6F,0x8EBEEFF9,0x17B7BE43,0x60B08ED5,
 0xD6D6A3E8,0xA1D1937E,0x38D8C2C4,0x4FDFF252,0xD1BB67F1,0xA6BC5767,0x3FB506DD,0x48B2364B,0xD80D2BDA,0xAF0A1B4C,0x36034AF6,0x41047A60,0xDF60EFC3,0xA867DF55,0x316E8EEF,0x4669BE79,0xCB61B38C,0xBC66831A,0x256FD2A0,0x5268E236,0xCC0C7795,0xBB0B4703,0x220216B9,0x5505262F,0xC5BA3BBE,0xB2BD0B28,0x2BB45A92,0x5CB36A04,0xC2D7FFA7,0xB5D0CF31,0x2CD99E8B,0x5BDEAE1D,0x9B64C2B0,0xEC63F226,0x756AA39C,0x26D930A,0x9C0906A9,0xEB0E363F,0x72076785,0x5005713,
 0x95BF4A82,0xE2B87A14,0x7BB12BAE,0xCB61B38,0x92D28E9B,0xE5D5BE0D,0x7CDCEFB7,0xBDBDF21,0x86D3D2D4,0xF1D4E242,0x68DDB3F8,0x1FDA836E,0x81BE16CD,0xF6B9265B,0x6FB077E1,0x18B74777,0x88085AE6,0xFF0F6A70,0x66063BCA,0x11010B5C,0x8F659EFF,0xF862AE69,0x616BFFD3,0x166CCF45,0xA00AE278,0xD70DD2EE,0x4E048354,0x3903B3C2,0xA7672661,0xD06016F7,0x4969474D,0x3E6E77DB,0xAED16A4A,0xD9D65ADC,0x40DF0B66,0x37D83BF0,0xA9BCAE53,0xDEBB9EC5,0x47B2CF7F,0x30B5FFE9,
 0xBDBDF21C,0xCABAC28A,0x53B39330,0x24B4A3A6,0xBAD03605,0xCDD70693,0x54DE5729,0x23D967BF,0xB3667A2E,0xC4614AB8,0x5D681B02,0x2A6F2B94,0xB40BBE37,0xC30C8EA1,0x5A05DF1B,0x2D02EF8D
};

unsigned int* thash1;
if (version == 201) thash1=thash1_201;
else                thash1=thash1_2;

strcpy(imei_buf,imei);

u1=0xffffffff;

for (i=0;i<strlen(imei_buf);i++)
  u1=thash1[(u1&0xff)^(unsigned int)imei_buf[i]] ^ (u1>>8);

hash1=~u1;

r0=(hash1^(hash1>>31))-(hash1>>31);
if (r0 != 0) {  // 50d87324
  for(i=0;;i++) {
    cx=r0*(unsigned long long)0x66666667;
    r3=(cx>>34)-(r0>>31);
    r1=r3;
    r3=r0-((r3<<1)+(r3<<3))+0x30;
    r0=r1;
    buf1016[i]=(r3&0xff);
    if (r1 == 0) break;
  }
  r0=i;
} else  r0=-1;


if (r0 > 0) {
  if (r0!=8) r0=8;
  for (i=0;i<r0;i++) buf20[i]=buf1016[r0-1-i];
  buf20[r0]=0;
}
else strcpy(buf20,"99999999");

// определение знака
resbuf[0]=0;
//if (hash1 < 0) strcpy(resbuf,"-");
strcat(resbuf,buf20);

// замещение байтов
if ((resbuf[0] == '0') ||(resbuf[0] == '-') ||(resbuf[0] == ' ')) resbuf[0]='9';
}


//
//  Калькулятор nlock-кодов для модемов Huawei с новым алгоритмом v201
//
//  Автор - forth32
//  2014 год
//
//  Ветка для вычисления кодов с индексом 2
//



#define imeilen 15;

void encrypt_3(char* imei,char* resbuf,int version) {

int i;
int r0,r1,r2,r3,r4,r5,r6,r12,lr;
unsigned int u1;
unsigned long long cx;
unsigned int arg;

char imeibuf[500];
unsigned int* imeiptr=(unsigned int*)imeibuf;
unsigned int hash2[4]={0x67452301,0xefcdab89,0x98badcfe,0x10325476};
unsigned int copyhash[4];

unsigned char*  hashptr;

unsigned char buf104[120];

resbuf[0]=0;

memset(imeibuf,0,500);
strcpy(imeibuf,imei);
imeibuf[15]=0x80;
imeibuf[0x38]=0x78;

r2=hash2[0];
r4=hash2[1];
r12=hash2[2];
lr=hash2[3];

arg=r12^lr;

#define encpass1(hconst,nbyte,shift,rsum,ra,rb,rc) \
  rsum=ra+rotr32(rsum+imeiptr[nbyte/4]+((unsigned int)hconst)+((arg&ra)^rb),shift); \
  arg=ra^rc;

#define encpass1_25(hconst,nbyte) encpass1(hconst,nbyte,25,r2, r4, lr, r12)
#define encpass1_20(hconst,nbyte) encpass1(hconst,nbyte,20,lr, r2, r12,r4 )
#define encpass1_15(hconst,nbyte) encpass1(hconst,nbyte,15,r12,lr, r4, r2 )
#define encpass1_10(hconst,nbyte) encpass1(hconst,nbyte,10,r4, r12,r2, lr )

encpass1_25(0xD76AA478,0)
encpass1_20(0xE8C7B756,4)
encpass1_15(0x242070DB,8)
encpass1_10(0xC1BDCEEE,0xc)

encpass1_25(0xF57C0FAF,0x10)   // 50d8769c
encpass1_20(0x4787C62A,0x14)   // 50D876C0
encpass1_15(0xA8304613,0x18)   // 50D876E4
encpass1_10(0xFD469501,0x1c)   // 50D87708

encpass1_25(0x698098D8,0x20)   // RAM:50D8772C
encpass1_20(0x8B44F7AF,0x24)   // RAM:50D87750
encpass1_15(0xffff5bb1,0x28)   // RAM:50D87774
encpass1_10(0x895CD7BE,0x2c)   // 50D87798

encpass1_25(0x6B901122,0x30)   // RAM:50D877BC
encpass1_20(0xFD987193,0x34)   // RAM:50D877E0
encpass1_15(0xA679438E,0x38)   // RAM:50D87804
encpass1_10(0x49B40821,0x3c)   // RAM:50D87828

// смена алгоритма

#define encpass2(hconst,nbyte,shift,rsum,ra,rb,rc) \
 rsum+=imeiptr[nbyte/4]+(((rb^ra)&rc)^rb)+hconst;     \
 rsum=ra+rotr32(rsum,shift);

// r2=r2+imeiptr[4/4]+(((r12^r4)&lr)^r12)+0xF61E2562;
// rsum                   rb ra  rc   rb
// r2  =  r4+rotr32(r2,27);
//rsum    ra

#define encpass2_27(hconst,nbyte) encpass2(hconst,nbyte,27,r2, r4, r12,lr )
#define encpass2_23(hconst,nbyte) encpass2(hconst,nbyte,23,lr, r2, r4 ,r12)
#define encpass2_18(hconst,nbyte) encpass2(hconst,nbyte,18,r12,lr, r2 ,r4 )
#define encpass2_12(hconst,nbyte) encpass2(hconst,nbyte,12,r4, r12,lr ,r2 )

encpass2_27(0xF61E2562,4)
encpass2_23(0xC040B340,0x18)  // 50D87870
encpass2_18(0x265E5A51,0x2c)  // RAM:50D87894
encpass2_12(0xE9B6C7AA,0)   // 50d878b8

encpass2_27(0xD62F105D,0x14)  // 50d878dc
encpass2_23(0x02441453,0x28)   // RAM:50D87900
encpass2_18(0xD8A1E681,0x3c) // 50D87924
encpass2_12(0xE7D3FBC8,0x10) // RAM:50D87948

encpass2_27(0x21E1CDE6,0x24) //50d8796c
encpass2_23(0xC33707D6,0x38) //50d87990
encpass2_18(0xF4D50D87,0x0c) // RAM:50D879B4
encpass2_12(0x455A14ED,0x20) // 50D879d8

encpass2_27(0xA9E3E905,0x34) // 50d879fc
encpass2_23(0xFCEFA3F8,0x08) //50d87a20
encpass2_18(0x676F02D9,0x1c) // 50d87a44
encpass2_12(0x8D2A4C8A,0x30) // 50d87a68

// смена алгоритма

// r2=r4+rotr32(r2+imeiptr[0x14/4]+((lr^r12)^r4)+0xFFFA3942,28);        // 50d87a84
// rs ra        rs                   rb  rc  ra    hconst   shift

#define encpass3(hconst,nbyte,shift,rsum,ra,rb,rc) \
 rsum=ra+rotr32(rsum+imeiptr[nbyte/4]+((rb^rc)^ra)+hconst,shift);

#define encpass3_28(hconst,nbyte) encpass3(hconst,nbyte,28,r2, r4, lr, r12)
#define encpass3_21(hconst,nbyte) encpass3(hconst,nbyte,21,lr, r2, r12,r4 )
#define encpass3_16(hconst,nbyte) encpass3(hconst,nbyte,16,r12,lr, r4, r2 )
#define encpass3_9(hconst,nbyte) encpass3(hconst,nbyte,9 ,r4, r12,r2, lr )

encpass3_28(0xFFFA3942,0x14)  // 50d87a84
encpass3_21(0x8771F681,0x20)  // 50d87aa4
encpass3_16(0x6D9D6122,0x2c)  // 50d87ac4
encpass3_9 (0xFDE5380C,0x38)  // 50d87ae4

encpass3_28(0xA4BEEA44,0x04)  // 50d87b04
encpass3_21(0x4BDECFA9,0x10)  // 50d87b24
encpass3_16(0xF6BB4B60,0x1c)  // 50d87b44
encpass3_9 (0xBEBFBC70,0x28)  // 50d87b64

encpass3_28(0x289B7EC6,0x34)  // 50d87b84
encpass3_21(0xEAA127FA,0x00)  // 50d87ba4
encpass3_16(0xD4EF3085,0x0c)  // 50d87bc4
encpass3_9 (0x04881D05,0x18)  // 50d87be4

encpass3_28(0xD9D4D039,0x24)  // 50d87c04
encpass3_21(0xE6DB99E5,0x30)  // 50d87c24
encpass3_16(0x1FA27CF8,0x3c)  // 50d87c44
encpass3_9 (0xC4AC5665,0x08)  // 50d87c64

// алгоритм 4

#define encpass4(hconst,nbyte,shift,rsum,ra,rb,rc) \
 rsum=ra+rotr32(rsum+imeiptr[nbyte/4]+(((~rb)|ra)^rc)+hconst,shift);

#define encpass4_26(hconst,nbyte)  encpass4(hconst,nbyte,26,r2, r4, lr, r12)
#define encpass4_22(hconst,nbyte)  encpass4(hconst,nbyte,22,lr, r2, r12, r4)
#define encpass4_17(hconst,nbyte)  encpass4(hconst,nbyte,17,r12,lr, r4,  r2)
#define encpass4_11(hconst,nbyte)  encpass4(hconst,nbyte,11,r4, r12,r2,  lr)


//  r2=r4+rotr32(r2+imeiptr[0/4]+(((~lr)|r4)^r12)+0xF4292244,26);
//  rs ra        rs                  rb  ra  rc

encpass4_26(0xF4292244,0x00)   // 50d87c88
encpass4_22(0x432AFF97,0x1c)   // 50d87cac
encpass4_17(0xAB9423A7,0x38)   // 50d87cd0
encpass4_11(0xFC93A039,0x14)   // 50d87cf4

encpass4_26(0x655B59C3,0x30)   // 50d87d18
encpass4_22(0x8F0CCC92,0x0c)   // 50d87d3c
encpass4_17(0xFFEFF47D,0x28)   // 50d87d60
encpass4_11(0x85845DD1,0x04)   // 50d87d84

encpass4_26(0x6FA87E4F,0x20)   // 50d87da8
encpass4_22(0xFE2CE6E0,0x3c)   // 50d87dcc
encpass4_17(0xA3014314,0x18)   // 50d87df0
encpass4_11(0x4E0811A1,0x34)   // 50d87e14

encpass4_26(0xF7537E82,0x10)   // 50d87e38
encpass4_22(0xBD3AF235,0x2c)   // 50d87e5c
encpass4_17(0x2AD7D2BB,0x08)   // 50d87e80

// 50d87eb4 - алгоритм обработки немного другой
hash2[1]=hash2[1]+r12+rotr32(r4+imeiptr[0x24/4]+(((~r2)|r12)^lr)+0xEB86D391,11);
hash2[0]+=r2;
hash2[2]+=r12;
hash2[3]+=lr;

memcpy(copyhash,hash2,16);
hash2[0]=0;
memset(buf104,0,110);

hashptr=(unsigned char*)copyhash;
if (version == 201) hashptr+=5;  // пказываем на байт 5

memcpy(buf104,hashptr,8);

r2=buf104[0];
cx=(unsigned long long)r2*(unsigned long long)0xcccccccd;
r3=(cx>>32)&0xffffffff;
r3>>=3;
r1=r3<<3;
r3<<=1;
r3=r3+r1;
r2=r2-r3;

if (r2 != 0) {
  buf104[0]=r2;
}
else {
  for(i=8;i<14;i++) {
    r3=buf104[i];
    cx=(unsigned long long)0xcccccccd*(unsigned long long)r3;   // r2:r7
    r2=(cx>>32)&0xffffffff;
    r2>>=3;
    r1=r2<<3;
    r2<<=1;
    r2+=r1;
    r3=r3-r2;
    r2=r3;
    if (r2 != 0) break;
  }
  if (r2 == 0) buf104[0]=5;
  else         buf104[0]=r2;
}

for (i=0;i<8;i++) {
  r1=buf104[i];
  r3=(r1-0x30)&0xff;
  cx=(unsigned long long)0xcccccccd*(unsigned long long)r1;   // r2:r7
  r2=(cx>>32)&0xffffffff;
  if (r3>9) {
    r3=r2>>3;
    r2=r3<<3;
    r3<<=1;
    r3+=r2;
    r3=r1-r3;
    r3+=0x30;
    buf104[i]=r3;
  }
}
memcpy(resbuf,buf104,8);
resbuf[8]=0;


}


//
//  Калькулятор nlock-кодов для модемов Huawei с новым алгоритмом v201
//
//  Автор - forth32
//  2014 год
//
//  Ветка для вычисления кодов с индексом 3
//




void rehash4(unsigned int* hash,unsigned  int* rhash) {

int r0,r1,r4,r6,r7,r10,r12,lr;

int inthash[16];

memcpy(inthash,rhash,64);

r7=hash[0];
r6=hash[1];
r12=hash[2];
lr=hash[3];

//  начинаются алгоритмы свертки

//   r1=r6+rotr32(r7+inthash[0]+((r6&r12)|((~r6)&lr))+0xD76AA478,25);
//  rsum ra       rb        nb    ra  rc     ra   rd    hconst   shift
#define encpass1(hconst,nbyte,shift,rsum,ra,rb,rc,rd) \
   rsum=ra+rotr32(rb+inthash[nbyte/4]+((ra&rc)|((~ra)&rd))+hconst,shift);

encpass1(0xD76AA478,0,25,r1,r6,r7,r12,lr)

// lr=r1+rotr32(lr+inthash[4/4]+((r6&r1)|(~r1&r12))+0xE8C7B756,20);
// rs ra        rb                rc ra   ra   rd

encpass1(0xE8C7B756,4,20,lr,r1,lr,r6,r12)   //50d88854
encpass1(0x242070DB,8,15,r12,lr,r12,r1,r6)  //50d8887c
encpass1(0xC1BDCEEE,0xc,10,r0,r12,r6,lr,r1) // 50d888a4

encpass1(0xF57C0FAF,0x10,25,r1,r0,r1,r12,lr) // 50d888cc
encpass1(0x4787C62A,0x14,20,lr,r1,lr,r0,r12) // 50d888f8
encpass1(0xA8304613,0x18,15,r12,lr,r12,r1,r0) // 50d8891c
encpass1(0xFD469501,0x1c,10,r0,r12,r0,lr,r1)  // 50D88944

encpass1(0x698098D8,0x20,25,r1,r0,r1,r12,lr)  // 50d88970
encpass1(0x8B44F7AF,0x24,20,lr,r1,lr,r0,r12)  // 50d88994
encpass1(0xffff5c00-0x4f,0x28,15,r12,lr,r12,r1,r0) // 50d889c0
encpass1(0x895CD7BE,0x2c,10,r0,r12,r0,lr,r1)     //    9e4

encpass1(0x6B901122,0x30,25,r1,r0,r1,r12,lr)  // a0c
encpass1(0xFD987193,0x34,20,lr,r1,lr,r0,r12)   // a38
encpass1(0xA679438E,0x38,15,r12,lr,r12,r1,r0)  // a5c
encpass1(0x49B40821,0x3c,10,r0,r12,r0,lr,r1)   // a88

//   r1=r0+rotr32(r1+inthash[4/4]+(lr&r0|(~lr&r12))+0xF61E2562,27);
//   rs  ra       rb               rc ra   rc  rd

#define encpass2(hconst,nbyte,shift,rsum,ra,rb,rc,rd) \
   rsum=ra+rotr32(rb+inthash[nbyte/4]+((ra&rc)|((~rc)&rd))+hconst,shift);

encpass2(0xF61E2562,0x04,27,r1,r0,r1,lr,r12);  // 50d88aac
encpass2(0xC040B340,0x18,23,lr,r1,lr,r12,r0);  //acc
encpass2(0x265E5A51,0x2c,18,r12,lr,r12,r0,r1)  // af0
encpass2(0xE9B6C7AA,0x00,12,r0,r12,r0,r1,lr)      // b18

encpass2(0xD62F105D,0x14,27,r1,r0,r1,lr,r12) //b44
encpass2(0x02441453,0x28,23,lr,r1,lr,r12,r0) //b68
encpass2(0xD8A1E681,0x3c,18,r12,lr,r12,r0,r1)  // b90
encpass2(0xE7D3FBC8,0x10,12,r0,r12,r0,r1,lr)      // bb4

encpass2(0x21E1CDE6,0x24,27,r1,r0,r1,lr,r12)  // be0
encpass2(0xC33707D6,0x38,23,lr,r1,lr,r12,r0) //  c00
encpass2(0xF4D50D87,0x0c,18,r12,lr,r12,r0,r1)  // c28
encpass2(0x455A14ED,0x20,12,r0,r12,r0,r1,lr)   // c50

encpass2(0xA9E3E905,0x34,27,r1,r0,r1,lr,r12)  // c7c
encpass2(0xFCEFA3F8,0x08,23,lr,r1,lr,r12,r0) //  ca0
encpass2(0x676F02D9,0x1c,18,r12,lr,r12,r0,r1)  // cc8
encpass2(0x8D2A4C8A,0x30,12,r0,r12,r0,r1,lr)   // cec

//----------------------------------------------

#define encpass3(hconst,nbyte,shift,rsum,ra,rb,rc) \
   rsum=ra+rotr32(rsum+inthash[nbyte/4]+(ra^(rb^rc))+hconst,shift);


//r1=r0+rotr32(r1+inthash[0x14/4]+(r0^(lr^r12))+0xFFFA3942,28);
//rs ra        rs                  ra  rb  rc
//r3=r12^r0;

encpass3(0xFFFA3942,0x14,28,r1,r0,lr,r12);   // 0x50d88d10
encpass3(0x8771F681,0x20,21,lr,r1,r12,r0)    // d2c
encpass3(0x6D9D6122,0x2c,16,r12,lr,r0,r1)    // d48
encpass3(0xFDE5380C,0x38,9,r0,r12,r1,lr)    // d64

encpass3(0xA4BEEA44,0x04,28,r1,r0,lr,r12);   // d84
encpass3(0x4BDECFA9,0x10,21,lr,r1,r12,r0)    // da0
encpass3(0xF6BB4B60,0x1c,16,r12,lr,r0,r1)    // dc4
encpass3(0xBEBFBC70,0x28,9,r0,r12,r1,lr)    // de0

encpass3(0x289B7EC6,0x34,28,r1,r0,lr,r12);   // e00
encpass3(0xEAA127FA,0x00,21,lr,r1,r12,r0)    // e24
encpass3(0xD4EF3085,0x0c,16,r12,lr,r0,r1)    // e40
encpass3(0x4881D05,0x18,9,r0,r12,r1,lr)      // e60

encpass3(0xD9D4D039,0x24,28,r1,r0,lr,r12);   // e84
encpass3(0xE6DB99E5,0x30,21,lr,r1,r12,r0)    // e9c
encpass3(0x1FA27CF8,0x3c,16,r12,lr,r0,r1)    // ebc
encpass3(0xC4AC5665,0x08,9,r0,r12,r1,lr)      // edc

//------------------------------------------------------

// r1=r0+rotr32(r1+inthash[0/4]+(r12^(r0|(~lr)))+0xF4292244,26);
// rs ra        rs               rb   ra   rc

#define encpass4(hconst,nbyte,shift,rsum,ra,rb,rc) \
   rsum=ra+rotr32(rsum+inthash[nbyte/4]+(rb^(ra|(~rc)))+hconst,shift);

encpass4(0xF4292244,0x00,26,r1,r0,r12,lr) // 50d88f00
encpass4(0x432AFF97,0x1c,22,lr,r1,r0,r12) // f24
encpass4(0xAB9423A7,0x38,17,r12,lr,r1,r0)  // f44
encpass4(0xFC93A039,0x14,11,r0,r12,lr,r1)  // f68

encpass4(0x655B59C3,0x30,26,r1,r0,r12,lr) // f88
encpass4(0x8F0CCC92,0x0c,22,lr,r1,r0,r12) // fac
encpass4(0xFFEFF47D,0x28,17,r12,lr,r1,r0)  // fd0
encpass4(0x85845DD1,0x04,11,r0,r12,lr,r1)  // ff4

encpass4(0x6FA87E4F,0x20,26,r1,r0,r12,lr) // 50d89018
encpass4(0xFE2CE6E0,0x3c,22,lr,r1,r0,r12) // 03c
encpass4(0xA3014314,0x18,17,r12,lr,r1,r0)  // 060
encpass4(0x4E0811A1,0x34,11,r0,r12,lr,r1)  // 084

encpass4(0xF7537E82,0x10,26,r1,r0,r12,lr) // 0a4
encpass4(0xBD3AF235,0x2c,22,lr,r1,r0,r12) // 0c4
encpass4(0x2AD7D2BB,0x08,17,r12,lr,r1,r0)  // 0f0

// В этой точке:
// R-здесь  R-в прошивке значение
//   R0          R4       4606f40d
//   R1          r10      c4863aae
//   R12         R12      5747f475
//   LR          R7       3e1096a1

//encpass4(0xEB86D391,0x24,11,r0,r12,lr,r1)  // 124
// переходим на новые регистры
r4=r0;
r10=r1;
r7=lr;
// последний проход хеша и сохранение результата

hash[0]+=r10;
hash[1]+=r12+rotr32(inthash[0x24/4]+r4+((~r10|r12)^r7)+0xEB86D391,11);
hash[2]+=r12;
hash[3]+=r7;

}


void enc4_sub_2(unsigned int* hash,char* hbuf,int hlen) {

int r0,r2,r3,r4,r5,r12;


r2=hash[4];
r3=r2;
r12=hlen<<3;
r3+=r12;
hash[4]=r3;
r0=r3;
r2>>=3;
if (r12 > r0) hash[5]++;
r2&=0x3f;
r5=0x40-r2;

r3=hash[5];
r3+=hlen>>29;
hash[5]=r3;


if (hlen < r5) {
   memcpy((unsigned char*)hash+24+r2,hbuf,hlen);
   return;
}
memcpy((unsigned char*)hash+24+r2,hbuf,r5);
rehash4(hash,hash+6);

if (hlen>(r5+0x3f)) {
  r4=r5;  // индекс в буфере
  do {
    r5+=0x40;
    rehash4(hash,(unsigned int*)(hbuf+r4));
    r4+=0x50;
    printf("\n iter r5=%08x  r4=%08x",r5,r4);
  } while (hlen > (r5+0x3f));
}
memcpy((unsigned char*)hash,hbuf+r5,hlen-r5);

}


void enc4_sub_1(char* hbuf, int* zvar) {

int r2,r3;
int hc[]={0x67452301,0xEFCDAB89,0x98BADCFE,0x10325476};
char hdata[]={0x80,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
                 0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0, 0,0 };

int hlen;
unsigned int hash[30];  // на самом деле юзаются 22 слова
char buf26[40];

hlen=strlen(hbuf);
memset(hash,0,88);

hash[0]=hc[0];
hash[1]=hc[1];
hash[2]=hc[2];
hash[3]=hc[3];
hash[4]=0;
hash[5]=0;
enc4_sub_2(hash,hbuf,hlen);   // hash=r0  hbuf=r1 hlen=r2
memcpy(buf26,hash+4,8);

r3=(hash[4]>>3)&0x3f;
if (r3>0x37) r2=0x78-r3;
       else  r2=0x38-r3;

enc4_sub_2(hash,hdata,r2);
enc4_sub_2(hash,buf26,8);
memcpy(zvar,hash,16);

}


void encrypt_4(char* imei,char* resbuf,int version) {

int i;
int r0,r1,r2,r3;
int zvar[5];

char buf12[20];
char buf256[256];
char buf44[100];

memset(buf12,0,20);
if (version==2) strcpy(buf12,"hwideadatacard");
else strcpy(buf12,"dfkdkfllekkodk");

memset(buf256,0,256);
memset(zvar,0,20);
enc4_sub_1(buf12,zvar);

strcpy(buf256,imei);
memcpy(buf256+strlen(imei),zvar,16); // buf256 - это imei + вычисленный zvar
memset(resbuf,0,8); //  очищаем приемник результата
enc4_sub_1(buf256,(int*)buf12);

for (i=0;i<4;i++) {
  r2=buf12[i];
  r0=buf12[i+4];
  r3=buf12[i+8];
  r1=buf12[i+12];
  r2=(r2^r0)^(r3^r1);
  buf44[i]=r2;
}

r2=0;
for(i=0;i<4;i++) {
  r2=(r2<<8)|(buf44[i]&0xff);
}
r2|=0x2000000;
r2=r2&(~0xfc000000);
sprintf(resbuf,"%i",r2);
}

//
//  Калькулятор nlock-кодов для модемов Huawei с новым алгоритмом v2
//
//  Автор - forth32
//  2014 год
//
//  Ветка для вычисления кодов с индексом 3
//



void GetEncrySStr(char* buf, char* phash, int off) {

int i;
char c;
for(i=0;i<2;i++) {
 c=buf[off+i*8];
 phash[i]=c;
}
}

int rexor(char* phash) {

char c;
int i,len;

len=strlen(phash);
c=phash[0];
if (len == 1) return (c^'Z');
for(i=1;i<len;i++) {
  c^=phash[i];
}
return (unsigned int)c;
}

//====================================================================================

void encrypt_5_v2(char* imei,char* resbuf) {

int r0,r1,r2,r3,r4,r5,r6,r7,r8,r9,r10,r12,lr;
int i;
char PW_table[]="5739146280098765432112345678905\000";

char buf[140];
int hash[4];
char* phash=(char*)hash;

memset(buf,0,140);
strcpy(buf,imei);
buf[15]='Z';

for(i=0;i<8;i++) {
  memset(hash,0,16);
  GetEncrySStr(buf,phash,i);
  r0=rexor(phash)&0xff;
  resbuf[i]=r0;
}

for(i=0;i<8;i++) {
  r3=resbuf[i]&0xff;
  r2=r3&0xf;
  r1=PW_table[(r3>>4)+r2];
  resbuf[i]=r1;
}

if (resbuf[0]=='0') {
  for(i=1;i<8;i++){
    if (resbuf[i] != '0') break;
  }
  resbuf[0]=i+'0';
}
resbuf[8]=0;

}

//
//  Калькулятор nlock-кодов для модемов Huawei с новым алгоритмом v201
//
//  Автор - forth32
//  2014 год
//
//  Ветка для вычисления кодов с индексом 5 и 6 - они одинаковые и отличаются только номерами выбираемых числе из хеша
//



void rehash5(unsigned int* hash) {

unsigned int r0,r1,r2,r3,r4,r5,r6,r7,r12,lr;
int i;

int sbuf[88];  // буфер sp+00

char* chash=(char*)hash+0x1f;

for(i=0;i<16;i++) {

 r1=*(chash-3)&0xff;
 r3=*(chash-2)&0xff;
 r0=*(chash-1)&0xff;
 r2=*(chash-0)&0xff;

 chash+=4;
 r3<<=16;
 r3|=(r1<<24);
 r3|=r2;
 r3|=(r0<<8);

 sbuf[i]=r3;
}



for(i=13;i<77;i++) {

 r3=sbuf[i];
 r0=sbuf[i-5];
 r2=sbuf[i-11];
 r1=sbuf[i-13];
 r3=r3^r0;
 r2=r2^r1;
 r3=r3^r2;
 r3=rotr32(r3,31);
 sbuf[i+3]=r3;
}

r6=hash[0];
lr=hash[1];
r4=hash[2];
r7=hash[3];
r0=hash[4];

for(i=0;i<20;i++) {
  if (i != 0) {
   r6=r1;
   r7=r3;
  }
  r1=lr&r4;
  r12=sbuf[i];
  r3=~lr;
  r3&=r7;
  r1|=r3;
  r0+=rotr32(r6,27);
  r1+=r12;
  r12=rotr32(lr,2);
  r2=r0+0x5A827999;
  r3=r4;
  r1+=r2;
  r4=r12;
  r0=r7;
  lr=r6;
}
// 50d89714
r5=r1;
r1=r7;
r4=r6;
r6=r3;


for(i=0;i<20;i++){
  if (i != 0) {
    r5=r3;
    r12=lr;
    r6=r2;
  }
  r3=r4^r12;
  r0=sbuf[i+20];
  r1+=rotr32(r5,27);
  r3^=r6;
  r2=r1+0x6ED9EBA1;
  r3+=r0;
  r3+=r2;
  lr=rotr32(r4,2);
  r2=r12;
  r1=r6;
  r4=r5;
}

// 50d89774

r7=r3;
r4=r12;
for(i=0;i<20;i++) {
  if (i != 0) {
    r7=r3;
    r4=r2;
  }
  r3=lr|r4;
  r3&=r5;
  r0=lr&r4;
  r12=sbuf[i+40];
  r1+=rotr32(r7,27);
  r6=rotr32(r5,2);
  r3|=r0;
  r2=r1+0x8F1BBCDC;
  r3+=r12;
  r3+=r2;
  r1=r4;
  r2=lr;
  r5=r7;
  lr=r6;
}

// 50d897d4
lr=r3;
r12=r7;
r5=r2;

for(i=0;i<20;i++) {
  if (i != 0) {
    lr=r2;
    r6=r0;
    r5=r3;
  }
  r0=sbuf[i+60];
  r3=r12^r6;
  r1+=rotr32(lr,27);
  r3^=r5;
  r3+=r0;
  r2=r1+0xCA62C1D6;
  r2+=r3;
  r0=rotr32(r12,2);
  r3=r6;
  r1=r5;
  r12=lr;
}

//50d89830

hash[0]+=r2;
hash[1]+=lr;
hash[2]+=r0;
hash[3]+=r6;
hash[4]+=r5;
hash[23]=0;

}

// mode:
//  2 - Ветка 6 алгоритма 2
//  5 - Ветка 5 алгоритам 201
//  6 - Ветка 6 алгоритма 201
//
//

void encrypt_6(char* imei,char* resbuf, int mode) {

int i;
unsigned int r0,r1,r2,r3,r4,r5,r6,r7,r12,lr;
unsigned int u1;
unsigned long long cx;

unsigned int hash[26];
unsigned char* chash;
unsigned char* phash;

char varstr[30];

chash=(unsigned char*)hash+0x1c;  // тот самый buf40-0xac
phash=(unsigned char*)hash;

memset(hash,0,26*4);

hash[0]=0x67452301;
hash[5]=0;
hash[6]=0;
hash[23]=0;
hash[1]=0xEFCDAB89;
hash[2]=0x98BADCFE;
hash[3]=0x10325476;
hash[4]=0xC3D2E1F0;
hash[24]=0;
hash[25]=0;

for (i=0;i<15;i++) {
 r2=hash[23];

 r3=imei[i];

 chash[r2]=r3;
 r2++;
 hash[23]=r2;

 r3=hash[5];
 r3+=8;
 hash[5]=r3;
 r2=r3;

 if (r3 == 0) {
   r3=hash[6];
   r3++;
   hash[6]=r3;
   r2=r3;
   if (r3 == 0) {
     r3++;
     hash[25]=r3;
   }
 }

 r3=hash[23];
 if (r3 == 0x40) rehash5(hash);
 if (hash[25] != 0) break;
}

if (hash[25] != 0) {
  printf("\nERROR-- could not compute message digest");
  return;
}

r0=hash[24];
if (r0 != 0) goto bypass1;  //---------------- 50d89cb4, не проверить никак!

//r3=hash[23];
//if (r3<=0x37) ;    // 50d89bc0, продолжение кода, странное, пока не стал делать. Далее следует переход на 50d89db4

r3=hash[23];
chash[r3]=0x80;
r3++;
hash[23]=r3;
r2=r3;

if (r2 <=0x37) {
  r1=r0;
  do {
   r3=hash[23];
   chash[r3]=r1;
   r3++;
   hash[23]=r3;
   r2=r3;
  } while (r2 <= 0x37);
}

r2=hash[6];
r2>>=24;
phash[0x54]=r2&0xff;
r3=hash[6];
r3>>=16;
phash[0x55]=r3&0xff;
r2=hash[6];
r2>>=8;
phash[0x56]=r2&0xff;

r1=hash[6];
phash[0x57]=r1&0xff;

r3=hash[5];
r3>>=24;
phash[0x58]=r3&0xff;

r2=hash[5];
r2>>=16;
phash[0x59]=r2&0xff;

r3=hash[5];
r3>>=8;
phash[0x5a]=r3&0xff;

r2=hash[5];
phash[0x5b]=r2&0xff;

rehash5(hash);
hash[24]=1;

//*****************
bypass1:

switch(mode) {
  case 2:
    sprintf(varstr,"%u",hash[0]);
    break;

  case 5:
    sprintf(varstr,"%u",hash[1]);
    break;

  case 6:
    sprintf(varstr,"%u",hash[2]);
    break;
}
r0=strlen(varstr);

if (strlen(varstr)>7) {
  strncpy(resbuf,varstr,8);
  resbuf[8]=0;
  return;
}
strcpy(resbuf,varstr);

switch(mode) {
  case 2:
    sprintf(varstr,"%u",hash[1]);
    break;

  case 5:
    sprintf(varstr,"%u",hash[4]);
    break;

  case 6:
    sprintf(varstr,"%u",hash[3]);
    break;
}

strcat(resbuf,varstr);
resbuf[8]=0;

}


//
//  Калькулятор nlock-кодов для модемов Huawei с новым алгоритмом v201
//
//  Автор - forth32
//  2014 год
//
//  Ветка для вычисления кодов с индексом 7
//




void rehash7_2(int* hash,int* srcbuf) {

int r4,r5,r6,r7,r10,lr;
int inthash[30];

memcpy(inthash+1,srcbuf,64);  // переносим 16 слов в inthash

r4=hash[0];
r6=hash[1];
r7=hash[2];
r5=hash[3];
//--------- часть 1 --------

#define encpass1(hconst,nbyte,shift,rsum,ra,rb,rc) \
 rsum=ra+rotr32(rsum+inthash[nbyte/4]+((ra&rb)|(rc&(~ra)))+hconst,shift);

// r4=r6+rotr32(r4+inthash[4/4]+(r6&r7|(r5&(~r6)))+0xD76AA478,25);
// rs ra        rs          nb   ra rb  rc   ra               sh

encpass1(0xD76AA478,0x04,25,r4,r6,r7,r5)   // RAM:50D8A7F8
encpass1(0xE8C7B756,0x08,20,r5,r4,r6,r7)   // RAM:50D8A824
encpass1(0x242070DB,0x0c,15,r7,r5,r4,r6)   // RAM:50D8A84c
encpass1(0xC1BDCEEE,0x10,10,r6,r7,r5,r4)   // RAM:50D8A870

encpass1(0xF57C0FAF,0x14,25,r4,r6,r7,r5)   // RAM:50D8A898
encpass1(0x4787C62A,0x18,20,r5,r4,r6,r7)   // RAM:50D8A8c0
encpass1(0xA8304613,0x1c,15,r7,r5,r4,r6)   // RAM:50D8A8ec
encpass1(0xFD469501,0x20,10,r6,r7,r5,r4)   // RAM:50D8A914

encpass1(0x698098D8,0x24,25,r4,r6,r7,r5)   // RAM:50D8A938
encpass1(0x8B44F7AF,0x28,20,r5,r4,r6,r7)   // RAM:50D8A960
encpass1(0xffff5bb1,0x2c,15,r7,r5,r4,r6)   // RAM:50D8A988
encpass1(0x895CD7BE,0x30,10,r6,r7,r5,r4)   // RAM:50D8A9b0

encpass1(0x6B901122,0x34,25,r4,r6,r7,r5)   // RAM:50D8A9d8
encpass1(0xFD987193,0x38,20,r5,r4,r6,r7)   // RAM:50D8AA00
encpass1(0xA679438E,0x3c,15,r7,r5,r4,r6)   // RAM:50D8Aa28
encpass1(0x49B40821,0x40,10,r6,r7,r5,r4)   // RAM:50D8Aa54


//-------- алгоритм 2 --------------

//r4=r6+rotr32(r4+inthash[8/4]+(r6&r5|(r7&(~r5)))+0xF61E2562,27);
//rs ra        rs               ra rb  rc   rb

#define encpass2(hconst,nbyte,shift,rsum,ra,rb,rc) \
 rsum=ra+rotr32(rsum+inthash[nbyte/4]+((ra&rb)|(rc&(~rb)))+hconst,shift);

encpass2(0xF61E2562,0x08,27,r4,r6,r5,r7)   // 50d8aa78
encpass2(0xC040B340,0x1c,23,r5,r4,r7,r6)   // 50d8aa98
encpass2(0x265E5A51,0x30,18,r7,r5,r6,r4)   // 50d8aabc
encpass2(0xE9B6C7AA,0x04,12,r6,r7,r4,r5)   // 50d8aae4

encpass2(0xD62F105D,0x18,27,r4,r6,r5,r7)   // 50d8ab0c
encpass2(0x02441453,0x2c,23,r5,r4,r7,r6)   // 50d8ab38
encpass2(0xD8A1E681,0x40,18,r7,r5,r6,r4)   // 50d8ab5c
encpass2(0xE7D3FBC8,0x14,12,r6,r7,r4,r5)   // 50d8ab80

encpass2(0x21E1CDE6,0x28,27,r4,r6,r5,r7)   // 50d8aba8
encpass2(0xC33707D6,0x3c,23,r5,r4,r7,r6)   // 50d8abcc
encpass2(0xF4D50D87,0x10,18,r7,r5,r6,r4)   // 50d8abf4
encpass2(0x455A14ED,0x24,12,r6,r7,r4,r5)   // 50d8ac20

encpass2(0xA9E3E905,0x38,27,r4,r6,r5,r7)   // 50d8ac44
encpass2(0xFCEFA3F8,0x0c,23,r5,r4,r7,r6)   // 50d8ac6c
encpass2(0x676F02D9,0x20,18,r7,r5,r6,r4)   // 50d8ac94
encpass2(0x8D2A4C8A,0x34,12,r6,r7,r4,r5)   // 50d8acc0

// Алгоритм 3

//r4=r6+rotr32(r4+inthash[0x18/4]+(r5^r7^r6)+0xFFFA3942,28);
//rs  ra       rs                  rb rc ra

#define encpass3(hconst,nbyte,shift,rsum,ra,rb,rc) \
 rsum=ra+rotr32(rsum+inthash[nbyte/4]+(rb^rc^ra)+hconst,shift);

encpass3(0xFFFA3942,0x18,28,r4,r6,r5,r7)  //50D8ACDC
encpass3(0x8771F681,0x24,21,r5,r4,r7,r6)  //50D8Acfc
encpass3(0x6D9D6122,0x30,16,r7,r5,r6,r4)  //50D8Ad18
encpass3(0xFDE5380C,0x3c,9 ,r6,r7,r4,r5)  //50D8Ad34

encpass3(0xA4BEEA44,0x08,28,r4,r6,r5,r7)  //50D8Ad54
encpass3(0x4BDECFA9,0x14,21,r5,r4,r7,r6)  //50D8Ad70
encpass3(0xF6BB4B60,0x20,16,r7,r5,r6,r4)  //50D8AD94
encpass3(0xBEBFBC70,0x2c,9 ,r6,r7,r4,r5)  //50D8ADB0

encpass3(0x289B7EC6,0x38,28,r4,r6,r5,r7)  //50D8ADD0
encpass3(0xEAA127FA,0x04,21,r5,r4,r7,r6)  //50D8ADF0
encpass3(0xD4EF3085,0x10,16,r7,r5,r6,r4)  //50D8AE14
encpass3(0x04881D05,0x1c,9 ,r6,r7,r4,r5)  //50D8AE30

encpass3(0xD9D4D039,0x28,28,r4,r6,r5,r7)  //50D8AE50
encpass3(0xE6DB99E5,0x34,21,r5,r4,r7,r6)  //50D8AE70
encpass3(0x1FA27CF8,0x40,16,r7,r5,r6,r4)  //50D8AE94
encpass3(0xC4AC5665,0x0c,9 ,r6,r7,r4,r5)  //50D8AEB0

// Алгоритм 4
// смена регистра   r4 --> r0

// r0=r6+rotr32(r0+inthash[4/4]+((~r5|r6)^r7)+0xF4292244,26);
// rs ra        rs                 rb ra  rc

#define encpass4(hconst,nbyte,shift,rsum,ra,rb,rc) \
 rsum=ra+rotr32(rsum+inthash[nbyte/4]+((~rb|ra)^rc)+hconst,shift);

encpass4(0xF4292244,0x04,26,r4,r6,r5,r7)  // 50d8aed4
// смена регистра   r5 --> r1
encpass4(0x432AFF97,0x20,22,r5,r4,r7,r6)  // 50d8aef8
encpass4(0xAB9423A7,0x3c,17,r7,r5,r6,r4)  // 50d8af18
encpass4(0xFC93A039,0x18,11,r6,r7,r4,r5)  // 50d8af3c

encpass4(0x655B59C3,0x34,26,r4,r6,r5,r7)  // 50d8af60
encpass4(0x8F0CCC92,0x10,22,r5,r4,r7,r6)  // 50d8aF88
encpass4(0xFFEFF47D,0x2c,17,r7,r5,r6,r4)  // 50d8aFB4
encpass4(0x85845DD1,0x08,11,r6,r7,r4,r5)  // 50d8aFD4

encpass4(0x6FA87E4F,0x24,26,r4,r6,r5,r7)  // 50d8AFFC
encpass4(0xFE2CE6E0,0x40,22,r5,r4,r7,r6)  // 50d8B020
encpass4(0xA3014314,0x1c,17,r7,r5,r6,r4)  // 50d8B044
encpass4(0x4E0811A1,0x38,11,r6,r7,r4,r5)  // 50d8B068

encpass4(0xF7537E82,0x14,26,r4,r6,r5,r7)  // 50d8B088
encpass4(0xBD3AF235,0x30,22,r5,r4,r7,r6)  // 50d8B0A8
encpass4(0x2AD7D2BB,0x0c,17,r7,r5,r6,r4)  // 50d8B0CC

//  Смена накопительных регистров
//старый    новый
//  R4       R10
//  R5       LR
//  R6       R4
//  R7       R5
//
r10=r4;
lr=r5;
r4=r6;
r5=r7;

// Последнее слагаемое - прямо в массив хешей, разбираем отдельно
hash[0]+=r10;
hash[1]+=r5+rotr32(r4+(((~r10)|r5)^lr)+inthash[0x28/4]+0xEB86D391,11);
hash[2]+=r5;
hash[3]+=lr;

}

//====================================================================================================

//
//  Len = r2
//
void rehash7_1(int* hash,char* buf,int len) {

int r0,r1,r2,r3,r4,r5,r7,r12;

r2=hash[4];
r7=len;
r3=r2;
r12=r7<<3;
r3+=r12;
hash[4]=r3;
r0=r3;
r2>>=3;
r2&=0x3f;
if (r12>r0) hash[5]++;
r5=0x40-r2;
r3=hash[5];
r3=r3+(r7>>29);
hash[5]=r3;
if (r7 < r5) {
  // CC
  r5=0;
  r1=0;
  memcpy(hash+6+(r2>>2),buf,r7-r5);
  return;
}

//CS- 50D8b2ec

memcpy(hash+6+(r2>>2),buf,r5);
rehash7_2(hash,hash+6);
r3=r5+0x3f;
if (r7>r3) {
  r4=r5;  // buf* - индекс
  do {
    rehash7_2(hash,(int*)(buf+r4));
    r5+=0x40;
    r3=r5+0x3f;
    r4+=0x40;
  } while (r7>r3);
}
  // 50D8B338
  r1=r5;
  r2=0;
  memcpy(hash+6+(r2>>2),buf+r5,r7-r5);

}

//========================================================================================================
void encrypt_7(char* imei,char* resbuf,int version) {

int i;
int r0,r1,r2,r3,r4,r5,r6,r7,r8,r12,lr;
unsigned int u1;
long long cx;
long long  rr5,rr12;

char buf128[128];

// структура в памяти из кучи буферов, отдельные ссылки смотрят в этот общий массив

char fullbuf[100];

char* buf16=fullbuf+16;
char* buf17=fullbuf;
char* bufc=fullbuf+32;
char* buf30=fullbuf+44;
//char* cbufptr=fullbuf+52; - используется для адресации остальных смещений, нам не нужна

char cb_201[]={0xB, 0xD,0x11,0x13,0x17,0x1D,0x1F,0x25,0x29,0x2B,0x3B,0x61};
char cb_2[]={1,   1,   2,   3,   5,   8,   0xD,0x15,0x22,0x37,0x59,0x90};
char* cb;

char c;

char pattern80[64];

int hash[30];  // на самом деле 22
int hsum;


if (version == 201) cb=cb_201;
else                cb=cb_2;
memset(resbuf,0,9);
memset(buf128,0,128);
memset(buf16,0,16);
memcpy(bufc,cb,12);
memset(pattern80,0,64);
pattern80[0]=0x80;

// i==r1

for(i=0;i<15;i++) {
  cx=(long long)i*(long long)0x55555556;
  r2=(cx>>32)-(i>>31);
  r3=(r2<<1)+r2;
  u1=i-r3;
  r2=(int)imei[i];
  r3=r2;
  switch(u1) {

   case 0: {  //50d8b400
    r3<<=6;
    r3|=((int)r2)>>2;
    break;
  }

   case 1: {
    r3<<=5;
    r3|=((int)r2)>>3;
    break;
  }

   default: {
    r3>>=4;
    r3|=((int)r2)<<4;
    break;
   }


  }
  r3&=0xff;
  buf128[i]=r3;
}

// 50D8B48C

hsum=0;

for (i=0;i<7;i++) hsum+=((unsigned int)buf128[14-i]&0xff)+(((unsigned int)buf128[i]&0xff)<<8);
hsum+=(buf128[8]&0xff);
r8=0;

for(i=15;i<0x80;i++) {
   r6=i;
   r3=i>>31;
   lr=0x2AAAAAAB;
   cx=(long long)lr*(long long)i;
   r1=cx>>32;
   cx=(long long)lr*(long long)r8;
   lr=cx>>32;
   r0=r8>>31;
   r2=r0;
   r5=(r1>>1)-r3;
   r12=r5<<4;
   r0=(lr>>1)-r0;
   r2=(lr>>1)-r2;
   r1=r0<<4;
   r12=r12-(r5<<2);
   r3=r2<<4;
   lr=r6-r12;
   r1=r1-(r0<<2);
   r7=r5+lr;
   r3=r3-(r2<<2);
   r1=r8-r1;
   r2=r5+r1;
   r3=r8-r3;
   r12=r12-0x18;
   if (r7 > 0xb) r7=r7-0xc;
   r3+=r5;
   if (r5>1) r3=r2+r12;
//RAM:50D8B5E8
   r0=hsum;
   r1=r6;
   if (r8 == 0) {
     r4=buf128[r3];
     r0=r0%r1;
     r1=bufc[r7];
     r4=r4&r1;
     r12=buf128[r0];
     r3=buf128[r0+1];
     r4|=r12;
   }
   else {
   //RAM:50D8B5F8
     r1=r6;
     r0=hsum;
     r4=buf128[r3];
     r0=r0%r1;
     r1=r8;
     r5=buf128[r0];
     r0=hsum;
     r0=r0%r1;
     r3=buf128[r0];
     r2=bufc[r7];
     r4=r4&r2;
     r4=r4|r5;
   }
// RAM:50D8B54C
   r3=~r3;
   r3|=r4;
   r3&=0xff;
   buf128[i]=r3;
   r8++;
}

// RAM:50D8B678

r8=0;

for(i=0;i<7;i++) {
  r1=imei[i];
  r2=imei[i+1];
  r2=r2|(r1<<8);
  r8+=r2;
}

r8+=imei[14];

memset(hash,0,120);

hash[5]=0;
hash[4]=0;
hash[0]=0x67452301;
hash[1]=0xEFCDAB89;
hash[2]=0x98BADCFE;
hash[3]=0x10325476;

//50D8B710
rehash7_1(hash,buf128,0x80);

memcpy(buf30,hash+4,8);
r3=hash[4];
r3>>=3;
r2=r3&0x3f;
if (r2 <= 0x37) r2=0x38-r2;
else            r2=0x78-r2;
rehash7_1(hash,pattern80,r2);

//50D8B748

rehash7_1(hash,buf30,8);  //50D8B754
//50D8B758

memcpy(buf16,hash,16);
r5=r8&3;  // 50D8B768

memcpy(buf17,buf16,16);
r12=*((int*)(buf17+(r5<<2)));
r4=0;
r0=0;


// Этап 1 - пытаемся наскрести в buf16 готовых ascii-цифр

for(r2=0;r2<=15;r2++) {
 c=buf16[r2];
 if ((c>0x2f)&&(c <= 0x39)) resbuf[r4++]=c;
 if (r4>7) break;   //50D8B7D4
}
if (r4 > 7) goto bdone; // набрали все 8 цифр - нам хватит

// Этап 2 - попытка полцчить недостающие цифры.

//printf("\n 1cycle: imei=%s r4=%i",imei,r4);

r6=0;
u1=(3-r5)<<2;    // индекс в buf17
r5=0xcccccccd;
rr5=r5&0xffffffff;

do {
  r0=r6^1;
  rr12=r12&0xffffffff;
  cx=rr12*rr5;
  r2=((cx>>32)&0xffffffff)>>3;
  if (r2 == 0) {
    r0&=1;
  }
  else r0=0;
  r3=r2<<1;
  r1=r2<<3;
  r3=r3+r1;
  r3=r12-r3;
  r3+=0x30;
  r12=r2;
  r3&=0xff;
  resbuf[r4++]=r3;
  if (r0 != 0) {
    r12=*((int*)(buf17+u1));
    r6=1;
  }
} while ((r4<8) && (r12 != 0));

//50D8B888

if (r4>7) goto bdone;  // наконец наскребли нужное количество цифр
r12=r5;
r0=0;
//printf("\n 2cycle: imei=%s r4=%i",imei,r4);

loc_50D8B8DC:
 if ((r4>7)&&(resbuf[0] != 0)) goto bdone;

//50D8B8F4
  r3=buf16[r0]&0xff;
  rr12=r12&0xffffff;
  rr5=r3&0xffffff;
  cx=rr12*r5;
  r2=(cx>>32)&0xffffffff;
  r2>>=3;
  r1=r2<<3;
  r2<<=1;
  r2+=r1;
  r3=r3-r2;
  r3+='0';
  r0++;
if (r4 == 8) resbuf[0]=r3&0xff;
else resbuf[r4++]=r3&0xff;

if (r0 == 16) goto bdone;
  goto loc_50D8B8DC;
//50D8B89C   r4!= 8

//  --- Все, буфер готов ! -------
bdone:
  if(resbuf[0] == '0') {
    r2=0;
    if (r8 != 0) r2=1;
    r3=(buf16[r2]&7)+'1';
    resbuf[0]=r3;
  }
  resbuf[8]=0;


}

//
//  Калькулятор nlock-кодов для модемов Huawei
//
//  Автор - forth32
//  2014 год
//
//  Ветка для вычисления флеш-кодов
//
#include <openssl/md5.h>


void encrypt_v1(char* imei, char* resbuf,char* hstr) {

unsigned char xbytes[17];
char ybytes[100];
char hash[100];
unsigned int rh[30];
unsigned char res[4];

int i;

memset(xbytes,0,17);
MD5((unsigned char*)hstr,strlen(hstr),xbytes);

//printf("\n xbytes (1) =\n");
//for(i=0;i<16;i++) printf(" %02x",xbytes[i]&0xff);
for(i=0;i<16;i++) sprintf(ybytes+(i*2),"%02x",xbytes[i]&0xff);

strcpy(hash,imei);
strncat(hash,ybytes+8,16);
hash[31]=0;
MD5((unsigned char*)hash,31,xbytes);

for (i=0;i<16;i++) rh[i]=xbytes[i]&0xff;
for(i=0;i<4;i++) res[3-i]=rh[i]^rh[i+4]^rh[i+8]^rh[i+12];
i=*((unsigned int*)&res);
i|=0x2000000;
i&=0x3FFFFFF;
sprintf(resbuf,"%i",i);
}


//
//  Калькулятор NCK-кодов для модемов Huawei с новым алгоритмом v201
//
//  Автор - forth32
//  2014 год
//



// Вычисление индекса обработчика по хешу IMEI, и вызов его
//
// На входе - буфер с imei и буфер для записи результата
// Возвращает индекс обработчика
//************************************************************************
// Вычисление индекса обработчика по хешу IMEI для алгоритма v201
//
// version=2 для v2 и 201 для v201

int proc_index(char* imeibuf,int version) {

int i;
int csum=0; // хеш IMEI
int c1,ch;
long long cx;
int index;


for(i=0;i<strlen(imeibuf);i++)  {
   ch=imeibuf[i];
   if (version==201) csum+=((ch+i+1)*ch)*(ch+0x139);
   else              csum+=((ch+i+1)*(i+1));
}

cx=((long long)-0x6db6db6d*(long long)csum)>>32;
c1=((cx+csum)>>2)-(csum>>31);
index=csum-((c1<<3)-c1);
return index;
}


//**************************
// циклический сдвиг вправо
//**************************

unsigned int  rotr32(unsigned int val, int n) {
   return ((val>>n)&0x7fffffff)|(val<<(32-n));
}

//****************************************
// Вычисление кода по алгоритму 201
//****************************************

int calc201(char* imeibuf, char* resbuf) {


int index;

//
// Вычисляем хеш IMEI
//
index=proc_index(imeibuf,201);

switch (index) {
  case 0:
    encrypt_1(imeibuf,resbuf,201);
    break;
  case 1:
    encrypt_2(imeibuf,resbuf,201);
    break;
  case 2:
    encrypt_3(imeibuf,resbuf,201);
    break;
  case 3:
    encrypt_4(imeibuf,resbuf,201);
    break;
  case 4:
    encrypt_6(imeibuf,resbuf,5);
    break;
  case 5:
    encrypt_6(imeibuf,resbuf,6);
    break;
  case 6:
    encrypt_7(imeibuf,resbuf,201);
    break;
  default:
    strcpy (resbuf," - N/A -");
    break;
}

return index;
}


//****************************************
// Вычисление кода по алгоритму 2
//****************************************

int calc2(char* imeibuf, char* resbuf) {


int index;

//
// Вычисляем хеш IMEI
//
index=proc_index(imeibuf,2);

switch (index) {
  case 0:
    encrypt_1(imeibuf,resbuf,2);
    break;
  case 1:
    encrypt_2(imeibuf,resbuf,2);
    break;
  case 2:
    encrypt_3(imeibuf,resbuf,2);
    break;
  case 3:
    encrypt_4(imeibuf,resbuf,2);
    break;
  case 4:
    encrypt_5_v2(imeibuf,resbuf);
    break;
  case 5:
    encrypt_6(imeibuf,resbuf,2);
    break;
  case 6:
    encrypt_7(imeibuf,resbuf,2);
    break;
  default:
    strcpy (resbuf," - N/A -");
    break;
}



return index;
}

























