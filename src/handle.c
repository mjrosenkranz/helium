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
    [DestroyNotify]		= handle_destroy_event,
    [ConfigureNotify]	= handle_configure_notify,
    [ConfigureRequest]	= handle_configure_request,
    [ClientMessage]		= handle_client_message,
    [ButtonPress]		= handle_button_press,
    [PropertyNotify]	= handle_property_notify,
	[Expose]			= handle_expose,
};
/* array of functions for client message events */
void (*client_event_handler[ipc_last])(long *data) = {
	[ipc_focus_cardinal]	= handle_focus_cardinal,
	[ipc_move_relative]		= handle_move_relative,
	[ipc_move_to]			= handle_move_to,
	[ipc_resize_relative]	= handle_resize_relative,
	[ipc_exit]				= handle_exit,
	[ipc_assign_tag]		= handle_assign_tag,
	[ipc_toggle_tag]		= handle_toggle_tag,
	[ipc_close_client]		= handle_close_client,
	[ipc_pointer]			= handle_pointer,
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

	if (get_cwindow(mre->window) == NULL) {
		/* manage new window */
		manage_window(mre->window, &win_attr);
	} else {
		XMapWindow(display, mre->window);
	}

	fprintf(stderr, "map request handled\n");
}

void handle_unmap_notify(XEvent *ev) {
	XUnmapEvent *uev = &ev->xunmap;
	cwindow *cw;
	/* gets the window only if it is a client (not a decoration) */
	cw = get_cwindow(uev->window);
	if (cw != NULL) {
		XUnmapWindow(display, cw->dec);
		fprintf(stderr, "unmap notify handled\n");
	} else {
		fprintf(stderr, "window to unmap not found\n");
	}
}

void handle_destroy_event(XEvent *ev) {
	XDestroyWindowEvent *dwe = &ev->xdestroywindow;
	cwindow *cw;
	cw = get_cwindow(dwe->window);
	/* make sure we have an actual cwindow */
	if (cw != NULL) {
		fprintf(stderr, "window decoration destroyed\n");
		XDestroyWindow(display, cw->dec);
		cwindow_del(cw);
	}

	fprintf(stderr, "%s\n", "destroy event handled");
}

void handle_configure_request(XEvent *ev) {

	cwindow *cw;
	XConfigureRequestEvent *cre = &ev->xconfigurerequest;
	XWindowChanges wc;

	wc.x = cre->x;
    wc.y = cre->y;
    wc.width = cre->width;
    wc.height = cre->height;
    wc.border_width = cre->border_width;
    wc.sibling = cre->above;
    wc.stack_mode = cre->detail;
    XConfigureWindow(display, cre->window, cre->value_mask, &wc);
    
    cw = get_cwindow_from_parent(cre->window);

    if (cw != NULL) {
		int dec_w = wc.width + 2 * conf_b_width;
		int dec_h = wc.height + 2 * conf_b_width + conf_t_height;
		XMoveResizeWindow(display, cw->dec, wc.x, wc.y, dec_w, dec_h);
		pix_mask(cw->dec, wc.x, wc.y, dec_w, dec_h, false);

		XMoveResizeWindow(display, cw->window, conf_b_width, conf_b_width + conf_t_height, wc.width, wc.height);
		pix_mask(cw->window, conf_b_width, conf_b_width + conf_t_height, wc.width, wc.height, conf_t_height > 0 && conf_t_height > conf_radius);
		fprintf(stderr, "configured");
	}

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
	cwindow *cw;
	int x, y, ocx, ocy, nx, ny, di;
    unsigned int dui;
    Window dummy;
    Window child;

    /* have xlib check where the cursor is and return the child (decoration) window */
    XQueryPointer(display, root, &dummy, &child, &x, &y, &di, &di, &dui);

	bev = &ev->xbutton;

	cw = get_cwindow_from_parent(child);

	if (cw == NULL) {
		fprintf(stderr, "%s\n", "not found");
	} else {
		if (cw != focused) {
			cwindow_focus(cw);
		}
	}

	fprintf(stderr, "button press handled\n");
}

