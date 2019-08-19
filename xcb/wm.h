#ifndef WM_H
#define WM_H

#include <xcb/xcb.h>
#include "vec.h"

extern bool isrunning;
extern xcb_window_t focused;
extern xcb_window_t prev_focused;

static void run(void);

#endif