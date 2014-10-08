/*
  GSM operators -- balance & gprs-apn functions

*/

#include "gmodem.h"

//+CGDCONT=1,"IP", "www.umc.ua".

gsm_operator gsm_operators[]={
    {
      .name="GSM"
    },
{ .name="RTK",.imsi="2500168",.ussd_balance="#100#",  .gprs_num="*99#",.apn="mts/mts@m2m.msk"  },
{ .name="RT-Mobile", .imsi="25039", .ussd_balance="*102#",  .gprs_num="*99***1#",.apn="/@internet.rt.ru"  },

   {
     .name="MTS",.imsi="25001",.ussd_balance="#100#", // smsc="+79168999100" at+csca=""+79168999100"
     .gprs_num="*99***1#",.apn="mts/mts@internet.mts.ru"
   },
   {
     .name="BEE",.imsi="25099",.ussd_balance="#102#",
     .gprs_num="*99***1#",
     .apn="beeline/beeline@internet.beeline.ru",
     //.apn="beeline/beeline@home.beeline.ru"
   },
   {
       .name=0
   }

   };
