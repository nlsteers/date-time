#include <stdio.h>
#include <sys/socket.h> /* socket(), bind(), listen(), accept() */
#include <string.h>     /* bzero() - should now use memset() */
#include <netinet/in.h> /* struct sockaddr_in, htons(), htonl() */
#include <unistd.h>     /* write(), close() */
#include <time.h>
#include <netdb.h> /* gethostbyaddr() */

#include <arpa/inet.h>

#define MAXLINE 4096
#define MAXQ 10

int main( int argc, char * argv[]) {
  int listenfd;
  int connfd;
  struct sockaddr_in servaddr;
  struct sockaddr_in cliaddr;
  struct hostent * hostptr; /* ptr to /etc/hosts entry for the client */
  int lenstruct;
  char buff[ MAXLINE];
  time_t ticks;

  /* Create socket for listening on and set up internet address strucure */
  listenfd = socket( AF_INET, SOCK_STREAM, 0);

  if( listenfd < 0) {
    perror( "socket error");
    return 1;
  }

  bzero( &servaddr, sizeof( servaddr));
  servaddr.sin_family = AF_INET;   /* should now use PF_INET! */
  servaddr.sin_port = htons( 60000);  /* daytime 'well known' port in use! */
  /* allow connection on any of my IP addresses - if I have more than one! */
  servaddr.sin_addr.s_addr = htonl( INADDR_ANY);
  lenstruct = sizeof( servaddr);

  /* bind the inet address structure to the listening socket ready for use */
  if( (bind( listenfd, (struct sockaddr *) &servaddr, lenstruct)) < 0) {
    perror( "bind error");
    return 1;
  }

  /* actually put the socket in listen mode and set the q length */
  if( listen( listenfd, MAXQ) < 0) {
    perror( "listen error");
    return 1;
  }
  for( ; ; ) {
    /* loop forever */
    /* wait (block) for a client request on the listen socket
     * when a request appears connect it to the new socket connfd
     * ready for the actual communication with the client.
     * A successful connection will also get client IP details.
     */
    connfd = accept( listenfd, (struct sockaddr *) &cliaddr, &lenstruct);
    if( connfd < 0) {
      perror( "accept error");
      return 1;
    }

    /* get the date and time, and write it to the connected socket */
    ticks = time( NULL);

    fprintf( stderr, "Addr of connected client: %s\n", inet_ntoa( cliaddr.sin_addr));

    hostptr = gethostbyaddr( (char *)&cliaddr.sin_addr, sizeof( cliaddr.sin_addr), cliaddr.sin_family);

    if( !hostptr) { /* if no entry available display an error message */
      perror( "gethostbyaddr error");
      return 1;
    }
    fprintf( stderr, "The client name is %s\n", hostptr -> h_name);
    snprintf( buff, sizeof( buff), "%.24s\r\n", ctime( &ticks));

    if( write( connfd, buff, strlen( buff)) < 0) {
      fprintf( stderr, "write error\n");
      return 1;
    }
    /* close the connected socket */
    close( connfd);
  }  /* for ever */

  /* should never get here, will need to kill the process
  * So the listen socket will remain open for a while -
  * until garbage collection clears up!
  */
  close( listenfd);
/* close the listening socket */
}  /* main */
