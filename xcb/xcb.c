#include <stdlib.h>
#include <stdio.h>
#include <xcb/xcb_ewmh.h>
#include <unistd.h>

#include "xcb.h"
#include "conf.h"
#include "vec.h"

xcb_connection_t *conn;
xcb_ewmh_connection_t *ewmh;
xcb_screen_t *screen;
vector *tags[NUM_TAGS];

bool
open_connection() {
	// number of the screen
	int scrn_num;
	// open display
	conn = xcb_connect(NULL, &scrn_num);
	fprintf(stderr, "screen %d\n", scrn_num);
	// check if there is an error in opening the display
	if (xcb_connection_has_error(conn)) {
		fprintf(stderr, "cannot connect to display\n");
		return false;
	}
	fprintf(stderr, "connected!\n");
	// assign the screen
	xcb_screen_iterator_t iter;
	iter = xcb_setup_roots_iterator(xcb_get_setup(conn));
	int iter_scrn_num = scrn_num;
	for (; iter.rem; --iter_scrn_num, xcb_screen_next(&iter)) {
		if (iter_scrn_num == 0)
			screen = iter.data;
	}

	if (!screen) {
		fprintf(stderr, "no screen found\n");
		return false;
	} else {
		fprintf(stderr, "screen found\n");
	}

	// create event mask
	unsigned int values[1] = {
		XCB_EVENT_MASK_SUBSTRUCTURE_REDIRECT |
		XCB_EVENT_MASK_SUBSTRUCTURE_NOTIFY |
		XCB_EVENT_MASK_BUTTON_PRESS
	};

	// allocate memory for ewmh
	ewmh = malloc(sizeof(xcb_ewmh_connection_t));
	if (ewmh == NULL) {
		fprintf(stderr, "not enough memory for hints");
		return false;
	}
	// request ewmh from xcb
	xcb_intern_atom_cookie_t *cookie = xcb_ewmh_init_atoms(conn, ewmh);
	if(!xcb_ewmh_init_atoms_replies(ewmh, cookie, (void *)0)){
        fprintf(stderr,"%s\n","xcb_ewmh_init_atoms_replies:faild.");
        exit(1);
    }

	xcb_ewmh_set_wm_pid(ewmh, screen->root, getpid());
	xcb_ewmh_set_wm_name(ewmh, screen->root, 6, "helium");
	// assign atoms we support
	xcb_atom_t net_atoms[] = {
		ewmh->_NET_SUPPORTED,              
		ewmh->_NET_ACTIVE_WINDOW,
		ewmh->_NET_WM_STATE,
		ewmh->_NET_WM_NAME,
		ewmh->_NET_WM_STATE_HIDDEN,
		ewmh->_NET_WM_WINDOW_TYPE,
		ewmh->_NET_WM_WINDOW_TYPE_DOCK,
		ewmh->_NET_WM_WINDOW_TYPE_DESKTOP,
		ewmh->_NET_WM_WINDOW_TYPE_TOOLBAR,
		ewmh->_NET_WM_PID,
		ewmh->WM_PROTOCOLS,
		ewmh->_NET_WM_STATE,
		ewmh->_NET_WM_STATE_FULLSCREEN
	};

	xcb_ewmh_set_supported(ewmh, scrn_num, sizeof(net_atoms)/sizeof*(net_atoms), net_atoms);
	// add event mask
	xcb_change_window_attributes(conn, screen->root, XCB_CW_EVENT_MASK, values);

	xcb_generic_error_t *error = xcb_request_check(conn,
			xcb_change_window_attributes_checked(conn, screen->root,
				XCB_CW_EVENT_MASK, values));

	xcb_flush(conn);

	if (error){
		fprintf(stderr,"%s\n","xcb_request_check:faild.");
		free(error);
		return false;
	} else {
		fprintf(stderr,"%s\n","request worked");
	}


	// create tags
	for (int i = 0; i < NUM_TAGS; i++) {
		tags[i] = create_vector();
	}

	return true;
}

void
close_connection() {
	fprintf(stderr, "%s\n", "closing connection");
}