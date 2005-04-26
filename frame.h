// frame.h

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
#include "extern.h"

class frame {
private :

	char event[3];
	char location[20];
	char baserunning[20];
	char comment[80];

	team *bat,*pit;		// Dynamic batting/pitching team vars
	int runadv();		// Advances runners . . .
	void runstats(int fc = 0);	// Does runners stats and pbp
	void rbi();		// Does RBIs
	
	int runchck(char *);	// Parses baserunner advances
	void runcat(int);	// Does default baserunner advances
	int three();		// Checks for three operands

	char error[80];		// Error string

	void batterup();	// brings the next batter up 

	void cleanup();			// frees up memory for an undo
	int backup(char*,char*);	// backs up one line in .cmd file
					// returns 1 if it is a legal command
public :

	frame(char*);
	frame(team*,team*,FILE*);	// First constructor called
	int decode();			// decode the parsed command line
	int update();			// update the stats
	void help(char*);		// Prints helpful? messages
	void frameput(void);		// Outpus info useful to user
	void who_stat(int);

//	~frame() {if (output != stdout) fprintf(stderr,"%s %s %s\n",
//			event,location,baserunning);}
};

#endif
