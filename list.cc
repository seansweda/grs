#include <stdio.h>
#include <string.h>
#include "list.h"

list::list()
{
    first = last = cursor = NULL;
}

    void
list::add( char *str )
{
    node *temp = new node;
    temp->next = NULL;
    snprintf( temp->cmd, MAX_INPUT, "%s", str );

    if ( this->last ) {
	temp->prev = this->last;
	this->last->next = temp;
	this->last = temp;
	// set cursor to last node
	this->cursor = this->last;
    }
    else {
	this->last = this->first = this->cursor = temp;
    }
}

    const char*
list::peek()
{
    if ( this->cursor ) {
	return( this->cursor->cmd );
    }
    else {
	return( "" );
    }
}

    int
list::pop()
{
    if ( this->last ) {
	node *temp = this->last;
	this->last = this->last->prev;
	this->last->next = NULL;
	// set cursor to last node
	this->cursor = this->last;
	delete( temp );
	return( 0 );
    }
    else {
	return( 1 );
    }
}

    void
list::start()
{
    this->cursor = this->first;
}

    void
list::end()
{
    this->cursor = this->last;
}

    void
list::next()
{
    if ( this->cursor ) {
	this->cursor = this->cursor->next;
    }
}

    void
list::prev()
{
    if ( this->cursor ) {
	this->cursor = this->cursor->prev;
    }
}

    void
list::dump()
{
    fprintf( stderr, "list:" );
    this->start();
    while ( strlen( this->peek() ) > 0 ) {
	fprintf( stderr, " \'%s\'", this->peek() );
	this->next();
    }
    fprintf( stderr, "\n" );
}

list::~list()
{
    int done = this->pop();
    while ( done == 0 ) {
	done = this->pop();
    }
}

