# Some notes on this makefile:
#
# DEBUG is for us (the creators/debuggers), leave it blank
# PORT has been used in the path for hard coding the runtime libraries
# 	into the executable.  In order to do this, add "-Bstatic" after
#	PORT (without the quotes).  What this does is allow you to
#	compile on one machine, but be able to move the executable
#	between many different machines of the same type.
# CC is your c++ compiler.  Change CC to whatever compiler you want to
#	use.  See the README file for compilers that have been used
#	successfully.


DEBUG =
PORT = 
CC = g++

grs: main.o frame.o update.o queue.o team.o player.o pitcher.o 
	$(CC) $(PORT) $(DEBUG) -o grs main.o frame.o update.o queue.o team.o player.o pitcher.o
main.o: main.cc frame.h team.h player.h pitcher.h
	$(CC) $(DEBUG) -c main.cc
frame.o: frame.cc update.cc frame.h queue.h team.h player.h pitcher.h extern.h
	$(CC) $(DEBUG) -c frame.cc
update.o: frame.cc update.cc frame.h queue.h team.h player.h pitcher.h extern.h
	$(CC) $(DEBUG) -c update.cc
team.o: team.cc team.h player.h pitcher.h
	$(CC) $(DEBUG) -c team.cc
queue.o: queue.cc queue.h pitcher.h
	$(CC) $(DEBUG) -c queue.cc
player.o: player.cc player.h 
	$(CC) $(DEBUG) -c player.cc
pitcher.o: pitcher.cc pitcher.h
	$(CC) $(DEBUG) -c pitcher.cc
