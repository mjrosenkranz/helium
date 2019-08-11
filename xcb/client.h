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
void apply_decoration(client *c);
void set_focus(client *c);
void apply_mask(xcb_window_t win, int w, int h, int diam);

#define WM_B_WIDTH 5
#define WM_T_HEIGHT 20
#define WM_B_COLOR 0x44e744
#define WM_B_RADIUS 10

#endif