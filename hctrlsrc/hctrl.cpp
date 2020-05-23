#include <sys/socket.h>
#include <sys/types.h>
#include <sys/un.h>
#include <unistd.h>
#include <string>
#include <iostream>
#include "../src/helium.h"

std::string send_msg(std::string);

int main(int argc, char *argv[]) {


	// create one string from arguments
	std::string send_str = "";
	for (int i = 1; i < argc; i++) {
		send_str += argv[i];
		send_str += " ";
	}

	std::string response = send_msg(send_str);

	if (!response.compare("success") == 0)
		std::cout << response << std::endl;

	return 0;
}

std::string send_msg(std::string send_str) {
	int s, t, len;
  char str[BUFFLEN];
	struct sockaddr_un remote;

	if ((s = socket(AF_UNIX, SOCK_STREAM, 0)) == -1) {
		perror("socket");
		exit(1);
	}

	remote.sun_family = AF_UNIX;
	strcpy(remote.sun_path, SOCKET_PATH);
	len = strlen(remote.sun_path) + sizeof(remote.sun_family);
	if (connect(s, (struct sockaddr *)&remote, len) == -1) {
			perror("connect");
			exit(1);
	}


	// send msg
	if (send(s, send_str.c_str(), send_str.length(), 0) == -1) {
		perror("send");
		exit(1);
	}

	// recieve response
	if ((t=recv(s, str, BUFFLEN, 0)) > 0) {
		str[t] = '\0';
		return std::string(str);
	} else {
		if (t < 0) perror("recv");
		exit(1);
	}

	close(s);
}
