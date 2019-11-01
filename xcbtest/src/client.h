#pragma once
#include <xcb/xcb.h>
#include "main.h"

class client {
public:
    int x, y;
    unsigned int w, h, tag;
    // might assign different colors to each tag
    unsigned long if_color, iu_color;
    unsigned long of_color, ou_color;
    bool decorated;
    xcb_window_t win;
    xcb_window_t dec;

    client(xcb_window_t win_id);
    client(int _x, int _y, int _w, int _h);
    void decorate(bool focused);
    void focus();

private:
    void mask();

};