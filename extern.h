#include "config.h"

#ifndef __EXTERN_H
#define __EXTERN_H

// global variables & functions in main

extern FILE	*pbpfp,		// play-by-play output file
		*stsfp,		// stats output file
		*cmdfp,		// commands output file
		*undofp,	// undo output file
		*output,	// where to direct output
		*input;		// where to read data

extern char filename[PATH_MAX];	// prefix for all output files
extern team *ibl[2];		// pointers to the two teams

extern void play();
extern void setup();
extern void setup( int );
extern int openfile( char* );

extern char* stripcr( char*, int );

#endif
