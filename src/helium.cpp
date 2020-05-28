// This file contains the meat of the wm.
#include <xcb/xcb.h>
#include <vector>
#include <deque>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/un.h>
#include <unistd.h>
#include <string>
#include <sstream>
#include <iostream>
#include <iterator>
#include <map>
#include <csignal>
#include <cmath>

#include "client.h"
#include "msg.h"
#include "helium.h"
#include "util.h"


bool running;
// sockets
unsigned int socket_fd, conn_fd, max_fd, socket2;
fd_set descriptors;
// socket addresses
struct sockaddr_un local, remote;
// length of socket msg
int slen;
// list of all clients in their tags
std::vector<Client *> tags[NUMTAGS + 1];
// status of these tags
bool visible[NUMTAGS + 1];
// clients in their focus order
std::deque<Client *> focus_queue;

xcb_connection_t *conn = NULL;
// the screen
xcb_screen_t *screen = NULL;
// number of screens connected
int num_screens;

// config stuff
std::map<std::string, unsigned int> config;

// a list of events we support
static void (*events[XCB_NO_OPERATION])(xcb_generic_event_t *e);

// a map of strings to their corresponding functions
typedef std::string (*msg_handler)(std::vector<std::string>);
std::map<std::string, msg_handler> msg_map;

// utility functions
void cleanup(int);
static xcb_screen_t *xcb_screen_of_display(xcb_connection_t *, int);

// event handlers
static void printevent(xcb_generic_event_t *);
static void map_request(xcb_generic_event_t *);
static void destroy_notify(xcb_generic_event_t *);
static void button_press(xcb_generic_event_t *ev);
// static void expose(xcb_generic_event_t *ev);

// functions
static bool setup(int);
static bool socket_setup();
static void rw_socket();
static void run();
static bool handle_msg(char *);

int main(int argc, char **argv) {
	// screen number
	int scrnum = 0;

	// try to setup connection
	if (xcb_connection_has_error(conn = xcb_connect(NULL, &scrnum))) {
		std::clog << "could not connect!\n";
		exit(1);
	}

	// get a file descriptor for the connection
	conn_fd = xcb_get_file_descriptor(conn);

	// TODO: setup signal handlers
	signal(SIGABRT, cleanup);
	signal(SIGTERM, cleanup);
	signal(SIGINT, cleanup);


	// setup
	if(setup(scrnum) && socket_setup()) {
		std::clog << "connected to display!\n";
		running = true;
		run();
	}
}

bool setup(int scrnum) {
	// try to get a screen
	screen = xcb_screen_of_display(conn, scrnum);

	if (!screen)
		return false;

	// TODO: ewmh stuff at some point
	
	// valuemask we want of the root window
	unsigned int values[1] = {
		XCB_EVENT_MASK_SUBSTRUCTURE_REDIRECT
		| XCB_EVENT_MASK_SUBSTRUCTURE_NOTIFY
		| XCB_EVENT_MASK_PROPERTY_CHANGE
		| XCB_EVENT_MASK_BUTTON_PRESS
	};
	// setup root window with above valuemask
	xcb_generic_error_t *error = xcb_request_check(conn,
			xcb_change_window_attributes_checked(conn, screen->root,
			XCB_CW_EVENT_MASK, values));

	// flush changes
	xcb_flush(conn);

	if (error){
		std::clog << "could not connect to server\n";
		free(error);
		return false;
  }

  // setup events
  for (int i = 0; i < XCB_NO_OPERATION; i++)
		events[i] = printevent;

	// assign events we actually have
	events[XCB_MAP_REQUEST]					= map_request;
	events[XCB_DESTROY_NOTIFY]      = destroy_notify;
	events[XCB_BUTTON_PRESS]        = button_press;
	// events[XCB_EXPOSE]							= expose;

	// setup message handling
	msg_map["move"] = &msg_move_relative;
	msg_map["move_to"] = &msg_move_absolute;
	msg_map["print_tags"] = &msg_print_tags;
	msg_map["tag"] = &msg_change_tag;
	msg_map["focus"] = &msg_focus;
	msg_map["resize"] = &msg_resize;
	msg_map["kill"] = &msg_kill;
	msg_map["toggle"] = &msg_toggle;
	msg_map["config"] = &msg_config;

	// default config stuff
	config["move_mod"] = XCB_MOD_MASK_4;
	config["resize_mod"] = XCB_MOD_MASK_2;
	config["focus_color"] = 0x36424f;
	config["unfocus_color"] = 0x1a2026;
	config["outer_width"] = 10;
	config["inner_width"] = 3;
	config["snap"] = 20;


	// set all tags to visible
	for (int i = 0; i < NUMTAGS + 1; ++i) {
		visible[i] = true;
	}

  return true;
}

