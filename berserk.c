#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/epoll.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "./server.h"
#include "./berserk.h"


 /* 1) MAKE A SOCKET IN THE DEFINED PORT (getaddressinfo , create socket fd, setsockopt(reuse add, port), bind)
 *  2) set guid and uid to prevent root access
 *  3)return the socket file descriptor
 */

void set_socket_non_blocking(int sfd){

  int flags;

  /* get the current flag set for sfd */
  flags = fcntl(sfd, F_GETFL);

  if(flags < 0){
    perror("fcntl");
  }

  flags |= O_NONBLOCK;
  
  if (fcntl(sfd, F_SETFL, flags) < 0) {
    perror("fcntl");		
  }
  
}

/* step.1: returns the socket file descriptor */
int create_socket_listener(void)
{
  struct addrinfo hints, *result, *r_ptr;
  int s, sfd ;
  memset(&hints, 0, sizeof(struct addrinfo));
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_protocol = 0;
  /* https://man7.org/linux/man-pages/man3/getaddrinfo.3.html */

  s = getaddrinfo(NULL, PORT, &hints, &result );
  if(s != 0){
    fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(s));
    exit(EXIT_FAILURE);

  }

  /* address structure linked list returned by getaddrinfo call */
  for(r_ptr = result; r_ptr != NULL; r_ptr = r_ptr->ai_next){

    /* create a socket descriptor by calling socket fn: */
    sfd = socket(r_ptr->ai_family, r_ptr->ai_socktype, r_ptr->ai_protocol );

    if(sfd < 0)
      continue;

    s = setsockopt(sfd, SOL_SOCKET,SO_REUSEADDR, &s, sizeof(int) );

    if(s < 0){
      perror("setsockopt");
      exit(EXIT_FAILURE);
    }

    if (bind(sfd, r_ptr->ai_addr, r_ptr->ai_addrlen) == 0)
      break;

    /* Success */
    close(sfd);
  }

  if (r_ptr == NULL) {
    
    /* No address succeeded */
    perror("Could not bind\n");
    freeaddrinfo(result);

    exit(EXIT_FAILURE);

  }

  freeaddrinfo(result);

  if(getuid() == 0){

    if(setgid(100) != 0){

      fprintf(stderr, "Could not set group ID \n");
      exit(EXIT_FAILURE);
    }
    if(setuid(1000) != 0){
      fprintf(stderr, "Could not set user ID \n");
      exit(EXIT_FAILURE);
   
      
    }
  }

  return sfd;  
}
  
 
int main() {

  struct epoll_event event;
  lfd = create_socket_listener();

  /* make the socket non blocking */
  set_socket_non_blocking(lfd);

  /* listener socket 
   * SOMAXCONN = 128 backlogs */
  if(listen(lfd, SOMAXCONN) < 0){
    perror("listen");
    exit(EXIT_FAILURE);
  }
  
  /* create an epoll instance */
  if( (efd = epoll_create1(0)) < 0){
    close(lfd);
    perror("epoll_create");
  }
  event.data.fd = lfd;
  event.events = EPOLLIN | EPOLLET;
  
  /* add the efd to the epoll list */
  if (epoll_ctl(efd, EPOLL_CTL_ADD, lfd, &event ) < 0){
    close(lfd);
    perror("epoll_ctl add");
    exit(EXIT_FAILURE);
  }


  for( ; ; ){
    serve();
  }

  return 0;
}
