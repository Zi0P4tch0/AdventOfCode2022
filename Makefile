UNAME_S := $(shell uname -s)

EXTRA_CFLAGS := $(shell pkg-config --cflags glib-2.0)
LIBS := $(shell pkg-config --libs glib-2.0)
CFLAGS := -O2 -Wall -Wextra -pedantic -std=c11 -Iinclude $(EXTRA_CFLAGS)

ifeq ($(UNAME_S),Darwin)
    CC ?= clang
else
	CC ?= gcc
	CFLAGS += -march=native
endif

DAYS := $(shell seq -f "day%g" 1 14)

.PHONY: all clean benchmark

all: $(DAYS)

day%: obj/day%.o obj/io.o
	$(CC) $(CFLAGS) -o $@ $^ $(LIBS)

day9: obj/day9.o obj/io.o
	$(CC) $(CFLAGS) -o $@ $^ $(LIBS) $(shell pkg-config --libs ncurses)

day12: obj/day12.o obj/io.o
	$(CC) $(CFLAGS) -o $@ $^ $(LIBS) -fopenmp

day13: obj/day13.o obj/io.o obj/mpc.o
	$(CC) $(CFLAGS) -o $@ $^ $(LIBS)

obj/day%.o: src/day%.c obj
	$(CC) $(CFLAGS) -c -o $@ $< 

obj/day9.o: src/day9.c obj
	$(CC) $(CFLAGS) -c -o $@ $< $(shell pkg-config --cflags ncurses)

obj/day12.o: src/day12.c obj
	$(CC) $(CFLAGS) -c -o $@ $< -fopenmp

obj/day13.o: src/day13.c obj
	$(CC) $(CFLAGS) -c -o $@ $< -I 3rdparty/mpc

obj/io.o: src/io.c obj
	$(CC) $(CFLAGS) -c -o $@ $<

obj/mpc.o: 3rdparty/mpc/mpc.c obj
	$(CC) $(CFLAGS) -c -o $@ $<

obj:
	mkdir -p obj 2> /dev/null

clean:
	rm -rf $(DAYS) obj

benchmark: all
	for day in $(DAYS); do \
		hyperfine -N --warmup 10 "./$$day input/$$day.txt"; \
	done
