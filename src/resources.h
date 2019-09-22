#ifndef RESOURCES_H
#define RESOURCES_H

/* items we are looking for in xresources */
// enum config_elements {
// 	B_WIDTH,
// 	I_WIDTH,
// 	RADIUS,
// 	T_HEIGHT,
// 	U_COLOR,
// 	F_COLOR,
// 	FONT,
// 	CONF_LAST,
// };
/* type of the resource */
enum element_format {
	INT,
	COLOR,
	STRING,
};
/* struct for keeping track of prefernces set in xresources */
typedef struct {
	char *name;
	enum element_format format;
	void *dst;
} pref;


#endif
