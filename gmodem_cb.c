#include "gmodem.h"

// gmodem cellbroadcast service


int gmodem_cb(gmodem *m,char *cmd) {
if (lcmp(&cmd,"info")) {
   // gmodem_
   return 1;
   }
printf("cb info || show open, not implemented yet\n");
return 1; // bad
}
