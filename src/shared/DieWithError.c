#include <stdio.h>  /* for perror() */
#include <stdlib.h> /* for exit() */

/**
 * Exits the current program with an error message
*/
void DieWithError(char *errorMessage)
{
	perror(errorMessage);
	exit(1);
}
