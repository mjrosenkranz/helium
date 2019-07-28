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
#include <X11/Xft/Xft.h>
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
/* atoms */
static Atom net_atom[NetLast], wm_atom[WMLast], atom_tag_state, atom_winfo;
/* are we running the wm */
static bool running = true;
/* font */
static XftFont *font;
static XftColor tmp_color;
static XRenderColor r_color;

/* functions */
static void load_color(XftColor *dest_color, unsigned long raw_color);
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
static void draw_text(struct cwindow *cw, long text_color);
static void cwindow_save(struct cwindow *cw, int tag);
static void cwindow_del(struct cwindow *cw);
static void cwindow_focus(struct cwindow *cw);
static void change_color(struct cwindow *cw, unsigned long color);
static int send_icccm(struct cwindow *cw, Atom atom);
static struct cwindow *get_cwindow(Window w);
static bool is_border(Window w);
static void handle_focus_cardinal(long *data);
static void handle_move_relative(long *data);
static void handle_move_to(long *data);
static void cwindow_move(struct cwindow *cw, int dx, int dy);
static void handle_resize_relative(long *data);
static void handle_exit(long *data);
static void handle_reload(long *data);
static void handle_assign_tag(long *data);
static void handle_toggle_tag(long *data);
static void handle_close_client(long *data);
static void handle_get_info(long *data);
static void cwindow_show(struct cwindow *cw);
static void cwindow_hide(struct cwindow *cw);
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
	[ipc_assign_tag]		= handle_assign_tag,
	[ipc_toggle_tag]		= handle_toggle_tag,
	[ipc_close_client]		= handle_close_client,
};
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
	/* reports the state of the tabs */
	atom_tag_state = XInternAtom(display, "TAG_STATE", false);
	/* reports window info */
	atom_winfo = XInternAtom(display, "WINFO", false);
	/* update the wm properties */
	/* gives x an array of supported atoms */
	XChangeProperty(display, root, net_atom[NetSupported], XA_ATOM, 32, PropModeReplace, (unsigned char *) net_atom, NetLast);
	XChangeProperty(display, check, net_atom[NetWMCheck], XA_WINDOW, 32, PropModeReplace, (unsigned char *) &check, 1);
	XChangeProperty(display, root, net_atom[NetWMCheck], XA_WINDOW, 32, PropModeReplace, (unsigned char *) &check, 1);
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
	int x, y, ocx, ocy, nx, ny, di;
    unsigned int dui;
    Window dummy;

    XQueryPointer(display, root, &dummy, &dummy, &x, &y, &di, &di, &dui);

	bev = &ev->xbutton;

	cw = get_cwindow(bev->window);

	if (cw == NULL) {
		return;
	} else if (cw->window == root) {
		fprintf(stderr, "root window grabbed, ignoring\n");
	} else if (is_border(bev->window)) {
		if (cw != focused) {
			cwindow_focus(cw);
		}
		int ogx = cw->dims.x;
		int ogy = cw->dims.y;
		/* grab the pointer */
		if (XGrabPointer(display, root, False, PointerMotionMask|ButtonPressMask|ButtonReleaseMask, GrabModeAsync, GrabModeAsync, None, None, CurrentTime) != GrabSuccess) {
			return;
		}
		XEvent tmpe;
		do {
			XMaskEvent(display, PointerMotionMask|ButtonPressMask|ButtonReleaseMask, &tmpe);
			/* move the window */
			if (tmpe.type == MotionNotify) {
				cwindow_move(cw, ogx + (tmpe.xmotion.x - x), ogy + (tmpe.xmotion.y - y));
			}

		} while (tmpe.type != ButtonRelease);
		XUngrabPointer(display, CurrentTime);
	} else if (cw != focused) {
		cwindow_focus(cw);
	}

	fprintf(stderr, "button press handled\n");
}

static void handle_property_notify(XEvent *ev) {
	XPropertyEvent *pev = &ev->xproperty;
	struct cwindow *cw = get_cwindow(pev->window);
	if (cw == NULL) {
		return;
	}

	if (pev->atom == net_atom[NetWMName]) {
		XTextProperty tp;
		if (!XGetTextProperty(display, cw->window, &tp, net_atom[NetWMName])) {
			fprintf(stderr, "unable to change name\n");
			return;
		}
		char placeholder[512];
		/* copy title to placeholder */
		strncpy(placeholder, (char *)tp.value, sizeof(placeholder) - 5);
		/* add tag number to cwindow title */
		sprintf(cw->title, "%d -%s", cw->tag, placeholder);
		XFree(tp.value);

		if (cw == focused) {
			draw_text(cw, conf_u_color);
		} else {
			draw_text(cw, conf_f_color);
		}
	}

	fprintf(stderr, "property notify handled\n");
}

static void handle_expose(XEvent *ev) {
	XExposeEvent *eev;
	eev = &ev->xexpose;
	struct cwindow *cw;

	cw = get_cwindow(eev->window);

	if (cw == NULL) {
		return;
	}

	if (cw == focused) {
		draw_text(cw, conf_u_color);
	} else {
		draw_text(cw, conf_f_color);
	}

	fprintf(stderr, "expose handled\n");
}

