#ifndef list_h
#define list_h

#include <stdlib.h>
#include "config.h"

struct node
{
    char cmd[MAX_INPUT];
    node *next;
    node *prev;
};


class list
{
    private:
	node *first;
	node *last;
	node *cursor;

    public:
	list();
	void add(char *str);
	const char* peek();
	int pop();
	void dump();
	~list();
	// move cursor
	void start();
	void end();
	void next();
	void prev();
};

#endif
