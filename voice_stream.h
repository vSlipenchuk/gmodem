#ifndef VOICE_STREAM_H
#define VOICE_STREAM_H

typedef struct _voice_stream {
  char *name;   // extra name (for pulseaudio)
  int   incall; // flag if we in a call state? - runs read&write opewrations
  int   stop;   // flag for threads to terminate
  // some stat
  int rec_bytes,push_bytes; // mic2modem, modem2head
   // internal
  void  *cs; // mutex
  int   (*voice_push)(struct _voice_stream *v,void *data, int sz); // push data to sound card, called by e155_read_thread
  int   (*voice_rec)(struct _voice_stream *v,void *data, int sz); // (e1550 write) recorder from mic data handler
  void   *pa; // temp handle
  void  *ostream; // output stream, used for push
  void  *thr_read; // reading thread
  void  *thr_write; // write thread
  // by E1550
  char *comName ; // normally /dev/ttyUSB1
  int   serial; // opened serial com_port handle
  } voice_stream;

int voice_init(voice_stream *v); // returns >0 on success, <=0 on error

#endif
