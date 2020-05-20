#include <cstdio>
#include <xcb/xcb.h>

#ifndef CLIENT_H
#define CLIENT_H

class Client {
	private:
		xcb_drawable_t id;
		int x, y;
		unsigned int w, h;
		xcb_connection_t *conn;
	public:
		unsigned int tag; // tag this window is assigned to
		unsigned int idx; // position in that vector
		Client(xcb_window_t, xcb_connection_t *);
		void print(void);
		void change_tag(int);
		void remove_tag(void);
		void remove_focus(void);
		bool match_id(xcb_drawable_t);
		void map(void);
		void focus(void);
		void move_relative(int, int);
		void move_absolute(int, int);
};

#endif
