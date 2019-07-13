/* library includes */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
/*libraries */
#include <X11/Xlib.h>
/* my headers */
#include "ipc.h"

static void usage();
/* TODO
 * - get this working
 * - cardinal focus
 * - focus list only comprised of visible windows, just a list
 * - window list is array of ll for each tag
 */

static void usage() {
	fprintf(stderr, "usage: heliumc [-h help] <COMMAND> [arguments]\n");
	exit(EXIT_SUCCESS);;
}

/* array of ipc commands */
static struct command cmds[] = {
	{"focus_dir", ipc_focus_cardinal, 1},
};

int main(int argc, char *argv[]) {
	fprintf(stderr, "arguments:\n");
	if (argc < 2) {
		usage();
	}

	/* loop over all ipc commands for the one specified by commandline argument */
	for (int i = 0; i < ipc_last; i++) {
		if (strcmp(cmds[i].name, argv[1]) == 0) {
			fprintf(stderr, "%s\n", cmds[i].name);
		}
	}

	return 0;
}
