/**
 * A multi-threaded text search that takes file
 * names from the command line and prompts the user
 * to enter text to search for.
 *
 * @author Joshua Crum
 * @version 2/13/2018
 */

#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include "search.h"

#define READ_END 0
#define WRITE_END 1

int num_files;
static pid_t pid;

/* Array of all child processes. */
pid_t pros[10];


/**
 * Handles the shutdown of the program and its child processes when
 * ctrl+c is pressed.
 *
 * @param sigNum the signal number for ctrl+c.
 */
void sigHandler (int sigNum) {
    if(pid)
    {
        printf (" interrupt received.\n");

        /* Shutdown code. */
        for (int i = 0; i < num_files; i++)
        {
            printf("Shutting down child [%d]...\n", pros[i]);
            if ((kill (pros[i], SIGKILL)) == -1)
            {
                fprintf(stderr, "Child shutdown failed.\n");
                exit (1);
            }
        }
        
        sleep (1);
        printf ("Program will now exit.\n");
    }
    exit(0);
}


/**
 *
 */
int main (int argc, char **argv)
{
    signal(SIGINT, sigHandler);
    
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
    num_files = --argc;
    
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
    
    int pid_size;
    pid_t ppid, wpid;
    pid_t which_file[argc];
    
    for (int i = 0; i < num_files; i++)
    {
        
        pid = fork();
        pid_size = sizeof(pid);
        if (pid < 0)
        {
            fprintf (stderr, "Fork failed.\n");
            return 1;
        }
        
        /* Parent. */
        else if (pid)
        {
            ppid = getpid();
            pros[i] = pid;
            
            continue;
        }
        
        /* Child. */
        else
        {
            which_file[i] = getpid();
            break;
        }
        
    }
    
    int temp = getpid();
    
    /* Close unused pipe ends. */
    if (temp == ppid)
    {
        close (p_to_c[READ_END]);
        close (c_to_p[WRITE_END]);
    }
    
    /* Close child pipe ends. */
    else
    {
        close (p_to_c[WRITE_END]);
        close (c_to_p[READ_END]);
    }
    
    
    /* Parent write to pipe. */
    if (temp == ppid)
    {
        while (1)
        {
            /* Get search string from user. */
            printf("Enter a string to search for: ");
            char to_search[1024];
            fgets(to_search, 1024, stdin);
            
            /* Remove trailing newline. */
            int len = strlen(to_search);
            if (to_search[len - 1] == '\n')
                to_search[len - 1] = '\0';
            
            int size = sizeof(to_search);
            
            for (int i = 0; i < num_files; i++)
            {
                if ((write (p_to_c[WRITE_END], &to_search, size)) == -1)
                {
                    fprintf (stderr, "Pipe write failed.\n");
                    return 1;
                }
            }
            
            for (int j = 0; j < num_files; j++)
            {
                int val;
                if ((read (c_to_p[READ_END], &val, pid_size)) == -1)
                {
                    fprintf(stderr, "Read failed.\n");
                    return 1;
                }
                printf ("Count from child: %d\n", val);
            }

        }
    }
    
    /* Child reads from pipe. */
    else
    {
        while (1)
        {
            char *filename = (char *) malloc (32 * sizeof(char));
            for(int i = 0; i < num_files; i++)
            {
                if (which_file[i] == getpid())
                {
                    filename = argv[i + 1];
                }
            }
            
            char c_read[1024];
            if ((read (p_to_c[READ_END], &c_read, 1024)) == -1)
            {
                fprintf(stderr, "Pipe read failed.\n");
                return 1;
            }
            printf ("Child [%d] searching for '%s' in %s\n"
                    , getpid(), c_read, filename);
            
            int count = search_for (filename, c_read);
            
            write (c_to_p[WRITE_END], &count, sizeof(count));
            //free (filename);
        }
    }
    
    while ((wpid = wait(0)) > 0);
    
    return 0;
}

