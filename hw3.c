#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <fcntl.h>

#define arrSz 100

void sigint_handler(int sig)
{
    char msg[] = "Signal handled. \nCS361 >";
    write(1, msg, sizeof(msg));
}
void sigtstp_handler(int sig)
{
    char msg[] = "Signal handled. \nCS361 >";
    write(1, msg, sizeof(msg));
}
int main()
{
    int count = 0, pos, status, stdi = dup(0), stdo = dup(1);
    char *arg, *filename = NULL, *redir = NULL, *arr = (char *)malloc(arrSz * sizeof(char)),
               **args = malloc(arrSz * sizeof(char *)), **args2 = malloc(arrSz * sizeof(char *));

    signal(SIGINT, sigint_handler);
    signal(SIGTSTP, sigtstp_handler);
    do
    {
        printf("CS361 > ");
        arg = strtok(fgets(arr, arrSz, stdin), " \n");

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
            }
            else
            {
                args[count++] = arg;
                arg = strtok(NULL, " \n");
            }
        }
        /*
        for(int i =  0; i< count; i++)
        {
            printf("%s\n", args[i]);
        }*/
        pid_t pid = fork();

        if (pid > 0)
        {
            waitpid(pid, &status, WUNTRACED);
            if (strcmp(redir, ";") == 0)
            {
                pid_t pid2 = fork();
                if (pid2 == 0)
                    execvp(args2[0], args2);
                else if (pid2 > 0)
                    wait(NULL);
                else
                {
                    perror("fork() error");
                    exit(-1);
                }
            }
            printf("pid:%d status:%d\n", pid, status);
        }
        else if (pid < 0)
        {
            perror("fork() error");
            exit(-1);
        }

        else
        {
            if (strcmp(redir, ">") == 0)
            {
                dup2(stdo, 1);
                dup2(open(filename, O_WRONLY | O_CREAT | O_TRUNC, S_IWOTH), 1);
                execvp(args[0], args);
            }

            else if (strcmp(redir, "<") == 0)
            {
                dup2(stdi, 0);
                dup2(open(filename, O_RDONLY, S_IROTH), 0);
                execvp(args[0], args);
            }
            execvp(args[0], args);
        }
    } while (strcmp(args[0], "exit"));

    return 0;
}