// $Id$

#include "limits.h"

#ifndef __EXTERN_H
#define __EXTERN_H

// global variables & functions in main

extern FILE *stsfp,*pbpfp,*undofp,*cmdfp,*output,*input;

extern char filename[PATH_MAX];

extern char *buffer;

extern team *ibl[2];
extern queue *runners;
extern player *onbase[4];

extern int errflag;
extern int undo, cont, outs, atbat, inning, runs, linesize;
extern int **linescore;

extern void play();
extern void setup();
extern void setup(int);
extern int openfile(char*);

extern void outbuf(FILE*,char*,char *punc ="\0");

#endif
