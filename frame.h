// $Id$

#ifndef FRAME_H
#define FRAME_H

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <unistd.h>
#include "player.h"
#include "pitcher.h"
#include "team.h"
#include "queue.h"
#include "extern.h"

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
	
	int runchck( char* );	// Parses baserunner advances
	void runcat( int );	// Does default baserunner advances
	int three();		// Checks for three operands

	void batterup();	// brings the next batter up 

	void cleanup();			// frees up memory for an undo
	int backup( char*, char* );	// backs up one line in .cmd file
					// returns 1 if it is a legal command
public :

	static int undo, cont, outs, atbat, inning, runs, linesize, errflag;
	static queue *runners;		// queue to handle inherited runners
	static player *onbase[4];	// array of pointers to batter & runners

	frame( char* );
	frame( team*, team*, FILE* );	// First constructor called
	int decode();			// decode the parsed command line
	int update();			// update the stats
	void help( char* );		// Prints helpful? messages
	void frameput();		// Outpus info useful to user
	void who_stat( int );

	static void print_linescore( FILE* );
	void outbuf( FILE*, char*, char *punc ="\0" );

	~frame();
};

#endif
