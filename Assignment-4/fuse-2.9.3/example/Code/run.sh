gcc -w -g -Wall fuseLibrary.c `pkg-config fuse --cflags --libs` -o fuseLibrary
fusermount -u /tmp/fuse
./fuseLibrary -f /tmp/fuse &> log.txt
