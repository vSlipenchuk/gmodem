LANG="C"

all:	gmodem

SRC=main.c common.c gmodem.c gmodem_apdu.c  gmodem_call.c \
  gmodem_http.c \
  gmodem_cb.c  gmodem_cmd.c gmodem_crsm.c gmodem_device.c  gmodem_book.c\
  gmodem_oper.c gmodem_scan.c gmodem_sms.c  pa-e1550.c \
  phoenix_reader.c sec_packet.c sims_cfg.c sms_codec.c gmodem_gprs.c gmodem_sim800.c gmodem_m590.c gmodem_hi2115.c
  
LIBS=-lcrypto  -lpthread -lreadline -ltermcap

sim800test:
	    ./gmodem -D/dev/ttyUSB0 -S19200 -L1 'sim800 wget http://api.thingspeak.com/update.json?api_key=Y31691C0KOZAI3Z1&field1=10&field2=20&field3=30'
         
         
gmodem: $(SRC)
		$(CC)  -I . -I ../vos -Wno-pointer-sign \
		$(SRC) -o gmodem $(LIBS)
		
clean:		
		rm -f gmodem *.o
	
clear:
		rm  -f gmodem *.o
	

pa-e1550: pa-e1550.c
		$(CC) pa-e1550.c  -lpulse -o pa 
		
		
test_on__mt:
	./gmodem -D /dev/ttyUSB0 "http start" "sms on_mt ./on_sms"