#include "client.h"
#include "helium.h"
#include "util.h"

#include <xcb/xcb.h>
#include <xcb/xcb_aux.h>
#include <deque>
#include <cstdlib>

Client::Client (xcb_window_t _id, xcb_connection_t *conn) {
	id = _id;
	tag = -1;

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
}

void Client::print(void) {
	fprintf(stderr, "id: %x, tag: %d idx: %d x: %d, y: %d, w: %d, h: %d\n",
			id, tag, idx, x, y, w, h);
}



bool Client::match_id(xcb_drawable_t _id) {
	return id == _id;
}


void Client::map(xcb_connection_t *conn) {
	xcb_map_window(conn, id);
}

void Client::focus(xcb_connection_t *conn) {
	// focus the client
	// first add it to the front of the focus queue
	focus_queue.push_front(this);
	xcb_set_input_focus(conn, XCB_INPUT_FOCUS_POINTER_ROOT, id,
			XCB_CURRENT_TIME);
	xcb_flush(conn);
	print_focus();
}

void Client::add_to_tag(int t) {
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
}

void Client::remove_focus(void) {
	for (int i = 0; i < focus_queue.size(); i++) {
		if (focus_queue[i]->id == id)
			focus_queue.erase(focus_queue.begin() + i);
	}
}

void Client::raise(xcb_connection_t *conn) {
	uint32_t values[] = { XCB_STACK_MODE_ABOVE };

	xcb_configure_window(conn, id, XCB_CONFIG_WINDOW_STACK_MODE, values);
	xcb_flush(conn);
}
