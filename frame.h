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

	int runchck( char* );		// Parses baserunner advances
	void runcat( int );		// Does default baserunner advances
	void runcat( const char* );	// append to baserunning
	int three();			// Checks for three operands

	void batterup( int nobf = 1 );	// brings the next batter up

	void cleanup();			// frees up memory for an undo
	int backup( char*, char* );	// backs up one line in .cmd file
					// returns 1 if it is a legal command
public :

	static int undo,
		   cont,
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
	int update();			// update the stats
	void help( char* );		// Prints helpful? messages
	void frameput();		// Outputs info useful to user
	void who_stat( int, int );	// add stat to player @fielding pos
	int outsonplay( char* );	// how many outs in this play
	int batterout( char* );		// did the batter make an out?
	char *outputstr;

	static void print_linescore( FILE* );
	void outbuf( const char*, const char* = "\0" );
	void putcmd();

	~frame();
};

#endif
