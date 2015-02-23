AM_CPPFLAGS = -I$(top_srcdir)/include

bin_PROGRAMS = $(top_builddir)/bin/usb_flasher
__top_builddir__bin_usb_flasher_SOURCES = main.c arp.c bootp.c ether2.c ipv4.c rndis.c tftp.c udp.c utils.c
__top_builddir__bin_usb_flasher_CFLAGS = $(libusb_CFLAGS)
__top_builddir__bin_usb_flasher_LDADD = $(libusb_LIBS)