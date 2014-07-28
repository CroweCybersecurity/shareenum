GIT_VERSION := $(shell git describe --abbrev=4 --dirty --always)

CC=gcc
CFLAGS=-DGIT_VERSION=\"$(GIT_VERSION)\" -I/usr/local/samba/include -I. 
LDFLAGS=-Wl,-rpath=/usr/local/shareenum/lib -L/usr/local/samba/lib/private -L/usr/local/samba/lib -lsmbclient -ltalloc -ltdb -ldl

SOURCES=helpers.c smbresult.c smb.c main.c
OBJECTS=$(SOURCES:.c=.o)

all: shareenum

debug: CFLAGS += -g -O0 -DDEBUG -lresolv
debug: shareenum 

shareenum: $(OBJECTS)
	$(CC) $(LDFLAGS) -o shareenum $(OBJECTS)

%.o: %.c
	$(CC) -c $< -o $@ $(CFLAGS)

clean:
	rm shareenum *.o
