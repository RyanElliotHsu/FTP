#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <string.h>
#include <sys/wait.h>

void commandExecute(char **arguments, int socket)
{
  pid_t pid;
  int status;
  pid = fork();
  if (pid == 0)
  {
    dup2(socket, STDOUT_FILENO); // Redirecting the standard output straight to the server
    dup2(socket, STDERR_FILENO); // Redirecting the standard input straight to the server
    // Child process
    if (execvp(arguments[0], arguments) == -1)
    {
      perror("Error");
      exit(EXIT_FAILURE);
    }
    close(socket);
  }

  else if (pid < 0)
  {
    perror("Error");
  }else{
    wait(NULL);
  }
  wait(NULL);
}