#ifndef HANDLE_H
#define HANDLE_H

#include <X11/Xlib.h>
#include "ipc.h"
#include "hints.h"

extern void (*event_handler[LASTEvent])(XEvent *ev);
extern void (*client_event_handler[ipc_last])(long *data);
	
void handle_map_request(XEvent *ev);
void handle_unmap_notify(XEvent *ev);
void handle_destroy_event(XEvent *ev);
void handle_configure_request(XEvent *ev);
void handle_configure_notify(XEvent *ev);
void handle_client_message(XEvent *ev);
void handle_button_press(XEvent *ev);
void handle_property_notify(XEvent *ev);
void handle_expose(XEvent *ev);
void handle_focus_cardinal(long *data);
void handle_move_relative(long *data);
void handle_move_to(long *data);
void handle_resize_relative(long *data);
void handle_exit(long *data);
void handle_reload(long *data);
void handle_assign_tag(long *data);
void handle_toggle_tag(long *data);
void handle_close_client(long *data);
void handle_get_info(long *data);

#endif