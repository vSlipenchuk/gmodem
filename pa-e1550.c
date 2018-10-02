/*
 * Copy from microphone to speaker
 * pa-mic-2-speaker.c
 */


#ifdef P_AUDIO



#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "voice_stream.h"
#include <pulse/pulseaudio.h>

#define CLEAR_LINE "\n"
#define BUFF_LEN 4096

// From pulsecore/macro.h
#define pa_memzero(x,l) (memset((x), 0, (l)))
#define pa_zero(x) (pa_memzero(&(x), sizeof(x)))

static void *buffer = NULL;
static size_t buffer_length = 0, buffer_index = 0;

//int serial  = 0; // open serial for read

int verbose = 1;
int ret;
int raw = 0; // record file

/*
static pa_sample_spec sample_spec = {
  .format = PA_SAMPLE_S16LE,
  .rate = 44100,
  .channels = 2
};

*/

static pa_sample_spec sample_spec = {
  .format = PA_SAMPLE_S16LE, // BE? LE?
  .rate = 8000,  //8000, //44100,
  .channels = 1 //2
};




static pa_stream *istream = NULL;
                 //*ostream = NULL;

// This is my builtin card. Use paman to find yours
//static char *device = "alsa_input.pci-0000_00_1b.0.analog-stereo";
static char *idevice = NULL;
static char *odevice = NULL;

static pa_stream_flags_t flags = 0;

static size_t latency = 0, process_time = 0;
static int32_t latency_msec = 1, process_time_msec = 0;

void stream_state_callback(pa_stream *s, void *userdata) {
  assert(s);
//printf("++++ CALLBACK STATE\n");

  switch (pa_stream_get_state(s)) {
  case PA_STREAM_CREATING:
    // The stream has been created, so
    // let's open a file to record to
    //printf("Creating stream ..... \n");
    // fdout = creat(fname,  0711);
    buffer = pa_xmalloc(BUFF_LEN);
    buffer_length = BUFF_LEN;
    buffer_index = 0;
    break;

  case PA_STREAM_TERMINATED:
    // close(fdout);
    break;

  case PA_STREAM_READY:

    // Just for info: no functionality in this branch
    if (verbose) {
      const pa_buffer_attr *a;
      char cmt[PA_CHANNEL_MAP_SNPRINT_MAX], sst[PA_SAMPLE_SPEC_SNPRINT_MAX];

      //printf("Stream successfully created.");

      if (!(a = pa_stream_get_buffer_attr(s)))
	printf("pa_stream_get_buffer_attr() failed: %s", pa_strerror(pa_context_errno(pa_stream_get_context(s))));
      else {
	//printf("Buffer metrics: maxlength=%u, fragsize=%u", a->maxlength, a->fragsize);

      }

      //printf("Connected to device %s (%u, %ssuspended).",	     pa_stream_get_device_name(s),	     pa_stream_get_device_index(s),	     pa_stream_is_suspended(s) ? "" : "not ");
    }

    break;

  case PA_STREAM_FAILED:
  default:
    printf("Stream error: %s", pa_strerror(pa_context_errno(pa_stream_get_context(s))));
    exit(1);
  }
}


/*********** Stream callbacks **************/

//int incall = 0; // export var
int ttt = 0; // 320 byte time

/* This is called whenever new data is available */
static void stream_read_callback(pa_stream *s, size_t length, void *userdata) {
voice_stream *v = userdata;

  assert(s);
  assert(length > 0);
    return ; // ZUZUKA - ignore read thread

  // Copy the data from the server out to a file
  //fprintf(stderr, "--->>>  Can read %d\n", length);

  while (pa_stream_readable_size(s) > 0) {
    const void *data;
    size_t length, lout;

    // peek actually creates and fills the data vbl
    if (pa_stream_peek(s, &data, &length) < 0) {
      fprintf(stderr, "Read failed\n");
      exit(1);
      return;
    }
int serial = v->serial;
    if (serial>0 ) {
       static char buf[3200];  static int  len=0; // collector
       static int run=0; static int started = 0; static int sent=0;
       memcpy(buf+len,data,length); len+=length;
       int sz = 80; //320;
       if (run) {
           int to_send = ((ttt-started)*320)/sz; // length in packets
           if (len<sz) {
              run=0;
              printf("Stop - need collect!\n");
              }
           printf("tosend:%d sent:%d bytes have:%d ttt=%d\n",to_send,sent,len,ttt);
           while( (len>=sz) && (to_send>sent )) { // flash them
            int l = write(serial,buf,sz); // just write
            printf("WR:=%d bytes from %d \n",l,len);
            if (raw>0) write(raw,buf,sz); // record in local file
            len-=sz; memcpy(buf,buf+sz,len);
            sent++;
            }
          printf("REST:%d\n",len);

          } else { // check - readyy to flaskcollect
          if (len>2*320)  {
                write(serial,buf,320); len-=320; memcpy(buf,buf+320,len); // flash first
                run=1; // start run
                started = ttt; sent=2; // remember time 2x80 already sent
                printf("start FLASH already collected=%d\n",len);
                }


          }

      //hexdump("written",data,length);
      }
    /*

    //fprintf(stderr, "read %d\n", length);
    lout =  pa_stream_writable_size(ostream);
    //fprintf(stderr, "Writable: %d\n", lout);
    if (lout == 0) {
      fprintf(stderr, "can't write, zero writable\n");
      return;
    }
    if (lout < length) {
      fprintf(stderr, "Truncating read\n");
      length = lout;
  }
  */
/*
  if (pa_stream_write(ostream, (uint8_t*) data, length, NULL, 0, PA_SEEK_RELATIVE) < 0) {
    fprintf(stderr, "pa_stream_write() failed\n");
    exit(1);
    return;

  }
*/


    // STICK OUR CODE HERE TO WRITE OUT
    //fprintf(stderr, "Writing %d\n", length);
    //write(fdout, data, length);

    // swallow the data peeked at before
    pa_stream_drop(s);
  }
}