bool socket_setup() {

	// setup the ipc socket
	// setup socket descriptor
	if ((socket_fd = socket(AF_UNIX, SOCK_STREAM, 0)) < 0) {
		perror("socket");
		return false;
	}

	// bind socket to address
	local.sun_family = AF_UNIX;
	strcpy(local.sun_path, SOCKET_PATH);
	// remove socket if it already exists
	unlink(local.sun_path);
	// bind the socket
	slen = strlen(local.sun_path) + sizeof(local.sun_family);
	if (bind(socket_fd, (struct sockaddr *)&local, slen) == -1) {
		perror("bind");
		return false;
	}

	// listen for connections
	if (listen(socket_fd, 5) == -1) {
		perror("listen");
		return false;
  }

	return true;
}

void cleanup(int sig) {
	std::clog <<  "all cleaned up\n";
	// clean stuff up
	// free all vectors
	running = false;
	
	for (int i = 0; i <= NUMTAGS; i++) {
		for (Client *c : tags[i]) {
			free(c);
		}
		tags[i].clear();
	}
}

// grets the screen of the display
xcb_screen_t *
xcb_screen_of_display(xcb_connection_t *con, int s)
{
	xcb_screen_iterator_t iter;
	iter = xcb_setup_roots_iterator(xcb_get_setup(con));
	for (; iter.rem; --s, xcb_screen_next(&iter))
		if (s == 0)
			return iter.data;

	return NULL;
}

/*
 * Event stuff
 */
void printevent(xcb_generic_event_t *ev) {
	std::clog << "event: " << (ev->response_type & ~0x80) << "\n";
}

void map_request(xcb_generic_event_t *ev) {
	std::clog << "event: " << (ev->response_type & ~0x80) << " map request recieved\n";
	xcb_map_request_event_t *e = (xcb_map_request_event_t *) ev;

	// check if we are already managing this window
	// if so we dont want to do anything
	if (NULL != get_client(&e->window)) {
		std::clog << "client " << std::hex << e->window << "already managed\n";
		return;
	}
		
	
	// if not set up a new client
	Client *c = (Client *) malloc(sizeof(Client));
	*c = Client(e->window, conn);
	//c->decorate();
	c->map();
	c->change_tag(0);
	c->print("map:");
	c->focus();
	//c->raise(conn);
	// map the window
	xcb_flush(conn);
	std::clog << "map request handled\n";
}

void destroy_notify(xcb_generic_event_t *ev) {
	std::clog << "event: " << (ev->response_type & ~0x80) << " destroy notify recieved\n";

	xcb_destroy_notify_event_t *e = (xcb_destroy_notify_event_t *) ev;

	Client *c = get_client(&e->window);

	// check if we manage this window
	if (c == NULL) {
		std::clog << "client " << std::hex << e->window << " not managed\n";
		return;
	}

	c->unmanage();

	// free the pointer
	free(c);

	std::clog << "destroy notify handled\n";
	xcb_flush(conn);
}

