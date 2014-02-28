CC = gcc
FLAGS = -g -W -Wall -O2 -pthread -lpthread
TARGET = main

SRCFOLDER = src
INCFOLDER = includes
OBJFOLDER = obj
BINFOLDER = bin

SOURCES = $(wildcard $(SRCFOLDER)/*.c)
OBJECTS = $(patsubst %.c,$(OBJFOLDER)/%.o,$(notdir $(wildcard $(SRCFOLDER)/*.c)))

all: $(OBJECTS)
	$(CC) $(FLAGS) $(OBJECTS) $(LIBS) -o $(BINFOLDER)/$(TARGET)

dirs:
	@mkdir -p bin src includes obj

run: all
	$(BINFOLDER)/$(TARGET)

$(OBJFOLDER)/%.o: $(SRCFOLDER)/%.c
	$(CC) $(FLAGS) -c -o $@ $< -I/$(INCFOLDER)

clean:
	@rm -rf $(OBJFOLDER)/*.o $(BINFOLDER)/$(TARGET)
