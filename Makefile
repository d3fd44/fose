CC      ?= gcc
CFLAGS  ?= -O3 -Wall -Wextra
SRCS    ?= main.c
UTIL    ?= util/ft.c
LDLIBS  ?= -lraylib -lm -lcjson

.PHONY: default run clean

default:
	$(CC) $(CFLAGS) $(SRCS) -o fose $(LDLIBS)

ft:
	$(CC) $(CFLAGS) $(UTIL) -o $@ -lm

run: default
	./fose ./examples/car.json

clean:
	rm -f fose ft
