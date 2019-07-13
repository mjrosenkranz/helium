#ifndef IPC
#define IPC

enum ipc_command {
	ipc_focus_cardinal,
	ipc_last
};

struct command {
	char *name;
	enum ipc_command cmd;
	int num_args;
};
#endif
