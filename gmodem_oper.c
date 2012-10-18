/*
  GSM operators -- balance & gprs-apn functions

*/

#include "gmodem.h"

//+CGDCONT=1,"IP", "www.umc.ua".

gsm_operator gsm_operators[]={
    {
      .name="unknown"
    },
   {
     .name="MTS",.imsi="25001",.ussd_balance="#100#",
     .gprs_num="*99***1#",.apn="mts/mts@internet.mts.ru"
   },
   {
     .name="BEE",.imsi="25099",.ussd_balance="#102#",
     .gprs_num="*99***1#",.apn="beeline/beeline@internet.beeline.ru",
   },
   {
       .name=0
   }

   };
