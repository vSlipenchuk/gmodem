#include "gmodem.h"

/*
    common procs here...

*/

#include "../vos/bin2hexstr.c"
#ifdef __linux__
#include "../vos/vos_com_linux.c"
#include "../vos/vos_linux_kbhit.c"
#else
// WIN STAFF
#include "../vos/vos_com_win.c"
int kbhit2() { return kbhit();} // on win - not init functions...
void set_echo(int echo) {}; // do nothing
int gets_buf2(char *buf,int sz) { return gets(buf); }
#endif // __linux__



#include "../vos/vos.c"
#include "../vos/coders.c"
#include "../vos/strutil.c"
#include "../vos/hexdump.c"
#include "./gsm2utf.c"

//#include "../vos/sock.c"
//#include "../vos/httpSrv.c"

#include <string.h>



/* thats a com port functions */
vstream_proc vstream_com_procs = {
    .open = (void*)prt_open,
   .close = prt_close,
   .peek = prt_peek_len, // or _old
   .write  = prt_write
  };

#ifdef __linux__

//apt-get install libncurses5-dev  || yum install ncurses-devel ncurses
#include <curses.h>
//apt-get install libreadline-dev
#include <readline/readline.h>
#include <readline/history.h>

#define STDIN 0

void set_echo(int echo) {
        struct termios term;
        tcgetattr(STDIN, &term);
        if (!echo) term.c_lflag &= ~(ICANON | ECHO);
           else  { term.c_lflag &= ~(ICANON);  term.c_lflag |=  ECHO;}
       // term.
        tcsetattr(STDIN, TCSANOW, &term);
        setbuf(stdin, NULL);
}

int kbhit2() {

    static int initialized = 0;

    if (! initialized) {
        // Use termios to turn off line buffering
        set_echo(0);
        initialized = 1;
        //noecho(); // curses
    }

    int bytesWaiting;
    ioctl(STDIN, FIONREAD, &bytesWaiting);
    return bytesWaiting;
}

int gets_buf2(char *buf,int sz) {
set_echo(1);
char *g = readline(""); // read, no prompt
set_echo(0);
if (!g)  { *buf=0;return 0; }  // eof
int l = strlen(g);
if (l>=sz-1) l = sz-1;
memmove(buf,g,l); buf[l]=0;
add_history(buf);
free(g);
return 1; // OK
}

#endif

//#include "../vos/vos.c" // os spec. code here
