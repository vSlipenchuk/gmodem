#include "gmodem.h"

/*
    common procs here...

*/

#include "../vos/vos_com_linux.c"
#include "../vos/vos_linux_kbhit.c"
#include "../vos/vos.c"

#include <string.h>

/* thats a com port functions */
vstream_proc vstream_com_procs = {
    .open = (void*)prt_open,
   .close = prt_close,
   .peek = prt_peek,
   .write  = prt_write
  };

#include "curses.h"
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
memcpy(buf,g,l); buf[l]=0;
add_history(buf);
free(g);
return 1; // OK
}

//#include "../vos/vos.c" // os spec. code here
