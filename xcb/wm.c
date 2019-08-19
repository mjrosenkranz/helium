#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <xcb/xcb.h>

#include "xcb.h"
#include "handler.h"
#include "wm.h"

xcb_generic_event_t *event;
config conf;
bool isrunning;

xcb_window_t focused = XCB_WINDOW_NONE;
xcb_window_t prev_focused = XCB_WINDOW_NONE;

static void
run(void) {
	while ((event = xcb_wait_for_event(conn))) {
		if (event) {
			// get the event number
			if (events[event->response_type & ~0x80]) {
				events[event->response_type & ~0x80](event);
			} else {
				fprintf(stderr, "event num: %d\n", event->response_type & ~0x80);
			}
			free(event);
		}
	}
}

int
main (int argc, char **argv) {
	// assign config
	conf.b_width = 5;
	conf.b_radius = 10;
	conf.t_height = 20;
	conf.f_color = 0x00ff00;
	conf.u_color = 0xdfdfdf;
	atexit(close_connection);
	if (!open_connection()) {
		fprintf(stderr, "err\n");
		return EXIT_FAILURE;
	}
	run();
	return 0;
}