#include <iostream>
#include <xcb/xcb.h>
#include <vector>

#include "main.h"
#include "client.h"
#include "events.h"

using namespace std;

xcb_connection_t *conn;
xcb_screen_t *screen;
xcb_generic_event_t *event;
vector<client *> tags[NUM_TAGS];

bool open_connection() {
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

    // subscribe to events
    // create event mask
	unsigned int values[1] = {
		XCB_EVENT_MASK_SUBSTRUCTURE_REDIRECT |
		XCB_EVENT_MASK_SUBSTRUCTURE_NOTIFY |
		XCB_EVENT_MASK_BUTTON_PRESS
	};

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

    return true;
}

void run() {
    while ((event = xcb_wait_for_event(conn))) {
        if (event) {
            // check event type
            if (events[event->response_type & ~0x80]) {
				events[event->response_type & ~0x80](event);
			} else {
				fprintf(stderr, "event num: %d\n", event->response_type & ~0x80);
			}
			free(event);
        }
    }
}

void setupwm() {
    // setup the tags
    
}

int main(int argc, char **argv) {
    if(!open_connection()) {
        fprintf(stderr, "%s\n", "could not open connection");
        return 1;
    }
    setupwm();
    run();
    fprintf(stderr, "%s\n", "shutting down");
}
