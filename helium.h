#ifndef HELIUM_H
#define HELIUM_H
#include <vector>
#include <deque>
#include "client.h"
// the number of tags we have
#define NUMTAGS 8
#define BUFFLEN 100
#define SOCKET_PATH "/tmp/helium_socket"

// we use tag 0 as untagged so we need to add 1 to the # tags
extern std::vector<Client *> tags[NUMTAGS + 1];
extern bool visible[NUMTAGS + 1];
extern std::deque<Client *> focus_queue;

#endif
