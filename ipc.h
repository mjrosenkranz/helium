#ifndef IPC_H
#define IPC_H
#include <stdbool.h>

enum ipc_command {
	ipc_focus_cardinal,/* takes a direction */
	ipc_move_relative, /* takes x and y to move by */
	ipc_move_to, /* takes x and y to move to */
	ipc_resize_relative, /* takes an amount and a direction */
	ipc_exit, /* quit the wm */
	ipc_reload,
	ipc_assign_tag,
	ipc_toggle_tag,
	ipc_close_client,
	/*ipc_fullscreen,
	ipc_center, */
	ipc_last /* the length of the enum */
};

struct command {
	char *name;
	enum ipc_command cmd;
	int num_args;
	bool dir;
};

/* directions for use with commands */
enum direction {
	north,
	south,
	east,
	west,
	dir_last /* the length of the enum */
};

#define HELIUMC_EVENT "heliumc_event"
#endif
