#pragma once
#include <xcb/xcb.h>
#include "client.h"

bool is_managed(xcb_window_t w);
client *get_client(xcb_window_t w);
client *get_client_from_win(xcb_window_t w);
client *get_client_from_dec(xcb_window_t w);
