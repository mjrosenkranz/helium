#ifndef CWINDOW_H
#define CWINDOW_H value

#include <X11/Xlib.h>

struct cwindow_dims {
	int x, y, w, h;
};

struct cwindow {
	/* client window */
	Window window, dec;
	/* is it dectorated */
	bool decorated;
	/* dimentions of the window */
	struct cwindow_dims dims;
	/* next cwindow in list */
	struct cwindow *next, *f_next;
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
struct cwindow *get_cwindow(Window w);
bool is_border(Window w);
void cwindow_move(struct cwindow *cw, int dx, int dy);
void cwindow_show(struct cwindow *cw);
void cwindow_hide(struct cwindow *cw);
int distance(struct cwindow *a, struct cwindow *b);
void change_color(struct cwindow *cw, unsigned long color);

#endif