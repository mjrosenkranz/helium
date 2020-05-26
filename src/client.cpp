#include <xcb/xcb.h>
#include <cmath>
#include <xcb/xcb_aux.h>
#include <deque>
#include <cstdlib>
#include <iostream>

#include "client.h"
#include "helium.h"
#include "util.h"


Client::Client (xcb_window_t _id, xcb_connection_t *_conn) {
	id = _id;
	tag = -1;
	conn = _conn;

	dec = xcb_generate_id(_conn);

	// set geometry to dummy values
	x = 0;
	y = 0;
	w = 100;
	h = 100;
	// get the window geometry
	xcb_get_geometry_reply_t *geom = xcb_get_geometry_reply(conn,
			xcb_get_geometry(conn, id), NULL);


	int offset = config["outer_width"] + config["inner_width"];

	if (geom != NULL) {
		x = geom->x;
		y = geom->y;
		w = geom->width + (2 * offset);
		h = geom->height + (2 * offset);
	}

	free(geom);

	// snap the window's geometry
	snap();
	// apply snap to client
	uint32_t values[] = {
		(w - 2 * offset),
		(h - 2 * offset)};

	xcb_configure_window (conn, id,
			XCB_CONFIG_WINDOW_WIDTH
			| XCB_CONFIG_WINDOW_HEIGHT,
			values);
	uint32_t mask = XCB_CW_BACK_PIXEL;
	values[0] = config["focus_color"];


	xcb_create_window(
			conn, XCB_COPY_FROM_PARENT,
			dec, screen->root, //parent
			x, y,
			w, h,
			0, XCB_WINDOW_CLASS_INPUT_OUTPUT, //class
			screen->root_visual, // visual
			mask, values
	);

	values[0] = XCB_EVENT_MASK_BUTTON_PRESS
		| XCB_EVENT_MASK_SUBSTRUCTURE_NOTIFY;
	mask = XCB_CW_EVENT_MASK;
	xcb_change_window_attributes(conn,
					dec, mask, values);



	// resize child window

	xcb_reparent_window(conn,
	      id, dec, offset, offset);

	// let us get events for this window
	// this gets the button with not modifier
	xcb_grab_button(conn, false, id, XCB_EVENT_MASK_BUTTON_PRESS,
									// stop the cursor  continue the keyboard
	                XCB_GRAB_MODE_SYNC, XCB_GRAB_MODE_ASYNC,
	                XCB_NONE, XCB_NONE, XCB_BUTTON_INDEX_ANY, XCB_MOD_MASK_ANY);
}


void Client::unmanage(void) {
	remove_focus();
	// ungrab button
	xcb_ungrab_button(conn, XCB_BUTTON_INDEX_ANY, id, XCB_MOD_MASK_ANY);
	// remove the window from tags
	remove_tag();
	unfocus();

	// destroy the decoration
	xcb_destroy_window(conn, dec);
}


void Client::kill(void) {
	std::clog << "killing client\n";
	xcb_kill_client(conn, id);
	xcb_flush(conn);
}

void Client::print(std::string pre) {
	std::clog << pre << " id: " << std::hex << id
		<< " tag: " << std::hex << tag << std::dec
		<< " x: " << x
		<< " y: " << y
		<< " w: " << w
		<< " h: " << h
		<< std::endl;
}



bool Client::match_id(xcb_drawable_t _id) {
	return id == _id || dec == _id;
}


void Client::map(void) {
	xcb_map_window(conn, dec);
	xcb_map_window(conn, id);
}


void Client::change_tag(int t) {
	// check if we have a valid tag number
	if ( t > NUMTAGS + 1 || t < 0 )
		return;

	// check if we are already on that tag
	if ( t == (int) tag )
		return;
	
	// if the tag has been asigned before
	if ((int) tag != -1) {
	// remove from current tag
		for (int i = 0; i <(int) tags[tag].size(); ++i) {
			if (tags[tag].at(i) == this) {
				tags[tag].erase(tags[tag].begin() + i);
				break;
			}
		}
		
	}

	// add to new tag
	tag = t;
	tags[tag].push_back(this);
	std::clog << "added to tag " << t << std::endl;

	// if the tag is hidden we want to do the same
	if (!visible[t]) {
		// we want to no longer be in the focus list
		set_visible(false);
	}

	print("change tag");
}


void Client::remove_tag(void) {
		for (int i = 0; i < (int) tags[tag].size(); ++i) {
			if (tags[tag].at(i) == this) {
				tags[tag].erase(tags[tag].begin() + i);
				break;
			}
		}
}

void Client::focus(void) {
	std::clog << "focusing: " << std::hex << id << std::endl;

	// remove this window from the focuse queue
	for (int i = 0; i < (int) focus_queue.size(); i++) {
		if (focus_queue[i] == this)
			focus_queue.erase(focus_queue.begin() + i);
	}


	// unfocus the focused window
	Client *focused = focus_queue.front();
	if (focus_queue.size() > 0 && focused != this)
		focused->unfocus();
	
	// add to the front of the focus queue
	focus_queue.push_front(this);

	uint32_t mask = XCB_CONFIG_WINDOW_STACK_MODE;
	uint32_t values[] = { XCB_STACK_MODE_ABOVE };

	decorate(config["focus_color"]);
	// raise the window and change the color
	xcb_configure_window(conn, dec, mask, values);
	//xcb_configure_window(conn, id, mask, values);


	// focus the client
	xcb_set_input_focus(conn, XCB_INPUT_FOCUS_POINTER_ROOT, id,
			XCB_CURRENT_TIME);

	print_focus();
	xcb_flush(conn);
}

void Client::unfocus(void) {
	decorate(config["unfocus_color"]);
}

