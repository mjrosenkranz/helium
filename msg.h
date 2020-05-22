#include <string>
#include <vector>

#ifndef MSG_H
#define MSG_H

std::string msg_move_relative(std::vector<std::string>);
std::string msg_move_absolute(std::vector<std::string>);
std::string msg_change_tag(std::vector<std::string>);
std::string msg_print_tags(std::vector<std::string>);
std::string msg_focus(std::vector<std::string>);
std::string msg_resize(std::vector<std::string>);
std::string msg_kill(std::vector<std::string>);

#endif
