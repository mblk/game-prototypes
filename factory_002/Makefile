# Make sure the .ccls file matches what is used in the Makefile
# see: https://github.com/MaskRay/ccls/wiki/Project-Setup#ccls-file

CC=gcc
LD=gcc

RAYLIB_DIR=$(HOME)/workspace/repos/github/raylib/src/
RAYGUI_DIR=$(HOME)/workspace/repos/github/raygui/src/

CFLAGS_COMMON=-isystem $(RAYLIB_DIR) \
			  -isystem $(RAYGUI_DIR) \
	   		  -std=gnu99 \
			  -Wall \
			  -Wshadow \
			  -Werror \
			  -Wno-error=unused-variable \
			  -Wno-error=unused-function

CFLAGS_DEBUG=$(CFLAGS_COMMON) \
	         -g -O0 \
	         -fsanitize=address \
	         -fsanitize=undefined

CFLAGS_RELEASE=$(CFLAGS_COMMON) \
			   -O2

#CFLAGS=$(CFLAGS_DEBUG)
CFLAGS=$(CFLAGS_RELEASE)

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
	$(CC) -c $(CFLAGS) src/coord.c      -o obj/coord.o
	$(CC) -c $(CFLAGS) src/quad_tree.c  -o obj/quad_tree.o
	$(CC) -c $(CFLAGS) src/game_state.c -o obj/game_state.o
	$(CC) -c $(CFLAGS) src/renderer.c   -o obj/renderer.o
	$(CC) -c $(CFLAGS) src/raygui.c     -o obj/raygui.o
	$(LD) obj/*.o $(LIBS) -o $(BIN)

run:
	LSAN_OPTIONS=suppressions=asan_suppr.txt ./$(BIN)

