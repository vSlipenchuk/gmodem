LANG="C"

all:	gmodem

SRC=main.c common.c gmodem.c gmodem_apdu.c  gmodem_call.c \
 gmodem_cb.c  gmodem_cmd.c gmodem_crsm.c gmodem_device.c gmodem_http.c gmodem_book.c\
 gmodem_oper.c gmodem_scan.c gmodem_sms.c pa-e1550.c \
  phoenix_reader.c sec_packet.c sims_cfg.c sms_codec.c gmodem_gprs.c 
  
LIBS=-lcrypto -lpulse -lpthread -lreadline -ltermcap

gmodem: $(SRC)
		$(CC)  -I . -I ../vos -Wno-pointer-sign \
		$(SRC) -o gmodem $(LIBS)
		
clean:		
	rm gmodem
	

pa-e1550: pa-e1550.c
		$(CC) pa-e1550.c  -lpulse -o pa 
		
		
test_on__mt:
	./gmodem -D /dev/ttyUSB0 "http start" "sms on_mt ./on_sms"