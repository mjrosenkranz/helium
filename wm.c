/* std includes */
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <math.h>
#include <string.h>
/*libraries */
#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <X11/extensions/shape.h>
#include <X11/Xresource.h>
/* my includes */
#include "structs.h"
#include "ipc.h"
#include "resources.h"

/* display struct */
static Display *display;
/* screen number, width, and height*/
static int screen_num, disp_width, disp_height;
/* root window */
static Window root, check;
/* manages the list of existing windows and focus order*/
struct cwindow *cw_stack[NUM_TAGS], *f_stack = NULL;
/* currently focused window */
struct cwindow *focused = NULL;
/* atom for focus */
static Atom net_atom[NetLast], wm_atom[WMLast];
/* are we running the wm */
static bool running = true;

/* functions */
static void open_display();
static void close_display();
static void run();
static void handle_map_request(XEvent *ev);
static void handle_unmap_notify(XEvent *ev);
static void handle_configure_request(XEvent *ev);
static void handle_configure_notify(XEvent *ev);
static void handle_client_message(XEvent *ev);
static void handle_button_press(XEvent *ev);
static void handle_property_notify(XEvent *ev);
static void handle_expose(XEvent *ev);
static void manage_window(Window w, XWindowAttributes *wa);
static void create_decorations(struct cwindow *cw);
static void pix_mask(Window win, int x, int y, int w, int h, bool top);
static void cwindow_save(struct cwindow *cw);
static void cwindow_del(struct cwindow *cw);
static void cwindow_focus(struct cwindow *cw);
static void change_color(struct cwindow *cw, unsigned long color);
static int send_icccm(struct cwindow *cw, Atom atom);
static struct cwindow *get_cwindow(Window w);
static void handle_focus_cardinal(long *data);
static void handle_move_relative(long *data);
static void handle_move_to(long *data);
static void cwindow_move(struct cwindow *cw, int dx, int dy);
static void handle_resize_relative(long *data);
static void handle_exit(long *data);
static void handle_reload(long *data);
static int distance(struct cwindow *a, struct cwindow *b);
static void load_resource(XrmDatabase db, struct pref *item);
static void conf_init();

/* array of functions for cwindow events */
static void (*event_handler[LASTEvent])(XEvent *ev) = {
	[MapRequest]		= handle_map_request,
    [UnmapNotify]		= handle_unmap_notify,
    [ConfigureNotify]	= handle_configure_notify,
    [ConfigureRequest]	= handle_configure_request,
    [ClientMessage]		= handle_client_message,
    [ButtonPress]		= handle_button_press,
    [PropertyNotify]	= handle_property_notify,
	[Expose]			= handle_expose
};
/* array of functions for client message events */
static void (*client_event_handler[ipc_last])(long *data) = {
	[ipc_focus_cardinal]	= handle_focus_cardinal,
	[ipc_move_relative]		= handle_move_relative,
	[ipc_move_to]			= handle_move_to,
	[ipc_resize_relative]	= handle_resize_relative,
	[ipc_exit]				= handle_exit,
	[ipc_reload]			= handle_reload,
};
/* conf variables */
int conf_b_width	= DEFAULT_B_WIDTH;
int conf_radius		= DEFAULT_RADIUS;
int conf_t_height	= DEFAULT_T_HEIGHT;
long conf_u_color	= DEFAULT_U_COLOR;
long conf_f_color	= DEFAULT_F_COLOR;
char *conf_font		= DEFAULT_FONT;
/* array for finding preferences in xresources */
struct pref resource[] = {
	{"border_width", INT, &conf_b_width},
	{"border_radius", INT, &conf_radius},
	{"title_height", INT, &conf_t_height},
	{"focused_color", COLOR, &conf_f_color},
	{"unfocused_color", COLOR, &conf_u_color},
	{"font", STRING, &conf_font},
};

