#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/epoll.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/sendfile.h>
#include <sys/stat.h>
#include <netdb.h>

#include "./server.h"
#include "./berserk.h"

char buf[512];

void send_response( int sfd){
  struct stat stats_buf;
  int cfd = open("./index.html",O_RDONLY, 0777);
  if(cfd < 0) {
    perror("open:");
  }

  int byte_to_send = fstat(cfd, &stats_buf);
  if(byte_to_send < 0){
    perror("fstats");
  }

  char goodresponse[1024] = {"HTTP/1.1 200 OK\r\n\r\n"};
  send(sfd, goodresponse, strlen(goodresponse), 0);
  int ret = sendfile(sfd, cfd, NULL, stats_buf.st_size);
  if(ret < 0){
    perror("send file/response\r\n");
  }
  /* close the client connection after sending response */
  close(sfd);
}

void handle_client(int cfd){

  int count = read(cfd, buf, 512 );
  if(count < 0){
    perror("Nothing to read");
  }
  struct epoll_event event;
  event.data.fd = cfd;
  event.events = EPOLLOUT | EPOLLET;
  if (epoll_ctl(efd, EPOLL_CTL_MOD, cfd, &event) < 0) {
    perror("epoll_ctl");
  }
}

void accept_connection(){

  struct sockaddr addr;
  struct epoll_event event;
  socklen_t addr_len;
  int conn_fd;
  conn_fd = accept(lfd, &addr, &addr_len );

  if(conn_fd < 0){
    perror("accept connection");
  }
  /* set the socket to non blocking */
  set_socket_non_blocking(conn_fd) ;
  
  event.data.fd = conn_fd;
  event.events = EPOLLIN | EPOLLET;

  if (epoll_ctl(efd, EPOLL_CTL_ADD, conn_fd, &event) < 0) {
    close(conn_fd);
    perror("epoll_ctl");
  }
}

void serve() {
  printf("Started to serve\n\n");
  int evt_list;
  
  /* allocate memory to events */
  memset(&events, 0, MAX_EVENTS * sizeof(struct epoll_event));  
  evt_list = epoll_wait(efd, events, MAX_EVENTS, -1);
  
  if(evt_list < 0) {
    fprintf(stdin, "EPOLL WAIT: efd %d %s\n", evt_list, strerror(errno));
    abort();
  }
  for(int i=0; i < evt_list; i++) {

    /* if any error/hup,rdhup happens close the conn */
    if( events[i].events & (EPOLLERR | EPOLLHUP | EPOLLRDHUP))
      {
	printf("event [%d]: errno %d\n", i, events[i].events);
	close(events[i].data.fd);
	perror("closing socket");
      }
    else {
      if( events[i].data.fd == lfd) {
	  /* accept connection */
	  accept_connection();
	}
    
      else if(events[i].events & EPOLLIN) {
	/* READ: */
	/* handle data recvd from client */
	handle_client(events[i].data.fd);
      }

      else {
	/* WRITE - send response to client */
	send_response(events[i].data.fd);       
      }
    }
  }

}
