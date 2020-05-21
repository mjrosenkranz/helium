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

	// let us get events for this window
	// this gets the button with not modifier
	xcb_grab_button(conn, false, id, XCB_EVENT_MASK_BUTTON_PRESS,
									// stop the cursor  continue the keyboard
	                XCB_GRAB_MODE_SYNC, XCB_GRAB_MODE_ASYNC,
	                XCB_NONE, XCB_NONE, XCB_BUTTON_MASK_1, XCB_NONE);

	xcb_flush(conn);
}

void Client::print(void) {
	std::clog << "id: " << std::hex << id
		<< " tag: " << std::hex << tag << std::dec
		<< " idx: " << idx
		<< " x: " << x
		<< " y: " << y
		<< " w: " << w
		<< " h: " << h
		<< std::endl;
}



bool Client::match_id(xcb_drawable_t _id) {
	return id == _id;
}


void Client::map(void) {
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
		tags[tag].erase(tags[tag].begin() + idx);
		// update the idx of all windows in previous tag
		update_tag(tag);
	}

	// add to new tag
	tag = t;
	tags[tag].push_back(this);
	update_tag(tag);
}


void Client::remove_tag(void) {
	tags[tag].erase(tags[tag].begin() + idx);
	update_tag(tag);
}

void Client::focus(void) {
	// remove this window from the focuse queue
	remove_focus();
	
	// add to the front of the focus queue
	focus_queue.push_front(this);
	// focus the client
	xcb_set_input_focus(conn, XCB_INPUT_FOCUS_POINTER_ROOT, id,
			XCB_CURRENT_TIME);

	// raise the window
	uint32_t values[] = { XCB_STACK_MODE_ABOVE };

	xcb_configure_window(conn, id, XCB_CONFIG_WINDOW_STACK_MODE, values);

	xcb_flush(conn);
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

	xcb_configure_window (conn, id,
			XCB_CONFIG_WINDOW_X | XCB_CONFIG_WINDOW_Y, values);

	xcb_flush(conn);
}

void Client::move_absolute(int _x, int _y) {
	x = _x;
	y = _y;
	int values[] = { x, y };

	xcb_configure_window (conn, id,
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

	int values[] = { x, y, (int) w, (int) h };

	xcb_configure_window (conn, id,
			XCB_CONFIG_WINDOW_X
			| XCB_CONFIG_WINDOW_Y
			| XCB_CONFIG_WINDOW_WIDTH
			| XCB_CONFIG_WINDOW_HEIGHT,
			values);

	xcb_flush(conn);
	return true;

}
