#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>

#include <unistd.h>
#include <stdlib.h>

#include "parseinput.h"


#define clear() printf("\033[H\033[J")
#define PORT 9007
#define BUFFER_SIZE 2048

// // Function for getting the current working directory and returning it as a string
// char* GettingUserWorkingDirectory(){
//   char * cwd = getcwd(NULL, 0);
//   return cwd;
// }

// // Function for printing the terminal user line (with $ and path)
// void UserPrompt(){
//   char *path = GettingUserWorkingDirectory();
//   printf("\n%s:%s$>>", getenv("USER"), path);
// }

void commandrunner(char* command, char** tokens)
{   
    // if ()
    // printf("compare working\n");

    // char runc = "pwd";
    if (strcmp(tokens[0],"!PWD") == 0)
    {
        system("pwd");
        // execl ("/bin/pwd", "pwd", NULL);
    }

    if (strcmp(tokens[0],"!LIST") == 0)
    {
        system("ls");
    }
    if (strcmp(tokens[0],"!CWD") == 0)
    {   
        if (chdir(tokens[1])==NULL)
        {
            perror(" Error changing directory");
        }
        else{
            printf(" Local directory changed...");
        }
    }
}


int main()
{       
    char bufferc[BUFFER_SIZE];

    clear();
    printf("\n ----------------| FTP Client |----------------");
	//create a socket
	int network_socket;
	network_socket = socket(AF_INET , SOCK_STREAM, 0);

	//check for fail error
	if (network_socket == -1) {
        printf("socket creation failed..\n");
        exit(EXIT_FAILURE);
    }

	//setsock
	int value  = 1;
	setsockopt(network_socket,SOL_SOCKET,SO_REUSEADDR,&value,sizeof(value)); //&(int){1},sizeof(int)
	
	struct sockaddr_in server_address;
	bzero(&server_address,sizeof(server_address));
	server_address.sin_family = AF_INET;
	server_address.sin_port = htons(PORT);
	server_address.sin_addr.s_addr = INADDR_ANY;

	//connect
    if(connect(network_socket,(struct sockaddr*)&server_address,sizeof(server_address))<0)
    {
        perror("connect");
        exit(EXIT_FAILURE);
    }
	printf("\n Successfully connected to server ...\n");
	char buffer[1024];

	while(1)
	{
        char* command;
        int rcv_flag = 1;
		//request user for command
		// UserPrompt();
		//get input from user
        printf("\n ftp>");
        command = readInput();
        if (command[0] == '\0') // If the command is empty meaning the user has pressed enter then we continue and loop back
        {
        continue;
        }
        //     fgets(command,sizeof(command),command);
        //    command[strcspn(command, "\n")] = 0;  //remove trailing newline char from command, fgets does not remove it
      
       if(strcmp(command,"exit")==0)
        {
            printf("closing the connection to server \n");
        	close(network_socket);
            break;
        }
        
        // if ()
        // if(send(network_socket,command,strlen(command),0)<0)
        // {
        //     perror("send");
        //     exit(EXIT_FAILURE);
        // }
        // bzero(command,sizeof(command));



        char *rawcommand = malloc(strlen(command) + 1);
        strcpy(rawcommand, command);

        char **tokens = tokenizer(command);

        if (strstr(tokens[0], "!") == tokens[0]) {
            if (!(strcmp(tokens[0],"!CWD")==0) && !(strcmp(tokens[0],"!PWD")==0) && !(strcmp(tokens[0],"!LIST")==0))
            {
                printf("Invalid Command! \n");
                continue;
            }
            commandrunner(command, tokens);
            continue;
        }

        // printf("$%s$\n", rawcommand);

        // send(network_socket,rawcommand,strlen(rawcommand)+1,0);

        
        if((strcmp(tokens[0],"USER")==0) || (strcmp(tokens[0],"PASS")==0))
        {
            send(network_socket,rawcommand,strlen(rawcommand)+1,0);
        }
        else if((strcmp(tokens[0],"RETR")==0) || (strcmp(tokens[0],"STOR")==0) )
        {
            send(network_socket,rawcommand,strlen(rawcommand)+1,0);
        }
        else if((strcmp(tokens[0],"CWD")==0) || (strcmp(tokens[0],"PWD")==0) || (strcmp(tokens[0],"LIST")==0))
        {
            send(network_socket,rawcommand,strlen(rawcommand)+1,0);
        }
        else if(strcmp(tokens[0],"QUIT")==0)
        {
            send(network_socket,rawcommand,strlen(rawcommand)+1,0);
        }
        else
        {
            rcv_flag = 0;
            printf(" Invalid Command! \n");
        }

        if (rcv_flag == 1)
        {
            bzero(bufferc, BUFFER_SIZE);                        // Clearing the buffer back to the buffer size
            recv(network_socket, bufferc, sizeof(bufferc), 0); // Client receiving the buffer output from the server
            printf(" %s\n", bufferc);                              // print the buffer from the server on the client screen
            bufferc[0] = '\0';
        }

        free(command);
        free(tokens);
        free(rawcommand);
	}

	return 0;
}
