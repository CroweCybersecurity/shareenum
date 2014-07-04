#include "main.h"

const char banner[] = ANSI_COLOR_BOLDBLUE"    _____ "ANSI_COLOR_RED" _     "ANSI_COLOR_BOLDYELLOW"       "ANSI_COLOR_BOLDBLUE"      "ANSI_COLOR_GREEN"      "ANSI_COLOR_RED" _____\n"\
ANSI_COLOR_BOLDBLUE"   /  ___|"ANSI_COLOR_RED"| |    "ANSI_COLOR_BOLDYELLOW"       "ANSI_COLOR_BOLDBLUE"      "ANSI_COLOR_GREEN"      "ANSI_COLOR_RED"|  ___|\n"\
ANSI_COLOR_BOLDBLUE"   \\ `--. "ANSI_COLOR_RED"| |__  "ANSI_COLOR_BOLDYELLOW"  __ _ "ANSI_COLOR_BOLDBLUE" _ __ "ANSI_COLOR_GREEN"  ___ "ANSI_COLOR_RED"| |__   _ __   _   _  _ __ ___\n"\
ANSI_COLOR_BOLDBLUE"    `--. \\"ANSI_COLOR_RED"| '_ \\ "ANSI_COLOR_BOLDYELLOW" / _` |"ANSI_COLOR_BOLDBLUE"| '__|"ANSI_COLOR_GREEN" / _ \\"ANSI_COLOR_RED"|  __| | '_ \\ | | | || '_ ` _ \\\n"\
ANSI_COLOR_BOLDBLUE"   /\\__/ /"ANSI_COLOR_RED"| | | |"ANSI_COLOR_BOLDYELLOW"| (_| |"ANSI_COLOR_BOLDBLUE"| |   "ANSI_COLOR_GREEN"|  __/"ANSI_COLOR_RED"| |___ | | | || |_| || | | | | |\n"\
ANSI_COLOR_BOLDBLUE"   \\____/ "ANSI_COLOR_RED"|_| |_|"ANSI_COLOR_BOLDYELLOW" \\__,_|"ANSI_COLOR_BOLDBLUE"|_|   "ANSI_COLOR_GREEN" \\___|"ANSI_COLOR_RED"\\____/ |_| |_| \\__,_||_| |_| |_|\n"\
ANSI_COLOR_RESET;

