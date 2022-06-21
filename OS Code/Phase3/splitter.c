#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <string.h>
#include <sys/wait.h>

// Defining Macros and Delimiters
#define BUFFER_SIZE 20
#define TOKEN_DELIM " \'\n\"" //Token delimiters for parsing the user input
#define PIPE_DELIM "|" //Token delimiters for pipe splitting

// Function for splitting the line up into respective arguments in the array (Basically parsing)
char **lineSplitter(char *line) // Using the line (read line) passed as an argument 
{
  int bufferSize = BUFFER_SIZE, indexPosition = 0; // BUFFER_SIZE here is 32 indicating the number the arguments read in 
  char **tokens = malloc(bufferSize * sizeof(char*)); // Array of tokens dynamically allocated
  char *token; // Each token

  if (!tokens) { // If tokens memory wasnt allocated 
    fprintf(stderr, "Token memory allocation error\n");
    exit(EXIT_FAILURE);
  }

  token = strtok(line, TOKEN_DELIM); // Strtok breaks string line into a series of tokens using the token delimiters provided as Macro
  while (token != NULL) { // Till the last is not null 
    tokens[indexPosition] = token; // place the token into the array of tokens
    indexPosition++; // Increment the index postion for the token to be saved in the array

    if (indexPosition >= bufferSize) { // If index position is greater than the buffer 
      bufferSize += BUFFER_SIZE; // Increase the buffer
      tokens = realloc(tokens, bufferSize * sizeof(char*)); // Reallocate more memory 
      if (!tokens) { //If tokens memory wasnt allocated 
        fprintf(stderr, "Token memory allocation error\n");
        exit(EXIT_FAILURE);
      }
    }

    token = strtok(NULL, TOKEN_DELIM); // Strtok NULL tells the function to continue tokenizing the string that was passed first
  }

  tokens[indexPosition] = NULL; // last postion to be Null 

  return tokens; // return the array of tokens (arguments)
}

char **pipeSplitter(char *line, int pipeflag){
  int bufferSize = BUFFER_SIZE, indexPosition = 0;
  char **tokens = malloc(bufferSize * sizeof(char*)); // Array of tokens dynamically allocated
  char *token; // Each token (string between pipes)

  if (!tokens) { // If tokens memory wasnt allocated 
    fprintf(stderr, "Token memory allocation error\n");
    exit(EXIT_FAILURE);
  }

  token = strtok(line, PIPE_DELIM); // Strtok breaks string line into a series of tokens using the token delimiters provided as PIPE_DELIM Macro
  while (token != NULL) {
    tokens[indexPosition] = token; // place the token into the array of tokens
    indexPosition++;  // Increment the index postion for the token to be saved in the array
    if (indexPosition >= bufferSize) {
      bufferSize += BUFFER_SIZE;
      tokens = realloc(tokens, bufferSize * sizeof(char*));
      if (!tokens) {
        fprintf(stderr, "Token memory allocation error\n");
        exit(EXIT_FAILURE);
      }
    }
    token = strtok(NULL, PIPE_DELIM);  // Strtok NULL tells the function to continue tokenizing the string that was passed first
  }
  tokens[indexPosition] = NULL;
  return tokens; // return the array of tokens (arguments pipe seperated )
}