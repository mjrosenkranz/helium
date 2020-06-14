#include <xcb/xcb.h>
#include <vector>

#ifndef CLIENT_H
#define CLIENT_H

class Client {
	private:
		int x, y;
		unsigned int w, h;
		xcb_connection_t *conn;
		void snap();
	public:
		xcb_drawable_t id;
		xcb_drawable_t dec;
		unsigned int tag; // tag this window is assigned to
		Client(xcb_window_t, xcb_connection_t *);
		void unmanage();
		void kill();
		void print(std::string);
		void change_tag(int);
		void remove_tag();
		void remove_focus();
		bool match_id(xcb_drawable_t);
		void map();
		void focus();
		void unfocus();
		void decorate();
		void move_relative(int, int);
		void move_absolute(int, int);
		bool resize_relative(std::string, int);
		void resize_to(int, int);
		void resize_mouse(int, int, int, int);
		void set_visible(bool);
		std::vector<int> get_corners();
};

#endif
