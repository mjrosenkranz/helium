/*
 * Linked list of windows and convinience functions
 */

#include "types.h"

#ifndef LIST
#define LIST

typedef struct item item_t;
struct item {
	client_t *client;
	item_t *prev;
	item_t *next;
};

void print_list(item_t *list) {
	item_t *item;
	int i;
	
	// loop through list and print them
	for (item = list, i = 0; item != NULL; item = item->next, i++) {
		printf("item #: %d, stored at %p\n", i, (void*) item);
		print_client(item->client);
	}
}

item_t *add_client(item_t **list) {
	item_t *item;

	// try to allocate a new item
	if ((item = (item_t*) malloc(sizeof(item_t))) == NULL)
		return NULL;

	// if the list is empty then we can safely set prev and next to NULL
	if (*list == NULL) {
		item->prev = item->next = NULL;
	} else {
		// set current item's next to the head
		item->next = *list;
		// set current's previous to NULL
		item->prev = NULL;
		// set the former head's previous to the current
		item->next->prev = item;
	}

	*list = item;
	return item;
}

#endif
