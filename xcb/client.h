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

void manage_new_client(xcb_window_t win_id);
void unmanage_client(xcb_window_t win_id);
void apply_decoration(client *c, bool is_focused);
void update_border(client *c, bool is_focused);
void set_focus(client *c);
void apply_mask(xcb_window_t win, int w, int h, int diam);
client * get_client(xcb_window_t win);

#endif