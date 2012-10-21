#include "gmodem.h"

gsm_device gsm_devices[] = {
    {
        .name="default", .crlf=3 /*\r\n*/ // GSM-0707
    },
    {
        .name="E1550", .imei="3534430",.crlf=1   /*\n*/   // !!! ussd_coder !!! and other space!!!
    },
    {
        .name="E171",   .imei="3548070",.crlf=1   /*\n*/   // !!! ussd_coder !!! and other space!!!
    },


    {
        .name="Telit", .imei="3550940",.crlf=3   /*\n*/   // !!! ussd_coder !!! and other space!!!
    },
    {
        .name="HOJY",  .imei="3530360",.crlf=1   /*\n*/   // !!! ussd_coder !!! and other space!!!
    },


    {
        .name=0
    }

    };

// Как-то у меня получилось!!!

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

