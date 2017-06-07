/*
 * io_select.cpp
 *
 *  Created on: Jun 7, 2017
 *      Author: jccho
 */

#include <cstdio>
#include <cstring>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <netdb.h>
#include <sys/select.h>
#include <algorithm>
#include "stdalsp.h"

const int LISTEN_BACKLOG = 256;
const int MAX_FD_SOCKET = 0xff;

int fd_socket[MAX_FD_SOCKET]; /* array for socket file descriptors */
int cnt_fd_socket = 0;

/* IO multiplexing var */
fd_set fds_read;
int fd_biggest;
int add_socket(int fd);
int del_socket(int fd);
int mk_fds(fd_set *fds, int *a_fd_socket);

int main(int argc, char *argv[]) {
    int ret_recv, i;
    char *port, buf[1024];

    for (i = 0; i < MAX_FD_SOCKET; i++) {
        fd_socket[i] = -1;
    }

    if (argc != 2) {
        printf("%s [port number]\n", argv[0]);
    }
    if (argc == 2) {
        port = strdup(argv[1]);
    } else {
        port = strdup("0"); /* for random port */
    }

    struct addrinfo ai, *ai_ret;
    int rc_gai;

    memset(&ai, 0, sizeof(ai));
    ai.ai_family = AF_INET;
    ai.ai_socktype = SOCK_STREAM;
    ai.ai_flags = AI_ADDRCONFIG | AI_PASSIVE;
    if ((rc_gai = getaddrinfo(NULL, port, &ai, &ai_ret)) != 0) {
        pr_err("Fail: getaddrinfo():%s", gai_strerror(rc_gai));
        exit(EXIT_FAILURE);
    }

    int fd_listener;
    if ((fd_listener = socket(ai_ret->ai_family, ai_ret->ai_socktype,
            ai_ret->ai_protocol)) == -1) {
        pr_err("Fail: socket()");
        exit(EXIT_FAILURE);
    } /* prefer to adopt the SO_REUSEADDR option before calling bind func */

    int sockopt = 1;
    if (setsockopt(fd_listener, SOL_SOCKET, SO_REUSEADDR, &sockopt, sizeof(sockopt)) == -1) {
        exit(EXIT_FAILURE);
    }
    if (setsockopt(fd_listener, IPPROTO_TCP, TCP_NODELAY, &sockopt, sizeof(sockopt)) == -1) {
        exit(EXIT_FAILURE);
    }

    if (bind(fd_listener, ai_ret->ai_addr, ai_ret->ai_addrlen) == -1) {
        pr_err("Fail: bind()");
        exit(EXIT_FAILURE);
    }
    if (!strncmp(port, "0", strlen(port))) { /* in case random port num */
        struct sockaddr_storage saddr_s;
        socklen_t len_saddr = sizeof(saddr_s);
        getsockname(fd_listener, (struct sockaddr *) &saddr_s, &len_saddr);
        if (saddr_s.ss_family == AF_INET) { /* IPv4 */
            pr_out("bind:IPv4 Port: #%d",
                    ntohs(((struct sockaddr_in * )&saddr_s)->sin_port));
        } else if (saddr_s.ss_family == AF_INET6) {
            pr_out("bind:IPv6 Port: #%d",
                    ntohs(((struct sockaddr_in6 * )&saddr_s)->sin6_port));
        } else {
            pr_out("getsockname: ss_family=%d", saddr_s.ss_family);
        }
    } else {
        pr_out("bind: %s", port); /* in case fixed port num */
    }
    listen(fd_listener, LISTEN_BACKLOG);
    add_socket(fd_listener);

    while (true) {
        fd_biggest = mk_fds(&fds_read, fd_socket);
        int ret_select;
        if ((ret_select = select(fd_biggest + 1, &fds_read, NULL, NULL, NULL))
                == -1) {
            /* error */
        }
        if (FD_ISSET(fd_listener, &fds_read)) {
            struct sockaddr saddr_c;
            socklen_t len_saddr = sizeof(saddr_c);
            int fd = accept(fd_listener, (struct sockaddr *) &saddr_c,
                    &len_saddr);
            if (fd == -1) {
                pr_err("Error get connection from listen socket");
                continue;
            }
            if (add_socket(fd) == -1) {
                /* error */
            }
            pr_out("accept : add socket (%d)", fd);
            continue;
        }
        for (int i = 1; i < cnt_fd_socket; i++) {
            if (FD_ISSET(fd_socket[i], &fds_read)) {
                pr_out("FD_ISSET: normal-isband");
                if ((ret_recv = recv(fd_socket[i], buf, sizeof(buf), 0))
                        == -1) {
                    pr_err("fd(%d) recv() error (%s)", fd_socket[i],
                            strerror(errno));
                    /* error handle */
                } else {
                    if (ret_recv == 0) {
                        pr_out("fd(%d) : Session closed", fd_socket[i]);
                        del_socket(fd_socket[i]);
                    } else {
                        pr_out("recv(fd=%d,n=%d) = %.*s", fd_socket[i],
                                ret_recv, ret_recv, buf);
                    }
                }
            }
        }
    }
    return 0;
}

int add_socket(int fd) {
    if (cnt_fd_socket < MAX_FD_SOCKET) {
        fd_socket[cnt_fd_socket] = fd;
        return ++cnt_fd_socket;
    }
    else {
        return -1;
    }
}

int del_socket(int fd) {
    int i;
    int flag = 0;
    close(fd);
    for (i = 0; i < cnt_fd_socket; i++) {
        if (fd_socket[i] == fd) {
            if (i != (cnt_fd_socket-1)) {
                fd_socket[i] = fd_socket[cnt_fd_socket-1];
            }
            fd_socket[cnt_fd_socket-1] = -1;
            flag = 1;
            break;
        }
    }
    if (flag == 0) {
        return -1;
    }
    --cnt_fd_socket;
    return i;
}

#define MAX(a, b) ((a) > (b) ? (a):(b))
int mk_fds(fd_set *fds, int *a_fd_socket) {
    FD_ZERO(fds);
    int i;
    int fd_max;
    for (i = 0, fd_max = -1; i < cnt_fd_socket; i++) {
        fd_max = MAX(fd_max, a_fd_socket[i]);
        FD_SET(a_fd_socket[i], fds);
    }
    return fd_max;
}

