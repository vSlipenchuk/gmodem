#include "coders.h"

int gsm7_code(unsigned int offset, int max_out, unsigned char *input,
		   unsigned char *output, unsigned int *get_len, int maxLen); // coded below in this file
int gsm_getaddr(char *dst, unsigned char *src, int len);
unsigned char hex_ch(int n);

int sms_put_addr2(char *dst, char *addr) {
int i,l,hx;
if (addr[0]=='+') { dst[1]=0x91; addr++;} // International
          else    { dst[1]=0x81;} // Натионал
l = strlen(addr);
if (l>24) return -1; // Invalid length...
hx = l/2; if (l%2) hx++;
dst[0]= l; // Длина, которая должна получиться (без тона??)
dst+=2;
for(i=0;i<hx;i++) {
   unsigned char c1,c2;
   c1 = addr[0]-'0'; c2=addr[1];
   if (c2==0) c2=0xF ; else c2-='0';  // Code last symb...
   *dst= (c2<<4)|c1;
   addr+=2; dst++;
  }
return hx+2; // Len + TON + ADDR
}

int sms_submit(t_sms *sms,int rejDup, int repRequest,
  int mRef, uchar *phone, int pid,int dcs, int vp,
      int chainsTR, uchar *udh, uchar *text, int len) { // StartDecode me ...
unsigned char *s = sms->data; // Сюда кодируем
int udhlen,payload,smspayload=140,coded,l;
char buf[140]; // temp buffer
if (len<0) len = strlen(text);
sms->dcs = dcs; sms->szText = text; sms->Len = len;
udhlen=udh?udh[0]+1:0;  // Сначала определяем длину пользовательского хедера...
// Теперь - нужно понять какой пейлоад образовался
payload = smspayload; if (udhlen) payload -= udhlen; // udhlen еще будет содержать собственную длину - 1 байт
// Теперь нужно понять - вмещаемся мы своим объемом в оставшийся пейлоад или нет. Если нет - нужно добавлять хедер по чейнам???
if (payload<=0) { // no more space
    sprintf(sms->error,"sms_coder: too small space for payload %d (udhlen=%d,all=%d)",payload,udhlen,smspayload);
    return -3;
    }
sms->offset=0; sms->total = 1; sms->segment = 0;  sms->chains = 0; // no chains yet
sms->sm_udhl=0;
//printf(">>> udl_before_chainging %d\n",udhlen);
switch (dcs & (4+8)) { // Алфавит 0=>дефаулт,4=>8бит,8=>UCS2,12=>Reserved(=8bit)
case 0: // GSM Default
 // Сначала пытаемся закодировать то, что есть с использованием текущего хедера (без чейнов)
  if (udhlen) sms->offset = (7-((udhlen)%7))%7; // Offset of 7bit coding
//  int l=0;
  if (len) l=gsm7_code(sms->offset,payload,text,buf,&coded,len); // CodeMe
  //printf("gsm7_code: bytes=%d, codedChars=%d for len=%d\n",l,coded,len);
  if (coded<len) { // Не все сообщение убралось. жопа?
      if (chainsTR) { // Attach a new header to UDH
         sms->chains=chainsTR; // Yes
         if (!udhlen) { udhlen++; payload--;} // Add Header length
         udhlen+=5; // Simple Chain Header 00-03- TR NN AL
         payload-=5; // Remove
         if (payload<2) { sprintf(sms->error,"sms_coder: too small space for payload %d (udhlen=%d,all=%d)",payload,udhlen,smspayload);    return -3; }
         };
      // calc new total anyway
      int l = len; uchar *t = text;
      sms->total = 0; sms->offset = (7-((udhlen)%7))%7; // Вычисляем заново - с новым оффсетом
       while(l>0) { // Eval it now...
             //int sz =
             gsm7_code(sms->offset,payload,t,buf,&coded,l);
             //printf("szBytes=%d codedChars=%d for payload %d\n",sz,coded,payload);
             t+=coded; l-=coded; sms->total++;
             }
      }
    if (udhlen) sms->sm_udhl = ((udhlen) * 8 + 6) / 7; // Длина хедера в септетах!!!
  break;
case 8: // UCS2 - каждая буква = 2 байта, границы сообщений буква пересекать не должна!!!
  //printf("UCS2 code \n");
  if (payload<len && chainsTR) { // Добавляем в хедер
      if (!udhlen) { udhlen++; payload--;} // Add Header length
      udhlen+=5; payload-=5; sms->chains=chainsTR; // Simple Chain Header 00-03- TR NN AL
      }
  if (payload%2) payload--; // Только целые буквы -)))
  if (payload<2) { sprintf(sms->error,"sms_coder: too small space for payload %d (udhlen=%d,all=%d)",payload,udhlen,smspayload);  return -3; }
  sms->total = (len)/payload; if ((len)%payload) sms->total++; // Готово - определили размерчик
  sms->sm_udhl = udhlen;
  break;
default: // 8bit
  if (payload<len && chainsTR) { // Добавляем в хедер
      if (!udhlen) { udhlen++; payload--;} // Add Header length
      udhlen+=5; payload-=5; sms->chains=chainsTR; // Simple Chain Header 00-03- TR NN AL
      }
  if (payload<2) { sprintf(sms->error,"sms_coder: too small space for payload %d (udhlen=%d,all=%d)",payload,udhlen,smspayload);  return -3; }
  sms->total = len/payload; if (len%payload) sms->total++; // Готово - определили размерчик
  sms->sm_udhl = udhlen;
  break;
}
sms->payload_len = payload; // Сколько максимально можно засчитать?
s = sms->data; // Кодировать будем во внутреннее поле - text?
*s = 1 /* SUBMIT*/ | ( 0<<2) /*RD*/ | (0<<3) /*VP*/ | (0<<5) /*RP отчет?*/ |
       ( (udhlen?1:0)<<6) /*UDHI*/ | (0<<7) /*RS*/;
//if (rejDup) *s|=(1<<2);
if (repRequest) *s |= (1<<5) ; /*RS - отчет или RS?? */
len = 1; s++;
*s=mRef; s++; len++;  /*MR*/ // Потом -- MessageReference -- 1 octet
l = sms_put_addr2(s,phone); s+=l; len+=l; // Put address here...
if (l<0) {  sprintf(sms->error,"fail code dest address");  return -1;}  // Invalid address
*s=pid; s++; len++; // ProtocolIdentifier
*s = dcs; s++; len++; // DataCodingSheme
//printf("SetDataCode=%d udhlen(withlen)=%d\n",dcs,udhlen);
//*s=170; s++; len++; // OneByte Validity Period
sms->slen = s; s++; len++; sms->udh=0; // Тут будет лежать вычисляемая sm_length
if (udhlen) { // Если есть хедер - нужно его закодировать
    sms->udh = s; // Header+with length?
    *s=udhlen-1; s++; len++; // Собственно - длина буфера (без учета собственно этого поля)
    if (sms->chains) { // Автодобавляем чейн-буфер
        //printf("Add a chains total=%d\n",sms->total);
        s[0]=00; s[1]=03; s[2]=sms->chains; s[3]=sms->total; s[4]=sms->segment; // Это собственно весь мой хедер
        s+=5; len+=5;
        }
    if (udh) { // Добавляем то, что отдал юзер
        int l = udh[0];
        //printf("Add UserHeader %d len, totudhlen=%d\n",l,udhlen);
        memcpy(s,udh+1,l); s+=l; len+=l;
        }
    }
sms->smHead=sms->smLen = len; // Откуда начинаются реальные данные
return 1; // ok ?
}

