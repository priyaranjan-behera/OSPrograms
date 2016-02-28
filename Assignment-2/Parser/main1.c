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

void exececho(Cmd c, int inPipeId, int outPipeId);
int std_in, std_out, std_err;
int currPipe;
int pipes[20];

static void prCmd(Cmd c, int inPipeId, int outPipeId)
{
  int i;

  if ( c ) {
    
    /*
    printf("%s%s ", c->exec == Tamp ? "BG " : "", c->args[0]);
    if ( c->in == Tin )
      printf("<(%s) ", c->infile);
    if ( c->out != Tnil )
      switch ( c->out ) {
      case Tout:
	printf(">(%s) ", c->outfile);
	break;
      case Tapp:
	printf(">>(%s) ", c->outfile);
	break;
      case ToutErr:
	printf(">&(%s) ", c->outfile);
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
    //printf("\nThe C-In is: %d", c->in);
    //printf("\nThe C-Out is: %d", c->out);
    putchar('\n');
    */
    // this driver understands one command

    if(strcmp(c->args[0], "echo") == 0)
    {
      printf("Executing echo: in and out pipes are: %d %d", inPipeId, outPipeId );
      exececho(c, inPipeId, outPipeId);
    }

    if ( !strcmp(c->args[0], "end") )
      exit(0);

    putchar('\n');
  }
}

static void prPipe(Pipe p)
{
  int i = 0;
  Cmd c;
  int inPipeId, outPipeId;


  currPipe = 0;
  outPipeId = 0;
  inPipeId = 0;

  if ( p == NULL )
    return;

  //printf("Begin pipe%s\n", p->type == Pout ? "" : " Error");
  for ( c = p->head; c != NULL; c = c->next ) {
    //printf("  Cmd #%d: ", ++i);
    if(c->in == Tpipe || c->in == TpipeErr)
    {
      inPipeId = currPipe-1;
    }
    if(c->out == Tpipe || c->out == TpipeErr)
    {
      outPipeId = currPipe;
      pipe(pipes+(2*currPipe));
      currPipe++;
    }   

    prCmd(c, inPipeId, outPipeId);
  }
  //printf("End pipe\n");
  prPipe(p->next);
}

int main(int argc, char *argv[])
{
  Pipe p;
  char host[25];
  gethostname(host, 25);

  while ( 1 ) {
    printf("%s%% ", host);
    p = parse();
    currPipe = 0;
    prPipe(p);
    freePipe(p);
  }
}

void exececho(Cmd c, int inPipeId, int outPipeId)
{
  int newfd;


  if(fork() == 0)
    {  
      redirection(c, inPipeId, outPipeId);

      for (int i = 1; c->args[i] != NULL; i++ )
        printf("%s ", c->args[i]);
      exit(0);
    }
    else
    {
      wait(); 
    }

}

void redirection(Cmd c, int inPipeId, int outPipeId)
{

  int newfd; 
  if ( c->out != Tnil )
      switch ( c->out ) {
      case Tout:
        //printf(">(%s) ", c->outfile);
      
        if ((newfd = open(c->outfile, O_CREAT|O_TRUNC|O_WRONLY, 0644)) < 0) {
          perror(c->outfile); /* open failed */
          exit(1);
        }
        dup2(newfd, 1); 
        break;
      case Tapp:
        printf(">>(%s) ", c->outfile);
        printf(">(%s) ", c->outfile);
      
        if ((newfd = open(c->outfile, O_CREAT|O_APPEND|O_WRONLY, 0644)) < 0) {
          perror(c->outfile); /* open failed */
          exit(1);
        }
        dup2(newfd, 1); 
        break;
      case ToutErr:
        printf(">&(%s) ", c->outfile);
        if ((newfd = open(c->outfile, O_CREAT|O_TRUNC|O_WRONLY, 0644)) < 0) {
          perror(c->outfile); /* open failed */
          exit(1);
        }
        dup2(newfd, 1); 
        dup2(newfd, 2); 
        break;
      case TappErr:
        printf(">>&(%s) ", c->outfile);
        printf(">&(%s) ", c->outfile);
        if ((newfd = open(c->outfile, O_CREAT|O_APPEND|O_WRONLY, 0644)) < 0) {
          perror(c->outfile); /* open failed */
          exit(1);
        }
        dup2(newfd, 1);
        dup2(newfd, 2);
        break;
      case Tpipe:
        printf("| ");
        dup2((pipes+(2*outPipeId))[0],1);
        close((pipes+(2*outPipeId))[1]);
        break;
      case TpipeErr:
        printf("|& ");
        dup2((pipes+(2*outPipeId))[0],1);
        dup2((pipes+(2*outPipeId))[0],2);
        close((pipes+(2*outPipeId))[1]);
        break;
      default:
        fprintf(stderr, "Shouldn't get here\n");
      }

      if ( c->in != Tnil )
      switch(c->in)
      {
        case Tin:
        if ((newfd = open(c->infile, O_RDONLY)) < 0) {
          perror(c->infile); /* open failed */
          exit(1);
        }
        dup2(newfd, 0);
        break;

        case Tpipe:
        printf("| ");
        dup2((pipes+(2*inPipeId))[1],0);
        close((pipes+(2*inPipeId))[0]);
        break;
      }
}

/*........................ end of main.c ....................................*/
