// player.h

#ifndef __PLAYER_H
#define __PLAYER_H

#include <stdio.h>
#include <string.h>
#include "player.h"
#include "limits.h"

class player {
private :
	char name[NAMELEN];
	char team[TEAMLEN];
	char rtn[TEAMLEN];
	char pos[POSLEN * 5];
public :
	player(char*);
	player(char*, char*, char*, char*);
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
	void move(char*);
	void sout(FILE*);
	char *nout();
	char *pout();
	void pa(char);
	void new_pos(char*);
//	~player() {fprintf(stderr,"deleted %s\n",this->nout());}
};

#endif