int sms_fetch(t_sms *sms) { // Делает один фетч за другим - записывая данные...
int l,coded;
sms->segment++; sms->data[1]++; // Add a msgRef???
if (sms->segment<=0 || sms->segment>sms->total) return 0; // No More
if (sms->chains)  sms->udh[5]=sms->segment; // Устанавливаем в чейнах
sms->smLen = 0; // NoMore???
switch(sms->dcs&(8+4)) { // По разному это очень делается
case 0: // Однако - надо кодировать???
    l = 0;
    //if (sms->Len>0) ZUZUKA
    l = gsm7_code(sms->offset,sms->payload_len,sms->szText,sms->data+sms->smHead,&coded,sms->Len); // CodeMe
    if (l<0) {
        printf("GSM CODE ERROR error\n");
        return l; // SomeError ???
        }
    printf("Coded %d bytes of %d, payload:%d\n",coded,sms->Len,sms->payload_len);
    sms->szText+=coded; sms->Len-=coded; // Удаляем из буфера
    *(sms->slen) = sms->sm_udhl + coded; // Длина в септетах с учетом хедера!!!
    //printf("sms->slen=%d, sms->sm_udhl=%d\n",*sms->slen,sms->sm_udhl);
    sms->smLen = sms->smHead+l; // Полная длина СМС в байтах на отправку
    //printf("smLen:%d\n",sms->smLen);
    return sms->smLen;
case 8: // UNICODE
default: // Binary
    l = sms->payload_len; // Сколько букв забрать
    if (sms->Len<l) l = sms->Len; // Если только остаток -)))
    if (l>0) {
      memcpy(sms->data+sms->smHead, sms->szText,l); // Возврат - количество байтов
      *(sms->slen)=l + sms->sm_udhl; // Длина хедера должна быть в байтах!!!
      sms->smLen = sms->smHead + l; // Полная длина сообщения
      sms->szText+=l; sms->Len-=l;
      }
    break;
}
return sms->smLen;
}

