#include "queue.h"

queue::queue()
{ 
    first = last = 0;
}

    void 
queue::add( pitcher *new_entry )
{
   if ( this->last ) {
	item *temp = new item;
	temp->entry = new_entry;
	temp->next = 0;
	this->last->next = temp;
	this->last = temp;
    }
    else {
	item *temp = new item;
	temp->entry = new_entry;
	temp->next = 0;
	this->last = this->first = temp;
    }
}

    pitcher* 
queue::dequeue()
{
    if ( queue_empty() ) {
	fprintf( stderr, "attempting to dequeue non-existent runner!\n" );
	exit(1);
    }

    if ( this->first == 0 )
	return(0);
    else {
	item *temp1 = this->first;
	pitcher *temp2 = this->first->entry;

	if ( this->first == this->last )
	    this->first = this->last = 0;
	else
	    this->first = this->first->next;

	delete(temp1);
	return(temp2);
   }
}

    void 
queue::dump()
{
    item *curr = this->first;

    if ( curr ) {
	fprintf( stderr, "queue: " );
	while ( curr ) {
	    fprintf( stderr, "%s ", curr->entry->nout() );
	    curr = curr->next;
	}
	fprintf( stderr, "\n" );
    }
    else {
	fprintf( stderr, "queue:\n" );
    }
}


    int 
queue::queue_empty()
{
    if ( this->first == 0 )
	return(1);
    else
	return(0);
}

    void 
queue::clear()
{
    item *curr,
	 *prev;

    curr = prev = this->first;

    while( curr ) {
	curr = prev->next;
	delete(prev);
	prev = curr;
    }

    this->first = this->last = 0;
}

queue::~queue()
{
    clear();
}

