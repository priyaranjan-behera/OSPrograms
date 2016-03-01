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
#include <sys/resource.h>
#include <ctype.h>
#include <unistd.h>

extern char **environ;
void exececho(Cmd c, int inPipeId, int outPipeId);
void execsetenv(Cmd c, int inPipeId, int outPipeId);
static char *env;

int std_in, std_out, std_err;
int currPipe;
int pipes[50];


void flushall()
{
  fflush(stdout);
  fflush(stderr);
}

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

int is_valid_int(const char *str)
{
   // Handle negative numbers.
   //
   if (*str == '-' || *str == '+')
      ++str;

   // Handle empty string or just "-".
   //
   if (!*str)
      return 0;

   // Check for non-digit chars in the rest of the stirng.
   //
   while (*str)
   {
      if (!isdigit(*str))
         return 0;
      else
         ++str;
   }

   return 1;
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
    if(!isBuiltinCommand(c->args[0]))
    {
        execcommand(c, inPipeId, outPipeId);
    }
    else if(strcmp(c->args[0], "echo") == 0)
    {
      //printf("Executing echo: in and out pipes are: %d %d", inPipeId, outPipeId );
      exececho(c, inPipeId, outPipeId);
    }
    else if(strcmp(c->args[0], "pwd") == 0)
    {
      //printf("Executing echo: in and out pipes are: %d %d", inPipeId, outPipeId );
      execpwd(c, inPipeId, outPipeId);
    }
    else if(strcmp(c->args[0], "setenv") == 0)
    {
      //printf("Executing getenv: in and out pipes are: %d %d", inPipeId, outPipeId );
      execsetenv(c, inPipeId, outPipeId);
    }
    else if(strcmp(c->args[0], "unsetenv") == 0)
    {
      //printf("Executing getenv: in and out pipes are: %d %d", inPipeId, outPipeId );
      execunsetenv(c, inPipeId, outPipeId);
    }
    else if(strcmp(c->args[0], "nice") == 0)
    {
      //printf("Executing getenv: in and out pipes are: %d %d", inPipeId, outPipeId );
      execnice(c, inPipeId, outPipeId);
    }
    else if(strcmp(c->args[0], "cd") == 0)
    {
      //printf("Executing getenv: in and out pipes are: %d %d", inPipeId, outPipeId );
      execcd(c, inPipeId, outPipeId);
    }
    else if(strcmp(c->args[0], "where") == 0)
    {
      //printf("Executing getenv: in and out pipes are: %d %d", inPipeId, outPipeId );
      execwhere(c, inPipeId, outPipeId);
    }
    else if ( !strcmp(c->args[0], "logout") )
      exit(0);
    else if ( !strcmp(c->args[0], "end") )
      exit(0);

    //putchar('\n');
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
      //printf("Setting In Pipe %d for command %s", inPipeId, c->args[0]);
    }
    if(c->out == Tpipe || c->out == TpipeErr)
    {
      outPipeId = currPipe;
      pipe(pipes+(2*currPipe));
      currPipe++;
      //printf("Setting out Pipe %d for command %s", outPipeId, c->args[0]);
    }   

  

    //fprintf(stdout,"\nInitiating command: %s\n", c->args[0]);
    prCmd(c, inPipeId, outPipeId);
    //fprintf(stdout,"\nReturning from command: %s\n", c->args[0]);
  }
  //printf("End pipe\n");
  prPipe(p->next);
}

int main(int argc, char *argv[])
{
  Pipe p;
  char host[25];
  gethostname(host, 25);
  char* home=malloc(100);
  int fdin;
  backupIO();

  home = strdup(getenv("HOME"));
  strcat(home, "/.ushrc");

  if((fdin=open(home, O_RDONLY))==-1)
    perror("OPEN USHRC"); 
  else
  {
    dup2(fdin, 0);
    close(fdin);
    printf("%s%% ", host);
    fflush(stdout);
    p = parse();
    currPipe = 0;
    prPipe(p);
    //freePipe(p);
  }

  
  


  while ( 1 ) {

    restoreIO();
    flushall();
    printf("%s%% ", host);
    fflush(stdout);
    p = parse();
    currPipe = 0;
    prPipe(p);
    //freePipe(p);

  }
}

