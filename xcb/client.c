#include <stdlib.h>
#include <stdio.h>
#include <xcb/xcb.h>
#include <xcb/shape.h>
#include <xcb/xcb_ewmh.h>
#include <xcb/xcb_icccm.h>
#include <xcb/xproto.h>

#include "xcb.h"
#include "client.h"


void
manage_new_client(xcb_window_t win_id) {
	/* we don't wanna manage this window if it 
		is the root or if it is already managed 
		TODO: add search for existing clients*/
	if (win_id == screen->root) {
		fprintf(stderr, "root window, not managing\n");
		return;
	}

	// get window type
	xcb_get_window_attributes_cookie_t window_attributes_cookie;
	window_attributes_cookie = xcb_get_window_attributes(conn, win_id);

	xcb_get_property_cookie_t window_type_cookie;
	window_type_cookie = xcb_ewmh_get_wm_window_type(ewmh, win_id);

    /*xcb_get_property_cookie_t atom_name_cookie;
    atom_name_cookie = xcb_get_property(xcb_connection, 0,
        win_id, XCB_ATOM_WM_NAME, XCB_GET_PROPERTY_TYPE_ANY, 0, 256);	*/

	// lets make sure we actually want to manage this window
	xcb_get_window_attributes_reply_t *attr;
	attr = xcb_get_window_attributes_reply(conn, window_attributes_cookie, NULL);
    // we don't want to manage the window if it has override redirect
	if (attr && attr->override_redirect) {
		fprintf(stderr, "not managing this window\n");
		xcb_map_window(conn, win_id);
		xcb_flush(conn);
		return;
	}
    // make sure the ewmh type is one we want to manage
	xcb_ewmh_get_atoms_reply_t *window_type;


	if (xcb_ewmh_get_wm_window_type_reply(ewmh, window_type_cookie, window_type, NULL) == 1) {
		xcb_atom_t atom;
		fprintf(stderr, "len atoms: %d\n", window_type->atoms_len);
		for (int i = 0; i < window_type->atoms_len; i++) {
			atom = window_type->atoms[i];
			if (atom == ewmh->_NET_WM_WINDOW_TYPE_DOCK ||
				atom == ewmh->_NET_WM_WINDOW_TYPE_DESKTOP ||
				atom == ewmh->_NET_WM_WINDOW_TYPE_TOOLBAR) {
					fprintf(stderr, "not managing this window\n");
					// xcb_map_window(conn, win_id);
					// xcb_flush(conn);
					// xcb_ewmh_get_atoms_reply_wipe(window_type);
					return;
			}
		}
		xcb_ewmh_get_atoms_reply_wipe(window_type);
	}
    // set title (lets do this down the line)

	// create new client
	client *c = malloc(sizeof(client));
	// check if it is actually allocated
	if (c == NULL) {
		fprintf(stderr, "not enough memory for new client\n");
	}

	// get window geometry
	xcb_get_geometry_reply_t *geometry;
	geometry = xcb_get_geometry_reply(conn, xcb_get_geometry(conn, win_id), NULL);
	if (geometry == NULL) {
		fprintf(stderr, "%s\n", "no geometry found\n");
		return;
	}


	// populate the client
	c->window = win_id;
	c->x = geometry->x;
	c->y = geometry->y;
	c->w = geometry->width;
	c->h = geometry->height;
	// print client info
	fprintf(stderr, "x: %d\n", c->x);
	fprintf(stderr, "y: %d\n", c->y);
	fprintf(stderr, "w: %d\n", c->w);
	fprintf(stderr, "h: %d\n", c->h);

	// subscribte to events
	uint32_t values[1];
	values[0] = XCB_EVENT_MASK_ENTER_WINDOW |
	XCB_EVENT_MASK_FOCUS_CHANGE |
	XCB_EVENT_MASK_PROPERTY_CHANGE |
	XCB_EVENT_MASK_STRUCTURE_NOTIFY;
	xcb_change_window_attributes(conn, c->window, XCB_CW_EVENT_MASK, values);

    // set size using size hints

	// map the window and decorations!
	apply_decoration(c);
	xcb_flush(conn);
}

