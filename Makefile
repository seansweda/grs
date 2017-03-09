# -DDEBUG turns on debugging output
# -DWIN32 for building on Windows

# if you're not using git comment out these lines
USE_GIT = -DUSE_GIT
GIT_COMMIT = git_commit.h

CC = gcc
CPP = g++
CFLAGS = -g -Wall -Wextra -Wconversion -fno-stack-protector $(USE_GIT) $(OSX_COMPAT) #-DDEBUG
LDFLAGS =

all: grs

tarball:
	rm -f grs.tar grs.tar.gz
	tar cf grs.tar README CHANGES TODO Makefile *.h *.cc getopt grscat
	gzip grs.tar

grs: Makefile main.o frame.o update.o queue.o team.o player.o pitcher.o
	$(CPP) $(CFLAGS) $(LDFLAGS) -o grs main.o frame.o update.o queue.o team.o player.o pitcher.o

main.o: Makefile main.cc frame.h team.h player.h pitcher.h config.h $(GIT_COMMIT)
	$(CPP) $(CFLAGS) -c main.cc

frame.o: Makefile frame.cc update.cc frame.h queue.h team.h player.h pitcher.h extern.h config.h
	$(CPP) $(CFLAGS) -c frame.cc

update.o: Makefile frame.cc update.cc frame.h queue.h team.h player.h pitcher.h extern.h config.h
	$(CPP) $(CFLAGS) -c update.cc

team.o: Makefile team.cc team.h player.h pitcher.h config.h
	$(CPP) $(CFLAGS) -c team.cc

queue.o: Makefile queue.cc queue.h pitcher.h
	$(CPP) $(CFLAGS) -c queue.cc

player.o: Makefile player.cc player.h config.h
	$(CPP) $(CFLAGS) -c player.cc

pitcher.o: Makefile pitcher.cc pitcher.h config.h
	$(CPP) $(CFLAGS) -c pitcher.cc

getopt.o: Makefile getopt/getopt.c getopt/getopt.h
	$(CC) $(CFLAGS) -c getopt/getopt.c

git_commit.h:
	bash git_commit.sh > git_commit.h

clean:
	rm -f *.o grs grs.tar.gz git_commit.h