/* This is called whenever new data may be written to the stream */
// We don't actually write anything this time

//void *cs; // mutex
//char data[320*10];
//int  cnt=0,run=0;



static void stream_write_callback(pa_stream *s, size_t length, void *userdata) { // do we need it ?
 }


// This callback gets called when our context changes state.  We really only
// care about when it's ready or if it has failed
void state_cb(pa_context *c,voice_stream *vs) {
  pa_context_state_t state;
//  voice_stream *vs = userdata;
  //int *pa_ready = userdata;

 // printf("State changed\n");
  state = pa_context_get_state(c);
  switch  (state) {
    // There are just here for reference
  case PA_CONTEXT_UNCONNECTED:
  case PA_CONTEXT_CONNECTING:
  case PA_CONTEXT_AUTHORIZING:
  case PA_CONTEXT_SETTING_NAME:
  default:
    break;
  case PA_CONTEXT_FAILED:
  case PA_CONTEXT_TERMINATED:
 //   *pa_ready = 2;
    break;
  case PA_CONTEXT_READY: {
    pa_buffer_attr buffer_attr;

    //if (verbose)
//      printf(">>>>>Connection established.%s vs=%x\n", CLEAR_LINE,vs);

    if (!(istream = pa_stream_new(c, "JanCapture2", &sample_spec, NULL))) {
      printf("pa_stream_new() failed: %s", pa_strerror(pa_context_errno(c)));
      exit(1);
    }

 //   printf("stream i=%x  ---- >\n",istream);

    if (!(vs->ostream = pa_stream_new(c, "JanPlayback2", &sample_spec, NULL))) {
      printf("pa_stream_new() failed: %s", pa_strerror(pa_context_errno(c)));
      exit(1);
    }

    // Watch for changes in the stream state to create the output file
    pa_stream_set_state_callback(istream, stream_state_callback, vs);

    // Watch for changes in the stream's read state to write to the output file
    pa_stream_set_read_callback(istream, stream_read_callback, vs);

    pa_stream_set_write_callback(vs->ostream, stream_write_callback, vs);



    // Set properties of the record buffer
    pa_zero(buffer_attr);
    buffer_attr.maxlength = (uint32_t) -1;
    buffer_attr.prebuf = (uint32_t) -1;

    if (latency_msec > 0) {
      buffer_attr.fragsize = buffer_attr.tlength = pa_usec_to_bytes(latency_msec * PA_USEC_PER_MSEC, &sample_spec);
      flags |= PA_STREAM_ADJUST_LATENCY;
    } else if (latency > 0) {
      buffer_attr.fragsize = buffer_attr.tlength = (uint32_t) latency;
      flags |= PA_STREAM_ADJUST_LATENCY;
    } else
      buffer_attr.fragsize = buffer_attr.tlength = (uint32_t) -1;

    if (process_time_msec > 0) {
      buffer_attr.minreq = pa_usec_to_bytes(process_time_msec * PA_USEC_PER_MSEC, &sample_spec);
    } else if (process_time > 0)
      buffer_attr.minreq = (uint32_t) process_time;
    else
      buffer_attr.minreq = (uint32_t) -1;


  //buffer_attr.maxlength = 320;
      //printf("FragSize=%d min=%d\n",buffer_attr.fragsize,buffer_attr.minreq);
      //buffer_attr.fragsize = buffer_attr.tlength = 320;

    // and start recording
 if (pa_stream_connect_record(istream, idevice, &buffer_attr, flags) < 0) {
      printf("pa_stream_connect_record() failed: %s", pa_strerror(pa_context_errno(c)));
      exit(1);
    }


    if (pa_stream_connect_playback(vs->ostream, odevice, &buffer_attr, flags,
				   NULL,
				   NULL) < 0) {
      printf("pa_stream_connect_playback() failed: %s", pa_strerror(pa_context_errno(c)));
      exit(1); //goto fail;
    } else {
      //printf("Set playback callback OK\n");
    }

  }

    break;
  }
}

