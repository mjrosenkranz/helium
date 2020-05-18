#ifndef UTIL_H
#define UTIL_H

#include <xcb/xcb.h>
#include "client.h"

void print_tags();
void print_focus();
void update_tag(int);
Client *get_client(xcb_drawable_t*);

#endif
