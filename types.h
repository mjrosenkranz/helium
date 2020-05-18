/*
 * Definitions for the client and tag types
 */

#ifndef TYPES
#define TYPES

typedef struct client client_t;
struct client {
	xcb_drawable_t id;
	int x, y;
	int w, h;
};

void print_client(client_t *c) {
	printf("id: %x, x: %d, y: %d, w: %d, h: %d\n",
			c->id, c->x, c->y, c->w, c->h);
}

#endif
