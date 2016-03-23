#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

#define LEN	64

void sendMessageSocket(char* hostname, char* str, int port)
{
  struct hostent *hp;
  int s, rc, len;
  struct sockaddr_in sin;

  hp = gethostbyname(hostname);
   if ( hp == NULL ) {
       fprintf(stderr, "%s: host not found\n", hostname);
       exit(1);
     }

     s = socket(AF_INET, SOCK_STREAM, 0);
     if ( s < 0 ) {
    perror("socket:");
    exit(s);
  }

  sin.sin_family = AF_INET;
  sin.sin_port = htons(port);
  memcpy(&sin.sin_addr, hp->h_addr_list[0], hp->h_length);

  rc = connect(s, (struct sockaddr *)&sin, sizeof(sin));
  len = send(s, str, strlen(str), 0);
  if ( len != strlen(str) ) {
      perror("send");
      exit(1);
    }

  close(s);

}


void receiveMessageSocket(char* str, int port)
{
  printf("TP1");
  fflush(stdout);
  char host[64];
  struct hostent *hp, *ihp;
  int s, rc, len, p;
  struct sockaddr_in sin, incoming;

  gethostname(host, sizeof host);
  hp = gethostbyname(host);
  if ( hp == NULL ) {
    fprintf(stderr, "%s: host not found\n", host);
    exit(1);
  }
  
  s = socket(AF_INET, SOCK_STREAM, 0);
  if ( s < 0 ) {
    perror("socket:");
    exit(s);
  }
  printf("TP1");
  fflush(stdout);
  sin.sin_family = AF_INET;
  sin.sin_port = htons(port);
  memcpy(&sin.sin_addr, hp->h_addr_list[0], hp->h_length);
  
  /* bind socket s to address sin */
  rc = bind(s, (struct sockaddr *)&sin, sizeof(sin));
  if ( rc < 0 ) {
    perror("bind:");
    exit(rc);
  }

  rc = listen(s, 10);
  if ( rc < 0 ) {
    perror("listen:");
    exit(rc);
  }
  printf("TP1");
  fflush(stdout);
  len = sizeof(sin);
    p = accept(s, (struct sockaddr *)&incoming, &len);
    if ( p < 0 ) {
      perror("bind:");
      exit(rc);
    }

    ihp = gethostbyaddr((char *)&incoming.sin_addr, 
      sizeof(struct in_addr), AF_INET);
    printf(">> Connected to %s\n", ihp->h_name);
    fflush(stdout);
  hp = gethostbyaddr((char *)&incoming.sin_addr, 
      sizeof(struct in_addr), AF_INET);
  len = recv(p, str, 32, 0);
  printf("%s\n", str);
  printf("TP1");
  close(s);
  printf(">> Connection closed\n");
  
}


void main()
{
  char* str = malloc(100);
  int port = 8888;
  printf("Triggering Receive Message");
  fflush(stdout);
  receiveMessageSocket(str, port);
  return;
}