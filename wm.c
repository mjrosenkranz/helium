/* std includes */
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <X11/extensions/shape.h>
/*libraries */
#include <X11/Xlib.h>
#include <X11/Xatom.h>
/* my includes */
#include "structs.h"

/* display struct */
static Display *display;
/* screen number, width, and height*/
static int screen_num, disp_width, disp_height;
/* root window */
static Window root, check;
/* struct for storing configuration in */
struct config conf;
/* manages the list of existing windows and focus order*/
struct cwindow *cw_stack = NULL, *f_stack = NULL;
/* currently focused window */
struct cwindow *focused = NULL;
/* atom for focus */
static Atom net_atom[NetLast], wm_atom[WMLast];

/* functions */
void open_display();
void close_display();
void run();
void handle_map_request(XEvent *ev);
void handle_unmap_notify(XEvent *ev);
void handle_configure_request(XEvent *ev);
void handle_configure_notify(XEvent *ev);
void handle_client_message(XEvent *ev);
void handle_button_press(XEvent *ev);
void handle_property_notify(XEvent *ev);
void handle_expose(XEvent *ev);
void manage_window(Window w, XWindowAttributes *wa);
void create_decorations(struct cwindow *cw);
void pix_mask(Window win, int x, int y, int w, int h, bool top);
void cwindow_save(struct cwindow *cw);
void cwindow_del(struct cwindow *cw);
void cwindow_focus(struct cwindow *cw);
void change_color(struct cwindow *cw, unsigned long color);
int send_icccm(struct cwindow *cw, Atom atom);
struct cwindow *get_cwindow(Window w);

/* array of functions for ewindow events */
static void (*event_handler[LASTEvent])(XEvent *ev) = {
	[MapRequest]       = handle_map_request,
    [UnmapNotify]      = handle_unmap_notify,
    [ConfigureNotify]  = handle_configure_notify,
    [ConfigureRequest] = handle_configure_request,
    [ClientMessage]    = handle_client_message,
    [ButtonPress]      = handle_button_press,
    [PropertyNotify]   = handle_property_notify,
	[Expose] = handle_expose
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
	/* default config stuff */
	conf.b_width = 5;
	conf.radius = 15;
	conf.t_height = 20;
	conf.u_color = 0x343d41;
	conf.f_color = 0xb89ca8;
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
}

void close_display() {
	XCloseDisplay(display);
	fprintf(stderr, "closing display\n");
}

void run(){

	/* next queued event */
	XEvent qev;

	while(True) {
		/* outputs the next event in the queue to qev */
		XNextEvent(display, &qev);
		/* check if event is something we can handle */
		if(event_handler[qev.type]){
			event_handler[qev.type](&qev);
		}
	}
}

void handle_map_request(XEvent *ev) {
	
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

void handle_unmap_notify(XEvent *ev) {
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
		//free(cw);
		fprintf(stderr, "unmap notify handled\n");
	} else {
		fprintf(stderr, "window to unmap not found\n");
	}
}

void handle_configure_request(XEvent *ev) {

	fprintf(stderr, "configure request handled\n");
}

void handle_configure_notify(XEvent *ev) {

	fprintf(stderr, "configure notify handled\n");
}

void handle_client_message(XEvent *ev) {

	fprintf(stderr, "client message handled\n");
}

void handle_button_press(XEvent *ev) {

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

void handle_property_notify(XEvent *ev) {

	fprintf(stderr, "property notify handled\n");
}

void handle_expose(XEvent *ev) {
	fprintf(stderr, "expose handled\n");
}

void manage_window(Window w, XWindowAttributes *wa) {
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
	/* map window and decoration */
	XMapWindow(display, cw->window);
	/* focus the new window */
	cwindow_focus(cw);
	/* save to stack */
	cwindow_save(cw);

	/* raise the window */
	XRaiseWindow(display, cw->window);
	/* subscribe to window events */
	XSelectInput(display, cw->window, EnterWindowMask|FocusChangeMask|PropertyChangeMask|StructureNotifyMask);
}

void create_decorations(struct cwindow *cw) {
	/* calculate dummy window dimentions */
	int x = cw->dims.x;
	int y = cw->dims.y;
	int w = cw->dims.w + 2 * conf.b_width;
	int h = cw->dims.h + 2 * conf.b_width + conf.t_height;

	if (conf.b_width > 0 || conf.t_height > 0) {
		XMoveWindow(display, cw->window, x + conf.b_width, y + conf.b_width + conf.t_height);
		/* create the border window */
		Window dec = XCreateSimpleWindow(display, root, x, y, w, h, 0, conf.u_color, conf.u_color);
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

	pix_mask(cw->window, cw->dims.x, cw->dims.y, cw->dims.w, cw->dims.h, conf.t_height > 0);
	fprintf(stderr, "decorations created\n");
}

void pix_mask(Window win, int x, int y, int w, int h, bool top) {

	/* create rounded corners */
	/* mask for drawing to */
	Pixmap mask = XCreatePixmap(display, win, w, h, 1);
	/* graphics context */
	GC gc = XCreateGC(display, mask, 0, 0);

	int diam = 2 * conf.radius;
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
	XFillRectangle(display, mask, gc, conf.radius, 0, w - diam, h);
	/* left and right rectangles */
	XFillRectangle(display, mask, gc, 0, conf.radius, conf.radius, h - diam);
	XFillRectangle(display, mask, gc, w - conf.radius, conf.radius, conf.radius, h - diam);
	/* combine everything, masks out area painted black */
	XShapeCombineMask(display, win, ShapeBounding, 0, 0, mask, ShapeSet);
	XFreePixmap(display, mask);
}

void cwindow_save(struct cwindow *cw) {
	/* add client window to top of managed window stack */
	cw->next = cw_stack;
	cw_stack = cw;
	/* add client window to top of focused window stack */
	cw->f_next = f_stack;
	f_stack = cw;
}

void cwindow_del(struct cwindow *cw) {
	/* remove window from the cwindow list */
	if (cw == cw_stack) {
		cw_stack = cw_stack->next;
	} else {
		struct cwindow *tmp = cw_stack;
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

void cwindow_focus(struct cwindow *cw) {
	if (cw != NULL && focused != NULL) {
		fprintf(stderr, "unfocus color changed\n");
		change_color(focused, conf.u_color);
		/* send message to focus client */
		send_icccm(cw, wm_atom[WMTakeFocus]);
	}
	if (cw != NULL) {
		/* change color */
		change_color(cw, conf.f_color);
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

void change_color(struct cwindow *cw, unsigned long color) {
	if (cw->decorated) {
		fprintf(stderr, "color changed\n");
		XSetWindowBackground(display, cw->dec, color);
		XClearWindow(display, cw->dec);
	}
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

struct cwindow *get_cwindow(Window w) {
	for (struct cwindow *tmp = cw_stack; tmp != NULL; tmp=tmp->next) {
		if (tmp->window == w) {
			return tmp;
		} else if (tmp->decorated && tmp->dec == w) {
			return tmp;
		}
	}
	
	return NULL;
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
