#ifndef __TEAM_H
#define __TEAM_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "config.h"
#include "player.h"
#include "pitcher.h"

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
	char name[NAMELEN];
	stat_list *next;
	};

class team {
private :
	char ibl[TEAMLEN];
	struct pl_list *lineup;			// ptr to linked list
	struct pl_list *current;		// ptr to current batter
	struct pit_list *pitchers;		// ptr to linked list
	struct pl_list *findord_pl(int);

	struct stat_list extra_stats[9];	// 0: E
						// 1: PB
						// 2: GIDP
						// 3: SH
						// 4: SF
						// 5: HBP
						// 6: WP
						// 7: IBB
						// 8: BALK

public :
	team(int);
	team(char*);

	int make_lineups();
	int make_lineups_pit();
	int new_pit();
	void pos_change(int, char**);
	void insert(int, char**, const char* = "\0", const char* = "\0");

	void print_lineup(void);
	void check_defense(void);
	int verify_pos(const char*);
	char *posout(int);
	char *nout();
	player *up();
	player *next_up();
	pitcher *mound;
	int findord(player*);
	player *findord(int);

	int score;
	int team_hits();
	int errors;
	int lob;
	void newstat(char*, int);
	void decisions();
	void unearned(int);
	void box_score(FILE*);
	void printstat(FILE*, int);
	~team();
};

#endif
