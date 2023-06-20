# Make sure the .ccls file matches what is used in the Makefile
# see: https://github.com/MaskRay/ccls/wiki/Project-Setup#ccls-file

RAYLIB_DIR=/home/marcel/workspace/repos/github/raylib/src/
GLFW_DIR=/home/marcel/workspace/repos/github/glfw/build/src/

CC=gcc
LD=gcc
CFLAGS=-I$(RAYLIB_DIR) -Wall -DPLATFORM_DESKTOP -g -Wshadow
LIBS=-L$(RAYLIB_DIR) \
	 -L$(GLFW_DIR) \
	 -lraylib -lGL -lm -lpthread -ldl -lrt -lX11 \
	 -lwayland-client -lwayland-cursor -lwayland-egl -lxkbcommon -latomic -lglfw3
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