static void manage_window(Window w, XWindowAttributes *wa) {
	/* create the client window struct */
	struct cwindow *cw;
	cw = malloc(sizeof(struct cwindow));
	Atom prop, da;
	unsigned char *prop_ret = NULL;
    int di;
    unsigned long dl;
	if (XGetWindowProperty(display, w, net_atom[NetWMWindowType], 0, sizeof (Atom), False, XA_ATOM, &da, &di, &dl, &dl, &prop_ret) == Success) {
        if (prop_ret) {
            prop = ((Atom *)prop_ret)[0];
            if (prop == net_atom[NetWMWindowTypeDock] ||
                prop == net_atom[NetWMWindowTypeToolbar] ||
                prop == net_atom[NetWMWindowTypeUtility] ||
                prop == net_atom[NetWMWindowTypeDialog] ||
                prop == net_atom[NetWMWindowTypeMenu]) {
                fprintf(stderr, "Window is of type dock, toolbar, utility, menu, or splash: not managing\n");
                fprintf(stderr, "Mapping new window, not managed\n");
                XMapWindow(display, w);
                return;
            }
        }
    }
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
	cw->tag = 0;

	create_decorations(cw);
	/* save to stack */
	cwindow_save(cw, 0);
	/* focus the new window */
	cwindow_focus(cw);

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

		/* grab the mouse (left click), any modifier, over the decoration window */
		XGrabButton(display, 1, AnyModifier, cw->dec, True, ButtonPressMask, GrabModeAsync, GrabModeAsync, None, None);
		/* get exposure events for redrawing the title */
		XSelectInput (display, cw->dec, ExposureMask);
	} else {
		cw->decorated = false;
	}

	pix_mask(cw->window, cw->dims.x, cw->dims.y, cw->dims.w, cw->dims.h, conf_t_height > 0 && conf_t_height > conf_radius);
	/* make window show up */
	XMapWindow(display, cw->dec);
	/* map window and decoration */
	XMapWindow(display, cw->window);
	/* raise the window */
	XRaiseWindow(display, cw->window);
	
	draw_text(cw, conf_u_color);
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

