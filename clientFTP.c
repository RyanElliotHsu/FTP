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

int main()
{   
    clear();
    char* command;
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
	printf("\n Successfully connected to server ...\n ");
	char buffer[1024];

	while(1)
	{
		//request user for command
		// UserPrompt();
		//get input from user
        printf("ftp>");
        command = readInput();

    //     fgets(command,sizeof(command),command);
    //    command[strcspn(command, "\n")] = 0;  //remove trailing newline char from command, fgets does not remove it
      
       if(strcmp(command,"exit")==0)
        {
            printf("closing the connection to server \n");
        	close(network_socket);
            break;
        }
        
        // if ()
        if(send(network_socket,command,strlen(command),0)<0)
        {
            perror("send");
            exit(EXIT_FAILURE);
        }
        bzero(command,sizeof(command));			
	}

	return 0;
}
