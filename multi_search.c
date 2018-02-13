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
#include "search.h"

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
    
    int p_to_c[2];
    int c_to_p[2];
    int num_files = --argc;
    
    /* Get search string from user. */
    printf("Enter a string to search for: ");
    char to_search[32];
    fgets(to_search, 32, stdin);
    
    /* Remove trailing newline. */
    int len = strlen(to_search);
    if (to_search[len - 1] == '\n')
        to_search[len - 1] = '\0';
    
    pid_t pid, wpid;
    
    for (int i = 0; i < num_files; i++)
    {
        /* Create the pipe from parent to child. */
        if (pipe(p_to_c) == -1)
        {
            fprintf (stderr, "Pipe failed.\n");
            return 1;
        }
        
        /* Create the pipe from child to parent. */
        if (pipe(c_to_p) == -1)
        {
            fprintf (stderr, "Pipe failed.\n");
            return 1;
        }
        
        pid = fork();
        if (pid < 0)
        {
            fprintf (stderr, "Fork failed.\n");
            return 1;
        }
        
        /* Parent. */
        else if (pid)
        {
            close (p_to_c[READ_END]);
            close (c_to_p[WRITE_END]);
            
            write (p_to_c[WRITE_END], to_search, strlen(to_search) + 1);
            close (p_to_c[WRITE_END]);
            
            int p_read_msg;
            read (c_to_p[READ_END], &p_read_msg, 2);
            
            close (c_to_p[READ_END]);
            printf ("Got from child: %d\n", p_read_msg);
            
            continue;
        }
        
        /* Child. */
        else
        {
            char c_read_msg[32];
            close (p_to_c[WRITE_END]);
            close (c_to_p[READ_END]);
            
            read (p_to_c[READ_END], c_read_msg, 32);
            close (p_to_c[READ_END]);
            printf("Child of: %d \t My pid: %d \t\t Message: %s\n"
                   , getppid(), getpid(), c_read_msg);
            
            int count = search_for (argv[i + 1], c_read_msg);
            
            write (c_to_p[WRITE_END], &count, 2);
            close (c_to_p[WRITE_END]);
            break;
        }

    }
    
    while ((wpid = wait(0)) > 0);
    return 0;
}
