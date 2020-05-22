#include <xcb/xcb.h>
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


	if (geom != NULL) {
		x = geom->x;
		y = geom->y;
		w = geom->width;
		h = geom->height;
	}

	free(geom);

	uint32_t mask = XCB_CW_BACK_PIXEL;
	uint32_t values[] = {config["border_colorf"]};

	xcb_create_window(
			conn, XCB_COPY_FROM_PARENT,
			dec, screen->root, //parent
			x, y, w + 2 * config["border_width"], h + 2 * config["border_width"],
			0, XCB_WINDOW_CLASS_INPUT_OUTPUT, //class
			screen->root_visual, // visual
			mask, values
	);

	values[0] = XCB_EVENT_MASK_BUTTON_PRESS
		| XCB_EVENT_MASK_SUBSTRUCTURE_NOTIFY;
		//| XCB_EVENT_MASK_EXPOSURE;
	mask = XCB_CW_EVENT_MASK;

	xcb_change_window_attributes(conn,
					dec, mask, values);

 xcb_reparent_window(conn,
	      id, dec, config["border_width"], config["border_width"]);

	// let us get events for this window
	// this gets the button with not modifier
	xcb_grab_button(conn, false, id, XCB_EVENT_MASK_BUTTON_PRESS,
									// stop the cursor  continue the keyboard
	                XCB_GRAB_MODE_SYNC, XCB_GRAB_MODE_ASYNC,
	                XCB_NONE, XCB_NONE, XCB_BUTTON_INDEX_ANY, XCB_MOD_MASK_ANY);


	xcb_flush(conn);
}


void Client::unmanage(void) {
	if (focus_queue.front() == this) {
		// remove the window from the focus queue
		remove_focus();
		std::clog << "removed from front\n";
		if (focus_queue.size() > 0) {
			focus_queue.front()->focus();

		}
	} else {
		// remove the window from the focus queue
		remove_focus();
	}
	// remove the window from tags
	remove_tag();

	// destroy the decoration
	xcb_destroy_window(conn, dec);
	xcb_flush(conn);
}


void Client::kill(void) {
	std::clog << "killing client\n";
	xcb_kill_client(conn, id);
	xcb_flush(conn);
}

void Client::print(void) {
	std::clog << "id: " << std::hex << id
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
	if ( t == tag )
		return;
	
	// if the tag has been asigned before
	if (tag != -1) {
	// remove from current tag
		for (int i = 0; i < tags[tag].size(); ++i) {
			if (tags[tag].at(i) == this) {
				tags[tag].erase(tags[tag].begin() + i);
				break;
			}
		}
		
	}

	// add to new tag
	tag = t;
	tags[tag].push_back(this);
}


void Client::remove_tag(void) {
		for (int i = 0; i < tags[tag].size(); ++i) {
			if (tags[tag].at(i) == this) {
				tags[tag].erase(tags[tag].begin() + i);
				break;
			}
		}
}

void Client::focus(void) {
	// remove this window from the focuse queue
	remove_focus();

	// unfocus the focused window
	Client *focused = focus_queue.front();
	if (focused != NULL && focused != this)
		focused->unfocus();
	
	// add to the front of the focus queue
	focus_queue.push_front(this);

	uint32_t mask = XCB_CONFIG_WINDOW_STACK_MODE;
	uint32_t values[] = { XCB_STACK_MODE_ABOVE };

	decorate(config["border_colorf"]);
	// raise the window and change the color
	xcb_configure_window(conn, dec, mask, values);
	//xcb_configure_window(conn, id, mask, values);


	// focus the client
	xcb_set_input_focus(conn, XCB_INPUT_FOCUS_POINTER_ROOT, id,
			XCB_CURRENT_TIME);

	xcb_flush(conn);
}

void Client::unfocus(void) {
	decorate(config["border_coloru"]);
}

void Client::decorate(unsigned int color) {

	xcb_unmap_window(conn, dec);
	uint32_t mask = XCB_CW_BACK_PIXEL;
	uint32_t values[] = {color};

	// raise the window and change the color
	xcb_change_window_attributes(conn, dec, mask, values);
	//std::clog << "changing window color to " << std::hex << color << std::endl;

	xcb_flush(conn);
	xcb_map_window(conn, dec);
}

void Client::remove_focus(void) {
	for (int i = 0; i < focus_queue.size(); i++) {
		if (focus_queue[i]->id == id)
			focus_queue.erase(focus_queue.begin() + i);
	}
}

void Client::move_relative(int _x, int _y) {
	x += _x;
	y += _y;
	int values[] = { x, y};

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

	int values[] = { x, y,
		(int) (w + 2 * config["border_width"]),
		(int) (h + 2 * config["border_width"])};

	xcb_configure_window (conn, dec,
			XCB_CONFIG_WINDOW_X
			| XCB_CONFIG_WINDOW_Y
			| XCB_CONFIG_WINDOW_WIDTH
			| XCB_CONFIG_WINDOW_HEIGHT,
			values);

	values[0] = (int) w,
	values[1] = (int) h;

	xcb_configure_window (conn, id,
			XCB_CONFIG_WINDOW_WIDTH
			| XCB_CONFIG_WINDOW_HEIGHT,
			values);

	xcb_flush(conn);
	return true;

}


void Client::resize_to(int nw, int nh) {

	w = nw;
	h = nh;

	int values[] = { x, y,
		(int) (w + 2 * config["border_width"]),
		(int) (h + 2 * config["border_width"])};

	xcb_configure_window (conn, dec,
			XCB_CONFIG_WINDOW_WIDTH
			| XCB_CONFIG_WINDOW_HEIGHT,
			values);

	values[0] = (int) w,
	values[1] = (int) h;

	xcb_configure_window (conn, id,
			XCB_CONFIG_WINDOW_WIDTH
			| XCB_CONFIG_WINDOW_HEIGHT,
			values);

	xcb_flush(conn);
}

void Client::resize_mouse(int _x, int lx, int _y, int ly) {
	int rx = _x - x;
	int ry = _y - y;
	int dw = _x - lx;
	int dh = _y - ly;
	if (rx > (w / 2)) {
		resize_relative("east", dw);
	} else {
		resize_relative("west", -dw);
	}
	if (ry > (h / 2)) {
		resize_relative("south", dh);
	} else {
		resize_relative("north", -dh);
	}
}
