#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <time.h>
#include <errno.h>   

#include <unistd.h>
#include <stdlib.h>

#include "parseinput.h"


#define clear() printf("\033[H\033[J")
#define PORT 9007
#define BUFFER_SIZE 2048

int ftp_port;

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
        if (chdir(tokens[1])==-1)
        {
            perror(" Error changing directory");
        }
        else{
            printf(" Local directory changed...\n");
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

    
    struct sockaddr_in sin;
    socklen_t len = sizeof(sin);
    if (getsockname(network_socket, (struct sockaddr *)&sin, &len) == -1)
        perror("getsockname");
    else
        ftp_port = htons(sin.sin_port);

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
        else if (strcmp(tokens[0],"STOR")==0) 
        {
            char ftp_port_str[16];
            ftp_port+=1;

            printf("$something");
            sprintf(ftp_port_str, "%d", ftp_port); 
            
            printf("%d", ftp_port);

            send(network_socket,rawcommand,strlen(rawcommand)+1,0);
            send(network_socket, ftp_port_str, strlen(ftp_port_str)+1, 0);

            char filename[256];
            strcpy(filename, tokens[1]);
            FILE* file = fopen(filename, "rb");

            //check if file exists
            if(file == NULL){
                printf("\n File does not exist.");
            }
            else{
                
                int FTP_socket = socket(AF_INET,SOCK_STREAM,0);
                // printf("Server fd = %d \n",FTP_socket);
                
                //check for fail error
                if(FTP_socket<0)
                {
                    perror("socket:");
                    exit(EXIT_FAILURE);
                }

                //setsock
                int value  = 1;
                setsockopt(FTP_socket,SOL_SOCKET,SO_REUSEADDR,&value,sizeof(value)); //&(int){1},sizeof(int)
                
                //define server address structure
                struct sockaddr_in server_address;
                bzero(&server_address,sizeof(server_address));
                server_address.sin_family = AF_INET;
                server_address.sin_port = htons(ftp_port);
                server_address.sin_addr.s_addr = INADDR_ANY;


                //bind
                if(bind(FTP_socket, (struct sockaddr*)&server_address,sizeof(server_address))<0)
                {
                    perror("bind failed");
                    exit(EXIT_FAILURE);
                }

                //listen
                if(listen(FTP_socket,5)<0)
                {
                    perror("listen failed");
                    close(FTP_socket);
                    exit(EXIT_FAILURE);
                }

                int server_sd = accept(FTP_socket,0,0);
				printf("\n Server conected for FTP ...\n");

                //connection established, file transfer begin
                while (1)
                {
                    unsigned char storBuff[1024]={0};
                    int bytesRead = fread(storBuff, 1, 1024, file);

                    if(bytesRead > 0)
                    {
                        write(server_sd, storBuff, bytesRead);
                    }

                    //once bytes read does not fill buffer, it is the last portion of file
                    if (bytesRead < 1024)
                    {
                        break;
                    }
                }

                fclose(file);
                close(server_sd);
                close(FTP_socket);

                fflush(stdout);

            }

        }
        else if (strcmp(tokens[0],"RETR")==0)
        {   
            char ftp_port_str[16];
            ftp_port += 1;


            sprintf(ftp_port_str, "%d", ftp_port); 
            
            printf("$$$ %s", ftp_port_str);

            send(network_socket,rawcommand,strlen(rawcommand)+1,0);
            send(network_socket, ftp_port_str, strlen(ftp_port_str)+1, 0);
                
                int FTP_socket = socket(AF_INET,SOCK_STREAM,0);
                // printf("Server fd = %d \n",FTP_socket);
                
                //check for fail error
                if(FTP_socket<0)
                {
                    perror("socket:");
                    exit(EXIT_FAILURE);
                }

                //setsock
                int value  = 1;
                setsockopt(FTP_socket,SOL_SOCKET,SO_REUSEADDR,&value,sizeof(value)); //&(int){1},sizeof(int)
                


                //define server address structure
                struct sockaddr_in server_address;
                bzero(&server_address,sizeof(server_address));
                server_address.sin_family = AF_INET;
                server_address.sin_port = htons(ftp_port);
                server_address.sin_addr.s_addr = INADDR_ANY;


                //bind
                if(bind(FTP_socket, (struct sockaddr*)&server_address,sizeof(server_address))<0)
                {
                    perror("bind failed");
                    exit(EXIT_FAILURE);
                }

                //listen
                if(listen(FTP_socket,5)<0)
                {
                    perror("listen failed");
                    close(FTP_socket);
                    exit(EXIT_FAILURE);
                }

                int server_sd = accept(FTP_socket,0,0);
					printf("\n Server conected for FTP ...\n");
                

                char filename[256];
                strcpy(filename, tokens[1]);

                FILE* file;

                int bytesRead=0;
                char readBuff[1024];
                memset(readBuff, '0', sizeof(readBuff));
                file=fopen(filename, "wb");

                if(!file){
                    printf("Could not open file");
                    exit(1);
                }
                else
                {
                    while ((bytesRead = read(server_sd, readBuff, 1024)) >0)
                    {
                        fflush(stdout);
                        fwrite(readBuff, 1, bytesRead, file);
                        printf("%d", bytesRead);
                    }
                    
                }

                fclose(file);
                close(server_sd);
                close(FTP_socket);
                
        }
        else if (strcmp(tokens[0],"LIST")==0) 
        {
            char ftp_port_str[16];
            ftp_port += 1;

            sprintf(ftp_port_str, "%d", ftp_port); 
            
            send(network_socket,rawcommand,strlen(rawcommand)+1,0);
            send(network_socket, ftp_port_str, strlen(ftp_port_str)+1, 0);
                
                int FTP_socket = socket(AF_INET,SOCK_STREAM,0);
                // printf("Server fd = %d \n",FTP_socket);
                
                //check for fail error
                if(FTP_socket<0)
                {
                    perror("socket:");
                    exit(EXIT_FAILURE);
                }

                //setsock
                int value  = 1;
                setsockopt(FTP_socket,SOL_SOCKET,SO_REUSEADDR,&value,sizeof(value)); //&(int){1},sizeof(int)
                
                //define server address structure
                struct sockaddr_in server_address;
                bzero(&server_address,sizeof(server_address));
                server_address.sin_family = AF_INET;
                server_address.sin_port = htons(ftp_port);
                server_address.sin_addr.s_addr = INADDR_ANY;


                //bind
                if(bind(FTP_socket, (struct sockaddr*)&server_address,sizeof(server_address))<0)
                {
                    perror("bind failed");
                    exit(EXIT_FAILURE);
                }

                //listen
                if(listen(FTP_socket,5)<0)
                {
                    perror("listen failed");
                    close(FTP_socket);
                    exit(EXIT_FAILURE);
                }

                int server_sd = accept(FTP_socket,0,0);
					printf("\n Server conected for FTP ...\n");

                char list_message[256]; //to receive server's message 
                bzero(list_message,sizeof(list_message));
                printf("\r");
                while(recv(server_sd,list_message,sizeof(list_message),0) != 0)
                { // receive message from server: <list of files> 226 Transfer completed.
                    printf(" %s\n",list_message);
                    fflush(stdout);
                }


                // char lsBuff[1024];
                // recv(server_sd, lsBuff, sizeof(lsBuff), 0); // Client receiving the buffer output from the server
                // printf(" %s\n", lsBuff);
                       
                close(server_sd);
                close(FTP_socket);
        }
        else if((strcmp(tokens[0],"CWD")==0) || (strcmp(tokens[0],"PWD")==0) )
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