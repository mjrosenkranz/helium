#include <iostream>
#include <xcb/xcb.h>
#include "events.h"
#include "client.h"
#include "utils.h"

void (*events[XCB_NO_OPERATION])(xcb_generic_event_t *event) = {
    [XCB_CREATE_NOTIFY] = handle_create_notify,
};

static void handle_create_notify(xcb_generic_event_t *event) {
    // parse event
    xcb_create_notify_event_t *ev = (xcb_create_notify_event_t *) event;
    xcb_window_t w = ev->window;
    // check if we alread have that window
    if (is_managed(w)) {
        return;
    }
    // create client object to house our client window
    client tmp = client(w);
    tags[0].push_back(&tmp);
    fprintf(stderr, "%s\n", "window object created");
    tmp = client(w);
    tmp.decorate(true);
    //tmp.decorate(true);

    // add client to tag
}

static void handle_map_request(xcb_generic_event_t *event) {
    xcb_map_request_event_t *ev = (xcb_map_request_event_t *) event;
    xcb_window_t w = ev->window;

}
