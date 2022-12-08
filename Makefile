UNAME_S := $(shell uname -s)

EXTRA_CFLAGS := $(shell pkg-config --cflags glib-2.0)
LIBS := $(shell pkg-config --libs glib-2.0)
CFLAGS := -O2 -Wall -Wextra -pedantic -std=c11 -Iinclude $(EXTRA_CFLAGS)

ifeq ($(UNAME_S),Darwin)
    CC := clang
else
	CC := gcc
	CFLAGS += -march=native
endif

DAYS := $(shell seq -f "day%g" 4 8)

.PHONY: all clean

all: $(DAYS)

day%: obj/day%.o obj/io.o
	$(CC) $(CFLAGS) -o $@ $^ $(LIBS)

obj/day%.o: src/day%.c obj
	$(CC) $(CFLAGS) -c -o $@ $< 

obj/io.o: src/io.c obj
	$(CC) $(CFLAGS) -c -o $@ $<

obj:
	mkdir -p obj 2> /dev/null

clean:
	rm -rf $(DAYS) obj