#ifndef __PLAYER_H
#define __PLAYER_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "player.h"
#include "config.h"

class player {
private :
	char name[NAMELEN];
	char team[TEAMLEN];
	char rtn[TEAMLEN];
	char *pos;
public :
	player(char*, char*, char*, const char*);
	int posn;
	int ab;
	int h;
	int r;
	int rbi;
	int b2;
	int b3;
	int hr;
	int bb;
	int k;
	int sb;
	int cs;
	int pal;
	int par;

	void hit(int);
	void sout(FILE*);
	char *nout();
	char *pout();
	void pa(char);
	void new_pos(char*);
	int getpos(char*);
	~player();
};

#endif

