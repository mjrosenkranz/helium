#include <cstdio>
#include <xcb/xcb.h>

#ifndef CLIENT_H
#define CLIENT_H

class Client {
	private:
		xcb_drawable_t id;
		int x, y;
		unsigned int w, h;
	public:
		unsigned int tag; // tag this window is assigned to
		unsigned int idx; // position in that vector
		Client(xcb_window_t, xcb_connection_t *);
		void print(void);
		void add_to_tag(int);
		void remove_tag(void);
		void remove_focus(void);
		bool match_id(xcb_drawable_t);
		void map(xcb_connection_t *);
		void focus(xcb_connection_t *);
		void raise(xcb_connection_t *);
};

#endif