int main(int argc, char * argv[]) {
	FILE *          outfile;
	char *          target = NULL;
	int             linedelay = 0; // -r command line option
	int             recursion = 1; // -r command line option
	int             startline = 0; // -s command line argument
	int             c; //For getopt
	const char *    l; //For length of the hash, if we got one
	char            hash[33]; //To store our hash if the user gave us.

	//Print a header so we know the versions
	fprintf(stdout, "%s", banner);
	fprintf(stdout, ANSI_COLOR_BOLDWHITE"%*s %d.%d build %d\n"ANSI_COLOR_RESET, 57-numdigits(BUILD_NUMBER),"Version", MAJOR_REVISION, MINOR_REVISION, BUILD_NUMBER);


/*************************************************************************************
 * COMMAND LINE ARGUMENTS
 ************************************************************************************/

	/* Get all of our command line options with GetOpt
	 * Basically, here we use getopt to loop through the arguments
	 * It will pull one argument letter at a time and then
	 * we use a case statement to process only that argument and
	 * its value out to things.  Mostly just setting variables, 
	 * maybe doing some processing 
	 */
	opterr = 0;
	while ((c = getopt(argc, argv, "r:n:o:t:d:u:p:s:")) != -1) {
		switch (c) {
			case 'u': //Get the username
				gUsername = optarg;
				break;
			case 'p': //Get the password, but check if its a hash first.
				//Get the length of the string quickly. 
				for (l = optarg; *l; ++l);

				//If the length is 65 (LM:NL) and there's a : in the right place
				if((l - optarg) == 65 && optarg[32] == ':') {
					//Copy over the last 32 characters into a temp buffer
					strncpy(hash, optarg + 33, 32);

					//Make sure the last character is a null 
					//because we're good C programmers
					hash[32] = '\0';

					//Set the value of hash to our global
					gPassword = hash;

					//And set the "bool" to true as well. 
					gPassIsHash = 1;
				} else {
					//If its not a hash, just set the password. 
					gPassword = optarg;
				}

				break;
			case 'r': //How deep are we going?
				recursion = atoi(optarg);
				break;
			case 'd': //How many seconds are we waiting between targets.
				linedelay = atoi(optarg);
				break;
			case 't': //How many seconds are we timing out. 
				gTimeout = atoi(optarg);
				break;
			case 's': //What line in the file should we start at 
					  //Added because its a useful feature that McAtee
					  //thought up and the program kept crashing after 
					  //a while during testing.
				startline = atoi(optarg);
				break;
			case 'o': //What is our output file.  
				//Open the file
				outfile = fopen (optarg, "a");
				//Turn off buffering of the output file so McAtee
				//can get his tail on instantly. 
				setvbuf(outfile , NULL , _IONBF , 0);
				break;
			case '?': //If getopt doesn't know what the character is it comes here.
				if (isprint (optopt))
					//Yell at the user for being dumb and giving us 
					//something we don't want.
					fprintf(stderr, "Unknown option '-%c'.\n", optopt);

				usage();

				return 1;
			default:
				abort();
		}
	}

/*************************************************************************************
 * CHECK AND SETUP
 ************************************************************************************/

	//Get the target (IP or file) from the last item in argv.  Optind
	//is from getopt telling us where it stopped. 
	target = argv[optind];

	//If target isn't set, complain because its mandatory.
	if(target == NULL) {
		fprintf(stderr, ANSI_COLOR_BOLDRED "ERROR: No target specified!\n" ANSI_COLOR_RESET);
		usage();
		exit(1);
	}

	//If the outfile didn't open properly or isn't set, yell because its mandatory.
	if(outfile == NULL) {
		fprintf(stderr, ANSI_COLOR_BOLDRED "ERROR: No output file specified!\n" ANSI_COLOR_RESET);
		usage();
		exit(1);
	}

	//Tell the user what creds we're using, if any.  Because we want to make sure they know.
	fprintf(stdout, "\n");
	fprintf(stdout, ANSI_COLOR_BOLDGREEN "Username: " ANSI_COLOR_RESET "%s\n", gUsername);
	if(gPassIsHash == 1)
		fprintf(stdout, ANSI_COLOR_BOLDGREEN "Hash:     " ANSI_COLOR_RESET "%s\n", gPassword);
	else
		fprintf(stdout, ANSI_COLOR_BOLDGREEN "Password: " ANSI_COLOR_RESET "%s\n", gPassword);
	fprintf(stdout, "\n");

	//If debugging is on, print out the other variables we got on the cmd line.
#ifdef DEBUG
		fprintf(stdout, "Target: %s\n", argv[optind]);
		fprintf(stdout, "Start Point: %d\n", startline);
#endif

	//Wait three seconds because McAtee
	sleep(3);

	//Try and turn off buffering on stdout. 
	setbuf(stdout, NULL);

/*************************************************************************************
 * RUNNING OF SHARES
 ************************************************************************************/

	if(startline == 0)                                   //If we're starting at 0, print the headers.  
		fprintf(outfile, "\"USER\",\"HOST\",\"SHARE\",\"OBJECT\",\"TYPE\",\"PERMISSIONS\",\"HIDDEN\"\n");

	smbresultlist       *res;      //Struct to hold the results info
	smbresult           *tmp;      //Item to hold the temp results as we loop through objects
	char                *outbuf;

	//If the target we got is a file.
	if(file_exists(target)) {
		FILE            *infile = fopen ( target, "r" );  //Open the file
		int             current = 0;                     //Set a counter for where we're at in the file
		int             total = file_countlines(infile); //Count the total number of lines in the file
		int             totallen = numdigits(total);     //And the number of digits in that total for pretty output
		char            infile_buf[1024];                //And a buffer to read the file into!

		if ( infile != NULL ) {                          //If our input file loaded properly...
			while ( fgets ( infile_buf, sizeof(infile_buf), infile ) != NULL ) {       //Read our file one line at a time into the buffer
				current++;                               //We did it!  Increment where we're at

				if(current < startline) continue;        //Go ahead and skip until we get the line we want if set.

				if (infile_buf[strlen(infile_buf) - 1] == '\n') {      //Check to make sure that our file doesn't have any newlines or returns
					infile_buf[strlen(infile_buf) - 1] = '\0';
				}
				if (infile_buf[strlen(infile_buf) - 1] == '\r') {
					infile_buf[strlen(infile_buf) - 1] = '\0';
				}

				fprintf(stdout, ANSI_COLOR_BOLDBLUE "(%*d/%d) " ANSI_COLOR_BOLDCYAN "%-25s " ANSI_COLOR_RESET, totallen, current, total, infile_buf);

				res = runtarget(infile_buf, recursion);                     //Run the target and get results

				if(smbresultlist_length(res) > 1) {
					while(smbresultlist_pop(&res, &tmp)) {               //Loop through the llist of results and put them in tmpp
						smbresult_tocsv(*tmp, outbuf);                      //Convert tmp to a CSV
						fprintf(outfile, "\"%s\",%s\n", gUsername, outbuf); //Print it to our file
					}
					fprintf(stdout, "[" ANSI_COLOR_GREEN "x" ANSI_COLOR_RESET "] Got information on %d objects.\n", smbresultlist_length(res));
				} else {
					fprintf(stdout, "[" ANSI_COLOR_RED "!" ANSI_COLOR_RESET "] %s (Code: %d)\n", strerror(res->data->statuscode), res->data->statuscode);
				}
			}
		} else {
			perror ( target );                                          //If the file didn't load, print the error. 
		}

		fclose(outfile);                                                //Close our file handle, because we're good programmers

	//Otherwise we'll assume our target is an IP or hostname and won't do anything fancy
	} else {
		fprintf(stdout, ANSI_COLOR_BOLDCYAN "%-25s " ANSI_COLOR_RESET, target);

		res = runtarget(target, recursion);                             //Run the target and get results

		if(smbresultlist_length(res) > 1) {
			while(smbresultlist_pop(&res, &tmp)) {                  //Loop through the llist of results and put them in tmpp
				smbresult_tocsv(*tmp, outbuf);                               //Convert tmp to a CSV
				fprintf(outfile, "\"%s\",%s\n", gUsername, outbuf);         //Print it to our file
			}
			fprintf(stdout, "[" ANSI_COLOR_GREEN "x" ANSI_COLOR_RESET "] Got information on %d objects.\n", smbresultlist_length(res));
		} else {
			fprintf(stdout, "[" ANSI_COLOR_RED "!" ANSI_COLOR_RESET "] %s (Code: %d)\n", strerror(res->data->statuscode), res->data->statuscode);
		}
	}

	sleep(linedelay);                                                   //We'll delay as long as our user asked. 
}

