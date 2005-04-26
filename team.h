// team.h

#ifndef __TEAM_H
#define __TEAM_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h> 
#include <ctype.h> 
#include "player.h"
#include "pitcher.h"

extern FILE *pbpfp,*stsfp,*cmdfp,*undofp,*output,*input;

// Linked list of players
struct pl_list {
	int ord;
	player *head;
	pl_list *next;
	};

// Linked list of pitchers
struct pit_list { 
	pitcher *head;
	pit_list *next;
	};

// Linked list of names
struct stat_list {
	int ord;
	char name[80];
	stat_list *next;
	};

class team {
private :
	char ibl[4];
	struct pl_list *lineup;			// ptr to linked list
	struct pl_list *current;		// ptr to current batter
	struct pit_list *pitchers;		// ptr to linked list
	struct pl_list *findord_pl(int);

	struct stat_list extra_stats[9];

public :
	team();
	team(char*);
	int make_lineups();
	int make_lineups_pit();
	void box_score(FILE*);
	char *posout(int);	
	player *next_up();
	player *up();
	char* insert(int, char* ="", char* ="");
	char* pos_change(int);
	int new_pit();
	char *nout();
	int score;
	int team_hits();
	int errors;
	int lob;
	pitcher *mound;
	player *findord(int);
	void print_lineup(void);
	player *back_up();
	void newstat(char*, int);
	void printstat(FILE*, int);
	int what_ord(player*);
	void decisions();
	void unearned(int);
	~team();
};

#endif
