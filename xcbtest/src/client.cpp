#include <iostream>
#include "client.h"

client::client(xcb_window_t win_id) {
    // create decoration window
    dec = xcb_generate_id(conn);
}

client::client(int _x, int _y, int _w, int _h) {
    // create decoration window
    dec = xcb_generate_id(conn);
    dx = _x;
    dy = _y;
    x = dx + DEF_IB_WIDTH + DEF_OB_WIDTH;
    y = dy + DEF_IB_WIDTH + DEF_OB_WIDTH + DEF_T_HEIGHT;
    w = _w;
    h = _h;
    dw = w + (2 * DEF_IB_WIDTH + DEF_OB_WIDTH);
    dw = w + (2 * DEF_IB_WIDTH + DEF_OB_WIDTH);
    
    tag = 0;
    // might assign different colors to each tag
    if_color = DEF_IF_COLOR;
    iu_color = DEF_IU_COLOR;
    of_color = DEF_OF_COLOR;
    ou_color = DEF_OU_COLOR;
    // fprintf(stderr, "color: %lx\n", iu_color);
    decorated = true;


    //uint32_t mask = XCB_CW_BACK_PIXEL | XCB_CW_OVERRIDE_REDIRECT;
    uint32_t mask = XCB_CW_BACK_PIXEL;
    uint32_t values[1];

    values[0] = 0xffffffff;
    //values[1] = 1;
    // create decoration
    xcb_create_window(
        conn, XCB_COPY_FROM_PARENT,
        dec, screen->root, //parent
        x, y, w, h,
        0, XCB_WINDOW_CLASS_INPUT_OUTPUT, //class
        screen->root_visual, // visual
        mask, values
    );
    xcb_map_window(conn, dec);

}

void client::decorate(bool focused) {
    fprintf(stderr, "%s\n", "unmapping window to decorate");
    xcb_unmap_window(conn, dec);

    //uint32_t ic = focused ? if_color : iu_color;
    //uint32_t oc = focused ? of_color : ou_color;
    uint32_t oc = 0xff00ff00;
    // make the pixmap
    xcb_pixmap_t pix = xcb_generate_id(conn);
    xcb_create_pixmap(conn, screen->root_depth, pix, dec, dw, dh);
    // make graphics context 4 drawing
    xcb_gcontext_t gc = xcb_generate_id(conn);
    xcb_create_gc(conn, gc, pix, XCB_GC_FOREGROUND, &oc);

    // draw on the pixmap
    /*xcb_rectangle_t bg = {0, 0, dw, dh};
    xcb_poly_fill_rectangle(conn, pix, gc, 1, &bg);*/

    rounded(pix, gc, dw, dh, 10);


    fprintf(stderr, "%s\n", "changing attributes");
    uint32_t mask = XCB_CW_BACK_PIXMAP | XCB_CW_OVERRIDE_REDIRECT;
    uint32_t values[2];
    values[0] = pix;
    values[1] = 1;
    //values[1] = 0x00ff00;
    xcb_void_cookie_t cookie = xcb_change_window_attributes_checked(
            conn, dec,
            XCB_CW_BACK_PIXMAP, &values[0]);
    xcb_generic_error_t *error;

    if ((error = xcb_request_check(conn, cookie))) {
        
        fprintf(stderr, "%d\n", error->error_code);
        free(error);
    }

    // map decoration and window
    fprintf(stderr, "%s\n", "mapping decorated window");
    xcb_free_pixmap(conn, pix);
    xcb_free_gc(conn, gc);
    xcb_map_window(conn, dec);
    xcb_flush(conn);
}

void client::focus() {
    fprintf(stderr, "Focusing client %x\n", win);
    fprintf(stderr, "%s\n", "changing border color");
}

void client::mask() {

}

void client::rounded(xcb_pixmap_t pixmap, xcb_gcontext_t gc,
        unsigned int _w, unsigned int _h, unsigned int diam) {
    xcb_rectangle_t rects[] = {
        {diam / 2, 0, _w - diam, _h},
        {0, diam / 2, diam / 2, _h - diam},
        {_w - diam / 2, diam / 2, diam / 2, _h - diam},
    };

    xcb_arc_t arcs[] = {
        {  0,    0,    diam, diam, 0, 360 << 6 },
        {  0,    _h - diam, diam, diam, 0, 360 << 6 },
        { _w - diam, 0,    diam, diam, 0, 360 << 6 },
        { _w - diam, _h - diam, diam, diam, 0, 360 << 6 },
    };

    xcb_poly_fill_rectangle(conn, pixmap, gc, 3, rects);
    xcb_poly_fill_arc(conn, pixmap, gc, 4, arcs);
}
