#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <string.h>
#include <sys/wait.h>

// Defining Macros and Delimiters
#define INPUT_MAX 1024
#define BUFFER_SIZE 20
#define TOKEN_DELIM " \'\n\"" //Token delimiters for parsing the user input


char *readInput()
{
	int i, j;

	char *line = (char *)malloc(sizeof(char) * INPUT_MAX); // Dynamically Allocate Buffer for the line read
	char c;												   // character that is read at a time
	int indexPosition = 0, bufferSize = 100;
	if (!line) // Buffer allocation failed
	{
		printf("\nFailed allocating Buffer");
		exit(EXIT_FAILURE);
	}
	int len = strlen(line);
		for (int i = 0; i < len; i++)
		{
			if (line[0] == '\'' && line[len - 1] == '\"')
			{
				line[len - 1] = '\0';
				memmove(line, line + 1, len - 1);
			}
		}
	while (1) // Reading all input
	{
		c = getchar();			   // Getting the character from the input and storing it into the variable
		if (c == EOF || c == '\n') // If there is an end of file or a new line, then we overwite it with a the null character
		{

			line[indexPosition] = '\0'; // Replace the character at that possiton with Null
			return line;				// Returning the line
		}								// So basically untill the end of line do everything below or else put the null and return the line string
		else
		{
			line[indexPosition] = c; // line character array (string) at that postion is equal to the character read
		}
		indexPosition++;				 // Move over the character to the next one so its the nexr indexPosition
		if (indexPosition >= bufferSize) // If buffer is exceeded
		{
			bufferSize += INPUT_MAX;						 // Increase the buffer again with (1024)
			line = realloc(line, sizeof(char) * bufferSize); // Reallocate the dynamic memory
			if (!line)										 // Buffer allocation failed
			{
				printf("\nFailed allocating Buffer");
				exit(EXIT_FAILURE);
			}
		}
	}

}

char **tokenizer(char *line) // Using the line (read line) passed as an argument 
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