void Client::remove_focus(void) {

	// remove this window from the queue
	for (int i = 0; i <(int) focus_queue.size(); i++) {
		if (focus_queue[i] == this) {
			std::clog << "removing self from focus list\n";
			focus_queue.erase(focus_queue.begin() + i);
		}
	}


	// if the list isnt empty then focus the next window
	if (focus_queue.size() > 0) {
		Client *c = focus_queue.front();
		c->focus();
	}
}
void Client::decorate(unsigned int color) {

	xcb_unmap_window(conn, dec);

	xcb_pixmap_t pixmap = xcb_generate_id(conn);
    xcb_create_pixmap(conn, screen->root_depth, pixmap, dec, w, h);
    // make graphics context 4 drawing
    xcb_gcontext_t gc = xcb_generate_id(conn);
    xcb_create_gc(conn, gc, pixmap, XCB_GC_FOREGROUND, &color);

    // draw outer rect
    xcb_rectangle_t rects[] = {
		{0, 0, (uint16_t) w, (uint16_t) h}
	};
	xcb_poly_fill_rectangle(conn, pixmap, gc, 1, rects);

    // draw inner rect
    rects[0] = {
    	(int16_t) config["outer_width"],
    	(int16_t) config["outer_width"],
    	(uint16_t) (w - (2 * config["outer_width"])),
    	(uint16_t) (h - (2 * config["outer_width"]))
	};
	unsigned int tmp = 0x00ff00;
	xcb_change_gc(conn, gc, XCB_GC_FOREGROUND, &tmp);
	xcb_poly_fill_rectangle(conn, pixmap, gc, 1, rects);

	uint32_t mask = XCB_CW_BACK_PIXMAP;
	uint32_t values[] = {pixmap};

	// raise the window and change the color
	xcb_change_window_attributes(conn, dec, mask, values);
	//std::clog << "changing window color to " << std::hex << color << std::endl;

	xcb_flush(conn);
	xcb_map_window(conn, dec);
}


void Client::move_relative(int _x, int _y) {
	x += _x;
	y += _y;

	// std::clog << x << " vs " << nx << std::endl;
	snap();

	int values[] = { x, y };

	xcb_configure_window (conn, dec,
			XCB_CONFIG_WINDOW_X | XCB_CONFIG_WINDOW_Y, values);

	xcb_flush(conn);
}

void Client::move_absolute(int _x, int _y) {
	x = _x;
	y = _y;
	int values[] = { x, y };

	xcb_configure_window (conn, dec,
			XCB_CONFIG_WINDOW_X | XCB_CONFIG_WINDOW_Y, values);

	xcb_flush(conn);
}

bool Client::resize_relative(std::string dir, int amt) {

	if (dir.compare("north") == 0) {
		y -= amt;
		h += amt;
	} else if (dir.compare("south") == 0) {
		h += amt;
	} else if (dir.compare("east") == 0) {
		w += amt;
	} else if (dir.compare("west") == 0) {
		x -= amt;
		w += amt;
	} else {
		return false;
	}

	snap();

	int offset = config["outer_width"] + config["inner_width"];

	int values[] = { x, y,
		(int) (w),
		(int) (h)};

	xcb_configure_window (conn, dec,
			XCB_CONFIG_WINDOW_X
			| XCB_CONFIG_WINDOW_Y
			| XCB_CONFIG_WINDOW_WIDTH
			| XCB_CONFIG_WINDOW_HEIGHT,
			values);

	values[0] = (int) w - (2 * offset),
	values[1] = (int) h - (2 * offset);

	xcb_configure_window (conn, id,
			XCB_CONFIG_WINDOW_WIDTH
			| XCB_CONFIG_WINDOW_HEIGHT,
			values);

	xcb_flush(conn);

	if (focus_queue.front() == this) {
		decorate(config["focus_color"]);
	} else {
		decorate(config["unfocus_color"]);
	}

	return true;
}


void Client::resize_to(int nw, int nh) {

	w = nw;
	h = nh;

	int offset = config["outer_width"] + config["inner_width"];

	int values[] = {
		(int) (w - 2 * offset),
		(int) (h - 2 * offset)};

	xcb_configure_window (conn, id,
			XCB_CONFIG_WINDOW_WIDTH
			| XCB_CONFIG_WINDOW_HEIGHT,
			values);

	values[0] = (int) w,
	values[1] = (int) h;

	xcb_configure_window (conn, dec,
			XCB_CONFIG_WINDOW_WIDTH
			| XCB_CONFIG_WINDOW_HEIGHT,
			values);

	xcb_flush(conn);
}

void Client::resize_mouse(int _x, int dw, int _y, int dh) {
	int rx = _x - x;
	int ry = _y - y;


	if (rx > (int) (w / 2)) {
		resize_relative("east", dw);
	} else {
		resize_relative("west", -dw);
	}
	if (ry > (int) (h / 2)) {
		resize_relative("south", dh);
	} else {
		resize_relative("north", -dh);
	}
}

void Client::set_visible(bool state) {
	if (state) {
		std::clog << "mapping\n";
		// add to back of focus list
		focus_queue.push_back(this);
		xcb_map_window(conn, dec);
	} else {
		std::clog << "unmapping\n";
		remove_focus();
		unfocus();
		xcb_unmap_window(conn, dec);
	}

	xcb_flush(conn);
}

void Client::snap() {
	int nx, ny;
	nx = std::abs(x) + config["snap"]/2;
	nx -= nx % config["snap"];
	x = x > 0 ? nx : -nx;
	ny = std::abs(y) + config["snap"]/2;
	ny -= ny % config["snap"];
	y = y > 0 ? ny : -ny;
	w = w + config["snap"]/2;
	w -= w % config["snap"];
	h = h + config["snap"]/2;
	h -= h % config["snap"];
}
