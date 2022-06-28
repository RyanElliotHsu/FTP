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
        printf("CLIENT: ");
        printf("");
        system("pwd");
        // execl ("/bin/pwd", "pwd", NULL);
    }

    if (strcmp(tokens[0],"!LIST") == 0)
    {
        system("ls");
    }
    if (strcmp(tokens[0],"!CWD") == 0)
    {   
        char* run = "cd ";
        // char args[] = tokens[1];
        // printf("%s", args);
        strcat(run, tokens[1]);
        // run += tokens[1];
        printf("#%s", run);
        // system(run);
    }

}


int main()
{       
    char bufferc[BUFFER_SIZE];

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
    int client_port_no = htons(server_address.sin_port);

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
        //fgets(command,sizeof(command),command);
        //command[strcspn(command, "\n")] = 0;  //remove trailing newline char from command, fgets does not remove it
      
        // if(strcmp(command,"QUIT")==0)
        // {
        //     printf("closing the connection to server \n");
        // 	close(network_socket);
        //     break;
        // }
        
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

        // if (strstr(tokens[0], "!") == tokens[0])
        // {
        //     if (!(strcmp(tokens[0],"!CWD")==0) && !(strcmp(tokens[0],"!PWD")==0) && !(strcmp(tokens[0],"!LIST")==0))
        //     {
        //         printf("Invalid Command! \n");
        //         continue;
        //     }
        //     commandrunner(command, tokens);
        //     continue;
        // }
    

        printf("$%s$\n", rawcommand);
        send(network_socket,rawcommand,strlen(rawcommand)+1,0);

        if((strcmp(tokens[0],"USER")==0) || (strcmp(tokens[0],"PASS")==0))
        {
            send(network_socket,rawcommand,strlen(rawcommand),0);
        }
        else if((strcmp(tokens[0],"RETR")==0) || (strcmp(tokens[0],"STOR")==0) )
        {
            printf("111");

            // send(network_socket,rawcommand,strlen(rawcommand),0);
            char filename[256];
            strcpy(filename,tokens[1]);
            FILE *file = fopen(filename,"r");
            if(file==NULL)
            {	
                printf("No such file or directory.\n");
                continue;
            }
            else
            {
                printf("222");
                
                
                int client_sender_sd = socket(AF_INET,SOCK_STREAM,0);
                if(client_sender_sd<0)
                {
                    perror("Client Receiver Socket creation:");
                    exit(-1);
                }
                //setsock

                printf("333");
                int value  = 1;
                setsockopt(client_sender_sd,SOL_SOCKET,SO_REUSEADDR,&value,sizeof(value)); //&(int){1},sizeof(int)
                struct sockaddr_in client_sender_addr, server_data_addr;

                bzero(&client_sender_addr,sizeof(client_sender_addr));

                client_sender_addr.sin_family = AF_INET;
                client_sender_addr.sin_port = htons(9008);
                client_sender_addr.sin_addr.s_addr = htonl(INADDR_ANY); //INADDR_ANY, INADDR_LOOP

                //bind
                printf("444");
                if(bind(client_sender_sd, (struct sockaddr*)&client_sender_addr,sizeof(client_sender_addr))<0)
                {
                    perror("Client Receiver Socket: bind failed");
                    exit(-1);
                }
                //listen
                printf("555");
                if(listen(client_sender_sd,5)<0)
                {
                    perror("Client Receiver Socket: listen failed");
                    close(client_sender_sd);
                    exit(-1);
                }
                //send(network_socket, client_input, strlen(client_input), 0);

                //b.server connects to that port using port 20; client accepts connection
                unsigned int server_data_len = sizeof(server_data_addr);
                int server_data_sd = accept(client_sender_sd,(struct sockaddr *) &server_data_addr,&server_data_len);
                
                //4.after connection is established, send file data from client to server
                while(1)
                {
                    unsigned char buff[1024]={0};
                    int nread = fread(buff,1,1024,file);

                    if(nread > 0)
                    {
                        write(server_data_sd, buff, nread);
                    }
                    if (nread < 1024)
                    {
                        if (feof(file))
                        {
                            
                        }
                        if (ferror(file))
                        {
                            printf("Error reading\n");
                            break;
                        }
                    }
                }
                fclose(file);
                //5.close fconnection
                close(server_data_sd);
                close(client_sender_sd);
            }
        }
    }

    //if((strcmp(tokens[0],"CWD")==0) || (strcmp(tokens[0],"PWD")==0) || (strcmp(tokens[0],"LIST")==0))
    //{
    //    send(network_socket,rawcommand,strlen(rawcommand),0);
    //}
    // else if(strcmp(tokens[0],"QUIT")==0)
    // {
    //     send(network_socket,rawcommand,strlen(rawcommand),0);
    // }
    // else
    // {
    //     printf("Invalid Command! \n");
    // }

    bzero(bufferc, BUFFER_SIZE);                        // Clearing the buffer back to the buffer size
    recv(network_socket, bufferc, sizeof(bufferc), MSG_DONTWAIT); // Client receiving the buffer output from the server
    printf("%s\n", bufferc);                              // print the buffer from the server on the client screen
    bufferc[0] = '\0';

	

	return 0;

}