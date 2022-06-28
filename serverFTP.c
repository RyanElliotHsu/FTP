#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/time.h>
#include <sys/select.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/wait.h>

#include "parseinput.h"

#define clear() printf("\033[H\033[J")
#define PORT 9007
#define MAXUSER 100
#define MAX_BUFFER 2048

int listSize = 0;

struct User
{
	char username[16];
	char password[16];
	int usernameFlag;
	int loginFlag;
	int userFD;
	//char* path
};

struct User userList[MAXUSER];

struct User assignUser(char* username, char* password)
{
	struct User tempUser;

	//default username and login state is 0
	tempUser.usernameFlag = 0;
	tempUser.loginFlag = 0;
	strcpy(tempUser.username,username);
	strcpy(tempUser.password,password);

	return tempUser;
}

void getAuth()
{
	FILE *file = fopen("user.txt", "r"); 
    char *line = NULL; 
    size_t len = 0; 
    ssize_t lineSize;

	//error check for opening file
	if (file == NULL) { 
        perror("fopen"); 
        exit(EXIT_FAILURE); 
    }

	// char** userArray;
	//iterate through lines
	while ((lineSize = getline(&line, &len, file)) != -1) {
		
		// printf("#%s#", line);

		char **arg;
		arg = tokenizer(line);

		userList[listSize] = assignUser(arg[0], arg[1]); 
		
		// printf("\n#%s#%s#", tempUser.username, tempUser.password);

		listSize++;
    }
}

void userAuth(const char* username, int usernumber)
{
	for (int i=0; i<listSize; i++)
	{	
		//username found
		if (strcmp(username,userList[i].username)==0)
		{
			userList[i].userFD = usernumber;
			printf("\nLogged in as %s..", username);
			break;
		}
	}

	// printf("\n Username does not exist..");
}

void passAuth(const char* password, int userNum)
{
	if (strcmp(password,userList[userNum].password)==0)
	{
		//password is correct
		
	}
	else
	{
		printf("Incorrect password..");
	}
}

// command[BUFFER];

