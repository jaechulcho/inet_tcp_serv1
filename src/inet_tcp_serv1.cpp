//============================================================================
// Name        : inet_tcp_serv1.cpp
// Author      : josco92
// Version     :
// Copyright   : Your copyright notice
// Description : Hello World in C++, Ansi-style
//============================================================================

#include <iostream>
#include <cstdlib>
#include <cstring>
#include <CoreFoundation/CoreFoundation.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>

#include "stdalsp.h"
using namespace std;

enum {
	LISTEN_BACKLOG = 20,
	MAX_POOL = 3,
};

void start_child(int fd, int idx);
int main(int argc, char* argv[])
{
	if (argc > 2) {
		printf("%s [port number]\n", argv[0]);
		exit(EXIT_FAILURE);
	}
	short port;
	if (2 == argc) {
		port = (short)atoi((char *)argv[1]);
	}
	else {
		port = 0;	/* random port */
	}
	int fd_listener;
	if (-1 == (fd_listener = socket(AF_INET, SOCK_STREAM, 0))) {
		pr_err("[TCP server] : Fail: socket()");
		exit(EXIT_FAILURE);
	}
	struct sockaddr_in saddr_s;
	memset(&saddr_s, 0, sizeof(saddr_s));
	saddr_s.sin_family = AF_INET;
	saddr_s.sin_addr.s_addr = INADDR_ANY;
	saddr_s.sin_port = htons(port);
	if (-1 == bind(fd_listener, (struct sockaddr*)&saddr_s, sizeof(saddr_s))) {
		pr_err("[TCP server] Fail: bind()");
		exit(EXIT_FAILURE);
	}
	if (0 == port) {
		socklen_t len_saddr = sizeof(saddr_s);
		getsockname(fd_listener, (struct sockaddr*)&saddr_s, &len_saddr);
	}
	pr_out("[TCP server] Port : #%d", ntohs(saddr_s.sin_port));
	listen(fd_listener, LISTEN_BACKLOG);
	for (int i=0; i<MAX_POOL; i++) {
		switch (int pid = fork()) {
		case 0:
			start_child(fd_listener, i);
			exit(EXIT_SUCCESS);
			break;
		case -1:
			pr_err("[TCP server] : Fail: fork()");
			break;
		default:
			pr_out("[TCP server] Making child process No.%d", i);
			break;
		}
	}
	for(;;) pause();
	return 0;
}

void start_child(int sfd, int idx)
{
	struct sockaddr_storage saddr_c;
	for (;;) {
		socklen_t len_saddr = sizeof(saddr_c);
		int cfd = accept(sfd, (struct sockaddr*)&saddr_c, &len_saddr);
		if (-1 == cfd) {
			pr_err("[Child] Fail: accept()");
			close(cfd);
			continue;
		}
		if (AF_INET == saddr_c.ss_family) {
			pr_out("[Child:%d] accept (ip:port) (%s:%d)", idx,
					inet_ntoa(((struct sockaddr_in*)&saddr_c)->sin_addr),
					ntohs(((struct sockaddr_in*)&saddr_c)->sin_port));
		}
		for (;;) {
			char buf[40];
			int ret_len = recv(cfd, buf, sizeof(buf), 0);
			if (-1 == ret_len) {
				if (EINTR == errno) continue;
				pr_err("[Child:%d] Session closed", idx);
				break;
			}
			if (0 == ret_len) {
				pr_err("[Child:%d] Session closed", idx);
				close(cfd);
				break;
			}
			pr_out("[Child:%d] RECV(%.*s)", idx, ret_len, buf);
			if (send(cfd, buf, ret_len, 0) == -1) {
				pr_err("[Child:%d] Fail: send() to socket(%d)", idx, cfd);
				close(cfd);
			}
			sleep(1); /* meaning-less conde */
		} /* packet recv loop */
	} /* main for loop */
}
