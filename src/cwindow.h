#ifndef CWINDOW_H
#define CWINDOW_H value

#include <X11/Xlib.h>
#include "vector.h"

typedef struct cwindow cwindow;

typedef struct {
	int x, y, w, h;
} cwindow_dims ;

struct cwindow {
	/* client window */
	Window window, dec;
	/* is it dectorated */
	bool decorated;
	/* dimentions of the window */
	cwindow_dims dims;
	/* tag to which the window belongs */
	int tag;
	char title[512];
};

void manage_window(Window w, XWindowAttributes *wa);
void create_decorations(struct cwindow *cw);
void pix_mask(Window win, int x, int y, int w, int h, bool top);
void cwindow_save(struct cwindow *cw, int tag);
void cwindow_del(struct cwindow *cw);
void cwindow_focus(struct cwindow *cw);
int get_cwindow_index(vector *vec, cwindow *cw);
cwindow *get_cwindow(Window w);
cwindow *get_cwindow_from_parent(Window w);
cwindow *get_next_cwindow();
void cwindow_move(struct cwindow *cw, int dx, int dy);
void cwindow_show(struct cwindow *cw);
void cwindow_hide(struct cwindow *cw);
int distance(struct cwindow *a, struct cwindow *b);
void change_color(struct cwindow *cw, unsigned long color);

#endif