/* std includes */
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
/*libraries */
#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <X11/Xresource.h>
#include <X11/Xft/Xft.h>
/* my includes */
#include "wm.h"
#include "hints.h"
#include "config.h"
#include "handle.h"
#include "cwindow.h"
#include "ipc.h"
#include "resources.h"

/* display struct */
Display *display;
/* screen number, width, and height*/
int screen_num, disp_width, disp_height;
/* root window */
Window root, check;
/* manages the list of existing windows and focus order*/
struct cwindow *cw_stack[NUM_TAGS], *f_stack = NULL;
/* currently focused window */
struct cwindow *focused = NULL;
/* are we running the wm */
bool running = true;
/* font */
XftFont *font;
XftColor tmp_color;
XRenderColor r_color;

/* conf variables */
int conf_b_width	= DEFAULT_B_WIDTH;
int conf_radius		= DEFAULT_RADIUS;
int conf_t_height	= DEFAULT_T_HEIGHT;
long conf_u_color	= DEFAULT_U_COLOR;
long conf_f_color	= DEFAULT_F_COLOR;
char *conf_font		= DEFAULT_FONT;

/* array corresponding to each tag's visibility */
bool tag_visible[NUM_TAGS];
/* array of characters for outputting the state of the tags */
char tag_state[NUM_TAGS];

/* array for finding preferences in xresources */
struct pref resource[] = {
	{"border_width", INT, &conf_b_width},
	{"border_radius", INT, &conf_radius},
	{"title_height", INT, &conf_t_height},
	{"focused_color", COLOR, &conf_f_color},
	{"unfocused_color", COLOR, &conf_u_color},
	{"font", STRING, &conf_font},
};


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
	/* load resources */
	conf_init();
	fprintf(stderr, "resources loaded\n");
	/* initialize the tag pointers to null and the visibility to true */
	for (int i = 0; i < NUM_TAGS; i++) {
		cw_stack[i] = NULL;
		tag_visible[i] = true;
		tag_state[i] = '_';
	}
	XChangeProperty(display, root, atom_tag_state, XA_STRING, 8, PropModeReplace, tag_state, NUM_TAGS);
	/* font */
	font = XftFontOpenXlfd(display, screen_num, conf_font);
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



void draw_text(struct cwindow *cw, long text_color) {
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
    y = (conf_t_height / 2) + ((extents.y) / 2);
    x = (cw->dims.w - extents.width) / 2;

    XftColorAllocValue(display, DefaultVisual(display, screen_num), DefaultColormap(display, screen_num), &r_color, &xft_color);

	XClearWindow(display, cw->dec);
	draw = XftDrawCreate(display, cw->dec, DefaultVisual(display, screen_num), DefaultColormap(display, screen_num)); 
	XftDrawStringUtf8(draw, &xft_color, font, x, y, cw->title, len);

	XftDrawDestroy(draw);
	XftColorFree(display, DefaultVisual(display, screen_num), DefaultColormap(display, screen_num), &xft_color);
}




int send_icccm(struct cwindow *cw, Atom atom) {
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




void load_resource(XrmDatabase db, struct pref *item) {

	/* temporary variables variables used to assign to the config */
	char **sdst = item->dst;
	int *idst = item->dst;
	long *ldst = item->dst;

	/* the location in memory of the value we are looking for */
	XrmValue value;
	/* what we are searching with */
	char class[42], name[42];
	/* the type of the returned value */
	char *return_type;
	/* create the search criteria */
	snprintf(name, 42, "helium.%s", item->name);
	snprintf(class, 42, "Helium.%s", item->name);
	/* get the resource value */
	XrmGetResource(db, name, class, &return_type, &value);
	/* assign pointer to the correct value depending on type */
	switch (item->format) {
		case STRING:
			*sdst = value.addr;
			fprintf(stderr, "%s = %s\n", name, *sdst);
			break;
		case INT:
			*idst = strtoul(value.addr, NULL, 10);
			fprintf(stderr, "%s = %d\n", name, *idst);
			break;
		case COLOR:
			*ldst = strtoul(&value.addr[1], NULL, 16);
			fprintf(stderr, "%s = %x\n", name, *ldst);
			break;
	}
}

void conf_init() {
	XrmInitialize();
	/* string and database for resources */
	Display *tmpdisp;
	char *resources;
	XrmDatabase db;
	if (!(tmpdisp = XOpenDisplay(NULL))) {
		fprintf(stderr, "cannot open display\n");
		return;
	}
	XrmInitialize();
	/* load resouces into a string */
	resources = XResourceManagerString(tmpdisp);

	if (resources == NULL) {
		fprintf(stderr, "no resources found, please xrdb merge\nusing default values\n");
		//exit(EXIT_FAILURE);
		return;
	}

	db = XrmGetStringDatabase(resources);


	struct config_item *curr;

	struct pref *p;

	for (p = resource; p < resource + (sizeof(resource) / sizeof(struct pref)); p++) {
		load_resource(db, p);
	}

	//XrmDestroyDatabase(db);
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
