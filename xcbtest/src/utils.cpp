#include "utils.h"
#include "client.h"

bool is_managed(xcb_window_t w) {
    for (client *c : tags[0]) {
        if (c->win == w || c->dec == w) {
            fprintf(stderr, "found window\n");
            return true;
        }
    }
    fprintf(stderr, "window %x not found\n", w);
    return false;
}

client *get_client(xcb_window_t w) {
    for (client *c : tags[0]) {
        if (c->win == w || c->dec == w) {
            fprintf(stderr, "found window\n");
            return c;
        }
    }
    fprintf(stderr, "window %x not found\n", w);
    return NULL;
}
