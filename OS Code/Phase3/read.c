#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <string.h>
#include <sys/wait.h>

#define INPUT_MAX 1024

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