#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <X11/Xresource.h>
#include <X11/Xlib.h>
#include "resources.h"

static char *resources;
static XrmDatabase db;
/* temporary variables variables used to assign to the config */
static unsigned long return_long;
static int return_int;
static char *return_str;
/* set config array to default values */
union config conf[CONF_LAST] = {
	{.i = 0},
	{.i = 5},
	{.i = 5},
	{.c = 0x3f3f3f},
	{.c = 0x00ff00},
	{.s = "def font"},
};

/* making our array of config items */
struct config_item resource[] = {
	{"border_width", B_WIDTH, INT, &conf[B_WIDTH]},
	{"border_radius", RADIUS, INT, &conf[RADIUS]},
	{"title_height", T_HEIGHT, INT, &conf[T_HEIGHT]},
	{"focused_color", U_COLOR, COLOR, &conf[U_COLOR]},
	{"unfocused_color", F_COLOR, COLOR, &conf[U_COLOR]},
	{"font", FONT, STRING, &conf[FONT]},
};

void load_resource(struct config_item *item) {


	/* the location in memory of the value we are looking for */
	XrmValue *value;
	/* what we are searching with */
	char class[42], name[42];
	/* the type of the returned value */
	char *return_type;
	/* create the search criteria */
	snprintf(name, 42, "helium.%s", item->name);
	snprintf(class, 42, "Helium.%s", item->name);
	/* get the resource value */
	XrmGetResource(db, name, class, &return_type, value);

	if (value->addr == NULL) {
		fprintf(stderr, "resource %s not found, defaulting to:\n", name, conf[item->item]);
	}



	/* store the value we found in the proper way */
	switch (item->format) {
		case INT :
			return_int = strtoul(value->addr, NULL, 10);
			fprintf(stderr, "%s = %d\n", name, return_int);
			break;
		case COLOR :
			return_long = strtoul(&value->addr[1], NULL, 16);
			fprintf(stderr, "%s = %x\n", name, return_long);
			break;
		case STRING :
			return_str = (value->addr == NULL) ? conf[item->item].s : value->addr;
			fprintf(stderr, "%s = %s\n", name, return_str);
			break;
		default :
			fprintf(stderr, "not a valid format\n");
			break;
	}
}

int main(int argc, char **argv) {
	/* create and open display */
	Display *display;
	display = XOpenDisplay(NULL);
	/* load resouces into a string */
	resources = XResourceManagerString(display);

	//XrmInitialize();

	db = XrmGetStringDatabase(resources);

	if (db == NULL) {
		fprintf(stderr, "resource file not found\n");
		return 1;
	}

	struct config_item *curr;

	for (int i = 0; i < CONF_LAST; i++) {
		load_resource(&resource[i]);
	}

	XCloseDisplay(display);
}
