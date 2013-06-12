CC=gcc
CFLAGS=-c -Wall
all: boot

boot: main.o bootp.o udp.o ipv4.o
	$(CC) -I/usr/include/libusb-1.0 -lusb-1.0 main.o bootp.o udp.o -o bin/boot
	rm -rf *o

main.o: main.c
	$(CC) $(CFLAGS) -I/usr/include/libusb-1.0 main.c

bootp.o: bootp.c
	$(CC) $(CFLAGS) bootp.c

udp.o: udp.c
	$(CC) $(CFLAGS) udp.c

ipv4.o: ipv4.c
	$(CC) $(CFLAGS) ipv4.c

clean:
	rm -rf *o bin/boot