//recv()
int main()
{	
	clear();
	printf("\n ----------------| FTP Server |----------------");
	printf("\n Waiting for Client to join!");

	//read user.txt file
	getAuth();


	for (int i=0; i<listSize; ++i){
		printf("\nLIST:%s,%s", userList[i].username, userList[i].password);
	}
	
	int server_socket = socket(AF_INET,SOCK_STREAM,0);
	// printf("Server fd = %d \n",server_socket);
	
	//check for fail error
	if(server_socket<0)
	{
		perror("socket:");
		exit(EXIT_FAILURE);
	}

	//setsock
	int value  = 1;
	setsockopt(server_socket,SOL_SOCKET,SO_REUSEADDR,&value,sizeof(value)); //&(int){1},sizeof(int)
	
	//define server address structure
	struct sockaddr_in server_address;
	bzero(&server_address,sizeof(server_address));
	server_address.sin_family = AF_INET;
	server_address.sin_port = htons(PORT);
	server_address.sin_addr.s_addr = INADDR_ANY;


	//bind
	if(bind(server_socket, (struct sockaddr*)&server_address,sizeof(server_address))<0)
	{
		perror("bind failed");
		exit(EXIT_FAILURE);
	}

	//listen
	if(listen(server_socket,5)<0)
	{
		perror("listen failed");
		close(server_socket);
		exit(EXIT_FAILURE);
	}
	

	//DECLARE 2 fd sets (file descriptor sets : a collection of file descriptors)
	fd_set all_sockets;
	fd_set ready_sockets;


	//zero out/iniitalize our set of all sockets
	FD_ZERO(&all_sockets);

	//adds one socket (the current socket) to the fd set of all sockets
	FD_SET(server_socket,&all_sockets);

	while(1)
	{	

		//notice so far, we have created 2 fd_sets : all_sockets , ready_sockets
		//but we have only used the all_sockets and didn't touch the ready_sockets
		//that is because select() is destructive: it's going to change the set we pass in 
		//so we need a temporary copy; that is what the other fd_set ready_sockets is for
	
		//so that is why each iteration of the loop, we copy the all_sockets set into that temp fd_set
		ready_sockets = all_sockets;


		//now call select()
		//1st argument: range of file descriptors to check  [the highest file descriptor plus one] 
		//The maximum number of sockets supported by select() has an upper limit, represented by FD_SETSIZE (typically 1024).
		//you can use any number of max connections depending on your context/requirements

		//2nd argument: set of file descriptors to check for reading (the ones we want select to keep an eye on)
		//3rd argument: set of file descriptors to check for writing (usually NULL)
		//4th argument: set of file descriptors to check for errors/exceptions (usually NULL)
		//5th argument: optional timeout value specifying the time to wait for select to compplete
		if(select(FD_SETSIZE,&ready_sockets,NULL,NULL,NULL)<0)
		{
			perror("select error");
			exit(EXIT_FAILURE);
		}

		//when select returns, we know that one of our file descriptors has work for us to do
		//but which one??
		//select returns the fd_set containing JUST the file descriptors ready for reading
		//(because select is destructive, so that is why we made the temp fd_set ready_sockets copy because we didn't want to lose the original set of file descriptors that we are watching)
		
		//to know which ones are ready, we have to loop through and check
		//go from 0 to FD_SETSIZE (the largest number of file descriptors that we can store in an fd_set)
		for(int fd = 0 ; fd < FD_SETSIZE; fd++)
		{
			// printf("\nFD: %d / %d", fd, FD_SETSIZE);

			//check to see if that fd is SET
			if(FD_ISSET(fd,&ready_sockets))
			{
				//if it is set, that means that fd has data that we can read right now
				//when this happens, we are interested in TWO CASES
				
				//1st case: the fd is our server socket
				//that means it is telling us there is a NEW CONNECTION that we can accept
				if(fd==server_socket)
				{
					//accept that new connection
					printf("\n %d", server_socket);

					int client_sd = accept(server_socket,0,0);
					printf("\n Client Connected on file descriptor = %d \n",client_sd);
					
					//add the newly accepted socket to the set of all sockets that we are watching
					FD_SET(client_sd,&all_sockets);
					
				}

				//2nd case: when the socket that is ready to read from is one from the all_sockets fd_set
				//in this case, we just want to read its data
				else
				{
					char buffer[MAX_BUFFER];

					bzero(buffer,sizeof(buffer));
					int bytes = recv(fd, buffer,sizeof(buffer), 0);
					printf("\nBUFFER:$%s$\n", buffer);

					char *buffer_cpy = malloc(strlen(buffer) + 1);

					// char buffer_cpy[MAX_BUFFER];
    				strcpy(buffer_cpy, buffer);

					//tokenize buffer to separate command items
					char** commandToken = tokenizer(buffer_cpy);

					


						printf("111");
					if (strcmp(commandToken[0], "STOR") == 0)
					{


						int pid = fork();

						//child process to process STOR command
						if (pid==0){
							printf("111");

							int data_fd = socket(AF_INET, SOCK_STREAM, 0);
							if (data_fd < 0)
							{
								perror("Error opening Socket: Server Data.");
								return 1;
							}

							printf("222");

							//building data socket's Internet Address using the client port and IP received
							struct sockaddr_in client_data_addr,server_data_addr;

							bzero(&client_data_addr,sizeof(client_data_addr));
							client_data_addr.sin_family = AF_INET;

							int client_data_port = 9008;
							client_data_addr.sin_port = htons(client_data_port);
							inet_pton(AF_INET, inet_addr(INADDR_ANY), &(client_data_addr.sin_addr));

							bzero(&server_data_addr,sizeof(server_data_addr));
							server_data_addr.sin_family = AF_INET;	//address family
							server_data_addr.sin_port = htons(9000);	//using port 9000 because port 20 is priveledged
							server_data_addr.sin_addr.s_addr = inet_addr("127.0.0.1");

							printf("333");

							char server_data_ip[INET_ADDRSTRLEN]; 
							inet_ntop(AF_INET, &(client_data_addr.sin_addr), server_data_ip, INET_ADDRSTRLEN);
							printf("Server is connecting to IP: %s and Port: %d\n",INADDR_ANY,htons(client_data_port));
							//connecting to server socket
							int value =1;
							setsockopt(data_fd,SOL_SOCKET,SO_LINGER,&value,sizeof(value));
							if (bind(data_fd, (struct sockaddr*) &server_data_addr, sizeof(struct sockaddr_in)) == 0)
								printf("Server Data binded Correctly\n");
							else
								printf("Server Data unable to bind\n");

							if(connect(data_fd,(struct sockaddr*)&client_data_addr,sizeof(client_data_addr))<0)
							{
								perror("Connection Failed: Server Data.");
								exit(-1);
							}
							char fileName[256];
							strcpy(fileName, commandToken[1]);

							printf("444");

							FILE *newFile;
							int bytesReceived = 0;
                        	char receiveBuff[1024];
							newFile = fopen("new.txt", "w"); 
							printf("\nareeb");
							//error check for opening file
							if (!newFile){
								printf("\nCould not open file..");
							}

							else{
								//receive bytes into buffer and write to file
								printf("555");
								while (bytesReceived = recv(fd, receiveBuff, sizeof(receiveBuff), 0) > 0)
								{
									printf("1");
									fwrite(receiveBuff, 1, bytesReceived, newFile);
								}
								
								//file finished writing
								printf("File writing complete..");
							}
						}

						//parent process continues with other commands
						else{

						}
					}

					else if (strcmp(commandToken[0], "RETR") == 0)
					{
					
					}

					else if (strcmp(commandToken[0], "LIST") == 0)
					{
					
					}
					
					
					// user(bytes[1])
					if(bytes==0)   //client has closed the connection
					{
						printf("Client dieconnected.. \n");
						
						//we are done, close fd
						close(fd);

						//once we are done handling the connection, remove the socket from the list of file descriptors that we are watching
						FD_CLR(fd,&all_sockets);
						
						//if condiitons here
							// { send(fd, buffer)}


					}
					//displaying the message received 

					free(buffer_cpy); 	
					free(commandToken);
				}
			}
		}


	}

	//close
	close(server_socket);
	return 0;
}
