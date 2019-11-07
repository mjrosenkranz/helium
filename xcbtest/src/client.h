#pragma once
#include <xcb/xcb.h>
#include <vector>
#include "main.h"

class client {
public:
    int wx, wy, dx, dy;
    unsigned int ww, wh, dw, dh, tag;
    // might assign different colors to each tag
    uint32_t if_color, iu_color;
    uint32_t of_color, ou_color;
    bool decorated;
    xcb_window_t win, dec;

    client(xcb_window_t win_id);
    void decorate(bool focused);
    void reconfigure(int x, int y, int w, int h);
    void focus();

private:
    void mask();
    void rounded(xcb_pixmap_t pixmap, xcb_gcontext_t gc,
         int x, int y,
	     int w, int h, int diam);


};

extern std::vector<client *> tags[NUM_TAGS];
