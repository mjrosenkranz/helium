#include <cstdio>
#include <cstdlib>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/un.h>
#include <unistd.h>
#include "helium.h"


int main(int argc, char *argv[]) {
	int s, t, len;
  char str[100];
	struct sockaddr_un remote;

	if ((s = socket(AF_UNIX, SOCK_STREAM, 0)) == -1) {
		perror("socket");
		exit(1);
	}

	printf("Trying to connect...\n");

	remote.sun_family = AF_UNIX;
	strcpy(remote.sun_path, SOCKET_PATH);
	len = strlen(remote.sun_path) + sizeof(remote.sun_family);
	if (connect(s, (struct sockaddr *)&remote, len) == -1) {
			perror("connect");
			exit(1);
	}

	printf("Connected.\n");

	if (send(s, argv[1], strlen(argv[1]), 0) == -1) {
		perror("send");
		exit(1);
	}

	if ((t=recv(s, str, 100, 0)) > 0) {
		str[t] = '\0';
		printf("%s\n", str);
	} else {
		if (t < 0) perror("recv");
		else printf("Server closed connection\n");
		exit(1);
	}

	close(s);

	return 0;
}