static void open_display() {
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
	/* ewmh */
	net_atom[NetSupported]           = XInternAtom(display, "_NET_SUPPORTED", False);
    net_atom[NetNumberOfDesktops]    = XInternAtom(display, "_NET_NUMBER_OF_DESKTOPS", False);
    net_atom[NetActiveWindow]        = XInternAtom(display, "_NET_ACTIVE_WINDOW", False);
    net_atom[NetWMStateFullscreen]   = XInternAtom(display, "_NET_WM_STATE_FULLSCREEN", False);
    net_atom[NetWMCheck]             = XInternAtom(display, "_NET_SUPPORTING_WM_CHECK", False);
    net_atom[NetCurrentDesktop]      = XInternAtom(display, "_NET_CURRENT_DESKTOP", False);
    net_atom[NetWMState]             = XInternAtom(display, "_NET_WM_STATE", False);
    net_atom[NetWMName]              = XInternAtom(display, "_NET_WM_NAME", False);
    net_atom[NetClientList]          = XInternAtom(display, "_NET_CLIENT_LIST", False);
    net_atom[NetWMWindowType]        = XInternAtom(display, "_NET_WM_WINDOW_TYPE", False);
    net_atom[NetWMWindowTypeDock]    = XInternAtom(display, "_NET_WM_WINDOW_TYPE_DOCK", False);
    net_atom[NetWMWindowTypeToolbar] = XInternAtom(display, "_NET_WM_WINDOW_TYPE_TOOLBAR", False);
    net_atom[NetWMWindowTypeMenu]    = XInternAtom(display, "_NET_WM_WINDOW_TYPE_MENU", False);
    net_atom[NetWMWindowTypeSplash]  = XInternAtom(display, "_NET_WM_WINDOW_TYPE_SPLASH", False);
    /* iccm */
    wm_atom[WMDeleteWindow]          = XInternAtom(display, "WM_DELETE_WINDOW", False);
    wm_atom[WMTakeFocus]             = XInternAtom(display, "WM_TAKE_FOCUS", False);
	wm_atom[WMProtocols] = XInternAtom(display, "WM_PROTOCOLS", False);
	/* update the wm properties */
	/* gives x an array of supported atoms */
	XChangeProperty(display, root, net_atom[NetSupported], XA_ATOM, 32, PropModeReplace, (unsigned char *) net_atom, NetLast);
	XChangeProperty(display, check, net_atom[NetWMCheck], XA_WINDOW, 32, PropModeReplace, (unsigned char *) &check, 1);
	XChangeProperty(display, root, net_atom[NetWMCheck], XA_WINDOW, 32, PropModeReplace, (unsigned char *) &check, 1);
	/* load resources */
	conf_init();
	fprintf(stderr, "resources loaded\n");
	/* initialize the tag pointers to null */
	for (int i = 0; i < NUM_TAGS; i++) {
		cw_stack[i] = NULL;
	}
}

static void close_display() {

	XCloseDisplay(display);
	fprintf(stderr, "closing display\n");
}

static void run(){

	/* next queued event */
	XEvent qev;

	while(running) {
		/* outputs the next event in the queue to qev */
		XNextEvent(display, &qev);
		/* check if event is something we can handle */
		if(event_handler[qev.type]){
			event_handler[qev.type](&qev);
		}
	}
}

static void handle_map_request(XEvent *ev) {
	
	/* struct of window attributes */
	static XWindowAttributes win_attr;
	/* map request event struct */
	XMapRequestEvent *mre = &ev->xmaprequest;
	/* if the window does not have attributes then return */
	if (!XGetWindowAttributes(display, mre->window, &win_attr)) {
		return;
	}
	/* if the window has override then dont manage it */
	if (win_attr.override_redirect) {
		return;
	}
	/* manage new window */
	manage_window(mre->window, &win_attr);

	fprintf(stderr, "map request handled\n");
}

static void handle_unmap_notify(XEvent *ev) {
	XUnmapEvent *uev = &ev->xunmap;
	struct cwindow *cw;
	cw = get_cwindow(uev->window);

	if (cw != NULL) {
		XUnmapWindow(display, cw->dec);
		cwindow_del(cw);
		XDestroyWindow(display, cw->dec);
		fprintf(stderr, "window decoration destroyed\n");
		focused = f_stack;
		cwindow_focus(focused);
		fprintf(stderr, "unmap notify handled\n");
	} else {
		fprintf(stderr, "window to unmap not found\n");
	}
}

static void handle_configure_request(XEvent *ev) {

	fprintf(stderr, "configure request handled\n");
}

static void handle_configure_notify(XEvent *ev) {

	fprintf(stderr, "configure notify handled\n");
}

static void handle_client_message(XEvent *ev) {
	fprintf(stderr, "client message recieved\n");

	XClientMessageEvent *xev = &ev->xclient;
	long cmd, *data;
	cmd = xev->data.l[0];
	data = xev->data.l;

	if (xev->message_type == XInternAtom(display, HELIUMC_EVENT, False)) {
		fprintf(stderr, "%s handled\n", HELIUMC_EVENT);
		client_event_handler[cmd](data);
	}
}

