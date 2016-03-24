RECEIVE SOCKET


int s = createReceiveSocket(port, (struct sockaddr_in *)&sin);
  
  memset(&buf[0], 0, sizeof(buf));
  len = sizeof(sin);
  p = accept(s, (struct sockaddr *)&incoming, &len);
  if ( p < 0 ) {
  	perror("bind2:");
    exit(p);
  }
  ihp = gethostbyaddr((char *)&incoming.sin_addr, 
			sizeof(struct in_addr), AF_INET);
  printf(">> Connected to %s\n", ihp->h_name);
  len = recv(p, buf, 32, 0);
  if ( len < 0 ) {
	perror("recv");
	exit(1);
  }
  printf("%s - Recieved!\n", buf);






  SENDING SOCKET
  int s = createSendingSocket(str, port);
  len = send(s, str, strlen(str), 0);
  if ( len != strlen(str) ) {
      perror("send");
      exit(1);
    }