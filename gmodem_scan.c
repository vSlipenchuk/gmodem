/*
  scan procedures
*/

#include "gmodem.h"
#include "vstrutil.h"
// simple trim

/*

static char *dels=" \t\r\n"; // normal string trim-delimiters

char *trim(char *buf) { // trims left and right?
int l=strlen(buf);
while(l>0 && strchr(dels,*buf)) { buf++,l--;};
while(l>0 && strchr(dels,buf[l-1])) { l--;};
buf[l-1]=0;
return buf;
}

char *get_till(char **buf, char *del) { // removes till delimiters
char *str = *buf; int l =strlen(str); // in-string
char *r=str; // result
char *pos = strstr(str,del); // find delimiter
if (!pos) { // not found - all is result
   *buf+=l; return r;
   }
// well - now - gets till delimiter
l = pos-str; // length of
str[l]=0; // zero term
*buf+= (l+strlen(del)); // after delimiter
return r; // anyway
}*/

char *str_unquote(char *buf) {
int l = strlen(buf);
 //printf("UnquoteLen=%d\n",l);
if (l>=2 && strchr("\"'",*buf) && buf[0]==buf[l-1]) { buf++; l-=2; buf[l]=0 ;}
 //printf("UnquoteLen=%d\n",l);
 //printf("S=<%s>\n",buf);
return buf;
}
/*

int lcmp(char **s,char *cmd) {
int l=strlen(cmd);
if (memcmp(*s,cmd,l)!=0) return 0;
(*s)+=l;
while ( (*s)[0]==32 ) (*s)++;
return 1;
}
*/

