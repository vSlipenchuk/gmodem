#ifndef GMODEM_H_INCLUDED
#define GMODEM_H_INCLUDED

#include "../vos/vos.h"

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
    g_ok = 1, // success
    g_eof = -4,
    g_error = -1,
    g_no_carrier = -2,
   };

#define GMODEM_READ_BUF 512

enum { // call states
    callNone, // no call, cleared
    callRing, // = callConnectionPending, // MT: from first RING till collect information
    callPresent,     // MT:: MSISDN number here or timeout of waiting done
    callActive,      // Call Accepted
    callDisconnecting, // when release requested
    callReleased, // whell - wait for silence pause till disconnect
   };

typedef struct {
  vstream port; // main stream over comport or net connection
  int (*on_data)(); // handler - when any bytes of data accepted
  int (*on_line)(); // handler - when new line accepted
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
     unsigned long modified;
     char num[24]; // incoming number
     int (*on_call_state)();
     char release_request; // smb requested to kill a call (cleared by sent ath)
     char incall; // 0 - nothing, -1=kill, 1 = accept
    } o;
    unsigned long now; // its a time os_ticks returned
} gmodem;


int gmodem_init(gmodem *g, char *name); // open com port
int gmodem_run (gmodem  *g); // main @run@ procedure
int gmodem_At  (gmodem *g, char *cmd); // send At command at a modem
int gmodem_At2buf(gmodem *g,char *cmd,char *out, int size);
int gmodem_At2bufFilter(gmodem *g,char *cmd,char *filter,char *out, int size);

int gmodem_clear(gmodem *g, int ms); // ms sec silence
int gmodem_echo_off(gmodem *g);
int gmodem_clip_on(gmodem *g);
int gmodem_put(gmodem *g, char *out,int len); // push data to out stream

// calls
int gmodem_set_call_state(gmodem *g,int newstate);
int gmodem_run2 (gmodem  *g); // main @run@ procedure with call states

// ussd
int gmodem_ussd(gmodem *g,char *str); // call string

#endif // GMODEM_H_INCLUDED
