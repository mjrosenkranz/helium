#ifndef XCB_H
#define XCB_H

#include <stdbool.h>
#include <xcb/xcb.h>
#include <xcb/xcb_ewmh.h>

extern xcb_connection_t *conn;
extern xcb_ewmh_connection_t *ewmh;
extern xcb_screen_t *screen;

bool open_connection(void);


#endif