static void draw_text(struct cwindow *cw, long text_color) {
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


static void cwindow_save(struct cwindow *cw, int tag) {
	/* add client window to top of managed window stack */
	cw->next = cw_stack[tag];
	cw_stack[tag] = cw;
	/* add client window to top of focused window stack */
	cw->f_next = f_stack;
	f_stack = cw;
}

static void cwindow_del(struct cwindow *cw) {
	int tag = cw->tag;
	/* remove window from the cwindow list */
	if (cw == cw_stack[tag]) {
		cw_stack[tag] = cw_stack[tag]->next;
	} else {
		struct cwindow *tmp = cw_stack[tag];
		while (tmp != NULL && tmp->next != cw)
			tmp = tmp->next;
		
		tmp->next = tmp->next->next;
	}
	/* update the tag state if we are removingthe last window from the tag */
	if (cw_stack[tag] == NULL) {
		tag_state[tag] = '_';
		XChangeProperty(display, root, atom_tag_state, XA_STRING, 8, PropModeReplace, tag_state, NUM_TAGS);
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
	focused = f_stack;
	cwindow_focus(focused);
}

static void cwindow_focus(struct cwindow *cw) {
	if (cw != NULL && focused != NULL) {
		fprintf(stderr, "unfocus color changed\n");
		change_color(focused, conf_u_color);
		/* change text color */
		draw_text(focused, conf_f_color);
		/* send message to focus client */
		send_icccm(cw, wm_atom[WMTakeFocus]);
	}
	if (cw != NULL) {
		/* change color */
		change_color(cw, conf_f_color);
		/* remove focus from old window */
		XDeleteProperty(display, root, net_atom[NetActiveWindow]);
		/* move window to the end of the focus stack */
		if (cw != f_stack && f_stack != NULL) {
			struct cwindow *tmp = f_stack;
			while (tmp != NULL && tmp->f_next != cw) {
				tmp = tmp->f_next;
			}
			tmp->f_next = tmp->f_next->f_next;
			cw->f_next = f_stack;
			f_stack = cw;
		}
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
		/* change text color */
		draw_text(cw, conf_u_color);
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

static bool is_border(Window w) {
	for (int i = 0; i < NUM_TAGS; i++) {
		for (struct cwindow *tmp = cw_stack[i]; tmp != NULL; tmp=tmp->next) {
			if (tmp->window == w) {
				return false;
			} else if (tmp->decorated && tmp->dec == w) {
				return true;
			}
		}
	}
	return false;
}

static void handle_focus_cardinal(long *data) {
	fprintf(stderr, "cardinal focus event:\n");
	/* temporary cwindows for loop and focusing */
	struct cwindow *curr, *next_to_focus;

	curr = f_stack;
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
		curr = curr->f_next;
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
		cw->dims.x = dx;
		cw->dims.y = dy;
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
	conf_init();

	for (int i = 0; i < NUM_TAGS; i++) {
		for (struct cwindow *tmp = cw_stack[i]; tmp != NULL; tmp=tmp->next) {
			if (focused->decorated) {
				XDestroyWindow(display, focused->dec);
			}
			tmp->decorated = false;
			create_decorations(tmp);
		}
	}
}

static void handle_assign_tag(long *data) {
	/* check if the focused window exists */
	if (focused == NULL || f_stack == NULL) {
		fprintf(stderr, "no window focused\n");
		return;
	}
	int new_tag = data[1];
	int tag = focused->tag;
	/* change tag in title */
	draw_text(focused, conf_u_color);

	if (new_tag > NUM_TAGS || new_tag < 0) {
		fprintf(stderr, "tag %d does not exist\n", new_tag);
		return;
	}
	/* remove from previous tag */
	if (focused == cw_stack[tag]) {
		cw_stack[tag] = cw_stack[tag]->next;
	} else {
		struct cwindow *tmp = cw_stack[tag];
		while (tmp != NULL && tmp->next != focused)
			tmp = tmp->next;
		
		tmp->next = tmp->next->next;
	}

	/* add client window to top of managed window stack */
	focused->next = cw_stack[new_tag];
	cw_stack[new_tag] = focused;
	focused->tag = new_tag;

	/* change state of old tag */
	if (cw_stack[tag] == NULL) {
		tag_state[tag] = '_';
	}
	/* change state of new tag (if not 0) */
	if (new_tag != 0) {
		tag_state[new_tag] = 'a';
		if (!tag_visible[new_tag]) {
			tag_state[new_tag] = 'i';
			cwindow_hide(focused);
		}
	}
	
	fprintf(stderr, "window removed from tag %d\n", tag);
	fprintf(stderr, "window assigned to tag %d\n", new_tag);
	fprintf(stderr, "tag state %s\n", tag_state);
	XChangeProperty(display, root, atom_tag_state, XA_STRING, 8, PropModeReplace, tag_state, NUM_TAGS);
}

static void handle_toggle_tag(long *data) {
	int tag = data[1];
	/* check if the argument is valid */
	if (tag > NUM_TAGS || tag < 1) {
		fprintf(stderr, "tag %d does not exist\n", tag);
		return;
	}
	/* do northing if there are no windows in the selected tag */
	if (cw_stack[tag] == NULL) {
		fprintf(stderr, "tag %d empty\n", tag);
		return;
	}
	/* do different things depending on if the tag is currently visible */
	struct cwindow *tmp;
	if (tag_visible[tag]) {
		tag_visible[tag] = false;
		tag_state[tag] = 'i';
		for (tmp = cw_stack[tag]; tmp != NULL; tmp=tmp->next) {
			cwindow_hide(tmp);
		}
		XChangeProperty(display, root, atom_tag_state, XA_STRING, 8, PropModeReplace, tag_state, NUM_TAGS);
		fprintf(stderr, "tag %d now invisible\n", tag);
	} else {
		tag_visible[tag] = true;
		tag_state[tag] = 'a';
		for (tmp = cw_stack[tag]; tmp != NULL; tmp=tmp->next) {
			cwindow_show(tmp);
		}
		XChangeProperty(display, root, atom_tag_state, XA_STRING, 8, PropModeReplace, tag_state, NUM_TAGS);
		fprintf(stderr, "tag %d now visible\n", tag);
	}
}

static void handle_close_client(long *data) {
	if (focused == NULL) {
		return;
	}
	XEvent ev;
    ev.type = ClientMessage;
    ev.xclient.window = focused->window;
    ev.xclient.message_type = wm_atom[WMProtocols];
    ev.xclient.format = 32;
    ev.xclient.data.l[0] = wm_atom[WMDeleteWindow];
    ev.xclient.data.l[1] = CurrentTime;
    XSendEvent(display, focused->window, False, NoEventMask, &ev);
	fprintf(stderr, "closing client\n");
}

static void cwindow_show(struct cwindow *cw) {
			if (cw->decorated) {
				XMoveWindow(display, cw->window, cw->dims.x + conf_b_width, cw->dims.y + conf_b_width + conf_t_height);
				XMoveWindow(display, cw->dec, cw->dims.x, cw->dims.y);
			} else {
				XMoveWindow(display, cw->window, cw->dims.x, cw->dims.y);
			}
			/* add back to the focus list */
			cw->f_next = f_stack;
			f_stack = cw;
			if (focused == NULL) {
				fprintf(stderr, "no windows currently focused, taking focus");
				cwindow_focus(cw);
			}
}

static void cwindow_hide(struct cwindow *cw) {
	XMoveWindow(display, cw->window, -1000, -1000);
	XMoveWindow(display, cw->dec, -1000, -1000);
	change_color(cw, conf_u_color);
	/* remove from focus list */
	if (cw == f_stack) {
		f_stack = f_stack->f_next;
	} else {
		struct cwindow *tmp = f_stack;
		while (tmp != NULL && tmp->f_next != cw)
			tmp = tmp->f_next;

		tmp->f_next = tmp->f_next->f_next;
	}	
	/* focus next window if currently focused */
	if (cw == focused) {
		focused = f_stack;
		cwindow_focus(focused);
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
