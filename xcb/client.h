#ifndef CLIENT_H
#define CLIENT_H

#include <xcb/xcb.h>

typedef struct {
	xcb_window_t window;
	xcb_window_t dec;

	int x;
	int y;
	unsigned int w;
	unsigned int h;

	unsigned int tag;


} client;

void manage_new_window(xcb_window_t win_id);

#endif