void exececho(Cmd c, int inPipeId, int outPipeId)
{
  int newfd;

  if(c->next == NULL)
  {
    redirection(c, inPipeId, outPipeId);

      for (int i = 1; c->args[i] != NULL; i++ )
        printf("%s ", c->args[i]);
      putchar('\n');
  }
  else
  {
    if(fork() == 0)
      {  
        redirection(c, inPipeId, outPipeId);

        for (int i = 1; c->args[i] != NULL; i++ )
          printf("%s ", c->args[i]);
        putchar('\n');
        exit(0);
      }
      else
      {
        close((pipes+(2*outPipeId))[1]);
        wait(); 
      }
  }

}

int isBuiltinCommand(char* c)
{

  char *commands[] = {"echo", "pwd", "setenv", "cd", "end", "where", "unsetenv", "nice", "logout"};
  int i;
  i=0;

  for(i=0; i<9; i++)
  {
    if(strcmp(c, commands[i]) == 0)
    {
      return 1;
    }
  }

  return 0;
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
        putchar('\n');
      }
      else{
        //printf("TP");
        if(c->args[2] == NULL)
        {
            //printf("Invoking Putenv for %s to null",c->args[1]);
            str = strdup(c->args[1]);
            str = strcat(str, "=");
            putenv(str);
            //fprintf(stderr, "%s set to %s", c->args[1], getenv(c->args[1]));
        }
        else
        {
            //printf("Invoking Putenv for %s to %s",c->args[1],c->args[2]);
            str = strdup(c->args[1]);
            str = strcat(strcat(str,"="),c->args[2]);
            //printf("will execute %s",env);
            putenv(str);
            //fprintf(stderr, "%s set to %s", c->args[1], getenv(c->args[1]));
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
        putchar('\n');
      }
      else{
        //printf("TP");
         if(c->args[2] == NULL)
        {
            //printf("Invoking Putenv for %s to null",c->args[1]);
          str = strdup(c->args[1]);;
            str = strcat(str, "=");
            putenv(str);
            //fprintf(stderr, "%s set to %s", c->args[1], getenv(c->args[1]));
        }
        else
        {
            //printf("Invoking Putenv for %s to %s",c->args[1],c->args[2]);
            str = strdup(c->args[1]);
            str = strcat(strcat(str,"="),c->args[2]);
            //printf("will execute %s",env);
            putenv(str);
            //fprintf(stderr, "%s set to %s", c->args[1], getenv(c->args[1]));
            //printf("The Output of the method is: %d", putenv(env));
        }

      }

      exit(0);
    }
    else
    {
      close((pipes+(2*outPipeId))[1]);
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
        fflush(stdout);
        dup2(newfd, 1); 
        break;
      case Tapp:
        //printf(">>(%s) ", c->outfile);
        //printf(">(%s) ", c->outfile);
      
        if ((newfd = open(c->outfile, O_CREAT|O_APPEND|O_WRONLY, 0644)) < 0) {
          perror(c->outfile); /* open failed */
          exit(1);
        }
        fflush(stdout);
        dup2(newfd, 1); 
        break;
      case ToutErr:
        //printf(">&(%s) ", c->outfile);
        if ((newfd = open(c->outfile, O_CREAT|O_TRUNC|O_WRONLY, 0644)) < 0) {
          perror(c->outfile); /* open failed */
          exit(1);
        }
        fflush(stdout);
        fflush(stderr);
        dup2(newfd, 1); 
        dup2(newfd, 2); 
        break;
      case TappErr:
        //printf(">>&(%s) ", c->outfile);
        //printf(">&(%s) ", c->outfile);
        if ((newfd = open(c->outfile, O_CREAT|O_APPEND|O_WRONLY, 0644)) < 0) {
          perror(c->outfile); /* open failed */
          exit(1);
        }
        fflush(stdout);
        fflush(stderr);
        dup2(newfd, 1);
        dup2(newfd, 2);
        break;
      case Tpipe:
        //printf("| ");
        fflush(stdout);
        close((pipes+(2*outPipeId))[0]);
        dup2((pipes+(2*outPipeId))[1],1);
        close((pipes+(2*outPipeId))[1]);
        
        break;
      case TpipeErr:
        //printf("|& ");
        fflush(stdout);
        fflush(stderr);
        close((pipes+(2*outPipeId))[0]);
        dup2((pipes+(2*outPipeId))[1],1);
        dup2((pipes+(2*outPipeId))[1],2);
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
        ////fflush(0);
        //fprintf(stdout, "Yes! Inside file input \n" );
        dup2(newfd, 0);
        close(newfd);
        break;

        case Tpipe:
        //printf("| ");
        ////fflush(0);
        close((pipes+(2*inPipeId))[1]);
        dup2((pipes+(2*inPipeId))[0],0);
        close((pipes+(2*inPipeId))[0]);
        /*
        printf("Got the data from pipe:");
        if ((n = read((pipes+(2*inPipeId))[0], buf, 1024)) >= 0) {
          buf[n] = 0; // terminate the string 
          printf("read %d bytes from the pipe: \"%s\"\n", n, buf);
        }
        
        */

        
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
      close((pipes+(2*outPipeId))[1]);
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
      putchar('\n');


  }
  else
  {


    if(fork() == 0)
    {  
      redirection(c, inPipeId, outPipeId);
      //printf("TP");
      getcwd(cwd,sizeof(cwd));
      printf("%s", cwd);
      putchar('\n');

      exit(0);
    }
    else
    {
      close((pipes+(2*outPipeId))[1]);
      wait(); 
    }

  }

}

