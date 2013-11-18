#
# Makefile for cHat
#

CC=gcc
CFLAGS= -Wall -Wextra -Werror -g -c -O2 -std=gnu99
LDFLAGS= -lncurses
SOURCES= cHat.c ui.c
OBJECTS=$(SOURCES:.c=.o)
EXECUTABLE=cHat

all: $(SOURCES) $(EXECUTABLE)
		
$(EXECUTABLE): $(OBJECTS) 
		$(CC) $(LDFLAGS) $(OBJECTS) -o $@

.c.o:
		$(CC) $(CFLAGS) $< -o $@

clean:
	rm -f *~ *.o $(EXECUTABLE)

