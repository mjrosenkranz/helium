#include <iostream>
#include <xcb/xcb.h>
#include "events.h"
#include "client.h"
#include "utils.h"

void (*events[XCB_NO_OPERATION])(xcb_generic_event_t *event) = {
    [XCB_CREATE_NOTIFY] = handle_create_notify,
    [XCB_UNMAP_NOTIFY] = handle_unmap_notify,
    [XCB_MAP_REQUEST]   = handle_map_request,
    [XCB_CONFIGURE_REQUEST]   = handle_configure_request,
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
    client *tmp = NULL;
    tmp = new client(w);
    tags[0].push_back(tmp);
    fprintf(stderr, "%s\n", "window object created");
    //tmp->decorate(true);

    xcb_flush(conn);
}

static void handle_map_request(xcb_generic_event_t *event) {
    xcb_map_request_event_t *ev = (xcb_map_request_event_t *) event;
    xcb_window_t w = ev->window;
    fprintf(stderr, "%s\n", "map requested");
    client *tmp = get_client_from_win(w);
    if (tmp == NULL)
        return;
    xcb_map_window(conn, tmp->win);
    xcb_map_window(conn, tmp->dec);
    xcb_flush(conn);
}

static void handle_configure_request(xcb_generic_event_t *event) {
    xcb_configure_request_event_t *ev = (xcb_configure_request_event_t *) event;
    xcb_window_t w = ev->window;
    
    fprintf(stderr, "x: %d y: %d w: %d h: %d\n",
            ev->x, ev->y, ev->width, ev->height);
    client *tmp = get_client_from_win(w);
    if (tmp == NULL)
        return;
    tmp->reconfigure(ev->x, ev->y, ev->width, ev->height);
    tmp->decorate(true);
    xcb_flush(conn);
}

static void handle_unmap_notify(xcb_generic_event_t *event) {
    xcb_unmap_notify_event_t *ev = (xcb_unmap_notify_event_t *) event;
    xcb_window_t w = ev->window;
    client *tmp = get_client_from_win(w);
    if (tmp == NULL)
        return;
    fprintf(stderr, "%s\n", "unmapping");
    xcb_unmap_window(conn, tmp->win);
    xcb_unmap_window(conn, tmp->dec);
    xcb_flush(conn);
}
