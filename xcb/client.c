#include <stdlib.h>
#include <stdio.h>
#include <xcb/xcb.h>

#include "xcb.h"
#include "client.h"


void
manage_new_window(xcb_window_t win_id) {
	/* we don't wanna manage this window if it 
		is the root or if it is already managed 
		TODO: add search for existing clients*/
	if (win_id == screen->root) {
		return;
	}

	// get window type
    xcb_get_window_attributes_cookie_t window_attributes_cookie;
    window_attributes_cookie = xcb_get_window_attributes(

        xcb_connection, window_id);
	xcb_get_property_cookie_t window_type_cookie;
    window_type_cookie = xcb_ewmh_get_wm_window_type( ewmh_connection, window_id);

    /*xcb_get_property_cookie_t atom_name_cookie;
    atom_name_cookie = xcb_get_property(xcb_connection, 0,
        window_id, XCB_ATOM_WM_NAME, XCB_GET_PROPERTY_TYPE_ANY, 0, 256);	*/

	// lets make sure we actually want to manage this window
    xcb_get_window_attributes_reply *attr
    attr = xcb_get_window_attributes_reply(conn, window_attributes_cookie, NULL);
    // we don't want to manage the window if it has override redirect
    if (attr && attr->override_redirect) {
    	return;
    }
    // make sure the ewmh type is one we want to manage

    // set title (lets do this down the line)


	// create new client
	client *c = malloc(sizeof(client));
	// check if it is actually allocated
	if (c == NULL) {
		fprintf(stderr, "not enough memory for new client\n");
	}

	// populated the client

    // set size using size hints
}