// -- sms binary decoders

int gsm_gettime(char *dst,unsigned char *src, int len) {
int i;
if (len<7) return -2; // Недостаточная длина ...
for(i=0;i<6;i++) { // Печатаем адрес ..
  char a[4];
  sprintf(a,"%02x",*src);
  dst[0]=a[1]; dst[1]=a[0];
  src++; dst+=2;
  if (i==2) { *dst=' '; dst++; } // Пробел между годом и месяцем...
  }
// Последнее - 7 поле обозначает временное смещение, его игнорируем ...
*dst=0;
return 7;
}





int sms_get_addr(unsigned char *addr,unsigned char *src,int len) {
int l,i;
//printf("len=%d\n",len);
if (len<2) return -1; // Минимальный адрес = длина+тип
if (memcmp(src,"00",2)==0) {
    strcpy(addr,"null");
    return 2;
    }
l = *src; src++; len--;
//printf("l=%d\n",l);
if (l>12) return -2; // Макс. адрес 20 цифр (плюс префикс "+") =  GSM0411 & GSM 0902
if ( *src & (1<<4) ) { *addr='+'; addr++;} // Если интернатионал ...
src++; len --;
//printf("len=%d l=%d\n",len,l);
if (len<l) return -3; // Есть информация о длине, а буффер уже кончился ...
for(i=0;i<l-1;i++,addr+=2,src++) { // Разбираем адрес
   unsigned char b=*src;
   addr[0]=hex_ch(b&15); addr[1]=hex_ch(b>>4);
   }
if (i>0 && addr[-1]=='f') addr[-1]=0; // Не нужна послед... 'f'
return l+1; // Длина, забранная из буфера ...
}

// То-же, только длина трактуется по-другому (выходная)...
int sms_get_addr2(unsigned char *addr,unsigned char *src,int len) {
int l,i;
if (len<2) return -1; // Минимальный адрес = длина+тип
l = *src; src++; len--;
if (l>24) return -2; // Макс. адрес 20 цифр (плюс префикс "+") =  GSM0411 & GSM 0902
if (l==0) { *addr=0; return 2;} // Нулевая длина адреса...
//printf("\n---ADDR TON=%d ---- \n",*src);
l = 2+(l -1) / 2; // Выходная длина
if ( *src & (1<<4) ) { *addr='+'; addr++;} // Если интернатионал ...
src++; len --;
if (len<l) return -3; // Есть информация о длине, а буффер уже кончился ...
for(i=0;i<l-1;i++,addr+=2,src++) { // Разбираем адрес
   unsigned char b=*src;
   addr[0]=hex_ch(b&15); addr[1]=hex_ch(b>>4);
   }
*addr=0;
if (i>0 && addr[-1]=='f') addr[-1]=0; // Не нужна послед... 'f'
return l+1; // Длина, забранная из буфера ...
}

