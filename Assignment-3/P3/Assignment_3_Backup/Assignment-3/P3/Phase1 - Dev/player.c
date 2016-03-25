#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

#define LEN	64

int createSendingSocket(char* hostname, int port)
{
  struct hostent *hp;
  int s, rc, len;
  struct sockaddr_in sin;

  hp = gethostbyname(hostname);
   if ( hp == NULL ) {
       fprintf(stderr, "%s: host not found\n", hostname);
       exit(1);
     }
     //fprintf(stdout, "%s: host found\n", hostname);

     s = socket(AF_INET, SOCK_STREAM, 0);
     if ( s < 0 ) {
    perror("socket:");
    exit(s);
  }
  //fprintf(stdout, "%d: socket found\n", s);

  sin.sin_family = AF_INET;
  sin.sin_port = htons(port);
  memcpy(&sin.sin_addr, hp->h_addr_list[0], hp->h_length);

  rc = connect(s, (struct sockaddr *)&sin, sizeof(sin));
  if ( rc < 0 ) {
    perror("connect:");
    exit(rc);
  }

  return s;

}


int createReceiveSocket(int port, struct sockaddr_in* sin, int nplayers)
{
  printf("TP1");
  fflush(stdout);
  char host[64];
  struct hostent *hp, *ihp;
  int s, rc, len, p;

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
  //printf("TP1");
  fflush(stdout);
  sin->sin_family = AF_INET;
  sin->sin_port = htons(port);
  memcpy(&sin->sin_addr, hp->h_addr_list[0], hp->h_length);
  
  /* bind socket s to address sin */
  rc = bind(s, (struct sockaddr *)sin, sizeof(*sin));
  if ( rc < 0 ) {
    perror("bind1:");
    exit(rc);
  }

  rc = listen(s, nplayers);
  if ( rc < 0 ) {
    perror("listen:");
    exit(rc);
  }

  return s;
}

void main(int argc, char *argv[])
{
  char *str = argv[1];
  char buf[LEN] = "Player Reporting";
  int cport, lport, rport, id, port = 8888;
  int len;
  int32_t info;
  struct sockaddr_in sin, incoming;
  struct hostent *ihp;
  port = atoi(argv[2]);
  printf("\nTriggering Send Message By Host:: %s", str);
  fflush(stdout);
  int lplayer, rplayer, p1;
  int master = createSendingSocket(str, port);
  len = send(master, buf, strlen(buf), 0);
  if ( len != strlen(buf) ) {
      perror("send");
      exit(1);
    }
  printf("\nSent Message");
  printf("\nReady to receive message: ");
  memset(&buf[0], 0, sizeof(buf));


  len = recv(master, &info, sizeof(uint32_t), 0);
  if ( len < 0 ) {
  perror("recv");
  exit(1);
  }
  id = ntohl(info);
  printf("\nRecieved Id: %d", ntohl(info));

  len = recv(master, &info, sizeof(uint32_t), 0);
  if ( len < 0 ) {
  perror("recv");
  exit(1);
  }
  cport = ntohl(info);
  printf("\nRecieved CPORT: %d", ntohl(info));

  len = recv(master, &info, sizeof(uint32_t), 0);
  if ( len < 0 ) {
  perror("recv");
  exit(1);
  }
  lport = ntohl(info);
  printf("\nRecieved LPORT: %d", ntohl(info));

  len = recv(master, &info, sizeof(uint32_t), 0);
  if ( len < 0 ) {
  perror("recv");
  exit(1);
  }
  rport = ntohl(info);
  printf("\nRecieved RPORT: %d", ntohl(info));


  //Creating player specific port
  int playerport = createReceiveSocket(cport, (struct sockaddr_in *)&sin, 10);



  //Sending readiness to master

  info = htonl(1);
    len = send(master, &info, sizeof(uint32_t), 0);
    if ( len != sizeof(uint32_t) ) {
        perror("send");
        exit(1);
      }

  //get a go ahead from master to connect to right and left players
  len = recv(master, &info, sizeof(uint32_t), 0);
  if ( len < 0 ) {
  perror("recv");
  exit(1);
  }
  printf("\nRecieved GoAhead: %d", ntohl(info));

  //After this go ahead. Lets connect this node to the left and right nodes.

  lplayer = createSendingSocket(str, lport);
  rplayer = createSendingSocket(str, rport);


  while(1)
  {
      len = sizeof(sin);
  p1 = accept(playerport, (struct sockaddr *)&incoming, &len);
  if ( p1 < 0 ) {
    perror("bind2:");
    exit(p1);
  }
  ihp = gethostbyaddr((char *)&incoming.sin_addr, 
      sizeof(struct in_addr), AF_INET);
  printf(">> Connected to %s\n", ihp->h_name);
  }


  //neighbours have been connected.













  fflush(stdout);
  close(master);
  return;
}