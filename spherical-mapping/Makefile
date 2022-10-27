.SUFFIXES: .c .h .o

CC	= g++
INCLDIR	= include/
BINDIR	= bin/
SRCDIR	= src/

_BIN	= a.out
BIN	= $(addprefix $(BINDIR), $(_BIN))

SRC	= $(wildcard src/*.cpp /usr/include/glad/glad.c)

CFLAGS = -Wall -pedantic -g -std=c++17 -I$(INCLDIR) -I/usr/include/stb -lm -pthread -lGLEW -lglfw -lGLU -lGL -lXrandr -lXi -lX11 -lrt -ldl -lstdc++fs

$(BINDIR)/$(_BIN): $(SRC)
	@$(CC) $(SRC) $(CFLAGS) -o $@

gendir: 
	@mkdir include bin src

clean: 
	@rm bin/a.out

.PHONY: gendir
.PHONY: clean
