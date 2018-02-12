/**
 * A multi-threaded text search that takes file
 * names from the command line and prompts the user
 * to enter text to search for.
 *
 * @author Joshua Crum
 * @version 2/13/2018
 */

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

#define READ_END 0
#define WRITE_END 1



/**
 *
 */
int main (int argc, char **argv)
{
    if (argc < 2)
    {
        printf ("Pass file name(s) as arguments.\n");
        return 1;
    }
    else if (argc > 11)
    {
        printf ("No more than 10 files can be processed.\n");
        exit (0);
    }
    
    for (int i = 1; i < argc; i++)
    {
        if (access(argv[i], F_OK) != 0)
        {
            /* File does not exist. */
            printf ("File '%s' does not exist.\n", argv[i]);
            exit (0);
        }
    }
    
    int fd[2];
    int num_files = --argc;
    pid_t processes[num_files];
    
    /* Get search string from user. */
    printf("Enter a string to search for: ");
    char to_search[32];
    fgets(to_search, 32, stdin);
    
    /* Remove trailing newline. */
    int len = strlen(to_search);
    if (to_search[len - 1] == '\n')
        to_search[len - 1] = '\0';
    
    pid_t pid, cpid, wpid;
    
    for (int i = 0; i < num_files; i++)
    {
        /* Create the pipe. */
        if (pipe(fd) == -1)
        {
            fprintf (stderr, "Pipe failed.\n");
            return 1;
        }
        
        pid = fork ();
        
        if (pid < 0)
        {
            fprintf (stderr, "Fork failed.\n");
            return 1;
        }
        
        /* Parent. */
        if (pid)
        {
            close (fd[READ_END]);
            
            write (fd[WRITE_END], to_search, strlen(to_search) + 1);
            
            close (fd[WRITE_END]);

            continue;
            wait(0);
        }
        
        /* Child. */
        else
        {
            char read_msg[32];
            close (fd[WRITE_END]);
            
            read (fd[READ_END], read_msg, 32);
            printf("Got from parent through pipe :%s:\n", read_msg);
            
            close(fd[WRITE_END]);
            
            cpid = getpid();
            printf ("Child %d pid: %d. My parent is: %d File is: %s\n"
                   , i, cpid, getppid(), argv[i + 1]);
            processes[i] = cpid;
            
            while (1)
                ;
            
            break;
        }
    }
    while ((wpid = wait(0)) > 0);
    return 0;
}
