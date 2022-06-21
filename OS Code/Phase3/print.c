#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <string.h>
#include <sys/wait.h>

// Function for getting the current working directory and returning it as a string
char* GettingUserWorkingDirectory(){
  char * cwd = getcwd(NULL, 0);
  return cwd;
}

// Function for printing the terminal user line (with $ and path)
void UserPrompt(){
  char *path = GettingUserWorkingDirectory();
  printf("\n%s:%s$>>", getenv("USER"), path);
}