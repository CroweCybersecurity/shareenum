#include <stdio.h>		//Standard GNU IO Lib
#include <string.h>		//"Strings" as if C had them, just helper functions
#include <stdlib.h>		//The GNU Standard Library
#include <unistd.h>		//Unix system standard libraries

/* Get the number of lines that are in a file.  Attempts to count as 
 * efficiently as possible and should be somewhat compiler optimized.
 *
 * PARAMETERS:
 *   FILE - Pointer to a C file handle. Will be reset to the beginning 
 *        Position of the file after this is run. 
 *
 * RETURN (int): Number of lines in the file.  
 */
int file_countlines(FILE * fh);

/* Determines if a file exists within the filesystem path. 
 *
 * PARAMETERS:
 *   char * - Pointer to a string containing the path to
 *         the target file. 
 *
 * RETURN (int): 0 if the file does not exist, 1 if it does. 
 */
int file_exists(const char * filename);

/* Determine the number of digits in an integer
 *
 * PARAMETERS:
 *   int - Number to test
 *
 * RETURN (int): The number of digits in the integer. 
 */
int numdigits(int n);
