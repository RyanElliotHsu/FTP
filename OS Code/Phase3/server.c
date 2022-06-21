// OS Phase 3 Remote Server
// Dev and Soumen

#include <unistd.h>     // / header for unix specic functions declarations : fork(), getpid(), getppid(), sleep()
#include <stdio.h>      // header for input and output from console : printf, perror
#include <stdlib.h>     // header for general functions declarations: exit() and also has exit macros such as EXIT_FAILURE - unsuccessful execution of a program
#include <sys/socket.h> // header for socket specific functions and macros declarations
#include <netinet/in.h> //header for MACROS and structures related to addresses "sockaddr_in", INADDR_ANY
#include <string.h>     // header for string functions declarations: strlen()
#include <sys/types.h>
#include <sys/wait.h>
#include <poll.h>
#include <pthread.h> // header for thread functions declarations: pthread_create, pthread_join, pthread_exit
#include <signal.h>  // header for signal related functions and macros declarations

// Including the header files
#include "print.h"
#include "read.h"
#include "splitter.h"
#include "checkpipe.h"
#include "singlecommand.h"
#include "pipe.h"
#include "client.h"

#define clear() printf("\033[H\033[J") // Clearing the screen
#define PORT 5455                      // client server port number
#define BUFFER_SIZE 4096               // We made the buffer big at first to allow much as ouput possible (for long strings and command)
#define NUM_CLIENTS 10

void serverExitHandler(int sig_num)
{
  char *exitString = "srvexit";
  printf("\n Exiting server.  \n");
  // send(socket, exitString, strlen(exitSt ring) + 1, 0); // sending exit message to server
  // close(sock1);
  fflush(stdout); // force to flush any data in buffers to the file descriptor of standard output,, a pretty convinent function
  exit(0);
}

// Function that handle dedicated communication with a client
void *HandleClient(void *new_socket)
{
  pthread_detach(pthread_self()); // detach the thread as we don't need to synchronize/join with the other client threads, their execution/code flow does not depend on our termination/completion
  int socket = *(int *)new_socket;
  free(new_socket);
  printf("Handling new client in a thread using socket: %d\n", socket);
  printf("Listening to client..\n"); // while printing make sure to end your strings with \n or \0 to flush the stream, other wise if in anyother concurent process is reading from socket/pipe-end with standard input/output redirection, it will keep on waiting for stream to end.

  int i = 0; // to keep track of messages received
  // variable such as message buffers to receive and send messages
  char message[1024] = {0};  // to send message
  char message2[1024] = {0}; // to receive message
  int data_len;              // for flusing the received message

  int connectFlag = 0;                //
  int commandFlag = 0, pipeFlag = -1; // Flags to control the pipe and command input functions
  // char *command[BUFFER_SIZE] = {0};                                                        // command is the string input read from the user
  char **argument1, **argument2, **argument3, **argument4, **arguments; // to send the parsed arguments to the pipe
  char **pipe_cmd;                                                      // getting the pipe Command
  char buffer[BUFFER_SIZE];
  // clear(); // Clearing the screen

  while (1)
  {
    // bzero(command, BUFFER_SIZE);
    char command[BUFFER_SIZE];
    bzero(command, sizeof(command));

    recv(socket, command, sizeof(command), 0); // recevie hello message from client

    char *exitString = "exit";
    if (strcmp(command, exitString) == 0)
    { // comapring the command entered to the exit string
      printf(">Client on socket %d has disconnected.\n", socket);
      connectFlag = 0;
      close(socket);
      pthread_exit(NULL); // terminate the thread
    }
    // printf("Handling new client in a thread using socket: %d\n", socket);

    if (strcmp(command, "\0") == 0)
    {
      continue;
    }

    printf(">Server Received using socket %d: %s\n", socket, command); // print the received message

    pipeFlag = checkPipe(command); // Checks the number of pipes in the input and returns an Integer

    if (pipeFlag == 0)
    { // If there are no pipes

      arguments = lineSplitter(command); // get the arguments array from the line spliter (parsing)
      commandExecute(arguments, socket); // execute the single command
      free(arguments);                   // free the arugments after using since the array was dynamically allocated
    }
    else if (pipeFlag > 0) // if there is a pipe
    {
      if (pipeFlag > 3)
      {
        printf(">Sorry, You have entered more than 3 pipes\n");
        break;
      }
      int pipeflag = 0;

      pipe_cmd = pipeSplitter(command, pipeflag); // use the pipe splitter to get the array of commands parsed by the pipe delimiter

      if (pipeFlag == 1)
      { // if there is one pipe
        // Basically we are getting the whole string split from the pipe and then splitting that further example "ls -l" -> ["ls","-l"]
        argument1 = lineSplitter(pipe_cmd[0]); // getting the first argument from the pipe slitter and splitting that further into single arguments
        argument2 = lineSplitter(pipe_cmd[1]);

        if (argument1[0] != NULL && argument2[0] != NULL && pipe_cmd[0] != NULL && pipe_cmd[1] != NULL) // Checking is  the arguments aren't NULL (for no arguments | or || or |||)
        {
          singlepipeExecute(argument1, argument2, socket); // send the 2 arguments into the single pipe function
        }
        else
        {
          char pipeerror[] = (">Null Argument was detected. Please try again\n");
          send(socket, pipeerror, strlen(pipeerror) + 1, 0); // send error message to client
          continue;
        }
        // printf("Executed single pipe \n");
        free(argument1); // free the arugments after using since they were dynamically allocated
        free(argument2);
      }

      if (pipeFlag == 2)
      { // if there is two pipe  and the input is not null
        argument1 = lineSplitter(pipe_cmd[0]);
        argument2 = lineSplitter(pipe_cmd[1]);
        argument3 = lineSplitter(pipe_cmd[2]);

        if (argument1[0] != NULL && argument2[0] != NULL && argument3[0] != NULL && pipe_cmd[0] != NULL && pipe_cmd[1] != NULL && pipe_cmd[2] != NULL) // Checking is  the arguments aren't NULL (for no arguments | or || or |||)
        {
          // singlepipeExecute(argument1, argument2, socket); // send the 2 arguments into the single pipe function
          doublepipeExecute(argument1, argument2, argument3, socket); // send the 3 arguments into the double pipe function
        }
        else
        {
          char pipeerror[] = (">Null Argument was detected. Please try again\n");
          send(socket, pipeerror, strlen(pipeerror) + 1, 0); // send error message to client
          continue;
        }

        free(argument1); // free the arugments after using since they were dynamically allocated
        free(argument2);
        free(argument3);
      }

      if (pipeFlag == 3)
      { // if there is three pipe and the input is not null
        argument1 = lineSplitter(pipe_cmd[0]);
        argument2 = lineSplitter(pipe_cmd[1]);
        argument3 = lineSplitter(pipe_cmd[2]);
        argument4 = lineSplitter(pipe_cmd[3]);
        if (argument1[0] != NULL && argument2[0] != NULL && argument3[0] != NULL && argument4[0] != NULL && pipe_cmd[0] != NULL && pipe_cmd[1] != NULL && pipe_cmd[2] != NULL && pipe_cmd[3] != NULL) // Checking is  the arguments aren't NULL (for no arguments | or || or |||)
        {
          triplepipeExecute(argument1, argument2, argument3, argument4, socket); // send the 4 arguments into the triple pipe function
          continue;
        }
        else
        {
          char pipeerror[] = (">Null Argument was detected. Please try again\n");
          send(socket, pipeerror, strlen(pipeerror) + 1, 0); // send error message to client
          continue;
        }

        free(argument1); // free the arugments after using since they were dynamically allocated
        free(argument2);
        free(argument3);
        free(argument4);
      }
    }
  }

  close(socket);
  pthread_exit(NULL); // terminate the thread
}


