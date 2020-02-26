/* std includes */
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
/*libraries */
#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <X11/Xft/Xft.h>
/* my includes */
#include "wm.h"
#include "hints.h"
#include "config.h"
#include "handle.h"
#include "cwindow.h"
#include "ipc.h"
#include "vector.h"

/* display struct */
Display *display;
/* screen number, width, and height*/
int screen_num, disp_width, disp_height;
/* root window */
Window root, check;
/* currently focused window */
cwindow *focused = NULL;
/* are we running the wm */
bool running = true;
/* font */
XftFont *font;
XftColor tmp_color;
XRenderColor r_color;

/* global config variable */
conf_t conf;

/* array corresponding to each tag's visibility */
bool tag_visible[NUM_TAGS];
/* array of characters for outputting the state of the tags */
char tag_state[NUM_TAGS];
/* array of tags themselves */
vector *tags[NUM_TAGS];

void open_display() {
	/* opens display */
	display = XOpenDisplay(NULL);
	/* gets screen  number and info of opened display */
	screen_num = DefaultScreen(display);
	disp_width = XDisplayWidth(display, screen_num);
	disp_height = XDisplayHeight(display, screen_num);
	fprintf(stderr, "display dims: %dx%d\n", disp_width, disp_height);
	/* get the root window */
	root = XRootWindow(display, screen_num);
	/* create check window */
	check = XCreateSimpleWindow(display, root, 0, 0, 1, 1, 0, 0, 0);
	/* get events for the root window */
	XSelectInput(display, root, SubstructureRedirectMask|SubstructureNotifyMask|ButtonPressMask|Button1Mask);
	/* hints and stuff */
	init_hints();
	/* initialize the tag pointers to null and the visibility to true */
	for (int i = 0; i < NUM_TAGS; i++) {
		tags[i] = create_vector();
		tag_visible[i] = true;
		tag_state[i] = '_';
	}
	XChangeProperty(display, root, atom_tag_state, XA_STRING, 8, PropModeReplace, tag_state, NUM_TAGS);
	/* conf variables */
	conf.b_width	= DEFAULT_B_WIDTH;
	conf.i_width	= DEFAULT_I_WIDTH;
	conf.radius		= DEFAULT_RADIUS;
	conf.t_height	= DEFAULT_T_HEIGHT;
	conf.u_color	= DEFAULT_U_COLOR;
	conf.f_color	= DEFAULT_F_COLOR;
	conf.iu_color	= DEFAULT_IU_COLOR;
	conf.if_color	= DEFAULT_IF_COLOR;
	conf.tf_color	= DEFAULT_TF_COLOR;
	conf.tu_color	= DEFAULT_TU_COLOR;
	conf.font		= DEFAULT_FONT;
	/* font */
	font = XftFontOpenXlfd(display, screen_num, conf.font);
}

void close_display() {

	XCloseDisplay(display);
	fprintf(stderr, "closing display\n");
}

void run(){
	/* next queued event */
	XEvent qev;

	while(running) {
		/* outputs the next event in the queue to qev */
		XNextEvent(display, &qev);
		/* check if event is something we can handle */
		if(event_handler[qev.type]){
			event_handler[qev.type](&qev);
		} else {
			fprintf(stderr, "event num: %d\n", qev.type);
		}
	}
}



void draw_text(cwindow *cw, long text_color) {
	XftDraw *draw;
	if (!cw->decorated) {
		return;
	}
	/* convert color to xftcolor */
	XColor x_color;
	XRenderColor r_color;
	XftColor xft_color;
	x_color.pixel = text_color;
	XQueryColor(display, DefaultColormap(display, screen_num), &x_color);
	r_color.red = x_color.red;
	r_color.green = x_color.green;
	r_color.blue = x_color.blue;
	r_color.alpha = 0xffff;

    /* shorten text if need be */
	XGlyphInfo extents;
	int x, y, len;
	for (len = strlen(cw->title); len >= 0; len--) {
		XftTextExtentsUtf8(display, font, (XftChar8 *)cw->title, len, &extents);
		if (extents.width < cw->dims.w) {
			break;
		}
	}
    y = (conf.t_height + conf.i_width + conf.b_width) / 2  + (extents.y) / 2;
    x = (cw->dims.w - extents.width) / 2;
    if (extents.height > conf.t_height) {
    	fprintf(stderr, "%s\n", "border too small to draw text on");
		return;
	}

    XftColorAllocValue(display, DefaultVisual(display, screen_num), DefaultColormap(display, screen_num), &r_color, &xft_color);

	XClearWindow(display, cw->dec);
	draw = XftDrawCreate(display, cw->dec, DefaultVisual(display, screen_num), DefaultColormap(display, screen_num)); 
	XftDrawStringUtf8(draw, &xft_color, font, x, y, cw->title, len);

	XftDrawDestroy(draw);
	XftColorFree(display, DefaultVisual(display, screen_num), DefaultColormap(display, screen_num), &xft_color);
}




int send_icccm(cwindow *cw, Atom atom) {
	int n;
    Atom *protocols;
    int exists = 0;
    XEvent ev;

    if (XGetWMProtocols(display, cw->window, &protocols, &n)) {
        while (!exists && n--)
            exists = protocols[n] == atom;
        XFree(protocols);
    }

    if (exists) {
        ev.type = ClientMessage;
        ev.xclient.window = cw->window;
        ev.xclient.message_type = wm_atom[WMProtocols];
        ev.xclient.format = 32;
        ev.xclient.data.l[0] = atom;
        ev.xclient.data.l[1] = CurrentTime;
        XSendEvent(display, cw->window, False, NoEventMask, &ev);
    }

	return exists;
}





int main(int argc, char *argv[]) {
	
	display = XOpenDisplay(NULL);

	if (display == NULL) {
		fprintf(stderr, "could not connect\n");
		exit(EXIT_FAILURE);
	}

	open_display();
	fprintf(stderr, "display opened\n");
	run();
	close_display();

	return 0;
}
