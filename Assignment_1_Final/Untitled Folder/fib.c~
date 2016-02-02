/******************************************************************************
 *
 *  File Name........: fib.c
 *
 *  Description......:
 *
 *  Created by vin on 11/21/06
 *
 *  No warranty!  This program has not been compiled or tested because
 *  I do not have a library with which to do so.
 *
 *  It is provided for illusration.  
 *
 *****************************************************************************/

#include "mythread.h"
#include<stdio.h>

// evaluate a Fibonacci number:
//	fib(0) = 0
//	fib(1) = 1
//	fib(n) = fib(n-1) + fib(n-2)  [n>1]
// this function is messy because we have to pass everything as a
// generic parameter (void*).
// also, the function parameter is a value/result -- therefore it is a
// pointer to an integer.
//

void fib2(void *in)
{
}

void fib(void *in)
{
    MyThreadCreate(fib2, (void*)in);
    printf("Before Creating Child2");
    MyThreadCreate(fib2, (void*)in);
    // after creating children, wait for them to finish
    MyThreadJoinAll();
    //  write to addr n_ptr points; return results in addr pointed to
    //  by input parameter
  MyThreadExit();		// always call this at end
}



main(int argc, void *argv)
{
  int n = 10;



  printf("fib(%d) = ", n);
  MyThreadInit(fib, (void*)&n);
  printf("%d\n", n);
}


/*........................ end of fib.c .....................................*/
