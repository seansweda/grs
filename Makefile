CC = g++
CFLAGS = -g -Wall -Wextra -Wconversion #-DDEBUG
LDFLAGS =

all: grs

grs: main.o frame.o update.o queue.o team.o player.o pitcher.o 
	$(CC) $(CFLAGS) $(LDFLAGS) -o grs main.o frame.o update.o queue.o team.o player.o pitcher.o

main.o: main.cc frame.h team.h player.h pitcher.h limits.h
	$(CC) $(CFLAGS) -c main.cc

frame.o: frame.cc update.cc frame.h queue.h team.h player.h pitcher.h extern.h limits.h
	$(CC) $(CFLAGS) -c frame.cc

update.o: frame.cc update.cc frame.h queue.h team.h player.h pitcher.h extern.h limits.h
	$(CC) $(CFLAGS) -c update.cc

team.o: team.cc team.h player.h pitcher.h limits.h
	$(CC) $(CFLAGS) -c team.cc

queue.o: queue.cc queue.h pitcher.h
	$(CC) $(CFLAGS) -c queue.cc

player.o: player.cc player.h limits.h
	$(CC) $(CFLAGS) -c player.cc

pitcher.o: pitcher.cc pitcher.h limits.h
	$(CC) $(CFLAGS) -c pitcher.cc

clean: 
	rm -f *.o grs

