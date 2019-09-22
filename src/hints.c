#include <stdlib.h>
#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include "hints.h"
#include "wm.h"

Atom net_atom[NetLast];
Atom wm_atom[WMLast];
Atom atom_tag_state;
Atom atom_winfo;

void init_hints() {
	/* ewmh */
	net_atom[NetSupported]           = XInternAtom(display, "_NET_SUPPORTED", False);
    net_atom[NetNumberOfDesktops]    = XInternAtom(display, "_NET_NUMBER_OF_DESKTOPS", False);
    net_atom[NetActiveWindow]        = XInternAtom(display, "_NET_ACTIVE_WINDOW", False);
    net_atom[NetWMStateFullscreen]   = XInternAtom(display, "_NET_WM_STATE_FULLSCREEN", False);
    net_atom[NetWMFrameExtents]      = XInternAtom(display, "_NET_FRAME_EXTENTS", False);
    net_atom[NetWMCheck]             = XInternAtom(display, "_NET_SUPPORTING_WM_CHECK", False);
    net_atom[NetCurrentDesktop]      = XInternAtom(display, "_NET_CURRENT_DESKTOP", False);
    net_atom[NetWMState]             = XInternAtom(display, "_NET_WM_STATE", False);
    net_atom[NetWMName]              = XInternAtom(display, "_NET_WM_NAME", False);
    net_atom[NetClientList]          = XInternAtom(display, "_NET_CLIENT_LIST", False);
    net_atom[NetWMWindowType]        = XInternAtom(display, "_NET_WM_WINDOW_TYPE", False);
    net_atom[NetWMWindowTypeDock]    = XInternAtom(display, "_NET_WM_WINDOW_TYPE_DOCK", False);
    net_atom[NetWMWindowTypeToolbar] = XInternAtom(display, "_NET_WM_WINDOW_TYPE_TOOLBAR", False);
    net_atom[NetWMWindowTypeMenu]    = XInternAtom(display, "_NET_WM_WINDOW_TYPE_MENU", False);
    net_atom[NetWMWindowTypeSplash]  = XInternAtom(display, "_NET_WM_WINDOW_TYPE_SPLASH", False);
    /* iccm */
    wm_atom[WMDeleteWindow]          = XInternAtom(display, "WM_DELETE_WINDOW", False);
    wm_atom[WMTakeFocus]             = XInternAtom(display, "WM_TAKE_FOCUS", False);
	wm_atom[WMProtocols] = XInternAtom(display, "WM_PROTOCOLS", False);

	/* reports the state of the tabs */
	atom_tag_state = XInternAtom(display, "TAG_STATE", false);
	/* reports window info */
	atom_winfo = XInternAtom(display, "WINFO", false);
	/* update the wm properties */
	/* gives x an array of supported atoms */
	XChangeProperty(display, root, net_atom[NetSupported], XA_ATOM, 32, PropModeReplace, (unsigned char *) net_atom, NetLast);
	XChangeProperty(display, check, net_atom[NetWMCheck], XA_WINDOW, 32, PropModeReplace, (unsigned char *) &check, 1);
	XChangeProperty(display, root, net_atom[NetWMCheck], XA_WINDOW, 32, PropModeReplace, (unsigned char *) &check, 1);
	fprintf(stderr, "%s\n", "hints initialized");
}
