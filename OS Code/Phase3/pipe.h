#ifndef PIPE_H
#define PIPE_H

void singlepipeExecute(char **firstcommand, char **secondcommand, int socket);
void doublepipeExecute(char **firstcommand, char **secondcommand,char **thirdcommand,int socket );
void triplepipeExecute(char **firstcommand, char **secondcommand,char **thirdcommand,char **fourcommand, int socket);

#endif
