#pragma once
#include <xcb/xcb.h>
#include "main.h"

class client {
public:
    int x, y, dx, dy;
    unsigned int w, h, dw, dh, tag;
    // might assign different colors to each tag
    uint32_t if_color, iu_color;
    uint32_t of_color, ou_color;
    bool decorated;
    xcb_window_t win;
    xcb_window_t dec;

    client(xcb_window_t win_id);
    client(int _x, int _y, int _w, int _h);
    void decorate(bool focused);
    void focus();

private:
    void mask();
    void rounded(xcb_pixmap_t pixmap, xcb_gcontext_t gc,
        unsigned int _w, unsigned int _h, unsigned int diam);

};
