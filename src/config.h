#ifndef CONFIG_H
#define CONFIG_H

#define DEFAULT_B_WIDTH 5
#define DEFAULT_I_WIDTH 5
#define DEFAULT_RADIUS 5
#define DEFAULT_T_HEIGHT 25
#define DEFAULT_U_COLOR 0x3f3f3f
#define DEFAULT_F_COLOR 0xefefef
#define DEFAULT_IU_COLOR 0x222222
#define DEFAULT_IF_COLOR 0x40E446
#define DEFAULT_TU_COLOR 0xefefef
#define DEFAULT_TF_COLOR 0x00ff00
#define DEFAULT_FONT "-*-terminus-*-*-*-*-12-*-*-*-*-*-*-*"
#define NUM_TAGS 9

typedef struct {
	int b_width;
	int i_width;
	int radius;
	int t_height;
	long u_color;
	long f_color;
	long iu_color;
	long if_color;
	long tf_color;
	long tu_color;
	char *font;
} conf_t;

#endif
