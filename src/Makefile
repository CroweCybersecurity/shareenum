MAJOR_VERSION=2
MINOR_VERSION=0

BUILD_FILE=.build
BUILD_NUM=$(shell cat $(BUILD_FILE))

CC=gcc

CFLAGS=-DMAJOR_REVISION=$(MAJOR_VERSION) -DMINOR_REVISION=$(MINOR_VERSION) -DBUILD_NUMBER=$(BUILD_NUM) -I/usr/include/samba-4.0/ -I. -lsmbclient -ltalloc -ltdb -ldl
LDFLAGS=


SOURCES=helpers.c smbresult.c smb.c main.c
OBJECTS=$(SOURCES:.c=.o)

all: shareenum

release: $(BUILD_FILE) shareenum

debug: CFLAGS += -g -DDEBUG -lresolv
debug: shareenum $(BUILD_FILE)

shareenum: $(OBJECTS)
	$(CC) $(LDFLAGS) -o shareenum $(OBJECTS) $(CFLAGS)

%.o: %.c
	$(CC) $(LDFLAGS) -c $< -o $@ $(CFLAGS)

clean:
	rm shareenum *.o

$(BUILD_FILE): $(OBJECTS)
	@echo "Incrementing build number."
	@if ! test -f $(BUILD_FILE); then echo 0 > $(BUILD_FILE); fi
	@echo $$(($(BUILD_NUM) + 1)) > $(BUILD_FILE)
