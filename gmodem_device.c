#include "gmodem.h"

gsm_device gsm_devices[] = {
    {
        .name="default", .crlf=3 /*\r\n*/ // GSM-0707
    },
    {
        .name="E1550", .imei="3534430",.crlf=1   /*\n*/   // !!! ussd_coder !!! and other space!!!
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
