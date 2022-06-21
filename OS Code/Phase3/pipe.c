#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <string.h>
#include <sys/wait.h>

void singlepipeExecute(char **firstcommand, char **secondcommand, int socket)
{
    int fd1[2]; // pipe 1 for getting output of ls from child 1 and giving it to child 2 also
    int pid, pid2;
    int status;

    if (pipe(fd1) < 0)
        exit(EXIT_FAILURE);
    pid = fork();
    if (pid < 0)
        exit(EXIT_FAILURE);

    else if (pid == 0)
    {
        dup2(fd1[1], 1); // write by redirecting standard output to pipe 1
        close(fd1[1]); // Write end
        close(fd1[0]); // read end
        execvp(firstcommand[0], firstcommand);
        perror("Execvp failed while executing grep");
        exit(EXIT_FAILURE);
    }
    else
    {
        dup2(fd1[0], 0);
        close(fd1[0]);
        close(fd1[1]);

        pid2 = fork();
        if (pid2 == 0)
        {
            dup2(socket, STDOUT_FILENO); // Redirecting the standard output straight to the server
            dup2(socket, STDERR_FILENO); // Redirecting the standard input straight to the server
            close(fd1[0]);
            close(fd1[1]);
            execvp(secondcommand[0], secondcommand);
            perror("Execvp failed while executing grep");
        }
    }
}

void doublepipeExecute(char **firstcommand, char **secondcommand, char **thirdcommand, int socket)
{
    int fd1[2]; // pipe 1 for getting output from child 1 and giving it to child 2
    int fd2[2]; // pipe 2 for getting output from child 1 and giving it to parent
    int pid;
    if (pipe(fd1) < 0)
        exit(EXIT_FAILURE);
    if (pipe(fd2) < 0)
        exit(EXIT_FAILURE);

    pid = fork(); // Forking the child
    if (pid < 0)
        exit(EXIT_FAILURE);
    else if (pid == 0) // Child area
    {
        dup2(fd1[1], 1);                       // write by redirecting standard output to pipe 1
        close(fd1[1]);                         // Closing the writing
        close(fd1[0]);                         // Closing the reading
        close(fd2[0]);                         // Closing the reading
        close(fd2[1]);                         // Closing the writing
        execvp(firstcommand[0], firstcommand); // Exec vp using the 1st arguments passed as an array
        perror("Execvp failed");
        exit(EXIT_FAILURE);
    }
    else
    {
        pid = fork();
        if (pid == 0) // child area
        {
            dup2(fd1[0], 0); // reading redirected ouput of ls through pipe 1
            dup2(fd2[1], 1);
            // dup2(fd2[1], 1); // write by redirecting standard output to pipe 2
            close(fd1[1]);                           // Closing the writing
            close(fd1[0]);                           // Closing the reading
            close(fd2[1]);                           // Closing the writing
            close(fd2[0]);                           // Closing the reading
            execvp(secondcommand[0], secondcommand); // Exec vp using the 2nd arguments passed as an array
            perror("Execvp failed");
            exit(EXIT_FAILURE);
        }
        else // Parent area
        {
            dup2(fd2[0], 0);
            close(fd1[1]); // Closing the writing
            close(fd1[0]); // Closing the reading
            close(fd2[1]); // Closing the writing
            close(fd2[0]); // Closing the reading

            pid_t cpid;
            cpid = fork();
            if (cpid < 0)
            {
                exit(EXIT_FAILURE);
            }
            else
            {
                if (cpid)
                {

                    waitpid(cpid, NULL, 0);
                }
                else
                {
                    dup2(socket, STDOUT_FILENO); // Redirecting the standard output straight to the server
                    dup2(socket, STDERR_FILENO); // Redirecting the standard input straight to the server
                    close(socket);
                    execvp(thirdcommand[0], thirdcommand); // Exec vp using the 3rd arguments passed as an array
                    perror("Execvp failed");
                    exit(EXIT_FAILURE);
                }
            }
        }
    }
}