void execcommand(Cmd c, int inPipeId, int outPipeId)
{
  char cwd[100];
  pid_t  pid;
  pid = fork();
  int status;

    if( pid == 0)
    {  
      redirection(c, inPipeId, outPipeId);
      //printf("TP");
      //if((c->args+1) != NULL)
      //fprintf(std_out, "The command and arguments are: %s: %s\n", c->args[0], c->args[1]);
      //flushall();
      if(execvp(c->args[0], c->args) < 0)
        perror("");
      //else
        //execvp(c->args[0]);  
      //fprintf(stdout,"Processed Here\n");
      fflush(stdout);
      _exit(0);
    }
    else
    {
      close((pipes+(2*outPipeId))[1]);
      while(wait(&status) != pid); 
    }


}



void execwhere(Cmd c, int inPipeId, int outPipeId)
{
  int newfd;
  int i=0;
  char* str_path =  getenv("PATH");
  char* str;
  char* dir;
  char path[20];

  if(c->next == NULL)
  {
  //if(fork() == 0)
  //  {  
      redirection(c, inPipeId, outPipeId);
      
      str = strdup(str_path);
      //printf("PATH:%s\n",str);
      printf("%s:", c->args[1]);
      if(isBuiltinCommand(c->args[1]))
        printf(" Built-in command \n%s:",c->args[1]);
      while ((dir = strsep(&str, ":")))
      {
          strcpy(path, dir);  
          strcat(path,"/");
          strcat(path,c->args[1]);
          //printf("dir:%s\n", path);
          if(access(path, F_OK) == 0)
          {
            printf(" %s",path);
          }
      } 

      putchar('\n');






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
        
        str = strdup(str_path);
        //printf("PATH:%s\n",str);
        printf("%s:", c->args[1]);
        if(isBuiltinCommand(c->args[1]))
        printf(" Built-in command \n%s:",c->args[1]);
        while ((dir = strsep(&str, ":")))
        {
            strcpy(path, dir);  
            strcat(path,"/");
            strcat(path,c->args[1]);
            //printf("dir:%s\n", path);
            if(access(path, F_OK) == 0)
            {
              printf(" %s",path);
            }
        } 


      putchar('\n');


      exit(0);
    }
    else
    {
      close((pipes+(2*outPipeId))[1]);
      wait(); 
    }

  }

}


void execunsetenv(Cmd c, int inPipeId, int outPipeId)
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
        perror("Variable not input");
      }
      else{
        //printf("TP");
        
            //printf("Invoking Putenv for %s to null",c->args[1]);
            str = strdup(c->args[1]);
            unsetenv(str);

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

      if(c->args[1] == NULL)
      {
        perror("Variable not input");
      }
      else{
        //printf("TP");
        
            //printf("Invoking Putenv for %s to null",c->args[1]);
            str = strdup(c->args[1]);
            unsetenv(str);

      }

      exit(0);
    }
    else
    {
      close((pipes+(2*outPipeId))[1]);
      wait(); 
    }

  }

}