// Перекодируем юзерскую дату, начало - UDL !!!
int sms_decode_ud(t_sms *sm,unsigned char *s,int len) { // S - starts with udl = userDataLength ???
int l;

//hex_dump("DECODE UD",s,len);

sm->udl = l = *s; // UserDataLength...
int dcs = sm->dcs & 0xF ; // Только младшие 4 бита - это алфавит!!1
//printf("DECODE DCS=%d\n",dcs);
//printf("UserDataLength=%d, rest=%d\n",sm->udl,len);
s++; len--; // remove user data length, now starts with userdata or userdataheader???
//printf("Rest of UD len=%d udhi=%d\n",len,sm->udhi);
sm->ud = s; sm->udh=0;
if (sm->udhi) {
   int l,blen,slen;
  //hex_dump("DECODE_UDHI from:",s,len);
  //printf("UserData contains header , len=%d >",l);
  l = blen = slen =  s[0]+1; // Извлекаем из хедера длину, добавляем саму "длину длины хедера"
  sm->udh = s; // Устанавливаем на хедер после длины
  sm->offset = 0;
  if ((dcs&(8+4))==0) { // 7bit - выравниваем байт-длину на границу 7
         slen = (blen * 8 + 6) / 7; // Длина хедера в септетах
         sm->offset = (7-(blen%7))%7; // Смещение для вычисления 7bit?
         //printf("slen=%d sm->udl(in slen)=%d\n",slen,sm->udl);
         //if (sm->offset) slen++; // extra byte for shift 0x00 of message
         }
   //printf("!!!! total udl=%d offset=%d udhi[0]=%d slen=%d\n",sm->udl,sm->offset,s[0]+1,slen);
   sm->ud = sm->udh+blen; len-=blen; s+=blen; // Сдвигаемся на реальное количество байт
   sm->udl-=slen; // Удаляем количество септетов или байтов - по обстановке
   /*printf("blen=%d sm->udl(without header)=%d slen=%d\n",blen,sm->udl,slen);
   hex_dump("USER_MESSGAE",sm->ud,len);
   char out[1024];
         memset(out,0,1024);
         //int char_7bit_unpack(unsigned int offset, unsigned int in_length, unsigned int out_length,
		   //  unsigned char *input, unsigned char *output);
		 i = char_7bit_unpack(sm->offset, len, 1024, sm->ud, out);
		 printf("Rest Of UDL=%d\n",sm->udl);
         //i=gsm_7bit(out,1024,sm->udh-sm->offset,len); // JustDo It!!!
         printf(">>>>>> BlindOutText=LEN:%d:<%s>\n",i,out);
         hex_dump("BlindDump",out,i);
*/
  // Ну вот - с длиной разобрались - вычисляем UserData

  //for(i=0;i<len;i++) printf("%02x ",s[i]);
  //printf("\n");
  } ;
if (sm->udh) {// have -> UserDataHeader!!!
   uchar *d = sm->udh+1; int i,ul =sm->udh[0];
       for(i=0,d=sm->udh+1;i<ul;) { // check Total Here...
        uchar udh,udl,*udd;
        udh = d[0]; udl = d[1]; udd = d+2; // header here
        //printf("UDH[%d,%d]\n",udh,udl);
        if (udh==0x00 && udl==0x03) { // chain, ver 1
             sm->chains = udd[0];
             sm->total = udd[1];
             sm->segment = udd[2];
             }
        else if (udh==0x08 && udl == 0x04) { // SAR, 16 bit trans
             sm->chains = (udd[0])*256 + udd[1];
             sm->total = udd[2];
             sm->segment = udd[3];
             }
        udl+=2; // udh+udl
        i+=udl; d+=udl;
       }
   //printf("UDHI here!!!\n");
   }
//printf("UDL=%d,dcs=%d\n",sm->udl,dcs);
//hex_dump("USER_DATA!!!!",sm->ud,len);
switch(dcs&(8+4)) {
case 8:  { // UCS2, // Unicode ...
         int i; //unsigned char buf2[100];
         //printf("Unicode, udl=%d rest=%d\n",sm->udl,len);
         //i=gsm2win(sm->text,sm->ud,sm->udl);
         hexdump("sms_decode_ud:",sm->ud,sm->udl);
         len-=i*2;
         //printf("???\n");
         //if (len!=0) {sprintf(sm->error,"error: 16bit rest bytes %d",len); return -1;}
         //printf("UNITEXT:%s\n",sm->text);
         return 1; // Ok, done
         //printf("UniText2:%s\n",text);
         }
case 0:  { // def alpha
         int i;
          //i = char_7bit_unpack(sm->offset, len, sizeof(sm->text), sm->ud, sm->text); // not stopped by outbuf...
          //printf("
          i = gsm7_decode(sm->text,sm->ud,len,sm->offset);
          if (i==sm->udl+1) i--; // Специальный случай - когда есть "лишний" последний септет
		  //printf("GSM7_decode=%d sm->udl=%d len=%d\n",i,sm->udl,len); //         i=gsm_7bit(sm->text,sm->udl,sm->ud,len);
          //printf("TXT:<%s>,i=%d\n",sm->text,i);
          //i-=sm->udl;

          if (i!=sm->udl) {sprintf(sm->error,"error: 7bit (DEC:%d,SM->udl:%d,rest:%d,in_len:%d) offset:%d,text:'%s'",
             i,sm->udl,i-sm->udl,len,sm->offset,sm->text); return -1;}
          return 1; // Ok, done
           }
default:  { // Other -  8bit message.. think its a win1251...
          int i;

          for(i=0;i<len;i++) sprintf(sm->text+2*i,"%02x",sm->ud[i]);
          //printf("8bitlen=%d strlen()=%d\n",len,strlen(sm->text));
            //memcpy(sm->text,sm->ud,sm->udl);
           // sm->text[sm->udl]=0;
            len-=sm->udl;
            if (len!=0) {sprintf(sm->error,"error: 8bit rest bytes %d",len); return -1;}
            return 1; // Ok, done
            }
}
return 1;
}




