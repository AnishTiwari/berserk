#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/epoll.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <netdb.h>

#include "./server.h"
#include "./berserk.h"

void handle_client(int sfd){

  int bytes_read;
  char buf[4096];
  bytes_read = read(sfd, &buf, 4096); 
  if(bytes_read < 0){
    perror("No bytes found");
  }
  fprintf(stdin, "data recv: %s", buf);

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

  set_socket_non_blocking(conn_fd) ;
  
  event.data.fd = conn_fd;
  event.events = EPOLLIN | EPOLLET;

  if (epoll_ctl(efd, EPOLL_CTL_ADD, conn_fd, &event) < 0) {
    close(conn_fd);
    perror("epoll_ctl");
  }


  
}

void serve(){

  int evt_list;

  /* allocate memory to events */
  memset(&events, 0,MAX_EVENTS * sizeof(struct epoll_event));
  
  evt_list = epoll_wait(efd, events, MAX_EVENTS, -1);

  if(evt_list < 0){
    perror("epoll wait");
    exit(EXIT_FAILURE);
  }

  for(int i=0; i < evt_list; i++){

    /* check the flags Status  */

    /* if any error/hup,rdhup happens close the conn */
    if( events[i].events & (EPOLLERR | EPOLLHUP | EPOLLRDHUP))
      {
	printf("event [%d]: errno %d\n", i, events[i].events);
	close(events[i].data.fd);
	perror("closing socket");
   
      }

    else if(events[i].events | EPOLLIN) {

      /* accept connection */
      if( events[i].data.fd == lfd)
	{
	  accept_connection();
	}
      /* handle data recv from client */
      else{
	handle_client(events[i].data.fd);
      }

    }
    else if(events[i].events | EPOLLOUT){
      printf("EPOLLOUT");
      
    }
    
    
  }

}
