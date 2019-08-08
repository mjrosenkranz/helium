#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <xcb/xcb.h>

#include "handler.h"
#include "xcb.h"
#include "client.h"

void (*events[XCB_NO_OPERATION])(xcb_generic_event_t *event) = {
	[XCB_MAP_REQUEST]		= handle_map_request,
    [XCB_UNMAP_NOTIFY]		= handle_unmap_notify,
    [XCB_CONFIGURE_NOTIFY]	= handle_configure_notify,
    [XCB_CONFIGURE_REQUEST]	= handle_configure_request,
    [XCB_CLIENT_MESSAGE]	= handle_client_message,
    [XCB_BUTTON_PRESS]		= handle_button_press,
    [XCB_PROPERTY_NOTIFY]	= handle_property_notify,
	[XCB_EXPOSE]			= handle_expose,
	[XCB_DESTROY_NOTIFY]	= handle_destroy,
};

void
handle_map_request(xcb_generic_event_t *event) {
	// parse event
	xcb_map_request_event_t *ev = (xcb_map_request_event_t *) event;
	xcb_window_t w = ev->window;
	// map window
	xcb_map_window(conn, w);
	fprintf(stderr, "map request handled\n");
	xcb_flush(conn);
}
void
handle_unmap_notify(xcb_generic_event_t *event) {
	fprintf(stderr, "unmap notify handled\n");
}
void
handle_configure_notify(xcb_generic_event_t *event) {
	fprintf(stderr, "configure notify handled\n");
}
void
handle_configure_request(xcb_generic_event_t *event) {
	fprintf(stderr, "configure request handled\n");
}
void
handle_client_message(xcb_generic_event_t *event) {
	fprintf(stderr, "client message handled\n");
}
void
handle_button_press(xcb_generic_event_t *event) {
	fprintf(stderr, "button press handled\n");
}
void
handle_property_notify(xcb_generic_event_t *event) {
	fprintf(stderr, "property notify handled\n");
}
void
handle_expose(xcb_generic_event_t *event) {
	fprintf(stderr, "expose event handled\n");
}
void
handle_destroy(xcb_generic_event_t *event) {
	fprintf(stderr, "destroy event handled\n");
}
