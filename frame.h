#ifndef FRAME_H
#define FRAME_H

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include "player.h"
#include "pitcher.h"
#include "team.h"
#include "queue.h"

class frame {
private :

	char *event;
	char *location;
	char *baserunning;
	char *comment;
	char *error;			// Error string

	static int **linescore;
	static char *buffer;		// output buffer

	team *bat, *pit;		// Dynamic batting/pitching team vars
	int runadv();			// Advances runners . . .
	void runstats( int fc = 0 );	// Does runners stats and pbp
	void rbi();			// Does RBIs

	int base( const char );		// return base code (e.g. b=0, h=4)
	void runcat( int );		// does default baserunner advances
	void runcat( const char* );	// append to baserunning
	int runcheck( char*, int copy = 1 );	// parses baserunner advances

	void batterup( int nobf = 1 );	// brings the next batter up

	void cleanup();			// frees up memory for an undo
public :

	static int cont,
		   outs,
		   atbat,
		   inning,
		   runs,
		   linesize,
		   errflag,
		   count;
	static queue *runners;		// queue to handle inherited runners
	static player *onbase[4];	// array of pointers to batter & runners

	frame( char* );
	frame( team*, team* );		// First constructor called
	int decode();			// decode the parsed command line
	int validate( const char* );	// valid command?
	int update();			// update the stats
	void help( char* );		// Prints helpful? messages
	void frameput();		// Outputs info useful to user
	void who_stat( int, int );	// add stat to player @fielding pos
	int outsonplay( char* );	// how many outs in this play
	int batterout( char* );		// did the batter make an out?
	int get_spot();			// prompt user for spot in order
	char *outputstr;

	static void print_linescore( FILE* );
	void outbuf( const char*, const char* = "\0" );
	void putcmd();

	~frame();
};

#endif
