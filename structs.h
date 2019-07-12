#ifndef STRUCTS
#define STRUCTS
/* std includes */
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
/*libraries */
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
};

struct config {
	/* configured window border */
	int b_width, t_height, radius;
	unsigned long u_color, f_color;
};

enum atoms_net {
    NetSupported,
    NetNumberOfDesktops,
    NetActiveWindow,
    NetCurrentDesktop,
    NetClientList,
    NetWMStateFullscreen,
    NetWMCheck,
    NetWMState,
    NetWMName,
    NetWMWindowType,
    NetWMWindowTypeMenu,
    NetWMWindowTypeToolbar,
    NetWMWindowTypeDock,
    NetWMWindowTypeDialog,
    NetWMWindowTypeUtility,
    NetWMWindowTypeSplash,
    NetLast
};

enum atoms_wm {
    WMDeleteWindow,
    WMProtocols,
    WMTakeFocus,
    WMLast,
};
#endif
