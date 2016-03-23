#
#
CC=gcc
CFLAGS=-g

# comment line below for Linux machines
#LIB= -lhotpotato -lnsl

all: master player

master:	master.o
	$(CC) $(CFLAGS) -o $@ master.o $(LIB)

player:	player.o
	$(CC) $(CFLAGS) -o $@ player.o $(LIB)

master.o:	master.c

player.o:	player.c 

clean:
	\rm -f master player

squeaky:
	make clean
	\rm -f master.o player.o

tar:
	cd ..; tar czvf hotpotato.tar.gz hotpotato/Makefile hotpotato/master.c hotpotato/player.c hotpotato/README; cd hotpotato; mv ../hotpotato.tar.gz .

