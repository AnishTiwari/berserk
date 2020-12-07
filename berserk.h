#ifndef BERSERK_H
#define BERSERK_H
#include <sys/epoll.h>


#define PORT "80"
#define MAX_EVENTS 5

int lfd;
int efd;
struct epoll_event events[MAX_EVENTS];

void set_socket_non_blocking(int sfd);
int create_socket_listener(char* port);

#endif
