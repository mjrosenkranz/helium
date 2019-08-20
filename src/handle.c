#include <X11/Xlib.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>

#include "handle.h"
#include "ipc.h"
#include "cwindow.h"
#include "wm.h"
#include "config.h"

/* array of functions for cwindow events */
void (*event_handler[LASTEvent])(XEvent *ev) = {
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
void (*client_event_handler[ipc_last])(long *data) = {
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

void handle_map_request(XEvent *ev) {
	
	/* struct of window attributes */
	XWindowAttributes win_attr;
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

void handle_button_press(XEvent *ev) {

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

void handle_property_notify(XEvent *ev) {
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

void handle_expose(XEvent *ev) {
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
void handle_focus_cardinal(long *data) {
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

void handle_move_relative(long *data) {
	fprintf(stderr, "move relative event:\n");
	fprintf(stderr, "\tx:%d y:%d\n", data[1], data[2]);

	if (focused == NULL) {
		return;
	}

	cwindow_move(focused, data[1] + focused->dims.x, data[2] + focused->dims.y);
}

void handle_move_to(long *data) {
	fprintf(stderr, "move to event:\n");
	fprintf(stderr, "\tx:%d y:%d\n", data[1], data[2]);
	
	if (focused == NULL) {
		return;
	}

	cwindow_move(focused, data[1], data[2]);
}
void handle_resize_relative(long *data) {
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

void handle_exit(long *data) {
	running = false;
}

void handle_reload(long *data) {
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

void handle_assign_tag(long *data) {
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

void handle_toggle_tag(long *data) {
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

void handle_close_client(long *data) {
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
