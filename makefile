
TARGET ?= $(BUILD_DIR)/perlin.out
BUILD_DIR ?= build

CC := gcc
SRCS := $(wildcard *.c)
OBJS := $(patsubst %.c,build/%.o,$(wildcard *.c))
CFLAGS := -Wall -g -O2
LDFLAGS := -lSDL2 -lm


$(BUILD_DIR)/%.o: %.c  $(BUILD_DIR)
	$(CC) -c -o $@ $< $(CFLAGS)

$(TARGET): $(OBJS)
	$(CC) $(OBJS) -o $@ $(LDFLAGS)
	
$(BUILD_DIR):
	mkdir $@

.PHONY: clean
clean:
	$(RM) $(TARGET) $(OBJS)
