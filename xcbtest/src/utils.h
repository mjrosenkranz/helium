#pragma once
#include <xcb/xcb.h>
#include "client.h"

bool is_managed(xcb_window_t w);
client *get_client(xcb_window_t w);
