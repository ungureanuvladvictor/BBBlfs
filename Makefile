CC=gcc
CFLAGS=-c -Wall
OBJ_FOLDER=obj
all: boot

boot: main.o bootp.o udp.o ipv4.o ether2.o rndis.o
	mkdir -p bin
	$(CC) -I/usr/include/libusb-1.0 main.o ipv4.o bootp.o udp.o ether2.o rndis.o -lusb-1.0 -o bin/boot

main.o: main.c
	$(CC) $(CFLAGS) -I/usr/include/libusb-1.0 -lusb-1.0 main.c

bootp.o: bootp.c
	$(CC) $(CFLAGS) bootp.c

udp.o: udp.c
	$(CC) $(CFLAGS) udp.c

ipv4.o: ipv4.c
	$(CC) $(CFLAGS) ipv4.c

ether2.o: ether2.c
	$(CC) $(CFLAGS) ether2.c

rndis.o: rndis.c
	$(CC) $(CFLAGS) rndis.c

clean:
	rm -rf *o bin/boot
	