int sms_decode_submit(t_sms *sm,unsigned char *s,int len) {
//unsigned char done[100];
int l,code;
code = *s;  // В первом байте идет  MTI(2),RD,VPF,SRR,UDHI,RP
sm->mti = (code & 3); // Message Type Info - первые два бита ...
sm->rd = (code>>2) & 1; // RejectDuplicates - (если в SMSC есть сообще с DA и MR) -
sm->vpf = (code>>3) & 3; // Два бита - коды валидити периода ...
sm->rp = (code>>5) & 1; // Бит запроса на ReplayPath
sm->udhi = (code>>6)&1; // Наличие юзер дата хедера
sm->sr = (code>>7)&1; // Запрос на статус репорт...
s++; len--;
// Потом -- MessageReference -- 1 octet
sm->mr = *s; s++; len--;
l = sms_get_addr2(sm->da,s,len);
if (l<0) {
  sprintf(sm->error,"submit - unable extract dst_addr");
  return -1;
  }
s+=l; len-=l;
sm->pid = s[0]; // ProtocolIdentifier
sm->dcs = s[1]; // DataCodingSheme
s+=2; len-=2;
if (sm->vpf) { // Если есть валидити период...
   if (sm->vpf==2) { sprintf(sm->vp,".%d",*s); s++; len--;} // Один октет...
    else if (sm->vpf==3) { // Семь октетов...
         sms_oct(sm->vp,s,7); s+=7; len-=7;
         }
   else { sprintf(sm->error,"submit - invalid vpf=%d",sm->vpf); return -1;}
   }
//return 1; // NO TEXT
return sms_decode_ud(sm,s,len);
}

int sms_decode_deliver(t_sms *sm,unsigned char *s,int len) { // Отправленное сообщение ...
//int i ,
int  l;
// GSM 03.40 -  В первом байте должны идти TP-MTI, TP-MMS, TP-SRI, TP-UDHI, TP-RP
l = *s;
sm->mti = l&3;   // 2 bit, Мессадж тайп, 00 = деливер или деливер репорт
sm->mms = (l>>2)&1; // Море ту сенд - бит говорит что есть еще сообщения...
sm->rp = (l>>3)&1;   // Реплай пась - есть или нет...
sm->udhi = (l>>6)&1; // Пятый бит - указывает есть ли юзер дата хедер...
sm->sr = (l>>7)&1;  // Это бит статус-репорта (индикатор)...
s++; len--;
// Теперь идет - TP/DA (max - 12) - адрес дестинатиона ...
l = gsm_getaddr(sm->da,s,len);
if (l<0) {
  sprintf(sm->error,"delivr: error get dst address code=%d!!!\n",l);
  return -1;
  }
//printf("TP_DestAdress=%s\n",dst);
s+=l; len-=l;
// Теперь PID & DSC
if (l<2) {
  sprintf(sm->error,"delivr: Rest len too short - expect PID & DCS (2 bytes), rest=%d!!!\n",l);
  return -1;
  }
sm->pid = s[0]; sm->dcs=s[1];
//printf("ProtocolID=%d DataCodingScheme=%d\n",tp_pi,tp_cs);
s+=2; len-=2;
// Теперь -  TP SC TS - ServiceCentreTimeStamp...
l = gsm_gettime(sm->vp,s,len);
if (l<0) {
  sprintf(sm->error,"Error get send time code=%d",l);
  return -1;
  }
//printf("Sent '%s'\n",sent);
s+=l; len-=l;
// Потом TP-UDL (1 байт) - Ну  и наконец UD - UserData !!!
//printf("UserDataHere - %d octects: ",len);
//for(i=0;i<len;i++) printf("%02x ",s[i]);
//printf("\n");
return sms_decode_ud(sm,s,len);
}

int sms_decode_status(t_sms *sm,unsigned char *s,int len) { // Отправленное сообщение ...
//int i ,
 int l;
// GSM 03.40 -  В первом байте должны идти TP-MTI, TP-MMS, TP-SRI, TP-UDHI, TP-RP
l = *s;
sm->mti = l&3;   // 2 bit, Мессадж тайп, 00 = деливер или деливер репорт
sm->mms = (l>>2)&1; // Море ту сенд - бит говорит что есть еще сообщения...
 //sm->rp = (l>>3)&1;   // Реплай пась - есть или нет...
 //sm->udhi = (l>>6)&1; // Пятый бит - указывает есть ли юзер дата хедер...
 //sm->sr = (l>>7)&1;  // Это бит статус-репорта (индикатор)...
s++; len--;
sm->mr = *s; // MessageReference...
s++; len--;
// Теперь идет - TP/RA (max - 12) - адрес ресиптиона ...
l = gsm_getaddr(sm->da,s,len);
if (l<0) {
  sprintf(sm->error,"status: error get rcpt address code=%d!!!\n",l);
  return -1;
  }
//printf("TP_DestAdress=%s\n",dst);
s+=l; len-=l;
// Теперь - тайм стамп - приемки сообщения
l = gsm_gettime(sm->vp,s,len);
if (l<0) {
  sprintf(sm->error,"status: error get  SCTS time code=%d",l);
  return -1;
  }
s+=l; len-=l;
// Теперь - дисчаржд тайм ...
l = gsm_gettime(sm->dtime,s,len);
if (l<0) {
  sprintf(sm->error,"status: error get DT time code=%d",l);
  return -1;
  }
s+=l; len-=l;
if (len<1) {
  sprintf(sm->error,"status: unable get status field restlen=%d",len);
  return -1;
  }
sm->status = *s;
{
//char *sname[]={"delivered"}; char buf[10];
//sprintf(buf,"%s",sm->status);
sprintf(sm->text,"<status:%d, recv='%s',dis='%s',ref=%d>",
   sm->status,sm->vp,sm->dtime,sm->mr);
}
return 1; // Ok ...
}

