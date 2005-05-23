# $Id$

# -DDEBUG turns on debugging output
# -DWIN32 for building on Windows

CC = g++
CFLAGS = -g -Wall -Wextra -Wconversion #-DDEBUG
LDFLAGS =

all: grs

tarball:
	rm -f grs.tar grs.tar.gz
	tar cf grs.tar README CHANGES TODO Makefile *.h *.cc grscat
	gzip grs.tar

grs: Makefile main.o frame.o update.o queue.o team.o player.o pitcher.o 
	$(CC) $(CFLAGS) $(LDFLAGS) -o grs main.o frame.o update.o queue.o team.o player.o pitcher.o

main.o: Makefile main.cc frame.h team.h player.h pitcher.h config.h
	$(CC) $(CFLAGS) -c main.cc

frame.o: Makefile frame.cc update.cc frame.h queue.h team.h player.h pitcher.h extern.h config.h
	$(CC) $(CFLAGS) -c frame.cc

update.o: Makefile frame.cc update.cc frame.h queue.h team.h player.h pitcher.h extern.h config.h
	$(CC) $(CFLAGS) -c update.cc

team.o: Makefile team.cc team.h player.h pitcher.h config.h
	$(CC) $(CFLAGS) -c team.cc

queue.o: Makefile queue.cc queue.h pitcher.h
	$(CC) $(CFLAGS) -c queue.cc

player.o: Makefile player.cc player.h config.h
	$(CC) $(CFLAGS) -c player.cc

pitcher.o: Makefile pitcher.cc pitcher.h config.h
	$(CC) $(CFLAGS) -c pitcher.cc

clean: 
	rm -f *.o grs grs.tar.gz