//pa_thread()


#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>
#include <sys/stat.h>
#include <errno.h>

size_t write_all (int fd, const char* buf, size_t count)
{
	ssize_t out_count;
	size_t total = 0;
	unsigned errs = 10;

	while (count > 0)
	{
		out_count = write (fd, buf, count);
		if (out_count <= 0)
		{
			if(errno == EINTR || errno == EAGAIN)
			{
				errs--;
				if(errs != 0)
					continue;
			}
			break;
		}
//		fprintf(stdout, "%*s", out_count, buf);
		errs = 10;
		count -= out_count;
		buf += out_count;
		total += out_count;
	}
	return total;
}


int voice_write(int com) {
int f;
int incall=1; //ZU
//printf("===========WRITE Voice started\n");
while(1) {
  while(!incall) msleep(100);
  f = open("welcome.raw",O_RDONLY);

  int scale = 1;

  char buf[320];
  printf("WRITE thread play activated f=%d\n",f);
  //int sz=0;

  while(incall) {
      int sz = sizeof(buf)/scale;
      int l = read(f,buf,sz);
      if (l<sz) break;
      int w = write(com,buf,l);
      if (w<l) {
          printf("ERR_WRITE w=%d of %d\n",w,l);
          }
      //write_all(com,buf,l);
      //printf(":OUT=%d    \r",sz); sz+=l;
      //do_write(buf,l); //,length);
      msleep(19/scale);
      }
  printf("Play done, goto again\n");
  close(f);
  }
return 0;
}

int e1550_write(voice_stream *v,char *data,int len) { // called by recorder when new data ready
// just collect data by now
if (v->incall) {
  v->rec_bytes+=len;
  //
  }
return len;
}

int e1550_read_thread(voice_stream *v) { // read from modem and write to pulseaudio
int com = v->serial;
//printf("voice_read thread ready\n");
msleep(1000);
while (!v->stop) {
 char data[320];
    //while(!v->incall) msleep(10); // wait for call begins
 int length = read(com,data,sizeof(data));
 if (length<0) break;
 v->push_bytes+=length;
 if (v->voice_push) v->voice_push(v,data,length); // push to sound card
 //printf("e1550_read: %d push=%p\n ",v->push_bytes,v->voice_push);
 }
v->stop=1;
//printf("DONE VOICE READ\n");
return 0;
}


// -------------- PulseAudio wrappers ----------------------

int paudio_voice_push(voice_stream *v,char *data, int len) { // push to sound card
if (v->ostream)
if (pa_stream_write(v->ostream, (uint8_t*) data, len, NULL, 0, PA_SEEK_RELATIVE) < 0) {
     //fprintf(stderr, "pa_stream_write() failed\n");
    return -1;
    }
return len; //ok
}

int paudio_thread_loop(voice_stream *v) {

  pa_mainloop *pa_ml = v->pa;
 if (pa_mainloop_run(pa_ml, &ret) < 0) {
    printf("pa_mainloop_run() failed.");
    v->stop = -1;
  }
return 0;
}

int paudio_init(voice_stream *v) { // starts with pulseaudio

  pa_mainloop_api *pa_mlapi;
  pa_operation *pa_op;
  pa_context *pa_ctx;
  v->voice_push = paudio_voice_push;

  // Create a mainloop API and connection to the default server
  pa_mainloop *pa_ml = v->pa = pa_mainloop_new();
  pa_mlapi = pa_mainloop_get_api(pa_ml);
  pa_ctx = pa_context_new(pa_mlapi, v->name);
  // This function connects to the pulse server
  pa_context_connect(pa_ctx, NULL, 0, NULL);
  // This functi on defines a callback so the server will tell us its state.
  pa_context_set_state_callback(pa_ctx, (void*)state_cb, v);
 // printf("PULSE THREAD BEGIN\n");
 // thread_create()
  thread_create(paudio_thread_loop,v);
  //v->stop = 2;
  //printf("PULSE AUDIO TREAD STOPEED\n");
  return 1;
}


// ----------------- E1550 functions -----------------


int voice_init(voice_stream *v) { // Main PulseAudio thread
  v->stop = 0;
  v->serial = open(v->comName, O_RDWR | O_NOCTTY, 0);
  if (v->serial<=0) {
     fprintf(stderr,"voice_init: cant open %s\n",v->comName);
     return 0;
     }
  prt_cfg(v->serial,B115200); //zuzyka
  v->cs=mutex_create();
  v->voice_rec = e1550_write; // write recorder sound to modem audioport
  v->voice_push = paudio_voice_push;
  //raw = open("out.raw",O_RDWR | O_TRUNC | O_CREAT, S_IREAD|S_IWRITE); // audio logger sound-file
  paudio_init(v);
  thread_create(e1550_read_thread,v); // read e1550 voice -> play on playback pulseaudio
  //sleep(100);
  return 1; // OK anyway
}

#endif
