#ifndef RESOURCES_H
#define RESOURCES_H

/* items we are looking for in xresources */
enum config_elements {
	B_WIDTH,
	RADIUS,
	T_HEIGHT,
	U_COLOR,
	F_COLOR,
	FONT,
	CONF_LAST,
};
/* type of the resource */
enum element_format {
	INT,
	COLOR,
	STRING,
};
/* union data structure for the config */
union config {
	int i;
	long c;
	char s[50];
};
/* struct defining a config item */
struct config_item {
	char *name;
	enum config_elements item;
	enum element_format format;
	union config *conf;
};


#endif
