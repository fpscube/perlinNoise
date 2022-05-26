
TARGET ?= build/perlin.out

CC := gcc
SRCS := $(wildcard *.c)
OBJS := $(patsubst %.c,build/%.o,$(wildcard *.c))
CFLAGS := -Wall -g -O2 $(shell sdl2-config --cflags)
LDFLAGS := $(shell sdl2-config --libs)

build/%.o: %.c
	$(CC) -c -o $@ $< $(CFLAGS)

$(TARGET): $(OBJS)
	$(CC) $(OBJS) -o $@ -lSDL2

.PHONY: clean
clean:
	$(RM) $(TARGET) $(OBJS)
