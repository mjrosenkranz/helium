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
	/* tag to which the window belongs */
	int tag;
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

#define DEFAULT_B_WIDTH 5
#define DEFAULT_RADIUS 5
#define DEFAULT_T_HEIGHT 25
#define DEFAULT_U_COLOR 0x3f3f3f
#define DEFAULT_F_COLOR 0x40E446
#define DEFAULT_FONT "-*-terminus-*-*-*-*-12-*-*-*-*-*-*-*"
#define NUM_TAGS 9

#endif
