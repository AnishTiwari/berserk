#ifndef BERSERK_H
#define BERSERK_H

#define PORT "80"
#define MAX_EVENTS 5
#define READ_SIZE 10

static int lfd;
static int efd;
struct epoll_event events[MAX_EVENTS];

void set_socket_non_blocking(int);

#endif
