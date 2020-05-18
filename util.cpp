#include "util.h"
#include <cstdio>
#include <vector>
#include <xcb/xcb.h>
#include "client.h"
#include "helium.h"

void print_tags() {
	fprintf(stderr, "-----------\n");
	// loop through the tags
	for (int i = 0; i <= NUMTAGS; ++i) {
		// if it's empty say so
		if (tags[i].size() == 0) {
			fprintf(stderr, "Tag %d empty\n", i);
		} else {
			// if not print all the windows
			fprintf(stderr, "Tag %d\n", i);
			for (Client *c : tags[i]) {
				c->print();
			}
		}
	}
	fprintf(stderr, "-----------\n");
}

void print_focus() {
	fprintf(stderr, "-----------\n");
	fprintf(stderr, "Focus queue:\n");

	for (Client *c : focus_queue) {
		c->print();
	}
	fprintf(stderr, "-----------\n");
}

void update_tag(int t) {
	std::vector<Client *>::iterator it = tags[t].begin();
	int i = 0;
	// loop through the vector and reassign the idx of each client
	for (; it != tags[t].end(); ++it, ++i) {
		Client *tmp = *it;
		tmp->idx = i;
	}
}

Client *get_client(xcb_drawable_t *id) {
	
	for (int i = 0; i <= NUMTAGS; ++i) {
		// if it's empty say so
		if (tags[i].size() > 0) {
			// if not print all the windows
			for (Client *c : tags[i]) {
				c->print();
				if (c->match_id(*id)) {
					return c;
				}
			}
		}
	}

	return NULL;
}

