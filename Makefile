CC = gcc
CFLAGS = -std=c99 -Wall -Wextra -Iinclude

SRC = $(wildcard src/*.c)
OBJ = $(patsubst src/%.c,obj/%.o,$(SRC))
BIN = bin/filesys

all: $(BIN)

$(BIN): $(OBJ)
	@mkdir -p bin
	$(CC) $(CFLAGS) -o $@ $^

obj/%.o: src/%.c
	@mkdir -p obj
	$(CC) $(CFLAGS) -c $< -o $@

run: $(BIN)
	./$(BIN) fat32.img

clean:
	rm -rf obj bin

.PHONY: all run clean
