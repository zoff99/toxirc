CC = x86_64-w64-mingw32-gcc
CFLAGS = -Wall -Wextra -std=gnu99 $(wildcard /tmp/static_prefix/lib/*.a)
LDFLAGS = $(wildcard /tmp/static_prefix/lib/*.a)

SRC = $(wildcard src/*.c ) $(wildcard src/*/*.c) third-party/minini/dev/minIni.c
OBJ = $(SRC:.c=.o)
HEADERS = $(wildcard src/*.h) $(wildcard src/*/*.h)

DEBUG = 0
PREFIX = /usr/local
EXECUTABLE = toxirc

ifeq ($(DEBUG), 1)
	CFLAGS += -g
endif

CFLAGS += -I /tmp/static_prefix/include -D__USE_MINGW_ANSI_STDIO=1

all: $(SRC) $(EXECUTABLE)

$(EXECUTABLE): $(OBJ) $(HEADERS)
	$(CC) $(OBJ) $(LDFLAGS) -o $(EXECUTABLE) -lwsock32 -lws2_32 -lpthread -static-libgcc -liphlpapi -L /root/prefix/x86_64/lib -lsodium -lmsgpackc -lssp

.o:
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm src/*.o src/*/*.o $(EXECUTABLE)

rebuild: clean all

install: all
	@echo "Installing binary..."
	install -m 0755 $(EXECUTABLE) $(PREFIX)/bin/$(EXECUTABLE)

deps:
	@git submodule init
	@git submodule update

.PHONY: all clean
