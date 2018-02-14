/**
 *
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include "search.h"


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
        while(fscanf(file, "%s", read) == 1)
        {
            if (strcmp(s_for, read) == 0)
            {
                count++;   
            }
        }
    }
    return count;
}
