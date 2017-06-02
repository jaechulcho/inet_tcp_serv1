//============================================================================
// Name        : inet_tcp_serv1.cpp
// Author      : josco92
// Version     :
// Copyright   : Your copyright notice
// Description : Hello World in C++, Ansi-style
//============================================================================

#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <chrono>
#include <unistd.h>
#include <errno.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>

#include "stdalsp.h"
using namespace std;

enum {
	LISTEN_BACKLOG = 32,
	MAX_POOL = 3,
};

void setareadata(char *buf, int num);
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
	if ((fd_listener = socket(AF_INET, SOCK_STREAM, IPPROTO_IP)) == -1) {
		pr_err("[TCP server] : Fail: socket()");
		exit(EXIT_FAILURE);
	}
	struct sockaddr_in saddr_s;
	memset(&saddr_s, 0, sizeof(saddr_s));
	saddr_s.sin_family = AF_INET;
	saddr_s.sin_addr.s_addr = INADDR_ANY;
	saddr_s.sin_port = htons(port);
	if (bind(fd_listener, (struct sockaddr*)&saddr_s, sizeof(saddr_s)) == -1) {
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
		int pid = fork();
		switch (pid) {
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
			char buf[2048];
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
			if (strncmp(buf, "getVHPROFILE", 12) == 0) {
				sprintf(buf, "%d", 1);
			}
			else if (strncmp(buf, "getOBPROFILE", 12) == 0) {
				sprintf(buf, "%d", 7);
			}
			else if (strncmp(buf, "setOBPROFILE", 12) == 0) {
				sprintf(buf, "%s", "OK");
			}
			else if (strncmp(buf, "setVHPROFILE", 12) == 0) {
				sprintf(buf, "%s", "OK");
			}
			else if (strcmp(buf, "OBINFO") == 0) {
				setareadata(buf, 7);
			}
			else if (strcmp(buf, "VHINFO") == 0) {
				setareadata(buf, 1);
			}
			else {
				sprintf(buf, "%s", "NG");
			}
			ret_len = strlen(buf);
			pr_out("[Child:%d] SEND(%.*s)", idx, ret_len, buf)
			if (send(cfd, buf, ret_len, 0) == -1) {
				pr_err("[Child:%d] Fail: send() to socket(%d)", idx, cfd);
				close(cfd);
			}
//			sleep(1); /* meaning-less conde */
		} /* packet recv loop */
	} /* main for loop */
}

void setareadata(char *buf, int num)
{
	if (1 == num) {
		sprintf(buf, "%8f", 2.0		); buf += 8;	// for x
		sprintf(buf, "%8f", 1.0		); buf += 8;	// fox y
		sprintf(buf, "%8f", 0.1		); buf += 8;	// for x
		sprintf(buf, "%8f", 0.0		); buf += 8;	// fox y
		sprintf(buf, "%8f", 0.32	); buf += 8;	// for x
		sprintf(buf, "%8f", 0.35	); buf += 8; 	// fox y
		sprintf(buf, "%8f", 0.32	); buf += 8;	// for x
		sprintf(buf, "%8f", 5.0		); buf += 8; 	// fox y
		sprintf(buf, "%8f", 0.0		); buf += 8;	// for x
		sprintf(buf, "%8f", 5.0		); buf += 8; 	// fox y
		sprintf(buf, "%8f", -0.32	); buf += 8;	// for x
		sprintf(buf, "%8f", 5.0		); buf += 8; 	// fox y
		sprintf(buf, "%8f", -0.32	); buf += 8;	// for x
		sprintf(buf, "%8f", 0.35	); buf += 8; 	// fox y
		sprintf(buf, "%8f", -0.1	); buf += 8;	// for x
		sprintf(buf, "%8f", 0.0		); buf += 8; 	// fox y
	}
	else {
		sprintf(buf, "%8f", 2.0		); buf += 8;	// for x
		sprintf(buf, "%8f", 1.0		); buf += 8;	// fox y
		sprintf(buf, "%8f", 0.125	); buf += 8;	// for x
		sprintf(buf, "%8f", 0.15	); buf += 8;	// fox y
		sprintf(buf, "%8f", 0.125	); buf += 8;	// for x
		sprintf(buf, "%8f", 5.0		); buf += 8; 	// fox y
		sprintf(buf, "%8f", 0.125	); buf += 8;	// for x
		sprintf(buf, "%8f", 5.0		); buf += 8; 	// fox y
		sprintf(buf, "%8f", 0.0		); buf += 8;	// for x
		sprintf(buf, "%8f", 5.0		); buf += 8; 	// fox y
		sprintf(buf, "%8f", -0.125	); buf += 8;	// for x
		sprintf(buf, "%8f", 5.0		); buf += 8; 	// fox y
		sprintf(buf, "%8f", -0.125	); buf += 8;	// for x
		sprintf(buf, "%8f", 5.0		); buf += 8; 	// fox y
		sprintf(buf, "%8f", -0.125	); buf += 8;	// for x
		sprintf(buf, "%8f", 0.15	); buf += 8; 	// fox y
	}
}
