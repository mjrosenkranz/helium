#ifndef CONF_H
#define CONF_H

typedef struct {
	unsigned int b_width;
	unsigned int b_radius;
	unsigned int t_height;
	unsigned long f_color;
	unsigned long u_color;
} config;

extern config conf;

#define NUM_TAGS 7

#endif