#include <vector>
#include <xcb/xcb.h>
#include <iostream>

#include "util.h"
#include "client.h"
#include "helium.h"

void print_tags() {
	std::clog <<  "-----------\n";
	// loop through the tags
	for (int i = 0; i <= NUMTAGS; ++i) {
		// if it's empty say so
		if (tags[i].size() == 0) {
			std::clog << "Tag " << i << "empty\n";
		} else {
			// if not print all the windows
			std::clog << "Tag " << i << std::endl;
			for (Client *c : tags[i]) {
				c->print();
			}
		}
	}
	std::clog <<  "-----------\n";
}

void print_focus() {
	std::clog <<  "-----------\n";
	std::clog << "Focus queue:\n";

	for (Client *c : focus_queue) {
		c->print();
	}
	std::clog <<  "-----------\n";
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

