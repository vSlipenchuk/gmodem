#ifndef VSTRUTIL
#define VSTRUTIL
// string utilities
#include <string.h>

#ifndef strNcpy
#define strNcpy(dest,src) { memset(dest,0,sizeof(dest)); strncpy(dest,src,sizeof(dest)-1); }
#endif

//int lcmp(uchar **str,uchar *cmp);

//char *get_till(char **buf, char *del); // removes till delimite
//char *str_unquote(char *buf);
//char *trim(char *buf) ; // trims left and right?
//int lcmp(char **s,char *cmd);

#endif // VSTRUTIL

