CC = gcc
CFLAGS = -g -W -Wall -O2 
CFLAGS += `pkg-config --cflags libusb-1.0`
LIBS += `pkg-config --libs libusb-1.0`
TARGET = usb_flasher

SRCFOLDER = src
INCFOLDER = includes
OBJFOLDER = obj
BINFOLDER = bin

SOURCES = $(wildcard $(SRCFOLDER)/*.c)
OBJECTS = $(patsubst %.c,$(OBJFOLDER)/%.o,$(notdir $(wildcard $(SRCFOLDER)/*.c)))

all: $(OBJECTS)
	$(CC) $(CFLAGS) $(OBJECTS) $(LIBS) -o $(BINFOLDER)/$(TARGET)

run: all
	$(BINFOLDER)/$(TARGET)

$(OBJFOLDER)/%.o: $(SRCFOLDER)/%.c
	@mkdir -p ${OBJFOLDER}
	$(CC) $(CFLAGS) -c -o $@ $< -I$(INCFOLDER)

clean:
	@rm -rf $(OBJFOLDER) $(BINFOLDER)/$(TARGET)
