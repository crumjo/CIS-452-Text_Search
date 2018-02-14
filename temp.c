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
    char to_search[1024];
    fgets(to_search, 1024, stdin);
    
    /* Remove trailing newline. */
    int len = strlen(to_search);
    if (to_search[len - 1] == '\n')
        to_search[len - 1] = '\0';
    
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
    
    pid_t pid, ppid, wpid;
    pid_t pros[argc];
    
    for (int i = 0; i < num_files; i++)
    {
        
        pid = fork();
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
            
            close (p_to_c[READ_END]);
            close (c_to_p[WRITE_END]);
            
            continue;
        }
        
        /* Child. */
        else
        {
            printf ("Creating child process [%d]...\n", getpid());
            
            close (p_to_c[WRITE_END]);
            close (c_to_p[READ_END]);
            
            break;
        }
        
    }
    
    int size = sizeof(to_search);
    printf("Size: %d\n", size);
    
    /* Parent write to pipe. */
    int temp = getpid();
    
    if (temp == ppid)
    {
        for (int i = 0; i < num_files; i++)
        {
            printf("Parent writing to pipe with pid %d...\n", ppid);
            if ((write (p_to_c[WRITE_END], &to_search, size)) == -1)
            {
                fprintf (stderr, "Pipe write failed.\n");
                return 1;
            }
        }
    }
    
    /* Child reads from pipe. */
    else
    {
        printf("\nChild %d reading from pipe...\n", getpid());
        char c_read[size];
        if ((read (p_to_c[READ_END], &c_read, size)) == -1)
        {
            fprintf(stderr, "Pipe read failed.\n");
            return 1;
        }
        printf ("[%d] Pipe reading: %s\n", getpid(), c_read);
    }
    
    while ((wpid = wait(0)) > 0);
    
    //    printf("All child processes: \n");
    //    for(int i = 0; i < 2; i++)
    //    {
    //        printf("%d\n", pros[i]);
    //    }
    
    return 0;
}

