CC = gcc
CFLAGS = -Wall -std=c99 -pedantic
MAIN = fs_emulator
OBJS = main.o task1.o

all: $(MAIN)

$(MAIN) : $(OBJS) task1.h
	$(CC) $(CFLAGS) -o $(MAIN) $(OBJS)

main.o: main.c task1.h
	$(CC) $(CFLAGS) -c main.c

task1.o: task1.c task1.h
	$(CC) $(CFLAGS) -c task1.c

clean :
	rm *.o $(MAIN) core
