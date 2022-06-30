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
#include <time.h>
#include <errno.h>   

#include "parseinput.h"

#define clear() printf("\033[H\033[J")
#define PORT 9007
#define MAXUSER 100
#define MAX_BUFFER 2048


int msleep(long msec)
{
    struct timespec ts;
    int res;

    if (msec < 0)
    {
        errno = EINVAL;
        return -1;
    }

    ts.tv_sec = msec / 1000;
    ts.tv_nsec = (msec % 1000) * 1000000;

    do {
        res = nanosleep(&ts, &ts);
    } while (res && errno == EINTR);

    return res;
}


int listSize = 0;

struct User
{
	char username[16];
	char password[16];
	int usernameFlag;
	int loginFlag;
	int userFD;

	char path[256];

	//char* path
};

struct User userList[MAXUSER];

struct User assignUser(char* username, char* password)
{
	struct User tempUser;
	char cwd[256];

	//default username and login state is 0
	tempUser.usernameFlag = 0;
	tempUser.loginFlag = 0;
	tempUser.userFD = -1;

	if (getcwd(cwd, sizeof(cwd)) == NULL){
		perror("Error getting working directory..");	
	}
	else{
		strcpy(tempUser.path,cwd);
	}

	strcpy(tempUser.username,username);
	strcpy(tempUser.password,password);

	return tempUser;
}

// void send_msg(char* string, )

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

	printf("\n Loaded authentication file ...");
}

int userAuth(const char* username, int usernumber)
{
	for (int i=0; i<listSize; i++)
	{	
		//username found
		if ((strcmp(username,userList[i].username)==0)&&(userList[i].userFD == -1))
		{
			userList[i].userFD = usernumber;
			userList[i].usernameFlag = 1;
			printf("\n User found..");
			
			return 1;
		}
	}
		printf("\n User not found..");
		return 0;
}

int passAuth(const char* password, int userNum)
{
	if (strcmp(password,userList[userNum].password)==0)
	{
		userList[userNum].loginFlag = 1;
		printf("\n Password correct..");
		return 1;
	}
	else
	{
		printf("\n Incorrect password..");
		return 0;
	}
}

void send_to_client(int fd, char* message)
{
	int size = strlen(message) + 1;
	send(fd, message, size, 0);
}

void printRecords(){
	for(int i = 0; i < listSize; ++i)
	{
		// if (userList[i].userFD == fd){
			printf("\n || User: %s | Pass: %s | USER: %d | PASS: %d | FD: %d||", userList[i].username, userList[i].password, userList[i].usernameFlag, userList[i].loginFlag, userList[i].userFD);
			printf("\n PATH : %s", userList[i].path);
		// }
	}
}

