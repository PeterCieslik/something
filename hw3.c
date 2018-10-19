#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <fcntl.h>

#define arrSz 500

void sigint_handler(int sig)
{
    char msg[] = "caught sigint\nCS361 >";
    write(1, msg, sizeof(msg));
}
void sigtstp_handler(int sig)
{
    char msg[] = "caught sigstp\nCS361 >";
    write(1, msg, sizeof(msg));
}
int main()
{
    int count, pos, status = 0, stdi = dup(0), stdo = dup(1), runs = 1, semicolon = 0, redir1 = 0, redir2 = 0;
    char *arg, *filename = NULL, *redir = NULL, *arr = (char *)malloc(arrSz * sizeof(char)),
               **args = malloc(arrSz * sizeof(char *)), **args2 = malloc(arrSz * sizeof(char *));

    signal(SIGINT, sigint_handler);
    signal(SIGTSTP, sigtstp_handler);
    do
    {
        printf("CS361 > ");
        arr[0] = 0;
        for (int i = 0; i < arrSz; i++)
        {
            args[i] = NULL;
            args2[i] = NULL;
        }
        arg = strtok(fgets(arr, arrSz, stdin), " \n");
        if (strcmp(arg, "exit") == 0)
            return 0;
        dup2(stdo, 1);
        dup2(stdi, 0);
        count = 0;
        while (arg != NULL)
        {
            if (strcmp(arg, "<") == 0 || strcmp(arg, ">") == 0)
            {
                redir = arg;
                arg = strtok(NULL, " \n");
                filename = arg;
                arg = NULL;
            }
            else if (strcmp(arg, ";") == 0)
            {
                redir = arg;
                count = 0;
                arg = strtok(NULL, " \n");
                while (arg != NULL)
                {
                    args2[count++] = arg;
                    arg = strtok(NULL, " \n");
                }
            } /*
            else if (strcmp(arg, "exit") == 0)
            {
                runs = 0;
            }*/
            else
            {
                args[count++] = arg;
                arg = strtok(NULL, " \n");
            }
        }

        semicolon = 0;
        redir1 = 0;
        redir2 = 0;

        if (redir != NULL)
        {
            if (strcmp(redir, ";") == 0)
            {
                semicolon = 1;
            }
            else if (strcmp(redir, ">") == 0)
            {
                redir1 = 1;
            }
            else if (strcmp(redir, "<") == 0)
            {
                redir2 = 1;
            }
        }
        pid_t pid = fork();

        if (pid > 0)
        {
            wait(NULL);
            if (semicolon)
            {
                printf("redirection active");
                pid_t pid2 = fork();
                if (pid2 == 0)
                    execvp(args2[0], args2);
                else if (pid2 > 0)
                {
                    wait(NULL);
                    printf("\npid:%d status:%d\n", pid2, status);
                }

                else
                {
                    perror("fork() error");
                    exit(-1);
                }
            }
            printf("\npid:%d status:%d\n", pid, status);
        }
        else if (pid < 0)
        {
            perror("fork() error");
            exit(-1);
        }

        else
        {
            if (redir1)
            {
                dup2(stdo, 1);
                dup2(open(filename, O_WRONLY | O_CREAT | O_TRUNC, S_IRWXU | S_IRWXG | S_IRWXO), 1);
            }

            else if (redir2) //strcmp(redir, "<") == 0
            {
                dup2(stdi, 0);
                dup2(open(filename, O_RDONLY, S_IRWXU | S_IRWXG | S_IRWXO), 0);
            }
            execvp(args[0], args);
        }
    } while (1);
    free(args);
    free(args2);
    free(arr);
    return 0;
}