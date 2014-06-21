#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include "helpers.h"
#include "colors.h"
#include "smb.h"

/* Globals for the options we got from the command line.
 * These are most likely going to be externs in other C 
 * files and should be considered READ ONLY!
 */
char * 				gUsername = NULL;
char * 				gPassword = NULL;
int 				gPassIsHash = 0;
int 				gTimeout = 0;

/* Print the usage
 *
 * PARAMETERS: none
 * RETURN: none
 */
void usage();
