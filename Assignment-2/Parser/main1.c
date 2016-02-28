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
#include <string.h>

extern char **environ;
void exececho(Cmd c, int inPipeId, int outPipeId);
void execsetenv(Cmd c, int inPipeId, int outPipeId);
static char *env;

int std_in, std_out, std_err;
int currPipe;
int pipes[20];

void backupIO()
{
  std_in = dup(0);
  std_out = dup(1);
  std_err = dup(2);
}

void restoreIO()
{
  dup2(std_in,0);
  dup2(std_out,1);
  dup2(std_err,2); 
}

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
      //printf("Executing echo: in and out pipes are: %d %d", inPipeId, outPipeId );
      exececho(c, inPipeId, outPipeId);
    }
    if(strcmp(c->args[0], "pwd") == 0)
    {
      //printf("Executing echo: in and out pipes are: %d %d", inPipeId, outPipeId );
      execpwd(c, inPipeId, outPipeId);
    }
    else if(strcmp(c->args[0], "setenv") == 0)
    {
      //printf("Executing getenv: in and out pipes are: %d %d", inPipeId, outPipeId );
      execsetenv(c, inPipeId, outPipeId);
    }
    else if(strcmp(c->args[0], "cd") == 0)
    {
      //printf("Executing getenv: in and out pipes are: %d %d", inPipeId, outPipeId );
      execcd(c, inPipeId, outPipeId);
    }
    else if ( !strcmp(c->args[0], "end") )
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

  backupIO();

  while ( 1 ) {
    restoreIO();
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

boolean isBuiltinCommand(Cmd c)
{
  
}

void execsetenv(Cmd c, int inPipeId, int outPipeId)
{
  int newfd;
  int i=0;
  char* str;

  if(c->next == NULL)
  {
  //if(fork() == 0)
  //  {  
      redirection(c, inPipeId, outPipeId);

      if(c->args[1] == NULL)
      {
        i=0;
        while(environ[i]) {
          printf("%s\n", environ[i++]);
        }
      }
      else{
        //printf("TP");
        if(c->args[2] == NULL)
        {
            //printf("Invoking Putenv for %s to null",c->args[1]);
            env = strcat(c->args[1], "=");
            putenv(env);
        }
        else
        {
            //printf("Invoking Putenv for %s to %s",c->args[1],c->args[2]);
            env = strcat(strcat(c->args[1],"="),c->args[2]);
            //printf("will execute %s",env);
            putenv(env);
            //printf("The Output of the method is: %d", putenv(env));
        }

      }

    //  exit(0);
    //}
    //else
    //{
    //  wait(); 
    //}

  }
  else
  {


    if(fork() == 0)
    {  
      redirection(c, inPipeId, outPipeId);
      //printf("TP");
      if(c->args[1] == NULL)
      {
        i=0;
        while(environ[i]) {
          printf("%s\n", environ[i++]);
        }
      }
      else{
        //printf("TP");
        if(c->args[2] == NULL)
        {
            //printf("Invoking Putenv for %s to null",c->args[1]);
            env = strcat(c->args[1], "=");
            putenv(env);
        }
        else
        {
            //printf("Invoking Putenv for %s to %s",c->args[1],c->args[2]);
            env = strcat(strcat(c->args[1],"="),c->args[2]);
            //printf("will execute %s",env);
            putenv(env);
            //printf("The Output of the method is: %d", putenv(env));
        }

      }

      exit(0);
    }
    else
    {
      wait(); 
    }

  }

}

void redirection(Cmd c, int inPipeId, int outPipeId)
{
  char buf[1025];
  int n;

  int newfd; 
  if ( c->out != Tnil )
      switch ( c->out ) {
      case Tout:
        //printf(">(%s) ", c->outfile);
      
        if ((newfd = open(c->outfile, O_CREAT|O_TRUNC|O_WRONLY, 0644)) < 0) {
          perror(c->outfile); /* open failed */
          exit(1);
        }
        //fflush(1);
        dup2(newfd, 1); 
        break;
      case Tapp:
        printf(">>(%s) ", c->outfile);
        printf(">(%s) ", c->outfile);
      
        if ((newfd = open(c->outfile, O_CREAT|O_APPEND|O_WRONLY, 0644)) < 0) {
          perror(c->outfile); /* open failed */
          exit(1);
        }
        //fflush(1);
        dup2(newfd, 1); 
        break;
      case ToutErr:
        printf(">&(%s) ", c->outfile);
        if ((newfd = open(c->outfile, O_CREAT|O_TRUNC|O_WRONLY, 0644)) < 0) {
          perror(c->outfile); /* open failed */
          exit(1);
        }
        //fflush(1);
        //fflush(2);
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
        //fflush(1);
        //fflush(2);
        dup2(newfd, 1);
        dup2(newfd, 2);
        break;
      case Tpipe:
        printf("| ");
        //fflush(1);
        dup2((pipes+(2*outPipeId))[1],1);
        close((pipes+(2*outPipeId))[0]);
        break;
      case TpipeErr:
        printf("|& ");
        //fflush(1);
        //fflush(2);
        dup2((pipes+(2*outPipeId))[1],1);
        dup2((pipes+(2*outPipeId))[1],2);
        close((pipes+(2*outPipeId))[0]);
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
        ////fflush(0);
        dup2(newfd, 0);
        break;

        case Tpipe:
        printf("| ");
        ////fflush(0);
        dup2((pipes+(2*inPipeId))[0],0);
        /*
        printf("Got the data from pipe:");
        if ((n = read((pipes+(2*inPipeId))[0], buf, 1024)) >= 0) {
          buf[n] = 0; // terminate the string 
          printf("read %d bytes from the pipe: \"%s\"\n", n, buf);
        }
        
        */

        close((pipes+(2*inPipeId))[1]);
        break;
      }
}



void execcd(Cmd c, int inPipeId, int outPipeId)
{
  int newfd;
  int i=0;
  char* path;
  char cwd[100];

  if(c->next == NULL)
  {
      redirection(c, inPipeId, outPipeId);

      if(c->args[1] == NULL)
      {

        path = getenv("HOME");
        //printf("Changing the directory to %s", path);
        if(chdir(path) != 0)
        {
          perror("");
        }
        
      }
      else
      {
        if((c->args[1])[0] != '/')
        {   // true for the dir in cwd
          getcwd(cwd,sizeof(cwd));
          strcat(cwd,"/");
          strcat(cwd,c->args[1]);
          if(chdir(cwd) != 0)
          {
             perror(""); 
          }

        }
        else{//true for dir w.r.t. /
          if(chdir(c->args[1]) != 0)
          {
            perror(""); 
          }
        }
        

      }

      //getcwd(cwd,sizeof(cwd));
      //printf("The CWD is: %s", cwd);


  }
  else
  {


    if(fork() == 0)
    {  
      redirection(c, inPipeId, outPipeId);
      //printf("TP");
      if(c->args[1] == NULL)
      {

        path = getenv("HOME");
        //printf("Changing the directory to %s", path);
        if(chdir(path) != 0)
        {
          perror("");
        }
        
      }
      else
      {
        if((c->args[1])[0] != '/')
        {   // true for the dir in cwd
          getcwd(cwd,sizeof(cwd));
          strcat(cwd,"/");
          strcat(cwd,c->args[1]);
          if(chdir(cwd) != 0)
          {
             perror(""); 
          }

        }
        else{//true for dir w.r.t. /
          if(chdir(c->args[1]) != 0)
          {
            perror(""); 
          }
        }
        

      }

      //getcwd(cwd,sizeof(cwd));
      //printf("The CWD is: %s", cwd);

      exit(0);
    }
    else
    {
      wait(); 
    }

  }

}


void execpwd(Cmd c, int inPipeId, int outPipeId)
{
  char cwd[100];

  if(c->next == NULL)
  {
      redirection(c, inPipeId, outPipeId);


      getcwd(cwd,sizeof(cwd));
      printf("%s", cwd);


  }
  else
  {


    if(fork() == 0)
    {  
      redirection(c, inPipeId, outPipeId);
      //printf("TP");
      getcwd(cwd,sizeof(cwd));
      printf("%s", cwd);

      exit(0);
    }
    else
    {
      wait(); 
    }

  }

}



/*........................ end of main.c ....................................*/
