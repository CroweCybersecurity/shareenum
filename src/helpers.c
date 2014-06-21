#include "helpers.h"

int file_countlines(FILE * fh ) {
	int     nlines = 0;
	int     partial = 0;
	char    buf[10] = "";

	//Reset the file back to the beginning so we can count. 
	rewind(fh);

	//Loop through the file and pull down the size of the buffer. 
	while ( fgets( buf, sizeof(buf), fh ) != NULL ) {
		//If we have a newline char in the buffer, increase the counter
		if ( strchr(buf,'\n') ) {
			nlines++;
			partial = 0;
		//Otherwise we need to account for the fact that we may have a 
		//partial line at the end
		} else {
			partial = 1;
		}
	}

	//Return us back to the beginning of the file.  Might not be 
	//the best idea, but this function will not be used in 
	//this program for doing to the middle of a while. 
	rewind(fh);
	return nlines += partial;
}

int file_exists(const char * filename) {
	//See if we can access the filename. F_OK is just an existence test
	//it doesn't check any permissions.
	if(access(filename, F_OK) != -1) {
		return 1;
	}
	return 0;
}

int numdigits(int n) {
	int     c = 0;

	//Loop until we have no digits left in N.
	while(n!=0) {
		//Divide by 10 and increment our counter. 
		n/=10;
		++c;
	}
	return c;
}

