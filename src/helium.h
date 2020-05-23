#ifndef HELIUM_H
#define HELIUM_H
#include <vector>
#include <map>
#include <deque>
#include "client.h"
// the number of tags we have
#define NUMTAGS 8
#define BUFFLEN 100
#define SOCKET_PATH "/tmp/helium_socket"
#define MOVE_MOD XCB_MOD_MASK_4
// we use tag 0 as untagged so we need to add 1 to the # tags
extern std::vector<Client *> tags[NUMTAGS + 1];
extern bool visible[NUMTAGS + 1];
extern std::deque<Client *> focus_queue;

extern xcb_screen_t *screen;
// config stuff
extern std::map<std::string, unsigned int> config;

#endif
