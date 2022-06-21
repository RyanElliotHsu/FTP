// OS Phase 3 Remote Shell
// Dev and Soumen

#include <unistd.h>     // / header for unix specic functions declarations : fork(), getpid(), getppid(), sleep()
#include <stdio.h>      // header for input and output from console : printf, perror
#include <sys/socket.h> // header for socket specific functions and macros declarations
#include <stdlib.h>     // header for general functions declarations: exit() and also has exit macros such as EXIT_FAILURE - unsuccessful execution of a program
#include <netinet/in.h> //header for MACROS and structures related to addresses "sockaddr_in", INADDR_ANY
#include <string.h>     // header for string functions declarations: strlen()
#include <arpa/inet.h>  // header for functions related to addresses from text to binary form, inet_pton
#include <signal.h>     // header for signal related functions and macros declarations

#include "print.h"
#include "read.h"
#include "splitter.h"
#include "checkpipe.h"
#include "singlecommand.h"
#include "pipe.h"
#include "server.h"

#define clear() printf("\033[H\033[J") // Clearing the screen
#define BUFFER_SIZE 4096               // Buffer size
#define PORT 5455                      // client server port number

// Global Variable for accessing socket in terminating handler and using it to send exit message to server
int sock = 0;

void clientExitHandler(int sig_num)
{
  char *exitString = "exit";
  // printf("\nExiting shell successfully \n");
  send(sock, exitString, strlen(exitString) + 1, 0); // sending exit message to server
  close(sock);                           // close the socket/end the conection
  printf("\n Exiting client.  \n");
  fflush(stdout); // force to flush any data in buffers to the file descriptor of standard output,, a pretty convinent function
  exit(0);
}

int main()
{
  signal(SIGINT, clientExitHandler);
  int commandFlag = 0, pipeFlag = -1, finish = 0;                       // Flags to control the pipe and command input functions
  char *command;                                                        // command is the string input read from the user
  char **argument1, **argument2, **argument3, **argument4, **arguments; // to send the parsed arguments to the pipe
  char **pipe_cmd;
  char bufferc[BUFFER_SIZE];
  clear();

  int valread;
  struct sockaddr_in serv_addr; // structure for storing addres; Local host server IP and port number to be connected to

  // Creating socket file descriptor with communication: domain of internet protocol version 4, type of SOCK_STREAM for reliable/conneciton oriented communication, protocol of internet
  if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) // checking if socket creation fail
  {
    printf("\n Socket creation error \n");
    return -1;
  }

  // address details to connect socket to server
  serv_addr.sin_family = AF_INET;
  serv_addr.sin_port = htons(PORT);

  if (inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0) // Connecting to the server on the localhost IP
  {
    printf("\nInvalid address/ Address not supported \n");
    return -1;
  }

  // connect the socket with the adddress and establish connnection with the server
  if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
  {
    printf("\nConnection Failed \n");
    return -1;
  }
  printf("\t\t\t Welcome to D&S Remote Shell: Client\n");
  printf("\t ---------------------------------------------\n");
  printf("\t Enter your linux shell command to be sent to the server\n");
  printf("\t Enter <help> for the list of commands\n");
  printf("\t Enter <exit> to leave the shell\n\n");

  printf("\tSuccessfully connected to Remote shell server \n");

  while (1)
  {
    bzero(bufferc, sizeof(bufferc));                    // Clearing buffer
    recv(sock, bufferc, sizeof(bufferc), MSG_DONTWAIT); // Receives remaining buffer from server without blocking client (and using the flag to prevent waiting for a message)

    if (strlen(bufferc) != 0)
    {
      printf("%s", bufferc); // Print the string only when the buffer is not empty
      continue;
    }

    UserPrompt();
    command = readInput();  // Getting user input and saving it in command
    if (command[0] == '\0') // If the command is empty meaning the user has pressed enter then we continue and loop back
    {
      continue;
    }
    char *exitString = "exit"; // when the user press exit
    if (strcmp(command, exitString) == 0)
    { // comapring the command entered to the exit string
      printf("Exiting shell successfully  \n");
      send(sock, command, strlen(command) + 1, 0); // send the exiting message and close the socket
      close(sock);
      fflush(stdout); // force to flush any data in buffers to the file descriptor of standard output,, a pretty convinent function
      exit(0);
      // exit(EXIT_SUCCESS);
    }

    // *exitString = "./client"; // when the user press exit
    if (strcmp(command, "./client") == 0)
    { // comapring the command entered to the exit string
      printf("Cannot open client in client!  \n");
      continue;
    }
    if (strcmp(command, "./server") == 0)
    { // comapring the command entered to the exit string
      printf("Cannot open server in client!  \n");
      continue;
    }

    char *helpString = "help"; // If the user enters help then repend the help menu
    if (strcmp(command, helpString) == 0)
    {                                                                       // comapring the command entered to the help string
      printf("\tHere is the list of few shell commands you can enter: \n"); // printing the help menu
      printf("\tls: Command to display the files in a directory\n");
      printf("\ttouch: Command to create a file \n");
      printf("\tmkdir: Command to create a folder \n");
      printf("\tpwd: Command to print the current directory \n");
      printf("\trm: Command to remove objects \n");
      printf("\tcp: Command to copy  \n");
      printf("\trmdir: Command to remove a directory \n");
      printf("\tgrep: Command to search for a string of characters in a file \n");
      printf("\twc: Command to find the word count \n");
      printf("\tfind: Command that locates an object \n");
      printf("\tmv: Command that moves an object \n");
      printf("\tcat: Command that outputs the contents of a file \n");
      printf("\tps: Command that displays the running processes \n");
      printf("\tdf: Command to display the display the available disk space \n");
      printf("\twhoami: Command to print the name of the current user \n");
      printf("\t ------Pipes------\n");
      printf("\tOur shell allows maximum of 3 pipes\n");
      printf("\tSingle Pipe: eg. ls | grep c\n");
      printf("\tDouble Psipe: eg. ls -l | grep d | wc -c\n");
      printf("\tTripple Pipe: eg. cat xyz.txt | grep r | tee abc.txt | wc -l \n");
      continue;
    }

    send(sock, command, strlen(command) + 1, 0); // send the command to the server
    command[0] = '\0';                           // Setting the command string to empty

    sleep(1);                                           // Allow server time to execute commands promptly
    bzero(bufferc, BUFFER_SIZE);                        // Clearing the buffer back to the buffer size
    recv(sock, bufferc, sizeof(bufferc), MSG_DONTWAIT); // Client receiving the buffer output from the server
    printf("%s", bufferc);                              // print the buffer from the server on the client screen
    bufferc[0] = '\0';                                  // Setting the command string to empty
  }

  close(sock); // Closing the socket if slient is exiting
  exit(0);
}
