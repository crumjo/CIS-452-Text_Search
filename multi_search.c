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
 * Handles the shutdown of the program and its child processes when ctrl+c is
 * pressed.
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
 * Main method that accepts file names as parameters, checks if they exist,
 * and then spawns a child searcher process for each file. The parent process
 * gets input from the user and sends it to the child processes through a pipe.
 * The child processes search for the string in the file, and send its
 * frequency to the parent. The parent continues accepting user input until
 * crtl+c is pressed.
 */
int main (int argc, char **argv)
{
    signal(SIGINT, sigHandler);
    
    if (argc < 2)
    {
        printf ("Pass file name(s) as arguments.\n");
        return 1;
    }
    else if (argc > 4)
    {
        printf ("No more than 3 files can be processed.\n");
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
    pid_t ppid;
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
        if ((close (p_to_c[READ_END])) == -1)
        {
            fprintf(stderr, "Parent read pipe close failed.\n");
            return 1;
        }
        
        if ((close (c_to_p[WRITE_END])) == -1)
        {
            fprintf(stderr, "Parent write pipe close failed.\n");
            return 1;
        }
    }
    
    /* Close child pipe ends. */
    else
    {
        if ((close (p_to_c[WRITE_END])) == -1)
        {
            fprintf(stderr, "Child write pipe close failed.\n");
            return 1;
        }
        
        if ((close (c_to_p[READ_END])) == -1)
        {
            fprintf(stderr, "Child read pipe close failed.\n");
            return 1;
        }
    }
    
    
    /* Parent write to pipe. */
    if (temp == ppid)
    {
        while (1)
        {
            /* Get search string from user. */
            printf("\nEnter a string to search for: ");
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
                    fprintf (stderr, "Parent pipe write failed.\n");
                    return 1;
                }
            }
            
            for (int j = 0; j < num_files; j++)
            {
                int c_pid, file_pos, val;
                
                /* Read pid of child writing to pipe. */
                if ((read (c_to_p[READ_END], &c_pid, pid_size)) == -1)
                {
                    fprintf(stderr, "Parent pid read failed.\n");
                }
                
                /* Read occurences of search string from child. */
                if ((read (c_to_p[READ_END], &val, pid_size)) == -1)
                {
                    fprintf(stderr, "Parent read failed.\n");
                    return 1;
                }
                
                /* Figure out which child read which file. */
                for (int k = 0; k < num_files; k++)
                {
                    if (c_pid == pros[k])
                    {
                        file_pos = k;
                    }
                }
                
                printf ("> Child [%d] found %d occurence(s) of '%s' in %s.\n",
                        c_pid, val, to_search, argv[file_pos + 1]);
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
            
            pid_t my_pid = getpid();
            char c_read[1024];
            
            /* Read search string from parent. */
            if ((read (p_to_c[READ_END], &c_read, 1024)) == -1)
            {
                fprintf(stderr, "Pipe read failed.\n");
                return 1;
            }
            
            printf ("Child [%d] searching for '%s' in %s...\n",
                    getpid(), c_read, filename);
            
            int count = search_for (filename, c_read);
            
            /* Write child pid to parent. */
            if ((write (c_to_p[WRITE_END], &my_pid, sizeof(my_pid))) == -1)
            {
                fprintf(stderr, "Child pid write failed.\n");
            }
            
            /* Write count for search string to parent. */
            if ((write (c_to_p[WRITE_END], &count, sizeof(count))) == -1)
            {
                fprintf(stderr, "Child count write failed.\n");
                return 1;
            }
            
            //free (filename);
        }
    }
    
    return 0;
}