void handle_property_notify(XEvent *ev) {
	XPropertyEvent *pev = &ev->xproperty;
	cwindow *cw = get_cwindow(pev->window);
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
	cwindow *cw;

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
	cwindow *curr, *next_to_focus;

	curr = focused;
	next_to_focus = NULL;
	/* for keeping track of the distance */
	int min = 999999;
	/* loop through cwindows */
	
	for (int i = 0; i < NUM_TAGS; i ++) {
		for (int j = 0; j < tags[i]->size; j++) {
			if (tag_visible[i]) {
				curr = tags[i]->elements[j];
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
			}
		}
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
	int dec_w = w + 2 * conf_b_width;
	int dec_h = h + 2 * conf_b_width + conf_t_height;
	XMoveResizeWindow(display, focused->dec, x, y, dec_w, dec_h);
	pix_mask(focused->dec, x, y, dec_w, dec_h, false);

	XMoveResizeWindow(display, focused->window, conf_b_width, conf_b_width + conf_t_height, w, h);
	pix_mask(focused->window, conf_b_width, conf_b_width + conf_t_height, w, h, conf_t_height > 0 && conf_t_height > conf_radius);
	focused->dims.x = x;
	focused->dims.y = y;
	focused->dims.w = w;
	focused->dims.h = h;
	fprintf(stderr, "resize relative event handled:\n");
}

void handle_exit(long *data) {
	running = false;
}

void handle_assign_tag(long *data) {
	int new_tag = data[1];
	int tag = focused->tag;
	focused->tag = new_tag;
	/* change tag in title */
	draw_text(focused, conf_u_color);

	if (new_tag > NUM_TAGS || new_tag < 0) {
		fprintf(stderr, "tag %d does not exist\n", new_tag);
		return;
	}
	/* remove from previous tag */
	int index = get_cwindow_index(tags[tag], focused);
	fprintf(stderr, "%d\n", index);
	remove_from_vector(tags[tag], index);

	/* add client window to new tag */
	add_to_vector(tags[new_tag], focused);

	/* change state of old tag */
	if (tags[tag]->size == 0) {
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
	if (tags[tag]->size == 0) {
		fprintf(stderr, "tag %d empty\n", tag);
		return;
	}
	/* do different things depending on if the tag is currently visible */
	cwindow *tmp;
	if (tag_visible[tag]) {
		tag_visible[tag] = false;
		tag_state[tag] = 'i';
		/* loop thru and hide */
		for (int i = 0; i < tags[tag]->size; i++) {
			tmp = tags[tag]->elements[i];
			if (tmp == focused) {
				focused = get_next_cwindow();
				cwindow_focus(focused);
				fprintf(stderr, "%s\n", "oops focus someone else");
			}
			cwindow_hide(tmp);
		}
		XChangeProperty(display, root, atom_tag_state, XA_STRING, 8, PropModeReplace, tag_state, NUM_TAGS);
		fprintf(stderr, "tag %d now invisible\n", tag);
	} else {
		tag_visible[tag] = true;
		tag_state[tag] = 'a';
		for (int i = 0; i < tags[tag]->size; i++) {
			tmp = tags[tag]->elements[i];
			cwindow_show(tmp);
			cwindow_focus(tmp);
			if (focused == NULL) {
				focused = tmp;
				cwindow_focus(focused);
			}
		}
		XChangeProperty(display, root, atom_tag_state, XA_STRING, 8, PropModeReplace, tag_state, NUM_TAGS);
		fprintf(stderr, "tag %d now visible\n", tag);
	}
}

void handle_pointer(long *data) {
	
	int x, y, ocx, ocy, nx, ny, di;
    unsigned int dui;
    Window dummy, subject;
    cwindow *cw;

    XQueryPointer(display, root, &dummy, &subject, &x, &y, &di, &di, &dui);
    cw = get_cwindow_from_parent(subject);
    if (cw != NULL) {
    	fprintf(stderr, "%s\n", "grabbed window");
    	cwindow_focus(cw);
    }
    fprintf(stderr, "%s\n", "pointer handled");
//	if (cw == NULL) {
//		return;
//	} else {
//		if (cw != focused) {
//			cwindow_focus(cw);
//		}
//		int ogx = cw->dims.x;
//		int ogy = cw->dims.y;
//		/* grab the pointer */
//		if (XGrabPointer(display, root, False, PointerMotionMask|ButtonPressMask|ButtonReleaseMask, GrabModeAsync, GrabModeAsync, None, None, CurrentTime) != GrabSuccess) {
//			return;
//		}
//		XEvent tmpe;
//		do {
//			XMaskEvent(display, PointerMotionMask|ButtonPressMask|ButtonReleaseMask, &tmpe);
//			/* move the window */
//			if (tmpe.type == MotionNotify) {
//				cwindow_move(cw, ogx + (tmpe.xmotion.x - x), ogy + (tmpe.xmotion.y - y));
//			}
//
//		} while (tmpe.type != ButtonRelease);
//		XUngrabPointer(display, CurrentTime);
//	}
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
