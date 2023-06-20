# Make sure the .ccls file matches what is used in the Makefile
# see: https://github.com/MaskRay/ccls/wiki/Project-Setup#ccls-file

CC=gcc
LD=gcc

RAYLIB_DIR=$(HOME)/workspace/repos/github/raylib/src/

CFLAGS=-I$(RAYLIB_DIR) \
	   -g -O0 \
	   -std=c99 \
	   -Wall -Wshadow \
	   -fsanitize=address \
	   -fsanitize=undefined

LIBS=-L$(RAYLIB_DIR) \
	 -lasan \
	 -lubsan \
	 -lraylib -lGL -lm -lpthread -ldl -lrt \
	 -lwayland-client -lwayland-cursor -lwayland-egl \
	 -lxkbcommon -latomic

BIN=proto

all: clean build run

clean:
	-rm obj/*.o
	-rm $(BIN)

build:
	$(CC) -c $(CFLAGS) src/main.c       -o obj/main.o
	$(CC) -c $(CFLAGS) src/game_state.c -o obj/game_state.o
	$(LD) obj/*.o $(LIBS) -o $(BIN)

run:
	./$(BIN)

