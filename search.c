/**
 * Function that searches for a string in a file and returns the number of
 * times it occurs.
 *
 * @author Joshua Crum
 * @version 2/12/18
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include "search.h"


/**
 * Takes a file and searches through it for a provided string.
 *
 * @param *fname the name of the file to search.
 * @param *s_for the string to search for.
 * @return int the frequency of *s_for.
 */
int search_for (char *fname, char *s_for)
{
    int count = 0;
    char read[128];

    FILE *file = fopen (fname, "r");

    if (file == NULL)
    {
        fprintf(stderr, "File open error.\n");
        exit (1);
    }
    
    else
    {
        /* Loop through every string in the file. */
        while(fscanf(file, "%s", read) == 1)
        {
            /* Check if search string occurs and increment the counter. */
            if (strcmp(s_for, read) == 0)
            {
                count++;   
            }
        }
    }
    return count;
}