//If you don't get this, try again harder
void usage() {
	printf("Usage: shareenum -o FILE TARGET\n");
	printf("    TARGET  - REQUIRED Full path or a file of paths to list the shares, files\n");
	printf("              and directories within. Can be just an IP, a hostname, or even a\n");
	printf("              full path inside a share, such as smb://COOL-DC.DOMAIN/NETLOGON.\n");
	printf("    -o FILE - REQUIRED File to write results to in CSV format.\n");
	printf("    -u USER - Username, otherwise go anonymous.  If using a domain, it should\n");
	printf("              be in the format of DOMAIN\\\\user.\n");
	printf("    -p PASS - Password, otherwise go anonymous.  This can be a NTLM has in\n");
	printf("              the format LMHASH:NTLMHASH.  If so, we'll pass the hash.\n");
	printf("    -r NUM  - How many levels into the share or directory should we go.\n");
	printf("              Depending on the setting, this can pull a listing of every file\n");
	printf("              and directory.  (Default: None)\n");
	printf("    -t NUM  - Seconds to wait before connection timeout. (Default: 3s)\n");
	printf("    -s NUM  - In the event that the scan needs restarted, skip the first NUM\n");
	printf("              lines in the input file.  All output will be appended to the\n");
	printf("              output file so previous results will not be lost. (Default: 0)\n");
	printf("    -d NUM  - How many seconds should we wait between each target. (Default: 0)\n");
}
