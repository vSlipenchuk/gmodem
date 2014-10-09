#include "gmodem.h"
#include "coders.h"
#include "stdarg.h"

/*
 gmodem_scan fmt: d,s,u
*/

int gmodem_scan2(unsigned char *str,char *fmt, ... ) { // d s h ...
int res=0,i; vss V;
char buf[256],*s;
va_list ap;
va_start(ap,fmt);
void scan_v(vss *v) {
    if (*str=='\"' || *str=='\'') { // scan for match pair
         int ok=0;
         v->data=str+1; v->len=-1;
         for(i=1;str[i];i++) if (str[i]==str[0]) { i++; ok=1; break;};
         if (ok) v->len=i-2; else v->len=i-1;
         //printf("DONE_V: <%*.*s>\n",v->len,v->len,v->data);
         str+=i; fmt+=2; // done
      } else {
         v->data=str; // till next symb
         for(i=1;str[i];i++) if (fmt[2]==str[i]) break; // delimiter found!
         str+=i; fmt+=2; v->len=i;
         }
res++;
}
while(*fmt && *str) {
  for(i=0;str[i]&&strchr(" \t\r\n",str[i]);i++); str+=i; // skips empty spaces
  for(i=0;fmt[i]&&strchr(" \t\r\n",fmt[i]);i++); fmt+=i; // skips empty spaces
   //printf("----FMT:%s\nSTR:%s\nPRESS ENTER, NOWRES:%d\n",fmt,str,res);
   //{char buf[10]; gets(buf);}
    if (fmt[0]=='%') switch(fmt[1]) { // special case
  case 'd': // same as U for now...
  case 'u': // scan unsigned integer
     for(i=0;str[i]&&strchr("0123456789",str[i]);i++); // scan all digites here
     if (sscanf(str,"%d",va_arg(ap,int*))!=1) return -3; // read error
     fmt+=2; str+=i; res++;
     break;
  case 'v': // string. may be \" terminated
     scan_v( va_arg(ap,vss*) );
     break;
  case 'S': // UCS string -- need to convert to utf
       scan_v( &V ); if (V.len>128) V.len=128;
       s=va_arg(ap,char*);
       memcpy(s,V.data,V.len); s[V.len]=0;
       break;
  case 'U': // UCS string -- need to convert to utf
       scan_v( &V ); if (V.len>128) V.len=128;
       i = hexstr2bin( buf, V.data,V.len );
       //hexdump("bin",buf,i);
       gsm2utf( va_arg(ap,void*) , buf, i );
       break;
  default: // exactly match f[1];
     //printf("STR:%c FMT:%c\n",str[0],fmt[0]);
     if (*str!=fmt[1]) return -2; // match error
     fmt+=2; str++;
     break;
  } else { // exactly match?
     if (*str!=*fmt) return -4; // match error
     fmt++; str++;
  }
  }
return res; // returns count of scanned fields
}

/*
  Если в книжке максимум - 16 байт на слово, то в русском варианте это 8 букв :)
*/

int gmodem_book_read(gmodem *g,int slot) {
uchar out[256],cmd[80];
out[0]=0; sprintf(cmd,"+CPBR=%d",slot);
if (gmodem_At2bufFilter(g,cmd,"+CPBR:",out,sizeof(out))<=0) return -2; // invalid
if (out[0]==0) return gmodem_errorf(g,-2,"empty result unexpected");
//printf("OUT=<%s>\n",out);
  char num[256],name[256]; int numf;
  int res = gmodem_scan2(out,"%d,%S,%d,%U",&slot,num,&numf,name);
//printf("res=%d\n",res);
if (res==4) return gmodem_errorf(g,1,"%s %s",num,name);
 else return gmodem_errorf(g,-2,"fail#%d scan %s",res,out);
return 1;
}

int gmodem_book_list(gmodem *g) {
uchar out[200]; int min=1,max=100,i1=0,i2=0,cnt=0;
if (gmodem_At2bufFilter(g,"+CPBR=?","+CPBR:",out,sizeof(out))<=0) return -2;
gmodem_scan2(out,"(%d-%d),%d,%d",&min,&max,&i1,&i2);
sprintf(out,"+CPBR=%d,%d",min,max);
  int on_book_line(gmodem *g, char *line) {
       char num[256],name[256]; int numf,slot;
    if (4==gmodem_scan2(line,"%d,%S,%d,%U",&slot,num,&numf,name)) {
       printf(" %02d %s %s\n",slot,num,name);
       cnt++;
       }
  return 1;
  }
gmodem_At2Lines(g,out,"+CPBR:", on_book_line, g);
return gmodem_outf(g,1,"records:%d",cnt);
}

int gmodem_book_write(gmodem *g, int slot, uchar *num, uchar *name) {
char ucs[256],Name[512];
gmodem_logf(g,5,"begin_book_write: %d,%s,%s",slot,num,name);
int len = utf2gsm(ucs,name,-1);
   hexdump("UTF8",ucs,len);
   bin2hexstr(Name,ucs,len); // dump it...
if (gmodem_Atf(g,"+cpbw=%d,\"%s\",%d,\"%s\"",slot,num,
            num[0]=='+'?145:129, Name)>0) return gmodem_outf(g,1,"ok_set %d",slot);
return gmodem_errorf(g,-3,"error set slot %d",slot);
}

int gmodem_book_delete(gmodem *g, int slot) {
if (gmodem_Atf(g,"+cpbw=%d",slot)>0) return gmodem_outf(g,1,"deleted_ok %d",slot);
return gmodem_errorf(g,-3,"error dele slot %d",slot);
}


int gmodem_book_cmd(gmodem *g,uchar *cmd) {
int slot=-1;
if (lcmp(&cmd,"info")) {
    gmodem_Atf(g,"+CPBS=?"); // show all storages
    gmodem_Atf(g,"+CPBR=?"); // show record values
    gmodem_Atf(g,"+CSCS=\"UCS2\""); // set unicode output
   return 1;
   }
if (lcmp(&cmd,"ls") || lcmp(&cmd,"list")) {
   return gmodem_book_list(g);
   }
if (lcmp(&cmd,"read") || lcmp(&cmd,"get")) { // read value from a slot
    sscanf(cmd,"%d",&slot);
    return gmodem_book_read(g,slot);
   }
if (lcmp(&cmd,"write") || lcmp(&cmd,"set")) {
   int slot; char name[256],num[256];
   num[0]=0; name[0]=0;
   if (gmodem_scan2(cmd,"%d %S %S",&slot,num,name)!=3) {
      return gmodem_errorf(g,-2,"Expect <slot> <num> <name>");
      }
   //printf("NUM:<%s> NAME:<%s>\n",num,name);
   return gmodem_book_write(g,slot,num,name);
   }
if (lcmp(&cmd,"rm") || lcmp(&cmd,"del")) {
   int slot=-1;
   sscanf(cmd,"%d",&slot);
   return gmodem_book_delete(g,slot);
   }

return gmodem_errorf(g,-2,"gmodem_book info"); // not mine
}