void
apply_decoration(client *c) {
	if (c == NULL) {
		fprintf(stderr, "%s\n", "client not found");
		return;
	}

	// create decoration window
	c->dec = xcb_generate_id(conn);
	uint32_t mask = 0;
	uint32_t values[2];

	mask = XCB_CW_BACK_PIXEL | XCB_CW_OVERRIDE_REDIRECT;
	// values[0] = screen->white_pixel;
	values[0] = WM_B_COLOR;
	values[1] = 1;

	// make shit easier
	unsigned int diam = 2 * WM_B_RADIUS;
	unsigned int dec_w = c->w + 2 * WM_B_WIDTH;
	unsigned int dec_h = c->h + WM_T_HEIGHT + 2 * WM_B_WIDTH;

	xcb_create_window(conn,
		XCB_COPY_FROM_PARENT,
		c->dec,                           /* window Id     */
		screen->root,                  /* parent window */
		c->x, c->y,                          /* x, y          */
		dec_w, dec_h,                      /* width, height */
		0,                            /* border_width  */
		XCB_WINDOW_CLASS_INPUT_OUTPUT, /* class         */
		screen->root_visual,           /* visual        */
		mask, values);                 /* masks         */


	apply_mask(c->window, c->w, c->h, diam);
	// add roundedness to border
	apply_mask(c->dec, dec_w, dec_h, diam);

	// reparent
	xcb_reparent_window(conn, c->window, c->dec, WM_B_WIDTH, WM_B_WIDTH + WM_T_HEIGHT);
	// map decoration and window
	fprintf(stderr, "%s\n", "border created");
	xcb_map_window(conn, c->dec);
	xcb_map_window(conn, c->window);
	set_focus(c);


	xcb_flush(conn);


	return;

}

void
apply_mask(xcb_window_t win, int w, int h, int diam) {
	// create roundedness
	xcb_pixmap_t pixmask = xcb_generate_id(conn);
	xcb_create_pixmap(conn, 1, pixmask, win, w, h);

	xcb_gcontext_t black = xcb_generate_id(conn);
	xcb_gcontext_t white = xcb_generate_id(conn);

	xcb_create_gc(conn, black, pixmask, XCB_GC_FOREGROUND, (uint32_t[]){0, 0});
	xcb_create_gc(conn, white, pixmask, XCB_GC_FOREGROUND, (uint32_t[]){1, 0});

	// rectangles for the mask
	xcb_rectangle_t rects[] = {
		{diam / 2, 0, w - diam, h},
		{0, diam / 2, diam / 2, h - diam},
		{w - diam / 2, diam / 2, diam / 2, h - diam},
	};
	// rounded corners
	xcb_arc_t arcs[] = {
		{  0,    0,    diam, diam, 0, 360 << 6 },
		{  0,    h - diam, diam, diam, 0, 360 << 6 },
		{ w - diam, 0,    diam, diam, 0, 360 << 6 },
		{ w - diam, h - diam, diam, diam, 0, 360 << 6 },
	};
	xcb_rectangle_t bg = {0, 0, w, h};
	xcb_poly_fill_rectangle(conn, pixmask, black, 1, &bg);
	xcb_poly_fill_rectangle(conn, pixmask, white, 3, rects);
	xcb_poly_fill_arc(conn, pixmask, white, 4, arcs);

	// apply mask
	xcb_shape_mask(conn, XCB_SHAPE_SO_SET, XCB_SHAPE_SK_CLIP, win, 0, 0, pixmask);
	xcb_shape_mask(conn, XCB_SHAPE_SO_SET, XCB_SHAPE_SK_BOUNDING, win, 0, 0, pixmask);
	// change attributes
	uint32_t values = XCB_EVENT_MASK_SUBSTRUCTURE_NOTIFY;
	xcb_change_window_attributes(conn, win, XCB_CW_EVENT_MASK, &values);
	fprintf(stderr, "%s\n", "mask applied");

	xcb_free_pixmap(conn, pixmask);
}

void
set_focus(client *c) {
	// raise window
	uint32_t values [2];
	values[0] = XCB_STACK_MODE_ABOVE;
	xcb_configure_window(conn, c->dec, XCB_CONFIG_WINDOW_STACK_MODE, values);
	// change color of old window
	// set input focus
	xcb_set_input_focus(conn, XCB_INPUT_FOCUS_POINTER_ROOT, c->window, XCB_CURRENT_TIME);
	// change net active window
	values[0] = XCB_ICCCM_WM_STATE_NORMAL;
	values[1] = XCB_NONE;
	xcb_change_property(conn, XCB_PROP_MODE_REPLACE, c->window, ewmh->_NET_WM_STATE,
		ewmh->_NET_WM_STATE, 32, 2, values);
	xcb_change_property(conn, XCB_PROP_MODE_REPLACE, screen->root,
			ewmh->_NET_ACTIVE_WINDOW, XCB_ATOM_WINDOW, 32, 1, &c->window);
	fprintf(stderr, "%s\n", "focus set");
	xcb_flush(conn);
}