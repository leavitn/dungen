# roguelike makefile

CC=gcc
CFLAGS = -lncurses -Wall
DEPS = rl.h
OBJ = simpledungen.o util.o pf.o

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CLFAGS) # so that header changes get accounted for

rlmake: $(OBJ)
	$(CC) -o dungen $(OBJ) $(CFLAGS)
