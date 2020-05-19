#include "msg.h"
#include <iostream>
#include "helium.h"
//#include "client.h"

std::string msg_move(std::vector<std::string> args) {
	
	// check if we have the right number of arguments
	// we can have extra once and just ignore them
	if (args.size() >= 3) {
		
		// verify that these are inegers
		int x;
		int y;

		try {
			x = std::stoi(args[1], NULL);
			y = std::stoi(args[2], NULL);
		} catch(std::invalid_argument& e) {
			return "arguments must be integers";
		}


		std::clog << "move window x: " << x
			<< " y: " << y << std::endl;

		return "success";
	}

	return "move requires 2 arguments";

}


std::string msg_tags(std::vector<std::string> args) {
	std::string ret = "";
	// print state of the tags
	for (int i = 1; i < NUMTAGS + 1; ++i) {
		if (visible[i]) {
			ret += std::to_string(tags[i].size());
		} else {
			ret += "_";
		}
	}
	return ret;
}
