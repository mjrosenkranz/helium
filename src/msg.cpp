#include "msg.h"
#include <iostream>
#include <algorithm>
#include <deque>
#include <climits>
#include "helium.h"
#include "client.h"
#include "util.h"

std::string msg_move_relative(std::vector<std::string> args) {
	
	// check if we have the right number of arguments
	// we can have extra once and just ignore them
	if (args.size() < 3)
		return "move requires 2 arguments";
		
	// verify that these are inegers
	int x;
	int y;

	try {
		x = std::stoi(args[1], NULL);
		y = std::stoi(args[2], NULL);
	} catch(std::invalid_argument& e) {
		return "arguments must be integers";
	}


//	std::clog << "move window x: " << x
//		<< " y: " << y << std::endl;

	// get the currently focused window
	Client *c = focus_queue.front();

	if (c == NULL)
		return "no window focused";

	c->move_relative(x, y);

	return "success";
}

std::string msg_move_absolute(std::vector<std::string> args) {
	
	// check if we have the right number of arguments
	// we can have extra once and just ignore them
	if (args.size() < 3)
		return "move requires 2 arguments";
		
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

	// get the currently focused window
	Client *c = focus_queue.front();

	if (c == NULL)
		return "no window focused";

	c->move_absolute(x, y);

	return "success";


}

std::string msg_change_tag(std::vector<std::string> args) {
	if (args.size() < 2) 
		return "need a tag to change to";

	int t;
	try {
		t = std::stoi(args[1], NULL);
	} catch(std::invalid_argument& e) {
		return "tag must be an integer";
	}


	// get the currently focused window
	Client *c = focus_queue.front();

	if (c == NULL)
		return "no window focused";

	c->change_tag(t);

	return "success";
}

