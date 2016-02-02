gcc -c -Wall -O2 thread_library.h
ar rvs thread_library.a thread_library.h.gch
gcc -o passing.out passing.c thread_library.a