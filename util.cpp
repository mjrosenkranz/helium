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


Client *get_client(xcb_drawable_t *id) {
	
	for (int i = 0; i <= NUMTAGS; ++i) {
		// if it's empty say so
		if (tags[i].size() > 0) {
			// if not print all the windows
			for (Client *c : tags[i]) {
				if (c->match_id(*id)) {
					return c;
				}
			}
		}
	}

	return NULL;
}

