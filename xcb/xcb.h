#ifndef XCB_H
#define XCB_H

#include <stdbool.h>
#include <xcb/xcb.h>
#include <xcb/xcb_ewmh.h>
#include "vec.h"
#include "conf.h"

extern xcb_connection_t *conn;
extern xcb_ewmh_connection_t *ewmh;
extern xcb_screen_t *screen;

extern vector *tags[NUM_TAGS];

bool open_connection(void);
void close_connection(void);


#endif