// Декодирует в зависимости от типа MessageTypeIndicator...
int sms_decode(t_sms *sm,unsigned char *s,int len) {
int code;
sm->mti =(*s)&3;
//printf("decode MTI=%d\n",sm->mti);
switch(sm->mti) {
case SMS_DELIVER: code=sms_decode_deliver(sm,s,len); break;
case SMS_SUBMIT:  code=sms_decode_submit(sm,s,len); break; // Decode This...
case SMS_REPORT:  code=sms_decode_status(sm,s,len); break;
default:
  sprintf(sm->error,"sms_decode: mti=%d not supported",sm->mti);
  code=-1;
}
return code;;
}



// --------------------- gsm7bit coders below --------------------



#define GN_CHAR_ALPHABET_SIZE 128
#define GN_CHAR_ESCAPE 0x1b

unsigned char hex_ch(int n) {
if (n<=0) return '0';
if (n<10)  return n+48;
if (n<16) return  n-10+'a';
return 'n'; // Invalid...
}


int sms_oct(unsigned char *dst,unsigned char *src,int len) { // Вытаскивает октеты ...
int i;
for(i=0;i<len;i++,dst+=2,src++) { // Разбираем адрес
   unsigned char b=*src;
   dst[0]=hex_ch(b&15); dst[1]=hex_ch(b>>4);
   }
*dst=0;
return 1;
}




static unsigned char gsm_default_alphabet[GN_CHAR_ALPHABET_SIZE] = {

	/* ETSI GSM 03.38, version 6.0.1, section 6.2.1; Default alphabet */
	/* Characters in hex position 10, [12 to 1a] and 24 are not present on
	   latin1 charset, so we cannot reproduce on the screen, however they are
	   greek symbol not present even on my Nokia */

	'@',  0xa3, '$',  0xa5, 0xe8, 0xe9, 0xf9, 0xec,
	0xf2, 0xc7, '\n', 0xd8, 0xf8, '\r', 0xc5, 0xe5,
	'?',  '_',  '?',  '?',  '?',  '?',  '?',  '?',
	'?',  '?',  '?',  '?',  0xc6, 0xe6, 0xdf, 0xc9,
	' ',  '!',  '\"', '#',  0xa4,  '%',  '&',  '\'',
	'(',  ')',  '*',  '+',  ',',  '-',  '.',  '/',
	'0',  '1',  '2',  '3',  '4',  '5',  '6',  '7',
	'8',  '9',  ':',  ';',  '<',  '=',  '>',  '?',
	0xa1, 'A',  'B',  'C',  'D',  'E',  'F',  'G',
	'H',  'I',  'J',  'K',  'L',  'M',  'N',  'O',
	'P',  'Q',  'R',  'S',  'T',  'U',  'V',  'W',
	'X',  'Y',  'Z',  0xc4, 0xd6, 0xd1, 0xdc, 0xa7,
	0xbf, 'a',  'b',  'c',  'd',  'e',  'f',  'g',
	'h',  'i',  'j',  'k',  'l',  'm',  'n',  'o',
	'p',  'q',  'r',  's',  't',  'u',  'v',  'w',
	'x',  'y',  'z',  0xe4, 0xf6, 0xf1, 0xfc, 0xe0
};

static unsigned char gsm_reverse_default_alphabet[256];
static int reversed = 0;
//static char application_encoding[64] = "";

static void tbl_setup_reverse() {
	int i;

	if (reversed) return;
	memset(gsm_reverse_default_alphabet, 0x3f, 256);
	for (i = GN_CHAR_ALPHABET_SIZE - 1; i >= 0; i--)
		gsm_reverse_default_alphabet[ gsm_default_alphabet[i] ] = i;
	gsm_reverse_default_alphabet['?'] = 0x3f;
	reversed = 1;
}


unsigned char char_def_alphabet_encode(unsigned char value) {
	tbl_setup_reverse();
	return gsm_reverse_default_alphabet[value];
}

