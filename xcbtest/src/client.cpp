#include <iostream>
#include <xcb/shape.h>
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
    dh = geo->height + 2 * (DEF_IB_WIDTH + DEF_OB_WIDTH) + DEF_HEIGHT;
    // asign to tag
    tag = 0;
    // might assign different colors to each tag
    if_color = DEF_IF_COLOR;
    iu_color = DEF_IU_COLOR;
    of_color = DEF_OF_COLOR;
    ou_color = DEF_OU_COLOR;

    decorated = true;

    // subscribe to events
    uint32_t mask = XCB_CW_EVENT_MASK;
    uint32_t values[1];
    values[0] = XCB_EVENT_MASK_ENTER_WINDOW |
        XCB_EVENT_MASK_FOCUS_CHANGE |
        XCB_EVENT_MASK_PROPERTY_CHANGE |
        XCB_EVENT_MASK_STRUCTURE_NOTIFY;
    xcb_change_window_attributes(conn, win, mask, values);

    mask = XCB_CW_BACK_PIXEL;
    values[1];

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
    dh = h + 2 * (DEF_IB_WIDTH + DEF_OB_WIDTH) + DEF_HEIGHT;

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
    //apply_mask(7);
    xcb_map_window(conn, win);
}
/*
void client::apply_mask() {
    xcb_pixmap_t pmask = xcb_generate_id(conn);
    xcb_create_pixmap(conn, 1, pmask, win, dw, dh);
    // make graphics context 4 drawing
    xcb_gcontext_t gc = xcb_generate_id(conn);
    xcb_create_gc(conn, gc, pmask, XCB_GC_FOREGROUND, (uint32_t[]){0, 0});
    // make background black
    xcb_rectangle_t bg = {0, 0, ww, wh};
    xcb_poly_fill_rectangle(conn, pmask, gc, 1, &bg);
    
    // make mask white
    xcb_change_gc(conn, gc, XCB_GC_FOREGROUND, (uint32_t[]){1, 0});
    rounded(pmask, gc, 0, 0, ww, wh, 5);

    xcb_shape_mask(conn, XCB_SHAPE_SO_SET, XCB_SHAPE_SK_CLIP, win, 0, 0, pmask);
    xcb_shape_mask(conn, XCB_SHAPE_SO_SET, XCB_SHAPE_SK_BOUNDING, win, 0, 0, pmask);

    uint32_t values = XCB_EVENT_MASK_SUBSTRUCTURE_NOTIFY;
    xcb_change_window_attributes(conn, win, XCB_CW_EVENT_MASK, &values);
    
    xcb_free_pixmap(conn, pmask);
    xcb_free_gc(conn, gc);
}
*/

void client::apply_mask(int diam) {
    // create roundedness
    xcb_pixmap_t pixmask = xcb_generate_id(conn);
    xcb_create_pixmap(conn, 1, pixmask, win, dw, dh);

    xcb_gcontext_t black = xcb_generate_id(conn);
    xcb_gcontext_t white = xcb_generate_id(conn);
    uint32_t b = 0;
    uint32_t w = 1;
    xcb_create_gc(conn, black, pixmask, XCB_GC_FOREGROUND, &b);
    xcb_create_gc(conn, white, pixmask, XCB_GC_FOREGROUND, &w);
   /* 
    // rectangles for the mask
    xcb_rectangle_t rects[] = {
        {diam / 2, 0, dw - diam, dh},
        {0, diam / 2, diam / 2, dh - diam},
        {dw - diam / 2, diam / 2, diam / 2, dh - diam},
    };
    // rounded corners
    xcb_arc_t arcs[] = {
        {  0,    0,    diam, diam, 0, 360 << 6 },
        {  0,    dh - diam, diam, diam, 0, 360 << 6 },
        { dw - diam, 0,    diam, diam, 0, 360 << 6 },
        { dw - diam, dh - diam, diam, diam, 0, 360 << 6 },
    };
    xcb_poly_fill_rectangle(conn, pixmask, white, 3, rects);
    xcb_poly_fill_arc(conn, pixmask, white, 4, arcs);
    */
//    xcb_rectangle_t bg = {0, 0, ww, wh};
//    xcb_poly_fill_rectangle(conn, pixmask, black, 1, &bg);
    rounded(pixmask, white, 0, 0, ww, wh, 7);

    // apply mask
    xcb_shape_mask(conn, XCB_SHAPE_SO_SET, XCB_SHAPE_SK_CLIP, win, 0, 0, pixmask);
    xcb_shape_mask(conn, XCB_SHAPE_SO_SET, XCB_SHAPE_SK_BOUNDING, win, 0, 0, pixmask);
    // change attributes
    uint32_t values = XCB_EVENT_MASK_SUBSTRUCTURE_NOTIFY;
    xcb_change_window_attributes(conn, win, XCB_CW_EVENT_MASK, &values);
    fprintf(stderr, "%s\n", "mask applied");

    xcb_free_pixmap(conn, pixmask);
    xcb_free_gc(conn, black);
    xcb_free_gc(conn, white);

    xcb_flush(conn);
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
