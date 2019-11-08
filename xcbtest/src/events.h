#pragma once

static void handle_create_notify(xcb_generic_event_t *event);
static void handle_map_request(xcb_generic_event_t *event);
static void handle_unmap_notify(xcb_generic_event_t *event);
static void handle_configure_request(xcb_generic_event_t *event);
extern void (*events[XCB_NO_OPERATION])(xcb_generic_event_t *event);
