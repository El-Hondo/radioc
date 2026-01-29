CC = gcc
CFLAGS = -Wall -Wextra -Iinclude -g
LIBS = -lcurl -lvlc -lm

SRC = $(wildcard src/*.c) lib/cJSON.c
OBJ = $(patsubst %.c, obj/%.o, $(notdir $(SRC)))

# Helper to map source files to obj correctly
# We are flattening all objects into obj/ for simplicity
# This requires VPATH to find sources in src/ and lib/
VPATH = src:lib

TARGET = viberadio

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^ $(LIBS)

obj/%.o: %.c | obj
	$(CC) $(CFLAGS) -c $< -o $@

obj:
	mkdir -p obj

clean:
	rm -rf obj $(TARGET)

.PHONY: all clean
