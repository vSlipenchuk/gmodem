// GSM 11.11 headers and constants
#ifndef GSM_SIM
#define GSM_SIM
#include "gmodem.h"

typedef struct { // secure info
  unsigned char kic[24],kid[24];  // max 8bytes*3 keys
  long cntr; // 5 bytes used
  } sec_info;


enum { // Basic sim operations
 READ_BINARY=176, // 0xB0
 READ_RECORD=178,  // 0xB2
 GET_RESPONSE=192, // 0xC0
 UPDATE_BINARY=214, //0xD6
 UPDATE_RECORD=220, // 0xDC
 STATUS=242 // 0xF2
 };


// p73 contains draw of all files
#define EF_ICCID  0x2FE2 // pp 10.1.1
#define EF_MSISDN 0x6F40 // pp 10.3.4
//#define EF_SPN

typedef struct {
   char data[256]; // all data (copied)
   int   status; // status code
   unsigned char *tar,*counter; // copy of counter&tar
   unsigned char *ex_data; int ex_data_len; // extra_packet_data

   int command; unsigned char sw[2]; // 3 bytes extra...
  } porPacket;

typedef struct _cardJob { // global Variable
  // coding profile variables
  int spi, kic, kid; // SPI[2], kic,kid
  unsigned char tar[3]; //
  // personal
  unsigned char ICCID[24]; // just for info
  unsigned char KID[24],KIC[24]; // personal keys
  unsigned char  MSISDN[24],IMSI[24];
  int counter; // counter - if need ?

  unsigned char num[24]; // number for SMS-enveloper coding


  // decoding variales
  unsigned char SW[2]; // last SW decoded
  porPacket por; // last decoded POR

  int (*saveCounter)(struct _cardJob *j);
  }  CardJob;

extern struct _cardJob cardJob; // defined in gmodem_apdu

void cardJobSetup(struct _cardJob *j, int spi, int kic, int kid,  // profile data
                     uchar *ICCID,
                     uchar *KIC, uchar *KID, // personal data
                     int counter);

/*
enum {
0, PoR OK.
1, RC/CC/DS failed.
2, CNTR low.
3, CNTR high.
4, CNTR Blocked
5, Ciphering error.
6, Unidentified security error. This code is for the case where the Receiving Entity cannot correctly
   interpret the Command Header and the Response Packet is sent unciphered with no RC/CC/DS.
7, Insufficient memory to process incoming message.
8, This status code "more time" should be used if the Receiving Entity/Application needs more time
to process the Command Packet due to timing constraints. In this case a later Response Packet
should be returned to the Sending Entity once processing has been completed.
9, TAR Unknown
10, Insufficient security level
//Reserved for future use.
}
*/


#endif // GSM_SIM
