#ifndef __PITCHER_H
#define __PITCHER_H

#include <stdio.h>
#include <string.h>
#include <stdio.h>
#include <string.h>
#include "pitcher.h"
#include "config.h"

class pitcher {
private :
	char name[NAMELEN];
	char team[TEAMLEN];
	char rtn[TEAMLEN];
public :
	pitcher(char*, char*, char*, char);
	int out;
	int h;
	int r;
	int er;
	int bb;
	int k;
	int hr;
	int bf;

	char throws;
	void sout(FILE*);
	char *nout();
	char *tout();
	void hit(int);
	char dec;
	~pitcher();
};

#endif
