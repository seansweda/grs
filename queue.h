// queue.h

#ifndef queue_h
#define queue_h

#include "pitcher.h"

struct item
{

      pitcher *entry;	
      item *next;

};


class queue
{

   private:
      item *first,
           *last;

   public:
      queue(); 

      void add(pitcher *new_entry);

      pitcher* dequeue();

      int queue_empty();

      void clear();

      void dump();		// print info about the queue
      
      ~queue() { clear(); }

};

#endif
