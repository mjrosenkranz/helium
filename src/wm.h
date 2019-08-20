#ifndef WM_H
#define WM_H
#include <X11/Xft/Xft.h>
#include <X11/Xlib.h>
#include <X11/Xresource.h>
#include <X11/Xatom.h>
#include <stdbool.h>
#include "config.h"
#include "resources.h"
#include "hints.h"

extern Display *display;
extern struct cwindow *cw_stack[NUM_TAGS];
extern struct cwindow *f_stack;
extern struct cwindow *focused;
extern Window root;
extern Window check;
extern int conf_b_width;
extern int conf_radius;
extern int conf_t_height;
extern long conf_u_color;
extern long conf_f_color;
extern char *conf_font;
extern XftFont *font;
extern XftColor tmp_color;
extern XRenderColor r_color;
extern bool tag_visible[NUM_TAGS];
extern char tag_state[NUM_TAGS];
extern bool running;




/* functions */
void load_color(XftColor *dest_color, unsigned long raw_color);
void draw_text(struct cwindow *cw, long text_color);

void open_display();
void close_display();
void run();
int send_icccm(struct cwindow *cw, Atom atom);
void load_resource(XrmDatabase db, struct pref *item);
void conf_init();
#endif