static void handle_button_press(XEvent *ev) {

	XButtonPressedEvent *bev;
	struct cwindow *cw;

	bev = &ev->xbutton;

	cw = get_cwindow(bev->window);

	if (cw == NULL) {
		return;
	} else if (cw->window == root) {
		fprintf(stderr, "root window grabbed, ignoring\n");
	} else if (cw != focused) {
		cwindow_focus(cw);
	}



	/*XFree(bev);
	free(cw);*/

	fprintf(stderr, "button press handled\n");
}

static void handle_property_notify(XEvent *ev) {
	/* use thos later for changing the title */

	fprintf(stderr, "property notify handled\n");
}

static void handle_expose(XEvent *ev) {
	fprintf(stderr, "expose handled\n");
}

static void manage_window(Window w, XWindowAttributes *wa) {
	/* TODO check if the window is a dock */
	/* create the client window struct */
	struct cwindow *cw;
	cw = malloc(sizeof(struct cwindow));
	/* check that the memory was allocated */
	if (cw == NULL) {
		fprintf(stderr, "failed to allocate memory for new window");
		return;
	}
	/* assign values to cw */
	cw->window = w;
	cw->dims.x = wa->x;
	cw->dims.y = wa->y;
	cw->dims.w = wa->width;
	cw->dims.h = wa->height;

	create_decorations(cw);
	/* focus the new window */
	cwindow_focus(cw);
	/* save to stack */
	cwindow_save(cw);

	/* subscribe to window events */
	XSelectInput(display, cw->window, EnterWindowMask|FocusChangeMask|PropertyChangeMask|StructureNotifyMask);
}

static void create_decorations(struct cwindow *cw) {
	/* calculate dummy window dimentions */
	int x = cw->dims.x;
	int y = cw->dims.y;
	int w = cw->dims.w + 2 * conf_b_width;
	int h = cw->dims.h + 2 * conf_b_width + conf_t_height;
	/* calculate dummy window dimentions */
 
	if (conf_b_width > 0 || conf_t_height > 0) {
		XMoveWindow(display, cw->window, x + conf_b_width, y + conf_b_width + conf_t_height);
		/* create the border window */
		Window dec = XCreateSimpleWindow(display, root, x, y, w, h, 0, conf_f_color, conf_f_color);
		pix_mask(dec, x, y, w, h, false);
		/* assign the border window */
		cw->dec = dec;
		cw->decorated = true;
		/* get mouse input from window */

		/* make window show up */
		XMapWindow(display, cw->dec);
		/* grab the mouse (left click), any modifier, over the decoration window */
		XGrabButton(display, 1, AnyModifier, cw->dec, True, ButtonPressMask, GrabModeAsync, GrabModeAsync, None, None);
	} else {
		cw->decorated = false;
	}

	pix_mask(cw->window, cw->dims.x, cw->dims.y, cw->dims.w, cw->dims.h, conf_t_height > 0 && conf_t_height > conf_radius);
	/* map window and decoration */
	XMapWindow(display, cw->window);
	/* raise the window */
	XRaiseWindow(display, cw->window);
	
	fprintf(stderr, "decorations created\n");
}

static void pix_mask(Window win, int x, int y, int w, int h, bool top) {

	/* create rounded corners */
	Pixmap mask = XCreatePixmap(display, win, w, h, 1);
	/* graphics context */
	GC gc = XCreateGC(display, mask, 0, 0);

	int diam = 2 * conf_radius;
	/* make the window mask all black */
	XSetForeground(display, gc, 0);
	XFillRectangle(display, mask, gc, 0, 0, w, h);

	/* set mask forground color to white (keeps that area */
	XSetForeground(display, gc, 1);
	if (top) {
		XFillRectangle(display, mask, gc, 0, 0, w, diam);
	}
	/* arcs used for corners */
	XFillArc(display, mask, gc, 0, 0,
			diam, diam, 64 * 90, 64 * 90);
	XFillArc(display, mask, gc, w - diam, 0,
			diam, diam, 64 * 0, 64 * 90);
	XFillArc(display, mask, gc, 0, h - diam,
			diam, diam, 64 * 180, 64 * 90);
	XFillArc(display, mask, gc, w - diam, h - diam,
			diam, diam, 64 * 270, 64 * 90);
	/* middle rectangle */
	XFillRectangle(display, mask, gc, conf_radius, 0, w - diam, h);
	/* left and right rectangles */
	XFillRectangle(display, mask, gc, 0, conf_radius, conf_radius, h - diam);
	XFillRectangle(display, mask, gc, w - conf_radius, conf_radius, conf_radius, h - diam);
	/* combine everything, masks out area painted black */
	XShapeCombineMask(display, win, ShapeBounding, 0, 0, mask, ShapeSet);
	XFreePixmap(display, mask);
}

