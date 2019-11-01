#include <iostream>
#include "client.h"

client::client(xcb_window_t win_id) {
    // create decoration window
    dec = xcb_generate_id(conn);
}

client::client(int _x, int _y, int _w, int _h) {
    // create decoration window
    dec = xcb_generate_id(conn);
    x = _x;
    y = _y;
    w = _w;
    h = _h;
    tag = 0;
    // might assign different colors to each tag
    if_color = 0x00ff00;
    iu_color = 0x3f3f3f;
    of_color = 0x3f3f3f;
    ou_color = 0x00ff00;
    fprintf(stderr, "color: %lx\n", iu_color);
    decorated = true;
}

void client::decorate(bool focused) {
    fprintf(stderr, "%s\n", "decorating window");

    uint32_t mask = XCB_CW_BACK_PIXEL | XCB_CW_BORDER_PIXEL | XCB_CW_OVERRIDE_REDIRECT;
    // uint32_t mask = XCB_CW_BACK_PIXEL | XCB_CW_OVERRIDE_REDIRECT;
    uint32_t values[3];
    // uint32_t values[2];

    // values[0] = 0x00ff00;
    values[0] = if_color ? focused : iu_color;
    values[1] = of_color ? focused : ou_color;
    values[1] = 0x3f3f3f;
    values[2] = 1;

    // make shit easier
    int x = 100;
    int y = 100;
    int w = 200;
    int h = 200;

    xcb_create_window(
        conn, XCB_COPY_FROM_PARENT,
        dec, screen->root, //parent
        x, y, w, h,
        DEF_O_B_WIDTH, XCB_WINDOW_CLASS_INPUT_OUTPUT, //class
        screen->root_visual, // visual
        mask, values
    );

    // map decoration and window
    xcb_map_window(conn, dec);
    fprintf(stderr, "%s\n", "decorated");
}

void client::focus() {
    fprintf(stderr, "Focusing client %x\n", win);
    fprintf(stderr, "%s\n", "changing border color");
}

void client::mask() {

}