int main()
{	
	clear();
	printf("\n ----------------| FTP Server |----------------");
	printf("\n Waiting for Client to join...");

	//read user.txt file
	getAuth();


	char defaultPath[256];
	if (getcwd(defaultPath, sizeof(defaultPath)) == NULL){
		perror("Error getting working directory..");	
	}
	

	// For loop for debugging authtext file to server
	// for (int i=0; i<listSize; ++i){
	// 	printf("\nLIST:%s,%s", userList[i].username, userList[i].password);
	// }
	
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
					// printf("\n %d", server_socket);

					int client_sd = accept(server_socket,0,0);
					printf("\n Client Connected on file descriptor %d ...\n",client_sd);
					
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
					printf("\n SOCKET %d : %s", fd, buffer);
					
					int user_flag = 0, login_flag=0, recordNum = -1;
					
					for (int i=0; i<listSize; i++){

						if (userList[i].userFD == fd)
						{
							// username linked to client 
							recordNum = i;							
							userList[recordNum].usernameFlag = 1;
							user_flag = 1;


							if (userList[recordNum].loginFlag == 1)
							{
								login_flag = 1;
							}
						} 
					}


					if (bytes!=0)
					{
					
					chdir(userList[recordNum].path);
					char *buffer_cpy = malloc(strlen(buffer) + 1);
					
					// char buffer_cpy[MAX_BUFFER];
    				strcpy(buffer_cpy, buffer);

					//tokenize buffer to separate command items
					char** commandToken = tokenizer(buffer_cpy);

					for (int i=0; i<listSize; i++){

						if (userList[i].userFD == fd)
						{
							// printf("#");
							// username linked to client 
							recordNum = i;							
							userList[recordNum].usernameFlag = 1;
							user_flag = 1;


							if (userList[recordNum].loginFlag == 1)
							{
								login_flag = 1;
							}
						} 
					}
					
					//first check if the user is logged in 
					if (login_flag == 0)
					{
						if(user_flag == 0)
						{
							if (strcmp(commandToken[0], "USER") == 0)
							{
								int user_status = userAuth(commandToken[1], fd);
								
								if (user_status == 1)
								{
									user_flag = 1;
									send_to_client(fd, "331 Username OK, need passsword.");
								}
								else if (user_status == 0)
								{
									send_to_client(fd, "530 Not logged in.");
								}
								// continue;
							}
							else{
								send_to_client(fd, "530 Not logged in.");
								// continue;
							}
						}
						else
						{
							if (strcmp(commandToken[0], "PASS") == 0)
							{
								//first check if username auth is passsed and pass in user number
								int pass_status = passAuth(commandToken[1], recordNum);	

								if (pass_status == 1)
								{
									login_flag = 1;
									send_to_client(fd, "230 User logged in, proceed.");
								}
								else if (login_flag == 0)
								{
									char* send_msg = "530 Not logged in.";
									printf("%s \n", send_msg);
									send(fd, send_msg, 38, 0);
								}
								// continue;
							}
							else{
								send_to_client(fd, "530 Not logged in.");
							}
						}
					}
					else
					{
						if (strcmp(commandToken[0], "CWD") == 0)
						{

							if (chdir(commandToken[1])==-1)
							{
								perror("Error getting working directory..");
								send_to_client(fd, " Error changing server directory");
							}
							else
							{
								char tempPath[256];
								if (getcwd(tempPath, sizeof(tempPath)) == NULL){
									perror("Error changing working directory..");
								}
								else{
									strcpy(userList[recordNum].path, tempPath);
        							printf(" Server changed directory ...");
									send_to_client(fd, "200 directory changed to ");

								}

							}
						}
						else if (strcmp(commandToken[0], "PWD") == 0)
						{
							send_to_client(fd, userList[recordNum].path);
						}
						else if (strcmp(commandToken[0], "STOR") == 0)
						{
							int ftp_port;
							char ftp_port_str[16];
							bzero(ftp_port_str, 16);  
							recv(fd, ftp_port_str, sizeof(ftp_port_str), 0);

							ftp_port = atoi(ftp_port_str);

							pid_t child = fork();

							if (child == 0)
							{
							
							int FTP_server_socket;
							FTP_server_socket = socket(AF_INET , SOCK_STREAM, 0);

							//check for fail error
							if (FTP_server_socket == -1) {
								printf("socket creation failed..\n");
								exit(EXIT_FAILURE);
							}

							//setsock
							int value  = 1;
							setsockopt(FTP_server_socket,SOL_SOCKET,SO_REUSEADDR,&value,sizeof(value)); //&(int){1},sizeof(int)
							
							struct sockaddr_in FTP_server_address;
							bzero(&FTP_server_address,sizeof(FTP_server_address));
							FTP_server_address.sin_family = AF_INET;
							FTP_server_address.sin_port = htons(ftp_port);
							FTP_server_address.sin_addr.s_addr = INADDR_ANY;

							msleep(250);
							//connect
							if(connect(FTP_server_socket,(struct sockaddr*)&FTP_server_address,sizeof(server_address))<0)
							{
								perror("connect");
								exit(EXIT_FAILURE);
							}

							char filename[256];
							strcpy(filename, commandToken[1]);
							int bytesRead = 0;
							char readBuffer[1024];

							FILE* file = fopen(filename, "wb");

							//error check for opening file
							if(!file){
								printf("\nCould not open file..");
							}
							else
							{
								while ((bytesRead=read(FTP_server_socket, readBuffer, 1024)) > 0)
								{
									fflush(stdout);
									fwrite(readBuffer, 1, bytesRead, file);
								}

								// send_to_client(fd, "226 Transfer completed.");
							}

							printf("\nFile creation complete.");
							send_to_client(fd, " 226 Transfer completed ...");

							fclose(file);
							close(FTP_server_socket);
							fflush(stdout);
							exit(1);


							// char FTP_bufferc[128];
							// bzero(FTP_bufferc, 128);                        // Clearing the buffer back to the buffer size
							// recv(FTP_server_socket, FTP_bufferc, sizeof(FTP_bufferc), 0); // Client receiving the buffer output from the server
							// printf(" %s\n", FTP_bufferc);                              // print the buffer from the server on the client screen
							// FTP_bufferc[0] = '\0';

								// send_to_client(FTP_server_socket, "Working");

							}
							// send_to_client(fd, "still working on this");
						}
						else if (strcmp(commandToken[0], "RETR") == 0)
						{

							int ftp_port;
							char ftp_port_str[16];
							bzero(ftp_port_str, 16);  
							recv(fd, ftp_port_str, 16, 0);

							ftp_port = atoi(ftp_port_str);
							printf("\n $$%d", ftp_port);
							
							
								pid_t child = fork();
								if (child == 0)
								{
								
									int FTP_server_socket;
									FTP_server_socket = socket(AF_INET , SOCK_STREAM, 0);

									//check for fail error
									if (FTP_server_socket == -1) {
										printf("socket creation failed..\n");
										exit(EXIT_FAILURE);
									}

									//setsock
									int value  = 1;
									setsockopt(FTP_server_socket,SOL_SOCKET,SO_REUSEADDR,&value,sizeof(value)); //&(int){1},sizeof(int)
									
									struct sockaddr_in FTP_server_address;
									bzero(&FTP_server_address,sizeof(FTP_server_address));
									FTP_server_address.sin_family = AF_INET;
									FTP_server_address.sin_port = htons(ftp_port);
									FTP_server_address.sin_addr.s_addr = INADDR_ANY;

									msleep(250);

									//connect
									if(connect(FTP_server_socket,(struct sockaddr*)&FTP_server_address,sizeof(server_address))<0)
									{
										perror("connect");
										exit(EXIT_FAILURE);
									}

									FILE* file = fopen(commandToken[1], "rb");
									//check if file exists
									if(!file){
										printf("File does not exist..");
										char* response = "550 No such file or directory.";
										send_to_client(fd, response);
									} //file found, begin retreival
									else
									{
									
									while(1)
									{
										unsigned char buff[1024]={0};
										int bytesRead = fread(buff, 1, 1024, file);
										
										if(bytesRead>0)
										{
											// fflush(stdout);
											write(FTP_server_socket, buff, bytesRead);
										}
										
										if (bytesRead < 1024)
										{
											break;
										}
									}
									close(FTP_server_socket);
									fclose(file);		
									
									}		// bzero(FTP_bufferc, 128);                        // Clearing the buffer back to the buffer size

									send_to_client(fd, " 226 Transfer Completed ...");
									exit(1);
								}


						}
						else if (strcmp(commandToken[0], "LIST") == 0)
						{
							int ftp_port;
							char ftp_port_str[16];
							bzero(ftp_port_str, 16);  
							recv(fd, ftp_port_str, 16, 0);

							ftp_port = atoi(ftp_port_str);
							// printf("\n $$%d", ftp_port);
							
							
								pid_t child = fork();
								if (child == 0)
								{
								
									int FTP_server_socket;
									FTP_server_socket = socket(AF_INET , SOCK_STREAM, 0);

									//check for fail error
									if (FTP_server_socket == -1) {
										printf("socket creation failed..\n");
										exit(EXIT_FAILURE);
									}

									//setsock
									int value  = 1;
									setsockopt(FTP_server_socket,SOL_SOCKET,SO_REUSEADDR,&value,sizeof(value)); //&(int){1},sizeof(int)
									
									struct sockaddr_in FTP_server_address;
									bzero(&FTP_server_address,sizeof(FTP_server_address));
									FTP_server_address.sin_family = AF_INET;
									FTP_server_address.sin_port = htons(ftp_port);
									FTP_server_address.sin_addr.s_addr = INADDR_ANY;

									msleep(250);
									//connect
									if(connect(FTP_server_socket,(struct sockaddr*)&FTP_server_address,sizeof(server_address))<0)
									{
										perror("connect");
										exit(EXIT_FAILURE);
									}

									char file_line[256];
									//creating a pipe to send back the list of files in the server directory to the client
									FILE *list_file = popen("ls", "r");
									if (list_file)
									{
										while (fgets(file_line, sizeof(file_line), list_file))
										{
											if (send(FTP_server_socket,file_line,strlen(file_line),0) < 0)
											{
												perror("Error Sending file..\n");
												return 0;
											}
											bzero(&file_line,sizeof(file_line));
											fflush(stdout);
											
										}
										pclose(list_file);
									}
									send_to_client(fd,"226 Transfer completed...");
									

									/*
									FILE *file;
									FILE* tmpLS = fopen("tmpfile", "w");

									char *command = "ls";
									char c = 0;
									char* lsBuff = "";

									if (0 == (file = popen(command, "r")))
									{
										perror("Error executing LIST..");
									}

									while (fread(&c, sizeof(c), 1, file))
									{
										fwrite(c, 1, 1, tmpLS);
									}

									send_to_client(FTP_server_socket, "hiiiiii");
									*/ 

									// pclose(file);
									close(FTP_server_socket);
									exit(1);
								}
						}
						else
						{
							send_to_client(fd, "202 Command not implemented.");
						}
					}

					free(buffer_cpy); 	
					free(commandToken);

					// printRecords();
					
					}

					else   //client has closed the connection
					{
						printf("Client on sock %d disconnected... \n", fd);
						
						userList[recordNum].loginFlag = 0;
						userList[recordNum].usernameFlag = 0;
						userList[recordNum].userFD = -1;
						strcpy(userList[recordNum].path,defaultPath);

						//we are done, close fd
						close(fd);

						//once we are done handling the connection, remove the socket from the list of file descriptors that we are watching
						FD_CLR(fd,&all_sockets);
						
						//if condiitons here
							// { send(fd, buffer)}

					}
					
					// user(bytes[1])
					

					

				}
			}
		}


	}

	//close
	close(server_socket);
	return 0;
}
