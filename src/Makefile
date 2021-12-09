GIT_VERSION := $(shell git describe --abbrev=4 --dirty --always)

CC=gcc
CFLAGS=-DGIT_VERSION=\"$(GIT_VERSION)\" -I. 
LDFLAGS=-Wl,-lsmbclient -ltalloc -ltdb -ldl

SOURCES=helpers.c smbresult.c smb.c main.c
OBJECTS=$(SOURCES:.c=.o)

all: shareenum

debug: CFLAGS += -g -O0 -DDEBUG -lresolv
debug: shareenum 

%.o: %.c
	$(CC) -c $< -o $@ $(CFLAGS)

shareenum: $(OBJECTS)
	$(CC)  -o shareenum $(OBJECTS) $(LDFLAGS)

clean:
	rm shareenum *.o
