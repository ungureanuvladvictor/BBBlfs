CC = gcc
CFLAGS = -g -W -Wall -O2 
CFLAGS += `pkg-config --cflags libusb-1.0`
LIBS += `pkg-config --libs libusb-1.0`
TARGET = usb_flasher
CROSS_COMPILE ?= arm-linux-gnueabihf-

SRCFOLDER = src
INCFOLDER = includes
OBJFOLDER = obj
BINFOLDER = bin
TOOLFOLDER = tools
BINTESTFOLDER = bintest

SOURCES = $(wildcard $(SRCFOLDER)/*.c)
OBJECTS = $(patsubst %.c,$(OBJFOLDER)/%.o,$(notdir $(wildcard $(SRCFOLDER)/*.c)))

all: $(OBJECTS)
	$(CC) $(CFLAGS) $(OBJECTS) $(LIBS) -o $(BINFOLDER)/$(TARGET)

run: all
	$(BINFOLDER)/$(TARGET)

$(OBJFOLDER)/%.o: $(SRCFOLDER)/%.c
	@mkdir -p ${OBJFOLDER}
	$(CC) $(CFLAGS) -c -o $@ $< -I$(INCFOLDER)

uboot:	all
	@mkdir -p $(CURDIR)/$(BINTESTFOLDER)
	@cp -p $(BINFOLDER)/$(TARGET) $(BINFOLDER)/*.sh $(BINTESTFOLDER)/
	$(TOOLFOLDER)/uboot.sh $(CURDIR)/$(BINTESTFOLDER)
	@echo "Go to $(BINTESTFOLDER) directory to test the new uboot and spl images"

fit:	uboot
	$(TOOLFOLDER)/fit.sh $(CURDIR)/$(BINTESTFOLDER)
	@echo "Go to $(BINTESTFOLDER) directory to test the new fit image"

clean:
	@rm -rf $(OBJFOLDER) $(BINFOLDER)/$(TARGET) $(BINTESTFOLDER)

print_cross_cc:
	@echo $(CROSS_COMPILE)
