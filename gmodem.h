#ifndef GMODEM_H_INCLUDED
#define GMODEM_H_INCLUDED

#include "../vos/vos.h"
#include "voice_stream.h"


#define gmodem_version 0,0,0,8

// 0.0.0.7  -- added CSD calls
// 0.0.0.6   -- added build-in http-sever
// 0.0.0.5  -- added apdu getData (and delta for EEPORM and RAM)
// 0.0.0.4   -- apdu over at+CSIM or --phoenix reader
// 0.0.0.3 - add voice stream E1550 in pulse audio
// 0.0.0.2 - add draft for at+crsm (cnum,iccid,...)

/*
    gsm modem over serial line
*/

typedef struct { // vstream functions
 void* (*open)(void *); // constructor
 void  (*close)(void *);   // destructor
 int   (*peek)(void*,void*,int);    // read data
 int   (*write)(void*,void*,int);   // write data
 } vstream_proc;

typedef struct { // -- to move!!! - to stream!!!
 void *handle;
 vstream_proc *p; // interface functions
} vstream;

extern vstream_proc vstream_com_procs; // external comport

// --- done streams...

enum { // command result codes
    g_ok = 1,  g_connect = 2, // success
    g_eof = -4,
    g_error = -1,
    g_no_carrier = -2,
    g_busy = -3,
    g_no_dial_tone = -5,
   };

#define GMODEM_READ_BUF 512

enum { // call states
    callNone, // no call, cleared
    callRing, // = callConnectionPending, // MT: from first RING till collect information
    callPresent,     // MT:: MSISDN number here or timeout of waiting done
      callSetup, // when atd sent, but no OK received yet
      callInit,  // ok on call setup accepted - call in progress now
    callActive,      // Call Accepted = callConnect
    callDisconnecting, // when release requested
    callReleased, // whell - wait for silence pause till disconnect
   };

typedef struct _gmodem {
  vstream port; // main stream over comport or net connection
  int (*on_data)(); // handler - when any bytes of data accepted
  int (*on_line)(); // handler - when new line accepted
  char name[24]; // device name - copied by open
  int  in_len; char in[GMODEM_READ_BUF]; // input buffer (line collector)
  int  res; // result of async commands (=0 at first call)
  struct { // flags & status
    char eof; // modem disconnected, read-write error
    char to;  // time-out occcured
    char e1;  // "echo off" done
    char c1;  // "clip=1"
    char ring; // if we have a ring indicator 1=first, 2=notify 3=others
    char over; // overfull flag in buffer fillings
    char console; // console mode (on_char & getch printing)
    int  idle; // idle  read count loop
    } f;
  struct { // call & sms options
     char state; // 0 == callNone
     unsigned long modified,connected; // time when
     char num[24]; // incoming number, at callNone = '-'
     int (*on_call_state)();
     char release_request; // smb requested to kill a call (cleared by sent ath)
     char incall; // 0 - nothing, -1=kill, 1 = accept
     int ring; // ring counter. at callNone=0
     int dur; // connected duration (updated in gmodem_call.
    } o;
    //
    char imsi[16]; // updated by gmodem_imsi
    char imei[20]; // sgsn updated by gmodem_imei
    char iccid[24]; //
    char cnum[25]; // owner number EF_MSISDN
    struct _gsm_operator *oper; // ref to network
    struct _gsm_device   *dev;  // ref to device
    //
    unsigned long now; // its a time os_ticks returned
    char cusd[512]; // response of cusd
    //
    int logLevel; // level of logging
    unsigned char out[256]; // errors & string results here
    char *bin; // temp buffer for sending binary data (Book, SMS, Others)
    struct _gmodem *mon,*parent; // monitor port for Qualcomm (E1550)
    voice_stream *voice; // if we have voice_serial connected
    int mode; // 0 - AtComPort, 1-PhoenixCard reader
    int  cmt; // counter for mt-sms
    int (*on_mt)(); // called when cmt>0 if accessed
} gmodem;


int gmodem_init(gmodem *g, char *name); // open com port
int gmodem_setLogLevel(gmodem *g,int logLevel);
int gmodem_run (gmodem  *g); // main @run@ procedure
int gmodem_At  (gmodem *g, char *cmd); // send At command at a modem
int gmodem_At2buf(gmodem *g,char *cmd,char *out, int size);
int gmodem_At2bufFilter(gmodem *g,char *cmd,char *filter,char *out, int size);

/* call at_command, filter output and call on_line_call on every mathed line */
int gmodem_At2Lines(gmodem *g,char *at_cmd,char *filter,int (*on_line_call)(),void *handle);


int gmodem_clear(gmodem *g, int ms); // ms sec silence
int gmodem_echo_off(gmodem *g);
int gmodem_clip_on(gmodem *g);
int gmodem_put(gmodem *g, char *out,int len); // push data to out stream

// calls
int gmodem_set_call_state(gmodem *g,int newstate);
int gmodem_run2 (gmodem  *g); // main @run@ procedure with call states

// ussd
int gmodem_ussd(gmodem *g,char *str); // call string

int _gmodem_dial(gmodem *g,char *num,int voice);
int gmodem_dial(gmodem *g,char *num);
int gmodem_pppd(gmodem *g,char *opt); // dial pppd & wait for it...

// simemu & other
int gmodem_pin(gmodem *g,char *pin); // check pins & enter pin & wait for IMSI


typedef struct _gsm_operator {
 char *name; char *imsi; // name & mcc[3]+mnc[2]+zero
 char *gprs_num, *apn; //  gprs dial number & apn
 char *ussd_balance; // call for ussd balance string
 } gsm_operator;

extern gsm_operator gsm_operators[];

int gmodem_imsi(gmodem *g) ; // when define imsi - we can start to definde network
int gmodem_balance(gmodem *g); // send USSD based on imsi?


typedef struct _gsm_device {
 char *name; char imei[20]; // name & first imei numbers
 int   crlf; int ussd; // 7bit flags for huawei E1550
 int dmode; // 0: ATD->OK means start dial; 1: ATD->OK means connect
 } gsm_device;

int gmodem_imei(gmodem *g);
char *gmodem_crlf(gmodem *g);
int gmodem_cnum(gmodem *g); // try read cnum from a rsim?

// all cmd commands

int gmodem_outf(gmodem *g,int res, char *fmt,...);
#define gmodem_errorf gmodem_outf
void gmodem_logf(gmodem *g,int level, char *fmt,...);

int gmodem_Atf(gmodem *g,char *fmt, ... );


int gmodem_info(gmodem *g) ;
int gmodem_cmd(gmodem *g,char *cmd);
int gmodem_cb(gmodem *g,char *cmd);
int gmodem_sms(gmodem *g,uchar *sms);

// call

int gmodem_kill(gmodem *g);
int gmodem_dtmf(gmodem *g, char *c);


// CRSM
//char *gmodem_par(char **cmd,int skip);
int gmodem_crsm_cnum_get(gmodem *g); // on ok - result in g->out, returns LEN of CNUM record
int gmodem_crsm_cnum_set(gmodem *g,char *num);
int gmodem_crsm_iccid(gmodem *g); // on ok - result in g->out

// CSIM & APDU
int gmodem_apdu_cmd(gmodem *g, uchar *cmd);

// Phoenix commands

int gmodem_atr(gmodem *g); // Answer to reset...


// Phonebook sim card & phones

int gmodem_book_cmd(gmodem *g,uchar *cmd);

#endif // GMODEM_H_INCLUDED
