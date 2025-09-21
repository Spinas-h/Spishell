CC = gcc
VERSION = 1.0
CFLAGS = -Wall -g -DVERSION=\"$(VERSION)\"
SRC = main.c exec.c
OBJ = $(SRC:.c=.o)
TARGET = spishell

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) -o $@ $(OBJ)

%.o: %.c spishellz.h
	$(CC) $(CFLAGS) -c $<

install: $(TARGET)
	sudo cp $(TARGET) /usr/local/bin/

uninstall:
	sudo rm -f /usr/local/bin/$(TARGET)

clean:
	rm -f $(OBJ) $(TARGET) explain each line for me also what's this code called is it a bashscript???