std::string msg_print_tags(std::vector<std::string> args) {
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


// Focus cardinal, order and by #
std::string msg_focus(std::vector<std::string> args) {
	
	if (args.size() < 2) 
		return "need a window or direction to focus";

	// check if there are even other windows to focus
	if (focus_queue.size() < 2)
		return "success";

	// client we are focusing
	Client *to_focus;

	// is is previous or next?
	if (args[1].compare("prev") == 0) {
		std::rotate(focus_queue.begin(),
				focus_queue.begin() + 1,
				focus_queue.end());

		to_focus = focus_queue.front();

		if (to_focus == NULL)
			return "no window focused";
		to_focus->focus();
		return "success";
	}
	// TODO: fix this guy
	if (args[1].compare("next") == 0) {

		Client *last_focus = focus_queue.front();
		std::rotate(focus_queue.begin(),
				focus_queue.begin() + focus_queue.size() - 1,
				focus_queue.end());

		to_focus = focus_queue.front();
		if (to_focus == NULL)
			return "no window focused";
		to_focus->focus();
		last_focus->decorate();
		return "success";
	}

	// focus in direction
	if (args[1].compare("north") == 0) {
		// arbitrarily large number
		Client *last_focus = focus_queue.front();
		// corners of our focused window
		std::vector<int> fc = last_focus->get_corners();
		Client *to_focus =  focus_queue.front();
		double best = INT_MAX;
		// loop through all visible
		for (Client *tmp : focus_queue) {
			// disregard if its the last focus
			if (last_focus == tmp)
				continue;

			std::vector<int> tmpc = tmp->get_corners();
			// make sure the top corner is higher than ours
			if (tmpc[1] > fc[1])
				continue;

			for (int i = 0; i < 8; i+=2) {
				// get the closest corner to the top left
				// discard corner if we are "above it"
				// aka our y value is greater
				if (fc[1] < tmpc[i+1])
					continue;
				

				int dtl = dist(tmpc[i], tmpc[i+1], fc[0], fc[1]);

				if (dtl < best) {
					best = dtl;
					to_focus = tmp;
				}
				// get the closest corner to the top right
				int dtr = dist(tmpc[i], tmpc[i+1], fc[2], fc[3]);

				if (dtr < best) {
					best = dtr;
					to_focus = tmp;
				}
			}
		}

		to_focus->focus();
		// distance score based on sqrt of distances
		//
		return "success";
	}


	if (args[1].compare("south") == 0) {
		// arbitrarily large number
		Client *last_focus = focus_queue.front();
		// corners of our focused window
		std::vector<int> fc = last_focus->get_corners();
		Client *to_focus =  focus_queue.front();
		double best = INT_MAX;
		// loop through all visible
		for (Client *tmp : focus_queue) {
			// disregard if its the last focus
			if (last_focus == tmp)
				continue;


			std::vector<int> tmpc = tmp->get_corners();
			// make sure the top corner is lower than ours
			if (tmpc[1] < fc[1])
				continue;

			for (int i = 0; i < 8; i+=2) {
				// get the closest corner to the top left
				// discard corner if we are "above it"
				// aka our y value is greater

				int dtl = dist(tmpc[i], tmpc[i+1], fc[0], fc[1]);

				if (dtl < best) {
					best = dtl;
					to_focus = tmp;
				}
				// get the closest corner to the top right
				int dtr = dist(tmpc[i], tmpc[i+1], fc[0], fc[1]);

				if (dtr < best) {
					best = dtr;
					to_focus = tmp;
				}
			}
		}

		to_focus->focus();
		// distance score based on sqrt of distances
		//
		return "success";
	}

	// try to focus window by id
	try {
		unsigned int id = std::stoi(args[1], NULL, 16);
		to_focus = get_client((xcb_drawable_t*) &id);

		if (to_focus == NULL)
			return args[1] + " not a valid window";

		to_focus->focus();

		return "success";
	} catch(std::invalid_argument& e) {
		return args[1] + " not a valid direction";
	}
}

std::string msg_resize(std::vector<std::string> args) {

	Client *c = focus_queue.front();

	if (c == NULL)
		return "no window focused";

	if (args.size() < 3) 
		return "need a side and amount";

	try {
		int amt = std::stoi(args[2], NULL);

		if (c->resize_relative(args[1], amt)) {
			c->decorate();
		} else {
			return args[1] + " not a valid direction";
		}

		return "success";
		
	} catch(std::invalid_argument& e) {
		return args[2] + " not a valid amount";
	}

}

std::string msg_kill(std::vector<std::string>) {
	Client *c = focus_queue.front();
	c->kill();
	return "success";
}

// toggle a tag
std::string msg_toggle(std::vector<std::string> args) {
	if (args.size() < 2)
		return "move requires at least one argument";

	int t;
	try {
		t = std::stoi(args[1], NULL);
	} catch(std::invalid_argument& e) {
		return "tag must be an integer";
	}

	if (t == 0 || t > NUMTAGS + 1)
		return "tag " + args[1] + " does not exist";

	// toggle visibility
	visible[t] = !visible[t];
	if (visible[t]) {
		// if the focus queue is empty then we want to focus the new guys
		bool focus_last = focus_queue.size() == 0;
		// set things visible
		for (Client *c : tags[t]) {
			c->set_visible(true);
		}

		if (focus_last)
			focus_queue.front()->focus();
	} else {
		// hide tag members
		for (Client *c : tags[t]) {
			c->set_visible(false);
		}
	}


	return "success";
}

std::string msg_config(std::vector<std::string> args) {

	// check if the first value is a config variable
	if (config.find(args[1]) == config.end())
		return args[1] + " invalid config value";

	// colors are in base 16
	if (args[1].find("color") != std::string::npos) {

		unsigned int val;
		try {
			val = std::stoul(args[2], NULL, 16);
		} catch(std::invalid_argument& e) {
			return "value must be an integer";
		}
		config[args[1]] = val;

		// repaint all visible windows
		for (Client *c : focus_queue) {
			c->decorate();
		}

		return "success";
	} else {

		unsigned int val;
		try {
			val = std::stoul(args[2], NULL);
		} catch(std::invalid_argument& e) {
			return "value must be an integer";
		}

		config[args[1]] = val;
		return "success";
	}

}
