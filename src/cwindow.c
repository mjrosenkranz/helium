#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <X11/extensions/shape.h>
#include "wm.h"
#include "cwindow.h"



void manage_window(Window w, XWindowAttributes *wa) {
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

void create_decorations(struct cwindow *cw) {
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

void pix_mask(Window win, int x, int y, int w, int h, bool top) {

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
void cwindow_save(struct cwindow *cw, int tag) {
	/* add client window to top of managed window stack */
	cw->next = cw_stack[tag];
	cw_stack[tag] = cw;
	/* add client window to top of focused window stack */
	cw->f_next = f_stack;
	f_stack = cw;
}

void cwindow_del(struct cwindow *cw) {
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

void cwindow_focus(struct cwindow *cw) {
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

struct cwindow *get_cwindow(Window w) {
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

bool is_border(Window w) {
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

void cwindow_move(struct cwindow *cw, int dx, int dy) {
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


void cwindow_show(struct cwindow *cw) {
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

void cwindow_hide(struct cwindow *cw) {
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

int distance(struct cwindow *a, struct cwindow *b) {
	int x_diff, y_diff;
    x_diff = a->dims.x - b->dims.x;
    y_diff = a->dims.y - b->dims.y;
    return pow(x_diff, 2) + pow(y_diff, 2);
}

void change_color(struct cwindow *cw, unsigned long color) {
	if (cw->decorated) {
		fprintf(stderr, "color changed\n");
		XSetWindowBackground(display, cw->dec, color);
		XClearWindow(display, cw->dec);
	}
}
