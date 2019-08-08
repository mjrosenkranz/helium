#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <xcb/xcb.h>

#include "xcb.h"
#include "handler.h"
#include "wm.h"

xcb_generic_event_t *event;
bool isrunning;

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
	if (!open_connection()) {
		fprintf(stderr, "err\n");
		return EXIT_FAILURE;
	}
	run();
	return 0;
}