void triplepipeExecute(char **firstcommand, char **secondcommand, char **thirdcommand, char **fourcommand, int socket)
{
    int fd1[2]; // pipe 1 for getting output from child 1 and giving it to child 2
    int fd2[2]; // pipe 2 for getting output from child 2 and giving it to child 3
    int fd3[2]; // pipe 3 for getting output from child 3 and giving it to parent

    int pid;
    if (pipe(fd1) < 0)
        exit(EXIT_FAILURE);
    if (pipe(fd2) < 0)
        exit(EXIT_FAILURE);
    if (pipe(fd3) < 0)
        exit(EXIT_FAILURE);

    pid = fork();

    if (pid < 0)
        exit(EXIT_FAILURE);
    else if (pid == 0)
    {
        dup2(fd1[1], 1);                       // write by redirecting standard output to pipe 1
        close(fd1[1]);                         // Closing the writing
        close(fd1[0]);                         // Closing the reading
        close(fd2[1]);                         // Closing the writing
        close(fd2[0]);                         // Closing the reading
        close(fd3[1]);                         // Closing the writing
        close(fd3[0]);                         // Closing the reading
        execvp(firstcommand[0], firstcommand); // Exec vp using the 1st arguments passed as an array
        perror("Execvp failed");
        exit(EXIT_FAILURE);
    }
    else
    {

        int pid2 = fork();
        if (pid2 < 0)
            exit(EXIT_FAILURE);
        if (pid2 == 0)
        {
            dup2(fd1[0], 0);                         // reading redirected ouput of ls through pipe 1
            dup2(fd2[1], 1);                         // write by redirecting standard output to pipe 2
            close(fd1[1]);                           // Closing the writing
            close(fd1[0]);                           // Closing the reading
            close(fd2[1]);                           // Closing the writing
            close(fd2[0]);                           // Closing the reading
            close(fd3[1]);                           // Closing the writing
            close(fd3[0]);                           // Closing the reading
            execvp(secondcommand[0], secondcommand); // Exec vp using the 2nd arguments passed as an array
            perror("Execvp failed");
            exit(EXIT_FAILURE);
        }
        else
        {
            int pid3 = fork();
            if (pid3 < 0)
                exit(EXIT_FAILURE);
            if (pid3 == 0)
            {
                // waitpid(pid3, NULL, 0);   
                dup2(fd2[0], 0);                       // reading redirected ouput of ls through pipe 2
                dup2(fd3[1], 1);                       // write by redirecting standard output to pipe 3
                close(fd1[1]);                         // Closing the writing
                close(fd1[0]);                         // Closing the reading
                close(fd2[1]);                         // Closing the writing
                close(fd2[0]);                         // Closing the reading
                close(fd3[1]);                         // Closing the writing
                close(fd3[0]);                         // Closing the reading
                execvp(thirdcommand[0], thirdcommand); // Exec vp using the 3rd arguments passed as an array
                perror("Execvp failed");
                exit(EXIT_FAILURE);
            }
            else
            {   
                dup2(fd3[0], 0);
                close(fd1[1]);                         // Closing the writing
                close(fd1[0]);                         // Closing the reading
                close(fd2[1]);                         // Closing the writing
                close(fd2[0]);                         // Closing the reading
                close(fd3[1]);                         // Closing the writing
                close(fd3[0]);

                int pid4 = fork();
                if (pid4 < 0){
                    exit(EXIT_FAILURE);
                }
                else{
                    if (pid4){
                        waitpid(pid4, NULL, 0);
                    }
                    else{
                        dup2(socket, STDOUT_FILENO); // Redirecting the standard output straight to the server
                        dup2(socket, STDERR_FILENO); // Redirecting the standard input straight to the server
                        close(socket);
                        execvp(fourcommand[0], fourcommand); // Exec vp using the 4th arguments passed as an array
                        perror("Execvp failed while executing grep");
                        exit(EXIT_FAILURE); 
                    }

                }

            }
        }
    }
}

