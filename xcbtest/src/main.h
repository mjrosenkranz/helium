#pragma once

extern xcb_connection_t *conn;
extern xcb_screen_t *screen;

bool open_connection();
void dectorate();

// default settings
#define DEF_I_B_WIDTH 5
#define DEF_IF_COLOR 0x00ff00
#define DEF_IU_COLOR 0x3f3f3f

#define DEF_O_B_WIDTH 5
#define DEF_OF_COLOR 0x3f3f3f
#define DEF_OU_COLOR 0x00ff00