void button_press(xcb_generic_event_t *ev){
	std::clog << "event: " << (ev->response_type & ~0x80) << " button press recieved\n";

	xcb_button_press_event_t *e = (xcb_button_press_event_t *) ev;

	// find what window the pointer is over
	xcb_query_pointer_reply_t *q;
	q = xcb_query_pointer_reply(conn, xcb_query_pointer(conn, screen->root), NULL);
	if (q == NULL)
		return;
	// if its a clinet focus it
	Client *c = get_client(&q->child);
	if (c == NULL)
		return;
	
	c->focus();


	// check if a mod is down
	if (e->state != XCB_NONE) {
		// in that case we want to capture the cursor movement

		xcb_grab_pointer_reply_t *reply = xcb_grab_pointer_reply(conn,
				xcb_grab_pointer(conn, 0, screen->root,
					XCB_EVENT_MASK_BUTTON_RELEASE | XCB_EVENT_MASK_BUTTON_MOTION,
					XCB_GRAB_MODE_ASYNC,
					XCB_GRAB_MODE_ASYNC, XCB_NONE, XCB_NONE, XCB_CURRENT_TIME), NULL);

		// check if this worked
		if (reply == NULL || reply->status != XCB_GRAB_STATUS_SUCCESS) {
			std::clog << "couldn't grab pointer\n";
			free(reply);
			return;
		}
		free(reply);

		// if the button was middle then we can just kill the client
		if (e->detail == XCB_BUTTON_INDEX_2) {
			c->kill();
			xcb_ungrab_pointer(conn, XCB_CURRENT_TIME);
			free(q);
			return;
		}

		xcb_generic_event_t *tmpev;
		bool grabbing = true;
		// last x and y
		int lx = q->root_x;
		int ly = q->root_y;
		// total x and y
		int tx = 0;
		int ty = 0;
		// amount to move by
		int mx = 0;
		int my = 0;
		xcb_motion_notify_event_t *mv;

		do {


			while(!(tmpev = xcb_wait_for_event(conn)))
						xcb_flush(conn);

			switch (tmpev->response_type & ~0x80) {

				case XCB_MOTION_NOTIFY:
					mv = (xcb_motion_notify_event_t*) tmpev;

					// add to the total x movement
					tx += mv->root_x - lx;
					ty += mv->root_y - ly;

					// if total x is greater than the snap amount then snap
					if (std::abs(tx) >= config["snap"]) {
						// and change back to zero
						mx = tx;
						tx = 0;
					}
					if (std::abs(ty) >= config["snap"]) {
						// and change back to zero
						my = ty;
						ty = 0;
					}
					/*
					if (e->state == config["move_mod"]) {
						c->move_relative(mv->root_x - lx, mv->root_y - ly);
					} else if(e->state == config["resize_mod"]) {
						c->resize_mouse(mv->root_x, lx, mv->root_y, ly);
					}

					*/
					// set the last location
					lx = mv->root_x;
					ly = mv->root_y;

					if (e->state == config["move_mod"]) {
						c->move_relative(mx, my);
					} else {
						c->resize_mouse(mv->root_x, mx, mv->root_y, my);
					}

					mx = 0;
					my = 0;

					xcb_flush(conn);
					break;
				case XCB_BUTTON_PRESS:
				case XCB_BUTTON_RELEASE:
					grabbing = false;
					break;
			}

			free(tmpev);
		} while (grabbing);
		// ungrab the pointer
		xcb_ungrab_pointer(conn, XCB_CURRENT_TIME);

	}

	free(q);


	// allow the pointer to be used
	xcb_allow_events(conn, XCB_ALLOW_SYNC_POINTER, e->time);
	xcb_flush(conn);
	std::clog << "button press handled\n";
}

/*
void expose(xcb_generic_event_t *ev) {
	std::clog << "event: " << (ev->response_type & ~0x80) << " expose recieved\n";

	 xcb_expose_event_t *e = (xcb_expose_event_t *) ev;

	Client *c = get_client(&e->window);

	if (c == NULL)
		return;

	if (c == focus_queue.front()){
		c->decorate(config["border_colorf"]);
	} else {
		c->decorate(config["border_coloru"]);
	}

	xcb_flush(conn);
}
*/

/*
 * msgs
 */
bool handle_msg(char *buff) {

	// what are we telling our homie?
	std::string response;

	// convert the buffer to a stream
	std::istringstream ss(buff);

	// vectorize our stream
	std::vector<std::string> sv(
		(std::istream_iterator<std::string>(ss)), 
		std::istream_iterator<std::string>());

	// check if we can handle this kind of message
	if (msg_map.count(sv[0]) == 0) {
		std::clog << "msg " << sv[0] << " does not exist\n";
		response = "fail";
	} else {
		response = msg_map[sv[0]](sv);
	}
	
	// attempts to send our response
	if (send(socket2, response.c_str(), response.length(), 0) < 0) {
		perror("send");
	}

	return true;
}

void rw_socket(void) {
	// store the length of the message recieved
	int n;
	// buffer for our messages
	char buff[BUFFLEN];
	unsigned int t = sizeof(remote);
	// attempt to accept a new connection
	if ((socket2 = accept(socket_fd, (struct sockaddr *) &remote, &t)) < 0) {
		perror("accept");
		exit(1);
	}
	// std::clog << "connected to socket\n";

	// recieve message
	if ((n = recv(socket2, buff, BUFFLEN, 0)) >= 0 ) {
		// end the string at its end (not the buffer's)
		buff[n] = '\0';

		handle_msg(buff);
	} else {
		perror("recv");
	}

	// close the socket
	close(socket2);
}


// main loop
void run(void) {
	xcb_generic_event_t *ev  = NULL;

	while(running) {
		// reset the fd_set
		FD_ZERO(&descriptors);
		// look at our socket
		FD_SET(socket_fd, &descriptors);
		// look at the display
		FD_SET(conn_fd, &descriptors);
		max_fd = (socket_fd > conn_fd) ? socket_fd : conn_fd;

		// check if anyone is connected to the socket
		if (select(max_fd + 1, &descriptors, NULL, NULL, NULL) > 0) {
			if (FD_ISSET(socket_fd, &descriptors)) {
				rw_socket();
			}

			if (FD_ISSET(conn_fd, &descriptors)) {

				while ((ev = xcb_poll_for_event(conn)) != NULL) {
					events[ev->response_type & ~0x80](ev);
					// free the event we just allocated
					free(ev);
				}
			}
		}

	}
}

