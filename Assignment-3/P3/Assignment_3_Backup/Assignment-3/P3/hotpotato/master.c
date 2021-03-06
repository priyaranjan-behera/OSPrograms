#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

#define LEN	40960

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
  //printf("TP1");
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
  char str[LEN];
  int lport, rport, cport, port = 8888;
  int len, p[LEN], i, nplayers, nport, nhops, newsockets[LEN], lastplayer;
  char buf[LEN];
  struct sockaddr_in sin, incoming;
  struct hostent *ihp;
  //printf("Triggering Receive Message: %s", str);
  fflush(stdout);
  int32_t info;
  if ( argc < 4 ) {
    fprintf(stderr, "Usage: %s <port-number> <no. of players> <hops>\n", argv[0]);
    exit(1);
  }

  port = atoi(argv[1]);
  nport = port;
  nplayers = atoi(argv[2]);
  nhops = atoi(argv[3]);
  gethostname(str, sizeof str);
  srand(0);

  printf("\nPotato Master on %s", str);
  printf("\nPlayers = %d", nplayers);
  printf("\nHops = %d", nhops);

  int s = createReceiveSocket(port, (struct sockaddr_in *)&sin, nplayers);
  

  for(int i=0; i<nplayers; i++)
  {
    memset(&buf[0], 0, sizeof(buf));
    len = sizeof(sin);
    p[i] = accept(s, (struct sockaddr *)&incoming, &len);
    if ( p[i] < 0 ) {
    	perror("bind2:");
      exit(p[i]);
    }
    ihp = gethostbyaddr((char *)&incoming.sin_addr, 
  			sizeof(struct in_addr), AF_INET);
    printf("\nplayer %d is on %s\n",i,ihp->h_name);
    len = recv(p[i], buf, 32, 0);
    if ( len < 0 ) {
  	perror("recv");
  	exit(1);
    }
    //printf("%s - Received!\n", buf);
    //printf("Now sending the information to sender <id> <current port> <left port> <right port>:");
    memset(&buf[0], 0, sizeof(buf));

    info = htonl(i);
    len = send(p[i], &info, sizeof(uint32_t), 0);
    if ( len != sizeof(uint32_t) ) {
        perror("send");
        exit(1);
      }

    info = htonl(nplayers);
    len = send(p[i], &info, sizeof(uint32_t), 0);
    if ( len != sizeof(uint32_t) ) {
        perror("send");
        exit(1);
      }

    cport = port + i + 1;
    info = htonl(cport);
    len = send(p[i], &info, sizeof(uint32_t), 0);
    if ( len != sizeof(uint32_t) ) {
        perror("send");
        exit(1);
      }

    if(i == 0)
      lport = port + nplayers;
    else
      lport = port + i;
    info = htonl(lport);
    len = send(p[i], &info, sizeof(uint32_t), 0);
    if ( len != sizeof(uint32_t) ) {
        perror("send");
        exit(1);
      }

    if(i == (nplayers-1))
      rport = port + 1;
    else
      rport = port + i+2;
    info = htonl(rport);
    len = send(p[i], &info, sizeof(uint32_t), 0);
    if ( len != sizeof(uint32_t) ) {
        perror("send");
        exit(1);
      }

  }

  //receive readiness from the players
  for(int i=0; i<nplayers; i++) // Delivering Go Ahead
  {
    len = recv(p[i], &info, sizeof(uint32_t), 0);
    if ( len < 0 ) {
    perror("recv");
    exit(1);
    }
    //printf("\nRecieved Readiness: %d", ntohl(info));
  }

  //printf("\nSending Go Ahead!");
  fflush(stdout);
  for(int i=0; i<nplayers; i++) // Delivering Go Ahead
  {
    info = htonl(1);
    len = send(p[i], &info, sizeof(uint32_t), 0);
    if ( len != sizeof(uint32_t) ) {
        perror("send");
        exit(1);
      }
  }


  for(int i=0; i<nplayers; i++)
  {
    close(p[i]); //closing all sockets connected to master
  }


  //Create new sockets to connect to the player's waiting port
  //to announce start and termination of the game
  //Create Socket when required and end it

  /*
  for(int i=0; i<nplayers; i++)
  {
    printf("\nCreating New Sending Sockets from the ports listed : hostname: %s, port: %d", str, port + i + 1);
    newsockets[i] = createSendingSocket(str, port + i + 1);
  }

  */

  //Let the Game Begin
  i = rand() % nplayers;
  printf("\nAll players present, sending potato to player %d\n", i);
  newsockets[i] = createSendingSocket(str, port + i + 1);

  info = htonl(1);
  len = send(newsockets[i], &info, sizeof(uint32_t), 0);
  if ( len != sizeof(uint32_t) ) {
      perror("send");
      exit(1);
    }


  info = htonl(nhops);
  len = send(newsockets[i], &info, sizeof(uint32_t), 0);
  if ( len != sizeof(uint32_t) ) {
      perror("send");
      exit(1);
    }

  memset(&buf[0], 0, sizeof(buf));
  len = send(newsockets[i], buf, strlen(buf), 0);
  if ( len != strlen(buf) ) {
      perror("send");
      exit(1);
    }

  close(newsockets[i]);


  //N0w Master Waits for the Termination
  memset(&buf[0], 0, sizeof(buf));
  len = sizeof(sin);
  lastplayer = accept(s, (struct sockaddr *)&incoming, &len);
  if ( lastplayer < 0 ) {
    perror("bind2:");
    exit(lastplayer);
  }
  ihp = gethostbyaddr((char *)&incoming.sin_addr, 
      sizeof(struct in_addr), AF_INET);
  //printf(">> Connected to %s\n", ihp->h_name);

  memset(&buf[0], 0, sizeof(buf));
  len = recv(lastplayer, buf, LEN, 0);
  if ( len < 0 ) {
  perror("recv");
  exit(1);
  }
  printf("\nTrace of the potato: \n%s", buf);


  //Send Cease message to all nplayers
  for(int i=0; i<nplayers; i++)
  {
    //printf("\nCreating New Sending Sockets from the ports listed : hostname: %s, port: %d", str, port + i + 1);
    newsockets[i] = createSendingSocket(str, port + i + 1);
    info = htonl(2);
    len = send(newsockets[i], &info, sizeof(uint32_t), 0);
    if ( len != sizeof(uint32_t) ) {
        perror("send");
        exit(1);
      }


  }


  //printf("Processing Ends");
  fflush(stdout);
  close(s);
  printf("\n");
  return;
}