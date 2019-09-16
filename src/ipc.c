/* library includes */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
/*libraries */
#include <X11/Xlib.h>
#include <X11/Xatom.h>
/* my headers */
#include "ipc.h"

static void usage();
static void send_event();

/* array of ipc commands */
static command cmds[] = {
	{"focus", ipc_focus_cardinal, 1, true},
	{"move", ipc_move_relative, 2, false},
	{"move_to", ipc_move_to, 2, false},
	{"resize", ipc_resize_relative, 2, true},
	{"exit", ipc_exit, 0, false},
	{"tag", ipc_assign_tag, 1, false},
	{"toggle_tag", ipc_toggle_tag, 1, false},
	{"close", ipc_close_client, 0, false},
	{"pointer", ipc_pointer, 0, false},
};

static void usage() {
	fprintf(stderr, "usage: heliumc [-h help] <COMMAND> [arguments]\n");
	exit(EXIT_SUCCESS);;
}

static void send_event(command *c, char **args) {
	Display *display;
	Window root;
	XEvent xev;

	int endex = c->num_args; /* changes depending on if the item takes a direction */


	/* opens display */
	display = XOpenDisplay(NULL);
	/* gets the root window we are talking to */
	root = DefaultRootWindow(display);
	
	xev.xclient.type = ClientMessage;
	xev.xclient.message_type = XInternAtom(display, HELIUMC_EVENT, False);
	xev.xclient.format = 32;

	/* set all data to 0 initially */
	memset(xev.xclient.data.l, 0, sizeof(xev.xclient.data.l));
	xev.xclient.data.l[0] = c->cmd;

	/* change the command to an enum if it contains a direction */
	if (c->dir) {
		int index = c->num_args - 1;
		char *dir = args[c->num_args - 1];

		if (strcmp(dir, "north") == 0) {
			fprintf(stderr, "%d\n", north);
			xev.xclient.data.l[index + 1] = north;
		} else if (strcmp(dir, "south") == 0) {
			fprintf(stderr, "%d\n", south);
			xev.xclient.data.l[index + 1] = south;
		} else if (strcmp(dir, "east") == 0) {
			fprintf(stderr, "%d\n", east);
			xev.xclient.data.l[index + 1] = east;
		} else if (strcmp(dir, "west") == 0) {
			fprintf(stderr, "%d\n", west);
			xev.xclient.data.l[index + 1] = west;
		} else {
			fprintf(stderr, "%s not a valid direction\n", dir);
			exit(EXIT_FAILURE);
		}
		endex = index;
	}

	for (int i = 0; i < endex; i++) {
		xev.xclient.data.l[i + 1] = strtol(args[i], NULL, 10);
	}

	/* send the event to the server */
	XSendEvent(display, root, false, SubstructureRedirectMask, &xev);
	/* flushes the buffer to the xserver */
	XSync(display, false);
	/* closes the connection to the server  */
	XCloseDisplay(display);
}

int main(int argc, char *argv[]) {
	if (argc < 2) {
		usage();
	}

	/* loop over all ipc commands for the one specified by commandline argument */
	for (int i = 0; i < ipc_last; i++) {
		if (strcmp(cmds[i].name, argv[1]) == 0) {
			fprintf(stderr, "%s\n", cmds[i].name);
			if (argc - 2 == cmds[i].num_args) {
				send_event(&cmds[i], argv + 2);
				return 0;
			} else {
				fprintf(stderr, "incorrect number of arguments\n");
				return 1;
			}
		}
	}

	fprintf(stderr, "command \"%s\" not found\n", argv[1]);

	return 1;
}
