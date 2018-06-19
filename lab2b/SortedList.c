// NAME: Yanyin Liu
// EMAIL: yanyinliu8@gmail.com
// ID: 604952257

#include "SortedList.h"
#include <stdlib.h>
#include <stdio.h>
#include <sched.h>

int opt_yield;
void SortedList_insert(SortedList_t *list, SortedListElement_t *element)
{
	if(list == NULL || element == NULL) return;
	SortedListElement_t *curr=list;

	// yield
	if(opt_yield & INSERT_YIELD)
	{
		sched_yield();
	}
	if(curr->next == NULL) // (one one) last element of the list
	{
		curr->next = element;
		element->prev = curr;
		element->next = NULL;
		return;
	}
	else curr = curr->next;
	while(curr->next != NULL) // keep tracking the next element in the list
	{
		// if Return value < 0 then it indicates str1 is less than str2.
		// if Return value > 0 then it indicates str2 is less than str1.
		// if Return value = 0 then it indicates str1 is equal to str2.

		// stop comparing when found element should be place before the curr element
		// then means strcmp will return >0, then we will insert element before curr and after curr->prev
		if(strcmp(curr->key, element->key)>0) break; 
		curr = curr->next;
	}
	if(strcmp(curr->key, element->key)>0) // insert element before curr and after curr->prev
	{
		element->next = curr;
		element->prev = curr->prev;
		// curr->prev = element;
		curr->prev->next = element;
		curr->prev = element;

		
	}
	else // exist loop because of end of the list 
	{
		element->next = curr->next;
		element->prev = curr;
		curr->next = element;
	}
	return;

}



int SortedList_delete( SortedListElement_t *element)
{
	int checkp=0, checkn=0;
	// if(element == NULL || element->key == NULL) return 1;

	// Before doing the deletion, we check to make sure that
 	// next->prev and prev->next both point to this node
 	SortedListElement_t *previousele = element->prev;
 	SortedListElement_t *nextele = element->next;
 	if(opt_yield & DELETE_YIELD)
	{
		sched_yield();
	}
 	if((nextele) && (nextele->prev))
 	{
 		if(nextele->prev == element) checkn=1;
 	}
 	if((previousele) && (previousele->next))
 	{
 		if(previousele->next == element) checkp=1;
 	}

 	//  next->prev and prev->next do not both point to this node, elements
 	if((checkp == 0) && (checkn == 0))
 	{
 		return 1; // corrtuped prev/next pointers
 	}

 	if(checkp)
 	{
 		element->prev->next = element->next;
 	}
 	if(checkn)
 	{
 		element->next->prev = element->prev;
 	}

 	return 0; //return 0: element deleted successfully
}


SortedListElement_t *SortedList_lookup(SortedList_t *list, const char *key)
{
	if(list == NULL || key == NULL) return NULL;
	SortedListElement_t *curr = list;

	if(opt_yield & LOOKUP_YIELD) sched_yield();
	curr = curr->next;
	while(curr != NULL)
	{
		if(strcmp(curr->key, key) == 0) return curr;
		// if(opt_yield & LOOKUP_YIELD) sched_yield();
		curr = curr->next;
	}
	return NULL;
}


int SortedList_length(SortedList_t *list)
{
	if(list == NULL) return -1;
	int length = 0;
	SortedListElement_t *curr = list;

	if(opt_yield & LOOKUP_YIELD) sched_yield();
	curr = curr->next;
	while(curr != NULL)
	{
		// if((curr->next->prev != curr) || curr->prev->next !=curr) return -1;
		if(curr->prev->next !=curr) return -1;
		length++;
		curr = curr->next;
	}
	return length;
}