static unsigned char char_def_alphabet_ext_decode(unsigned char value)
{
    //printf("extDecode %d Esc=%d\n",value,0x1b);
	switch (value) {
	case 0x0a: return 0x0c; break; /* form feed */
	case 0x14: return '^';  break;
	case 0x28: return '{';  break;
	case 0x29: return '}';  break;
	case 0x2f: return '\\'; break;
	case 0x3c: return '[';  break;
	case 0x3d: return '~';  break;
	case 0x3e: return ']';  break;
	case 0x40: return '|';  break;
	case 0x65: return 0xa4; break; /* euro */
	default: return 0;    break; /* invalid character */
	}
}


static unsigned char char_def_alphabet_ext_encode(unsigned char value)
{
	switch (value) {
	case 0x0c: return 0x0a; /* from feed */
	case '^':  return 0x14;
	case '{':  return 0x28;
	case '}':  return 0x29;
	case '\\': return 0x2f;
	case '[':  return 0x3c;
	case '~':  return 0x3d;
	case ']':  return 0x3e;
	case '|':  return 0x40;
	case 0xa4: return 0x65; /* euro */
	default: return 0x00; /* invalid character */
	}
}

/*
static int char_def_alphabet_ext(unsigned char value) {
	wchar_t retval;

	//if (char_mbtowc(&retval, &value, 1, NULL) == -1) return false;
	return (value == 0x0c ||
		value == '^' ||
		value == '{' ||
		value == '}' ||
		value == '\\' ||
		value == '[' ||
		value == '~' ||
		value == ']' ||
		value == '|' ||
		retval == 0x20ac);
}
*/

int char_7bit_unpack(unsigned int offset, unsigned int in_length, unsigned int out_length,
		     unsigned char *input, unsigned char *output);

int gsm_getaddr(char *dst, unsigned char *src, int len) {
int dlen,xlen,i,ton;
if  (len<1) return -2; // Недостаточная длина ...
dlen = src[0]; // Это - сколько должно получиться букв ...
if (dlen<0 || dlen>24) return -2; // по GSM больше нельзя ...
ton = (src[1]>>4)&7;
xlen = dlen/2 + 2; // Каждой цифре по полбайта + длина + тип оф намбер...
if (dlen%2) xlen++; // Полбайта - это байт...
//printf("TON:%d src[1]=%d\n",ton,src[1]);
if (ton == 5) { // alpha-numberic, GSM alphabet...
    //printf("ALPHA:<%s>\n",src);
    //int i = char_7bit_unpack(0,1000,dlen/2,src+2,dst);
    i = gsm7_decode(dst,src+2,1+dlen/2,0);
    printf("ALPHADSTADDR:<%s>,len=%d,dlen=%d\n",dst,i,dlen);
    if (i<0) return -2; // decode error
    dst[i]=0;
    return xlen; // 7bit???
    }
if (len<xlen) return -2; // Недостаточный буфер ...
//printf("\ngsm_getaddr ton=0x%x src[0]=%d\n",src[1],src[0]);
if (src[1]==0x91) { *dst='+'; dst++; } // Тип 19 - интернатионал, остальное - нафиг...
src+=2; // Перемещаемся с длины и типа намбера...
i=dlen%2;
dlen=dlen/2; // Переводим в хекс значения длину...
if (i) dlen++;
for(i=0;i<dlen;i++) { // Печатаем адрес ..
  char a[4];
  sprintf(a,"%02x",*src);
  dst[0]=a[1]; dst[1]=a[0];
  src++; dst+=2;
  }
*dst=0;
if (dst[-1]=='f') dst[-1]=0; // Если терминатор == 'f';
return xlen;
}



#define GN_BYTE_MASK ((1 << bits) - 1)
#define OUT_CH(CH) { /*printf("<%c>\n",CH);*/ if (!waitspec) {if (dst) *dst=CH; waitspec=CH==GN_CHAR_ESCAPE; if (waitspec) cnt--; } else {  if (dst) *dst = char_def_alphabet_ext_decode(CH); waitspec=0;}; if (!waitspec) {if (dst) dst++; i++;} if (i>=cnt) break; }
//#define OUT_CH(CH) { printf("<%c><%d>\n",CH,CH); *dst=CH;  dst++; i++; if (i>=cnt) break; }
int gsm7_decode(uchar *dst, uchar *src, int len, int offset) {
int i,cnt = (len*8 - offset)/7; // целое количество септетов
int bits,waitspec=0,rest;
bits = offset ? offset : 7;
for(i=0,rest=0;i<cnt;src++) { // cnt - сколько на выход (без учета waitspec)
    uchar ch;
    ch = ((*src & GN_BYTE_MASK) << (7 - bits)) | rest;
    rest = *src >> bits; // for a next iteration
    OUT_CH(ch);  // Сначала - в обязательном порядке - сбрасываем текущй ch
    bits--;
    if (bits==0) { // Нулевой бит - нужно дополнительно сбросить rest
        OUT_CH(rest); rest=0; bits=7;
        };
    }
printf("DONE_7_decode_len:%d\n",cnt);
return cnt;
}

