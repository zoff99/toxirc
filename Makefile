CC = gcc
CFLAGS = -Wall -Wextra -std=gnu99 $(shell pkg-config --cflags libtoxcore) -g -O3 -fstack-protector-all -fPIC -Wno-unused-function -Wno-unused-variable -Wno-unused-function -Wno-unused-parameter -Wno-unused-value

LDFLAGS = $(shell pkg-config --libs libtoxcore) -g -O3 -fstack-protector-all -fPIC

SRC = $(wildcard src/*.c ) $(wildcard src/*/*.c) third-party/minini/dev/minIni.c
OBJ = $(SRC:.c=.o)
HEADERS = $(wildcard src/*.h) $(wildcard src/*/*.h)

PREFIX = /usr/local
EXECUTABLE = toxirc

all: $(SRC) $(EXECUTABLE)

$(EXECUTABLE): $(OBJ) $(HEADERS)
	$(CC) $(OBJ) $(LDFLAGS) -o $(EXECUTABLE) -l:libsodium.a

.o:
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm src/*.o src/*/*.o $(EXECUTABLE)

rebuild: clean all

install: all
	@echo "Installing binary..."
	install -m 0755 $(EXECUTABLE) $(PREFIX)/bin/$(EXECUTABLE)

.PHONY: all clean
