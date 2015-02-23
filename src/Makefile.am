AM_CPPFLAGS = -I$(top_srcdir)/include
bin_PROGRAMS = usb_flasher

usb_flasher_CFLAGS = $(libusb_CFLAGS)
usb_flasher_LDADD = $(libusb_LIBS)
usb_flasher_SOURCES = main.c arp.c bootp.c ether2.c ipv4.c rndis.c tftp.c udp.c utils.c