int char_7bit_unpack(unsigned int offset, unsigned int in_length, unsigned int out_length,
		     unsigned char *input, unsigned char *output)
{

    return gsm7_decode(output,input,in_length,offset);

	unsigned char *out_num = output; /* Current pointer to the output buffer */
	unsigned char *in_num = input;  /* Current pointer to the input buffer */
	unsigned char rest = 0x00;
	int bits,wait_spec=0;

	bits = offset ? offset : 7;

	while ((in_num - input) < in_length) {

		*out_num = ((*in_num & GN_BYTE_MASK) << (7 - bits)) | rest;
		rest = *in_num >> bits;

		/* If we don't start from 0th bit, we shouldn't go to the
		   next char. Under *out_num we have now 0 and under Rest -
		   _first_ part of the char. */
		if ((in_num != input) || (bits == 7)) {
		    if (wait_spec) { *out_num = char_def_alphabet_ext_decode(*out_num);
		       //printf("?%c?",*out_num);
		       out_num++;

		       wait_spec=0; }
		     else {
		         wait_spec = *out_num == GN_CHAR_ESCAPE;
		         if (!wait_spec) {
		          //   printf("+%c+",*out_num);
		              out_num++;
		          }
		         }

		    }
		in_num++;
		if ( (out_num - output) >= out_length) {
		    printf("outLength = %d exceeds, gets=%d\n",in_length,in_num - input);
		    break; // no more
		    }

		//if ((out_num - output) >= out_length) break;

		/* After reading 7 octets we have read 7 full characters but
		   we have 7 bits as well. This is the next character */
		if (bits == 1) {
		    if (wait_spec) {
		        *out_num = char_def_alphabet_ext_decode(rest);
		        if ( (in_num - input) >= in_length) {
		        printf("inLength1 = %d exceeds, gets=%d\n",in_length,in_num - input);
		        break; // no more
		        }

		        //printf("{%c}",*out_num);
		        out_num++;  wait_spec=0;
		        } else {
			    *out_num = rest;
			    wait_spec = rest ==  GN_CHAR_ESCAPE;
			    if (!wait_spec) {
			        //printf("<%c>",*out_num);
			        out_num++;
			        }
                }
			bits = 7;
			rest = 0x00;
		} else {
			bits--;
		}
	}

//printf("BITS=%d LEN=%d REST=%d offset=%d\n",bits,out_num - output,rest,offset);
	return out_num - output;
}



int gsm7_code(unsigned int offset, int max_out, unsigned char *input,
		   unsigned char *output, unsigned int *get_len, int maxLen)
{

	unsigned char *out_num = output; /* Current pointer to the output buffer */
	unsigned char *in_num = input;  /* Current pointer to the input buffer */
	int bits;		     /* Number of bits directly copied to
					the output buffer */
    //printf("gsm7_code called with max_out:%d, offset=%d, sept=%d\n",max_out,offset,(max_out*8)/7);
    (*get_len)=0;
	bits = (7 + offset) % 8;

	/* If we don't begin with 0th bit, we will write only a part of the
	   first octet */
	if (offset) {
		*out_num = 0x00;
		out_num++;
		max_out--;
	}

	while ( maxLen >0  ) {
		unsigned char byte1;
		int double_char = 0;

        if (max_out<=0) {
		    //printf("maxLen=%d,maxOut=%d,coded=%d txt='%s'\n",maxLen,max_out,*get_len,in_num);
		    break; // no more
		    }
		//printf("maxLen=%d,maxOut=%d,coded=%d txt='%s'\n",maxLen,max_out,*get_len,in_num);

        double_char = char_def_alphabet_ext_encode(*in_num); // Вдруг это двойной символ???
		if (double_char) {
			byte1 = GN_CHAR_ESCAPE;
			//printf("ExtChar!\n");
			if (max_out<2) break; // need 2 bytes
			(*get_len)++; maxLen--;
			goto skip;
next_char:
			byte1 = double_char; double_char = 0;

		} else {
			byte1 = char_def_alphabet_encode(*in_num);
			(*get_len)++; maxLen--;
		}
skip:
		*out_num = byte1 >> (7 - bits);
		/* If we don't write at 0th bit of the octet, we should write
		   a second part of the previous octet */
		if (bits != 7)
			*(out_num-1) |= (byte1 & ((1 << (7-bits)) - 1)) << (bits+1);

		bits--;

		if (bits == -1) bits = 7;
		else { out_num++; max_out--;

	     }

		if (double_char) goto next_char;

		in_num++; //maxLen--;
	}

	return (out_num - output);
}


