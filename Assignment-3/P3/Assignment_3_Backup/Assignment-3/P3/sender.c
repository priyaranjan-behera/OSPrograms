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

  printf("TP1");
  fflush(stdout);
  hp = gethostbyname(hostname);
   if ( hp == NULL ) {
       fprintf(stderr, "%s: host not found\n", hostname);
       exit(1);
     }

     printf("TP1");
  fflush(stdout);
     s = socket(AF_INET, SOCK_STREAM, 0);
     if ( s < 0 ) {
    perror("socket:");
    exit(s);
  }

  sin.sin_family = AF_INET;
  sin.sin_port = htons(port);
  memcpy(&sin.sin_addr, hp->h_addr_list[0], hp->h_length);
  printf("TP1");
  fflush(stdout);
  rc = connect(s, (struct sockaddr *)&sin, sizeof(sin));
  if ( rc < 0 ) {
    perror("connect:");
    exit(rc);
  }
  len = send(s, str, strlen(str), 0);
  if ( len != strlen(str) ) {
      perror("send");
      exit(1);
    }
    printf("TP1");
  fflush(stdout);
  close(s);

}

void receiveMessageSocket(char* str, int port)
{
  char host[64];
  struct hostent *hp, *ihp;
  int s, rc, len, p;
  struct sockaddr_in sin, incoming;
  printf("TP1");
  fflush(stdout);
  gethostname(host, sizeof host);
  printf("TP1");
  fflush(stdout);
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
  
  sin.sin_family = AF_INET;
  sin.sin_port = htons(port);
  memcpy(&sin.sin_addr, hp->h_addr_list[0], hp->h_length);
  
  /* bind socket s to address sin */
  rc = bind(s, (struct sockaddr *)&sin, sizeof(sin));
  if ( rc < 0 ) {
    perror("bind:");
    exit(rc);
  }

  rc = listen(s, 1);
  if ( rc < 0 ) {
    perror("listen:");
    exit(rc);
  }
  
  p = accept(s, (struct sockaddr *)&incoming, &len);
  if ( p < 0 ) {
      perror("bind:");
      exit(rc);
    }

    ihp = gethostbyaddr((char *)&incoming.sin_addr, 
      sizeof(struct in_addr), AF_INET);
    printf(">> Connected to %s\n", ihp->h_name);

  hp = gethostbyaddr((char *)&incoming.sin_addr, 
      sizeof(struct in_addr), AF_INET);
  len = recv(p, str, 32, 0);
  printf("%s\n", str);

  close(s);
  printf(">> Connection closed\n");
  
}


void main()
{
  char str[64] = "Hey! How are you!";
  int port = 8888;
  printf("message sending");
  fflush(stdout);
  sendMessageSocket("localhost", str, port);
  printf("message sent");
  fflush(stdout);

  return;
}