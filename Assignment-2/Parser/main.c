/******************************************************************************
 *
 *  File Name........: main.c
 *
 *  Description......: Simple driver program for ush's parser
 *
 *  Author...........: Vincent W. Freeh
 *
 *****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include "parse.h"
#include <stdlib.h>
#include <fcntl.h>

static void prCmd(Cmd c)
{
  int i;
  int newfd;


  if ( c ) {
    printf("%s%s ", c->exec == Tamp ? "BG " : "", c->args[0]);
    if ( c->in == Tin )
      printf("<(%s) ", c->infile);
    if ( c->out != Tnil )
      switch ( c->out ) {
      case Tout:
	printf(">(%s) ", c->outfile);
	if ((newfd = open(c->outfile, O_CREAT|O_TRUNC|O_WRONLY, 0644)) < 0) {
		perror(c->outfile);	/* open failed */
		exit(1);
	}
	printf("TP2\n");
	dup2(newfd, 1); 
	printf("TP3\n");
	break;
      case Tapp:
	printf(">>(%s) ", c->outfile);
	break;
      case ToutErr:
	printf(">&(%s) ", c->outfile);
	if ((newfd = open(c->outfile, O_CREAT|O_TRUNC|O_WRONLY, 0644)) < 0) {
		perror(c->outfile);	/* open failed */
		exit(1);
	}
	dup2(newfd, 1); 
	dup2(newfd, 2); 
	break;
      case TappErr:
	printf(">>&(%s) ", c->outfile);
	break;
      case Tpipe:
	printf("| ");
	break;
      case TpipeErr:
	printf("|& ");
	break;
      default:
	fprintf(stderr, "Shouldn't get here\n");
	exit(-1);
      }

    if ( c->nargs > 1 ) {
      printf("[");
      for ( i = 1; c->args[i] != NULL; i++ )
	printf("%d:%s,", i, c->args[i]);
      printf("\b]");
    }
    putchar('\n');
    // this driver understands one command
 	if(strcmp(c->args[0], "echo") == 0)
 	{
 		exececho(c);
 	}

    if ( !strcmp(c->args[0], "end") )
      exit(0);
  }
}

static void prPipe(Pipe p)
{
  int i = 0;
  Cmd c;

  if ( p == NULL )
    return;

  printf("Begin pipe%s\n", p->type == Pout ? "" : " Error");
  for ( c = p->head; c != NULL; c = c->next ) {
    printf("  Cmd #%d: ", ++i);
    prCmd(c);
  }
  printf("End pipe\n");
  prPipe(p->next);
}

int main(int argc, char *argv[])
{
  Pipe p;
  char *host = "armadillo";

  int std_output = dup(1);
  int std_error = dup(2);
  int std_input = dup(3);

  while ( 1 ) {
  	dup2(std_output,1);
  	dup2(std_erorr,2);
  	dup2(std_input,3);
    printf("%s%% ", host);
    p = parse();
    prPipe(p);
    freePipe(p);
  }
}

void exececho(Cmd c)
{
	if(fork() == 0)
 		{

		    

 			execvp("echo", c->args);
 			//printf("\nCalling from Child. My ID: %d and my parent: %d",getpid(),getppid());
 			//for ( i = 1; c->args[i] != NULL; i++ )
			//	printf("%s ", c->args[i]);
			
 			exit(0);
 		}
 		else
 		{
 			//printf("\nBW Calling from Parent. My Id: %d",getpid());
 			wait();
 			//printf("\nAW Calling from Parent. My Id: %d",getpid());
 			
 		}
}

/*........................ end of main.c ....................................*/
