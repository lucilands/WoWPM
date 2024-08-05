CFLAGS=-Wall -Wextra -pedantic


all: wpm bin


wpm: main.c cJSON.c bin
	gcc $(CFLAGS) main.c cJSON.c -o bin/wpm

bin:
	mkdir -p bin

install: all
	cp bin/wpm /usr/bin/wpm
