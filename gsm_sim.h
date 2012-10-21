// GSM 11.11 headers and constants
#ifndef GSM_SIM
#define GSM_SIM

enum { // Basic sim operations
 READ_BINARY=176,
 READ_RECORD=178,
 GET_RESPONSE=192,
 UPDATE_BINARY=214,
 UPDATE_RECORD=220,
 STATUS=242
 };


// p73 contains draw of all files
#define EF_ICCID  0x2FE2 // pp 10.1.1
#define EF_MSISDN 0x6F40 // pp 10.3.4
//#define EF_SPN



#endif // GSM_SIM
