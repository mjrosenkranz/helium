#ifndef HINTS_H
#define HINTS_H

#include <X11/Xatom.h>
#include <X11/Xlib.h>
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

/* atoms */
extern Atom net_atom[NetLast];
extern Atom wm_atom[WMLast];
extern Atom atom_tag_state;
extern Atom atom_winfo;

void init_hints(void);
#endif
