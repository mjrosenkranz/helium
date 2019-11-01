#include <iostream>
#include <xcb/xcb.h>

#include "main.h"
#include "client.h"

using namespace std;

xcb_connection_t *conn;
xcb_screen_t *screen;

bool open_connection() {
    // number of the screen
    int scrn_num;
    // open display
    conn = xcb_connect(NULL, &scrn_num);
    fprintf(stderr, "screen %d\n", scrn_num);
    // check if there is an error in opening the display
    if (xcb_connection_has_error(conn)) {
        fprintf(stderr, "cannot connect to display\n");
        return false;
    }
    fprintf(stderr, "connected!\n");

      // assign the screen
    xcb_screen_iterator_t iter;
    iter = xcb_setup_roots_iterator(xcb_get_setup(conn));
    int iter_scrn_num = scrn_num;
    
    for (; iter.rem; --iter_scrn_num, xcb_screen_next(&iter)) {
        if (iter_scrn_num == 0)
            screen = iter.data;
    }

    if (!screen) {
        fprintf(stderr, "no screen found\n");
        return false;
    } else {
        fprintf(stderr, "screen found\n");
    }

    return true;
}

int main(int argc, char **argv) {
    if(!open_connection()) {
        fprintf(stderr, "%s\n", "could not open connection");
        return 1;
    }
    client c = client(100, 100, 200, 200);
    c.decorate(false);
    
    getchar();
    fprintf(stderr, "%s\n", "shutting down");
}
