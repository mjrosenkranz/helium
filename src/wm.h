#ifndef WM_H
#define WM_H
#include <X11/Xft/Xft.h>
#include <X11/Xlib.h>
#include <X11/Xresource.h>
#include <X11/Xatom.h>
#include <stdbool.h>
#include "config.h"
#include "hints.h"
#include "cwindow.h"
#include "vector.h"

extern Display *display;
extern vector *tags[NUM_TAGS];
extern cwindow *focused, *p_focused;
extern Window root;
extern Window check;
extern conf_t conf;
extern XftFont *font;
extern XftColor tmp_color;
extern XRenderColor r_color;
extern bool tag_visible[NUM_TAGS];
extern char tag_state[NUM_TAGS];
extern bool running;




/* functions */
void load_color(XftColor *dest_color, unsigned long raw_color);
void draw_text(cwindow *cw, long text_color);

void open_display();
void close_display();
void run();
int send_icccm(cwindow *cw, Atom atom);
#endif