static void cwindow_save(struct cwindow *cw) {
	/* add client window to top of managed window stack */
	cw->next = cw_stack[0];
	cw_stack[0] = cw;
	/* add client window to top of focused window stack */
	cw->f_next = f_stack;
	f_stack = cw;
}

static void cwindow_del(struct cwindow *cw) {
	/* remove window from the cwindow list */
	if (cw == cw_stack[0]) {
		cw_stack[0] = cw_stack[0]->next;
	} else {
		struct cwindow *tmp = cw_stack[0];
		while (tmp != NULL && tmp->next != cw)
			tmp = tmp->next;
		
		tmp->next = tmp->next->next;
	}

	/* removes window from focus list */
	if (cw == f_stack) {
		f_stack = f_stack->f_next;
	} else {
		struct cwindow *tmp = f_stack;
		while (tmp != NULL && tmp->f_next != cw)
			tmp = tmp->f_next;
		
		tmp->f_next = tmp->f_next->f_next;
	}
	
}

static void cwindow_focus(struct cwindow *cw) {
	if (cw != NULL && focused != NULL) {
		fprintf(stderr, "unfocus color changed\n");
		change_color(focused, conf_u_color);
		/* send message to focus client */
		send_icccm(cw, wm_atom[WMTakeFocus]);
	}
	if (cw != NULL) {
		/* change color */
		change_color(cw, conf_f_color);
		/* remove focus from old window */
		XDeleteProperty(display, root, net_atom[NetActiveWindow]);
		/* set focused client variable */
		focused = cw;
		/* send message to focus client */
		send_icccm(cw, wm_atom[WMTakeFocus]);
		/* tell ewmh we want to focus this */
		XChangeProperty(display, root, net_atom[NetActiveWindow], XA_WINDOW, 32, PropModeReplace, (unsigned char *) &(cw->window), 1);
		/* set focus for input */
		XSetInputFocus(display, cw->window, RevertToPointerRoot, CurrentTime);
		/* bring to the front */
		if (cw->decorated == true) {
			XRaiseWindow(display, cw->dec);
		}
		XRaiseWindow(display, cw->window);
	}
}

static void change_color(struct cwindow *cw, unsigned long color) {
	if (cw->decorated) {
		fprintf(stderr, "color changed\n");
		XSetWindowBackground(display, cw->dec, color);
		XClearWindow(display, cw->dec);
	}
}

