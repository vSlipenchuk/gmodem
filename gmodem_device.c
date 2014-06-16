#include "gmodem.h"

 //case 1: return "\r"; // E1550 by default!!!
 //case 2: return "\n";

gsm_device gsm_devices[] = {
    {
        .name="default", .crlf=3 ,/*\r\n*/ // GSM-0707
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
{
        /*
         Mediatek based phone,
            CRSM for CNUM broked (in responce while get length)
        */
        .name="OT-808",.imei="3563890",.crlf=1   /*\n*/   // !!! ussd_coder !!! and other space!!!
    },


    {
        .name="E171",   .imei="3548070",.crlf=1   /*\n*/   // !!! ussd_coder !!! and other space!!!
    },


    {
        .name="Telit", .imei="3550940",.crlf=3
    },
    {
        .name="HOJY",  .imei="3530360",.crlf=1   /*\n*/   // !!! ussd_coder !!! and other space!!!
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

