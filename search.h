/**
 * Function that searches for a string in a file and returns the number of
 * times it occurs.
 *
 * @author Joshua Crum
 * @version 2/12/18
 */

#ifndef search_for_h
#define search_for_h


/**
 * Takes a file and searches through it for a provided string.
 *
 * @param *fname the name of the file to search.
 * @param *s_for the string to search for.
 * @return int the frequency of *s_for.
 */
int search_for (char *fname, char *s_for);

#endif /* search_for_h */