static int send_icccm(struct cwindow *cw, Atom atom) {
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

static struct cwindow *get_cwindow(Window w) {
	for (int i = 0; i < NUM_TAGS; i++) {
		for (struct cwindow *tmp = cw_stack[i]; tmp != NULL; tmp=tmp->next) {
			if (tmp->window == w) {
				return tmp;
			} else if (tmp->decorated && tmp->dec == w) {
				return tmp;
			}
		}
	}
	
	return NULL;
}

static void handle_focus_cardinal(long *data) {
	fprintf(stderr, "cardinal focus event:\n");
	/* temporary cwindows for loop and focusing */
	struct cwindow *curr, *next_to_focus;

	curr = cw_stack[0];
	next_to_focus = NULL;
	/* for keeping track of the distance */
	int min = 999999;
	/* loop through cwindows */
	while(curr != NULL) {
		int dist = distance(focused, curr);
		switch (data[1]) {
			case north :
				fprintf(stderr, "\tdir: north\n");
				if (curr->dims.y < focused->dims.y && dist < min) {
					min = dist;
					next_to_focus = curr;
				}
				break;
			case south :
				fprintf(stderr, "\tdir: south\n");
				if (curr->dims.y > focused->dims.y && dist < min) {
					min = dist;
					next_to_focus = curr;
				}
				break;
			case east :
				fprintf(stderr, "\tdir: east\n");
				if (curr->dims.x > focused->dims.x && dist < min) {
					min = dist;
					next_to_focus = curr;
				}
				break;
			case west :
				fprintf(stderr, "\tdir: west\n");
				if (curr->dims.x < focused->dims.x && dist < min) {
					min = dist;
					next_to_focus = curr;
				}
				break;
			default :
				break;
		}
		/* go to next cwindow */
		curr = curr->next;
	}

	if (next_to_focus == NULL) {
		fprintf(stderr, "\tno windows found\n");
	} else {
		fprintf(stderr, "\tfocusing next window\n");
		cwindow_focus(next_to_focus);
	}
}

static void handle_move_relative(long *data) {
	fprintf(stderr, "move relative event:\n");
	fprintf(stderr, "\tx:%d y:%d\n", data[1], data[2]);

	if (focused == NULL) {
		return;
	}

	cwindow_move(focused, data[1] + focused->dims.x, data[2] + focused->dims.y);
}

static void handle_move_to(long *data) {
	fprintf(stderr, "move to event:\n");
	fprintf(stderr, "\tx:%d y:%d\n", data[1], data[2]);
	
	if (focused == NULL) {
		return;
	}

	cwindow_move(focused, data[1], data[2]);
}

static void cwindow_move(struct cwindow *cw, int dx, int dy) {
	if (cw->decorated) {
		XMoveWindow(display, cw->window, dx + conf_b_width, dy + conf_b_width + conf_t_height);
		XMoveWindow(display, cw->dec, dx, dy);
		cw->dims.x = dx + conf_b_width;
		cw->dims.y = dy + conf_b_width + conf_t_height;
	} else {
		XMoveWindow(display, cw->window, dx, dy);
		cw->dims.x = dx;
		cw->dims.y = dy;
	}
}

static void handle_resize_relative(long *data) {
	fprintf(stderr, "resize relative event:\n");

	if (focused == NULL) {
		return;
	}

	int x, y, w, h;
	switch (data[2]) {
		case north :
			x = focused->dims.x;
			y = focused->dims.y - data[1];
			w = focused->dims.w;
			h = focused->dims.h + data[1];
			break;
		case south :
			x = focused->dims.x;
			y = focused->dims.y;
			w = focused->dims.w;
			h = focused->dims.h + data[1];
			break;
		case east :
			x = focused->dims.x;
			y = focused->dims.y;
			w = focused->dims.w + data[1];
			h = focused->dims.h;
			break;
		case west :
			x = focused->dims.x - data[1];
			y = focused->dims.y;
			w = focused->dims.w + data[1];
			h = focused->dims.h;
			break;
		default :
			break;
	}
	XMoveResizeWindow(display, focused->window, x, y, w, h);
	focused->dims.x = x;
	focused->dims.y = y;
	focused->dims.w = w;
	focused->dims.h = h;
	/* destroy decorations if they exist */
	if (focused->decorated) {
		XDestroyWindow(display, focused->dec);
	}
	create_decorations(focused);
	XRaiseWindow(display, focused->window);
}

static void handle_exit(long *data) {
	running = false;
}

static void handle_reload(long *data) {
	/*XrmDatabase srcdb = XrmGetStringDatabase(XResourceManagerString(display));
	XrmMergeDatabases(srcdb, &db);
	struct pref *p;
	for (p = resource; p < resource + (sizeof(resource) / sizeof(struct pref)); p++) {
		load_resource(p);
	}*/

	conf_init();


	for (int i = 0; i < NUM_TAGS; i++) {
		for (struct cwindow *tmp = cw_stack[i]; tmp != NULL; tmp=tmp->next) {
			tmp->decorated = false;
			XUnmapWindow(display, tmp->dec);
			XDestroyWindow(display, tmp->dec);
			create_decorations(tmp);
		}
	}
}

static int distance(struct cwindow *a, struct cwindow *b) {
	int x_diff, y_diff;
    x_diff = a->dims.x - b->dims.x;
    y_diff = a->dims.y - b->dims.y;
    return pow(x_diff, 2) + pow(y_diff, 2);
}

static void load_resource(XrmDatabase db, struct pref *item) {

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

static void conf_init() {
	XrmInitialize();
	/* string and database for resources */
	char *resources;
	XrmDatabase db;
	/* load resouces into a string */
	resources = XResourceManagerString(display);

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