int main() // main function
{
  int connectFlag = 0;                //
  int commandFlag = 0, pipeFlag = -1; // Flags to control the pipe and command input functions
  // char *command[BUFFER_SIZE] = {0};                                                        // command is the string input read from the user
  char **argument1, **argument2, **argument3, **argument4, **arguments; // to send the parsed arguments to the pipe
  char **pipe_cmd;                                                      // getting the pipe Command
  char buffer[BUFFER_SIZE];
  clear(); // Clearing the screen

  printf("\t\t Welcome to D&S Remote Shell: Server\n");
  printf("\t --------------------------------------------------------\n");
  signal(SIGINT, serverExitHandler);

  int sock1, sock2, valread;
  struct sockaddr_in address;    // structure for storing addres; local interface and port
  int addrlen = sizeof(address); // Getting the size off the address

  // Creating the file descripter for the socket to allow communication with the domain of Internet protocol and the socket stream for reliable connection
  if ((sock1 = socket(AF_INET, SOCK_STREAM, 0)) == 0) // checking if socket creation fail
  {
    perror(">Socket creation failed");
    exit(EXIT_FAILURE);
  }

  // Setting and defining the address for the socket to bind on
  address.sin_family = AF_INET;
  address.sin_addr.s_addr = htonl(INADDR_ANY);
  address.sin_port = htons(PORT);

  // Socket is being attached to the address on the specified adresss  (local IP and port)
  if (bind(sock1, (struct sockaddr *)&address, sizeof(address)) < 0)
  {
    perror(">Binding failed");
    exit(EXIT_FAILURE);
  }

  if (listen(sock1, NUM_CLIENTS) < 0) // defining for number of clients in queue
  {
    perror("Listen Failed");
    exit(EXIT_FAILURE);
  }

  while (1) // to keep server alive forever
  {

    printf(">Waiting for Client to connect\n"); // Server is listening and waiting for connection
    // accepting the client's connection with the creation of the new socket and that's establishes a connection between a client and server
    if ((sock2 = accept(sock1, (struct sockaddr *)&address,
                        (socklen_t *)&addrlen)) < 0)
    {
      perror("accept");
      exit(EXIT_FAILURE);
    }
    // connectFlag = 1; // Setting connection flag to be true
    printf(">Successfully Connected: Listening to client.\n");

    int rc;                                       // this int serves at return value from pthread_create to check if new thread is successfully created
    pthread_t thread_id;                          // the thread ID which is used to idenetify new thread
    int *new_socket = (int *)malloc(sizeof(int)); // Dynamically allocated memory for the new socket 
    if (new_socket == NULL)
    {
      fprintf(stderr, "Couldn't allocate memory for thread new socket argument.\n");
      free(new_socket); // Freeing the dynamically allocated socket number
      exit(EXIT_FAILURE);
      
    }
    *new_socket = sock2; 

    printf("New Client has Connected!\n");

    // create a new thread that will handle the communication with the newly accepted client
    rc = pthread_create(&thread_id, NULL, HandleClient, new_socket);
    if (rc) // if rc is > 0 imply could not create new thread
    {
      printf("\n ERROR: return code from pthread_create is %d \n", rc);
      free(new_socket); // Freeing the dynamically allocated socket number
      exit(EXIT_FAILURE);
    }
  }

  close(sock1);
  pthread_exit(NULL); // terminate the main thread
}
