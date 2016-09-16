#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

#include "get.h"
#include "die.h"

#define CLEAR_DATA(x) memset(&(x), 0, sizeof(x))

static size_t strip_headers(const char* str) {
	size_t i;
	int newlines = 0;
	char c;

	for (i = 0; newlines < 2 && (c = str[i]) != 0; i++) {
		if (c == '\n') {
			newlines++;
		}
		else if (c != '\r') {
			newlines = 0;
		}
	}

	return i;
}

static const char* GET_REQUEST = "GET / HTTP/1.0\r\n\r\n";

const char* perform_get(const char* host, uint16_t port) {
	struct hostent* server;
	struct sockaddr_in server_addr;

	int sockfd;

	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd < 0) {
		die("error: couldn't open socket");
	}

	server = gethostbyname(host);
	if (server == NULL) {
		die("error: host not found");
	}

	CLEAR_DATA(server_addr);
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(port);
	memcpy(&server_addr.sin_addr.s_addr, server->h_addr, server->h_length);

	int connection = connect(sockfd, (struct sockaddr*)(&server_addr), sizeof(server_addr));
	if (connection < 0) {
		die("error: couldn't connect");
	}

	int send_total = strlen(GET_REQUEST);
	int sent = 0;
	int just_sent = 0;

	do {
		just_sent = write(sockfd, GET_REQUEST + sent, send_total - sent);
		if (just_sent < 0) {
			die("error: couldn't write to socket");
		}
		else {
			fprintf(stderr, "sent %u b\n", just_sent);
		}
		sent += just_sent;
	} while(sent < send_total);

	fputs("sent all\n", stderr);

	const size_t MAX_RESPONSE_SIZE = 65536;
	char* response = (char*)(malloc(MAX_RESPONSE_SIZE));
	memset(response, 0, MAX_RESPONSE_SIZE);

	int recv_max = MAX_RESPONSE_SIZE - 1;
	int recv = 0;
	int just_recv = 0;

	do {
		just_recv = read(sockfd, response + recv, recv_max - recv);
		if (just_recv < 0) {
			die("error: couldn't read from socket");
		}
		else if (just_recv == 0) {
			break;
		}
		else {
			fprintf(stderr, "recv %u b\n", just_recv);
		}
		recv += just_recv;
	} while (recv < recv_max);

	fputs("recv all\n", stderr);

	close(sockfd);

	return response + strip_headers(response);
}