void execnice(Cmd c, int inPipeId, int outPipeId)
{
  int newfd;
  int i=0;
  char* str;
  int which = PRIO_PROCESS;
  id_t pid;

  if(c->next == NULL)
  {
  //if(fork() == 0)
  //  {  
      redirection(c, inPipeId, outPipeId);

      if(c->args[1] == NULL)
      {
        pid = getpid();
        if(setpriority(which, pid, 4) == -1)
          perror("Cannnot set priority");
        //fprintf(stderr, "The priority set is: %d\n", getpriority(which, pid));
      }
      else if(is_valid_int(c->args[1]) == 0)
      {
        pid = getpid();
        if(setpriority(which, pid, 4) == -1)
          perror("Cannnot set priority");
        //fprintf(stderr, "The priority set is: %d\n", getpriority(which, pid));
        if(c->args[2] != NULL)
        {
          c->args+=1;
          c->nargs-=1;
          //fprintf(stderr, "The priority set is: %d for command: %s\n", getpriority(which, pid),c->args[0]);
          prCmd(c, inPipeId, outPipeId);
        }
      }
      else{

        pid = getpid();
        if(setpriority(which, pid, atoi(c->args[1])) == -1)
          perror("Cannnot set priority");
        //fprintf(stderr, "The priority set is: %d\n", getpriority(which, pid));
        if(c->args[2] != NULL)
        {
          c->args+=2;
          c->nargs-=2;
          //fprintf(stderr, "The priority set is: %d for command: %s\n", getpriority(which, pid),c->args[0]);
          prCmd(c, inPipeId, outPipeId);
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

      if(c->args[1] == NULL)
      {
        pid = getpid();
        if(setpriority(which, pid, 4) == -1)
          perror("Cannnot set priority");
        //fprintf(stderr, "The priority set is: %d\n", getpriority(which, pid));
      }
      else if(is_valid_int(c->args[1]) == 0)
      {
        pid = getpid();
        if(setpriority(which, pid, 4) == -1)
          perror("Cannnot set priority");
        //fprintf(stderr, "The priority set is: %d\n", getpriority(which, pid));
        if(c->args[2] != NULL)
        {
          c->args+=1;
          c->nargs-=1;
          //fprintf(stderr, "The priority set is: %d for command: %s\n", getpriority(which, pid),c->args[0]);
          prCmd(c, inPipeId, outPipeId);
        }
      }
      else{

        pid = getpid();
        if(setpriority(which, pid, atoi(c->args[1])) == -1)
          perror("Cannnot set priority");
        //fprintf(stderr, "The priority set is: %d\n", getpriority(which, pid));
        if(c->args[2] != NULL)
        {
          c->args+=2;
          c->nargs-=2;
          //fprintf(stderr, "The priority set is: %d for command: %s\n", getpriority(which, pid),c->args[0]);
          prCmd(c, inPipeId, outPipeId);
        }
        

      }

      exit(0);
    }
    else
    {
      close((pipes+(2*outPipeId))[1]);
      wait(); 
    }

  }

}



/*
char** str_split(char* a_str, const char a_delim) //Reference: stackoverflow: http://stackoverflow.com/questions/9210528/split-string-with-delimiters-in-c
{
    char** result    = 0;
    size_t count     = 0;
    char* tmp        = a_str;
    char* last_comma = 0;
    char delim[2];
    delim[0] = a_delim;
    delim[1] = 0;

    
    while (*tmp)
    {
        if (a_delim == *tmp)
        {
            count++;
            last_comma = tmp;
        }
        tmp++;
    }

    count += last_comma < (a_str + strlen(a_str) - 1);

    count++;

    result = malloc(sizeof(char*) * count);

    if (result)
    {
        size_t idx  = 0;
        char* token = strtok(a_str, delim);

        while (token)
        {
            assert(idx < count);
            *(result + idx++) = strdup(token);
            token = strtok(0, delim);
        }
        assert(idx == count - 1);
        *(result + idx) = 0;
    }

    return result;
}

*/


/*........................ end of main.c ....................................*/
