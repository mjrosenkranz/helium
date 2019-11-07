#include <iostream>
#include "client.h"
client::client(xcb_window_t win_id) {
    win = win_id;
    // create decoration window
    dec = xcb_generate_id(conn);

    // get/set the geometry of the window
    xcb_get_geometry_reply_t *geo;
	geo = xcb_get_geometry_reply(conn, xcb_get_geometry(conn, win), NULL);
	if (geo == NULL) {
		fprintf(stderr, "%s\n", "no geometry found");
		return;
	}

    dx = geo->x;
    dy = geo->y;
    wx = dx + DEF_IB_WIDTH + DEF_OB_WIDTH;
    wy = dy + DEF_IB_WIDTH + DEF_OB_WIDTH + DEF_HEIGHT;
    ww = geo->width;
    wh = geo->height;
    dw = geo->width + 2 * (DEF_IB_WIDTH + DEF_OB_WIDTH);
    dh = geo->width + 2 * (DEF_IB_WIDTH + DEF_OB_WIDTH);
    // asign to tag
    tag = 0;
    // might assign different colors to each tag
    if_color = DEF_IF_COLOR;
    iu_color = DEF_IU_COLOR;
    of_color = DEF_OF_COLOR;
    ou_color = DEF_OU_COLOR;

    decorated = true;


    //uint32_t mask = XCB_CW_BACK_PIXEL | XCB_CW_OVERRIDE_REDIRECT;
    uint32_t mask = XCB_CW_BACK_PIXEL;
    uint32_t values[1];

    xcb_create_window(
        conn, XCB_COPY_FROM_PARENT,
        dec, screen->root, //parent
        wx, wy, dw, dh,
        0, XCB_WINDOW_CLASS_INPUT_OUTPUT, //class
        screen->root_visual, // visual
        mask, values
    );

    values[0] = 0xffffffff;
    xcb_map_window(conn, dec);
    xcb_map_window(conn, win);
}


void client::decorate(bool focused) {
    xcb_unmap_window(conn, dec);
    xcb_unmap_window(conn, win);
    uint32_t mask = XCB_CW_BACK_PIXEL;
    uint32_t values[2];

    values[0] = 0xffffffff;
    // create decoration
    
    uint32_t ic = focused ? if_color : iu_color;
    uint32_t oc = focused ? of_color : ou_color;
    uint32_t col = 0xffffff00;
    // make the pixmap
    xcb_pixmap_t pix = xcb_generate_id(conn);
    xcb_create_pixmap(conn, screen->root_depth, pix, dec, dw, dh);
    // make graphics context 4 drawing
    xcb_gcontext_t gc = xcb_generate_id(conn);
    xcb_create_gc(conn, gc, pix, XCB_GC_FOREGROUND, &oc);

    // draw the outer border
    rounded(pix, gc, 0, 0, dw, dh, 15);
    // change color to inner
    xcb_change_gc(conn, gc, XCB_GC_FOREGROUND, &ic);
    // draw the inner border
    rounded(pix, gc, DEF_OB_WIDTH, DEF_OB_WIDTH, dw - 2 * DEF_OB_WIDTH,
		    dh - 2 * DEF_OB_WIDTH, 10);


    fprintf(stderr, "%s\n", "changing attributes");
    //uint32_t values[2];
    //uint32_t mask = XCB_CW_BACK_PIXMAP | XCB_CW_OVERRIDE_REDIRECT;
    values[0] = pix;
    values[1] = 1;
    xcb_void_cookie_t cookie = xcb_change_window_attributes_checked(
            conn, dec,
            XCB_CW_BACK_PIXMAP, &values[0]);
    xcb_generic_error_t *error;

    if ((error = xcb_request_check(conn, cookie))) {
        
        fprintf(stderr, "%d\n", error->error_code);
        free(error);
    }

    // map decoration and window
    xcb_free_pixmap(conn, pix);
    xcb_free_gc(conn, gc);
    fprintf(stderr, "freed\n");
    xcb_map_window(conn, dec);
    xcb_map_window(conn, win);
    fprintf(stderr, "%s\n", "mapping decorated window");

    xcb_flush(conn);
}

void client::focus() {
    fprintf(stderr, "Focusing client %x\n", win);
    fprintf(stderr, "%s\n", "changing border color");
}

void client::reconfigure(int x, int y, int w, int h) {
    xcb_unmap_window(conn, dec);
    xcb_unmap_window(conn, win);

    dx = x;
    dy = y;
    wx = dx + DEF_IB_WIDTH + DEF_OB_WIDTH;
    wy = dy + DEF_IB_WIDTH + DEF_OB_WIDTH + DEF_HEIGHT;
    ww = w;
    wh = h;
    dw = w + 2 * (DEF_IB_WIDTH + DEF_OB_WIDTH);
    dh = w + 2 * (DEF_IB_WIDTH + DEF_OB_WIDTH);

    fprintf(stderr, "x: %d y: %d w: %d h: %d\n", dx, dy, dw, dh);
    fprintf(stderr, "x: %d y: %d w: %d h: %d\n", wx, wy, ww, wh);
    uint32_t mask = 0;
    mask |= XCB_CONFIG_WINDOW_X;
    mask |= XCB_CONFIG_WINDOW_Y;
    mask |= XCB_CONFIG_WINDOW_WIDTH;
    mask |= XCB_CONFIG_WINDOW_HEIGHT;
    uint32_t values[5];
    values[0] = dx;
    values[1] = dy;
    values[2] = dw;
    values[3] = dh;
    fprintf(stderr, "%s\n", "reconfiguring decoration");
    xcb_configure_window(conn, dec, mask, values);
    xcb_map_window(conn, dec);

    values[0] = wx;
    values[1] = wy;
    values[2] = ww;
    values[3] = wh;
    values[4] = XCB_STACK_MODE_ABOVE;
    mask |= XCB_CONFIG_WINDOW_STACK_MODE;
    fprintf(stderr, "%s\n", "reconfiguring window");
    xcb_configure_window(conn, win, mask, values);
    xcb_map_window(conn, win);
}

void client::mask() {

}

void client::rounded(xcb_pixmap_t pixmap, xcb_gcontext_t gc,
    int x, int y, int w, int h, int diam) {
    xcb_rectangle_t rects[] = {
        {x + diam / 2, y, w - diam, h},
        {x, y + diam / 2, diam / 2, h - diam},
        {x + w - diam / 2, y + diam / 2, diam / 2, h - diam},
    };

    xcb_arc_t arcs[] = {
        { x,    y,    diam, diam, 0, 360 << 6 },
        { x,    y + h - diam, diam, diam, 0, 360 << 6 },
        { x + w - diam, y,    diam, diam, 0, 360 << 6 },
        { x + w - diam, y + h - diam, diam, diam, 0, 360 << 6 },
    };

    xcb_poly_fill_rectangle(conn, pixmap, gc, 3, rects);
    xcb_poly_fill_arc(conn, pixmap, gc, 4, arcs);
}
