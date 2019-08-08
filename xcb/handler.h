#ifndef HANDLER_H
#define HANDLER_H

static void handle_map_request(xcb_generic_event_t *event);
static void handle_unmap_notify(xcb_generic_event_t *event);
static void handle_configure_notify(xcb_generic_event_t *event);
static void handle_configure_request(xcb_generic_event_t *event);
static void handle_client_message(xcb_generic_event_t *event);
static void handle_button_press(xcb_generic_event_t *event);
static void handle_property_notify(xcb_generic_event_t *event);
static void handle_expose(xcb_generic_event_t *event);
static void handle_destroy(xcb_generic_event_t *event);

extern void (*events[XCB_NO_OPERATION])(xcb_generic_event_t *event);

#endif