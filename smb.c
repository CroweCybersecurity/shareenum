#include "smb.h"

smb_result runhost(char * target, FILE * outfh, int maxdepth) {
	SMBCCTX *       context;
	char            buf[256];
	smb_result      res;
	int             rc;

	//Try to create a context, if it's null that means we failed, so let the user know.
	if((context = create_context()) == NULL) {
		res.code = 1;
		res.message = "Unable to create samba context.\n";
		//res.objects = 0;

		return res;
	}

	//Check to see if the target is a valid URL, if not add the smb:// to the front.
	if(strncmp("smb://", target, 6) != 0) {
		snprintf(buf, sizeof(buf), "smb://%s", target);
	} else {
		strncpy(buf, target, strlen(target));
	}

	//Browse to our file and get the goods
	res = browse(context, buf, outfh, maxdepth, 0);

	//Output to the user.
	if(res.code > 0) {
		fprintf(stdout, "[" ANSI_COLOR_RED "!" ANSI_COLOR_RESET "] %s (Code: %d)\n", res.message, res.code);
	} else if(res.code == 0) {
		fprintf(stdout, "[" ANSI_COLOR_BOLDYELLOW "*" ANSI_COLOR_RESET "] %s\n", res.message);
	} else {
		fprintf(stdout, "[" ANSI_COLOR_GREEN "x" ANSI_COLOR_RESET "] %s\n", res.message);
	}

	//Delete our context, so there's less segfaults.
	delete_context(context);
	return res;
}

void parsesmburl(char * url, char * host, char * share, char * object) {
	char         buf[1024];
	char *       token;
	char *       last;
	const char   sep[2] = "/";

	//Remove the smb:// from the front of our string, if we can't fail.
	if(strncmp("smb://", url, 6) == 0) {
		strncpy(buf, url+6, strlen(url) - 5);
	} else {
		strncpy(buf, url, strlen(url));
	}

	//Tokenize the URL by /, get the first one, should be our host. 
	token = strtok(buf, sep);
	if(token == NULL) {
		return;
	}
	//Set it to the host pointer.
	strncpy(host, token, strlen(token));

	//Get the second token, it should be our share. 
	token = strtok(NULL, sep);
	if(token == NULL)  {
		return;
	}
	//Set it to the share pointer. 
	strncpy(share, token, strlen(token));

	//Now loop through the rest of the tokens and make the 
	//file path.  Setting it to the directory pointerj
	while (token != NULL) {
		token = strtok(NULL, sep);
		if(token != NULL)
			sprintf(object, "%s/%s", object, token);
	}
}

static void auth_fn(
		const char *    pServer,
		const char *    pShare,
		char *          pWorkgroup,
		int             maxLenWorkgroup,
		char *          pUsername,
		int             maxLenUsername,
		char *          pPassword,
		int             maxLenPassword) {

	//Get our external globals that we got from the command line
	extern char *   gUsername;
	extern char *   gPassword;

	//We're always going to operate on a blank workgroup of WORKGROUP
	char			workgroup[256] = { '\0' };

	//Get the username, if its set, from the globals we set in the main.
	if(gUsername != NULL)
		strncpy(pUsername, gUsername, maxLenUsername - 1);

	//Same with the the password.  If its a hash, it'll also just be a string.
	if(gPassword != NULL)
		strncpy(pPassword, gPassword, maxLenPassword - 1);

	//Finally copy our blank workgroup over also.
	strncpy(pWorkgroup, workgroup, maxLenWorkgroup - 1);
}

static SMBCCTX* create_context(void) {
	SMBCCTX *		ctx;
	extern int 		gTimeout;
	extern int 		gPassIsHash;

#ifdef DEBUG
	fprintf(stdout, "SMB: Attempting to create context.\n");
#endif

	//Create the Samba context struct , if it didn't work quit. 
	if((ctx = smbc_new_context()) == NULL)
		return NULL;

#ifdef DEBUG
	fprintf(stdout, "SMB: Setting options in context.\n");

	//Set the options for our context.  a
	//If its enabled at the command line, turn on Samba library debugging
	smbc_setDebug(ctx, 1);

	//We want to log our errors to STDERR instead of STDOUT
	smbc_setOptionDebugToStderr(ctx, 1);
#endif

	//Set the function that the Samba library will call when it needs
	//to authenticate
	smbc_setFunctionAuthData(ctx, auth_fn);
	//Set the timeout, we get the command line option as seconds and the 
	//function takes milliseconds.
	smbc_setTimeout(ctx, gTimeout * 1000);
	//If we got a hash at the command line, let the context know we're
	//giving it a hash
	smbc_setOptionUseNTHash(ctx, gPassIsHash);

#ifdef DEBUG
	fprintf(stdout, "SMB: Initializing the context.\n");
#endif

	//Initialize our context with the options that we have set or null on fail.
	if(smbc_init_context(ctx) == NULL) {
		smbc_free_context(ctx, 1);
		return NULL;
	}

#ifdef DEBUG
	fprintf(stdout, "SMB: Context created.\n");
#endif

	return ctx;
}

static void delete_context(SMBCCTX* ctx) {
	//Trying to fix the error:  no talloc stackframe at ../source3/libsmb/cliconnect.c:2637, leaking memory
	TALLOC_CTX *frame = talloc_stackframe();

#ifdef DEBUG
	fprintf(stdout, "SMB: Purging cached servers.\n");
#endif

	//First we need to purge the cache of servers the context has.
	//This should also free all the used memory allocations.
	smbc_getFunctionPurgeCachedServers(ctx)(ctx);

	TALLOC_FREE(frame);

#ifdef DEBUG
	fprintf(stdout, "SMB: Freeing the context.\n");
#endif

	//Next we need to free the context itself, and free all the 
	//memory it used.
	smbc_free_context(ctx, 1);
}

static smb_result browse(SMBCCTX *ctx, char * path, FILE * outfh, int maxdepth, int depth) {
	SMBCFILE *              fd;
	struct smbc_dirent *    dirent;
	struct stat             st;

	char                    buf[256];

	char                    fullpath[1024];
	char                    acl[2048];
	long                    aclvalue;

	char *                  permission = NULL;
	char                    hidden = ' ';
	char *                  type;
	int                     ret;
	smb_result              returnstatus;
	smb_result              tempstatus;

	int                     successfulcount = 0;
	int                     shareerrorcount = 0;

		//Create some buffers for us to use later. 
		char    host[64] = "";
		char    share[128] = "";
		char    object[512] = "";


	//If we're at the maximum recursion depth as set on the command line, stop
	if(depth == maxdepth) {
		returnstatus.code = -2;
		returnstatus.message = "Recursion depth reached.";
		returnstatus.succeeds = 0;
		returnstatus.fails = 0;

		return returnstatus;
	}

#ifdef DEBUG
	fprintf(stdout, "Attempting to browse to '%s'\n", path);
#endif

	//Try and get a directory listing of the object we just opened.
	//This could be a workgroup, server, share, or directory and
	//we'll get the full listing.  If it doesn't work, return our error.
	//Errors will happen a lot in normal usage due to access denied.
	if ((fd = smbc_getFunctionOpendir(ctx)(ctx, path)) == NULL) {
		returnstatus.code = errno;
		returnstatus.message = strerror(errno);
		returnstatus.succeeds = 0;
		returnstatus.fails = 1;

		//Parse this out for the error if we got one
		parsesmburl(path, host, share, object);

		fprintf(outfh, "%s,%s,%s,,ERROR (%d): %s,\n", host, share, object, errno, strerror(errno));
		return returnstatus;
	}

	//Get the current entity of the directory item we're working on.
	while ((dirent = smbc_getFunctionReaddir(ctx)(ctx, fd)) != NULL) {
		//Check to see if what we're working on is blank, or one of the 
		//special directory characters. If so, skip them.
		if(strcmp(dirent->name, "") == 0) continue;
		if(strcmp(dirent->name, ".") == 0) continue;
		if(strcmp(dirent->name, "..") == 0) continue;

		//Create the full path for this object by concating it with the 
		//parent path.
		sprintf(fullpath, "%s/%s", path, dirent->name);

		//Parse out the various parts of the path for pretty output.
		parsesmburl(fullpath, host, share, object);

		//Depending on the type of object we're looking at do various things.
		//Most of them are just to set a string for our output, but some 
		//have other things.
		switch(dirent->smbc_type) {
			case SMBC_WORKGROUP:
				type = "WORKGROUP";
				break;
			case SMBC_SERVER:
				type = "SERVER";
				break;
			case SMBC_FILE_SHARE:
				type = "FILE_SHARE";
				//If its a share go ahead and recurse into it.  If we're at
				//max depth then we'll be fine. 
				tempstatus = browse(ctx, fullpath, outfh, maxdepth, depth + 1);
				successfulcount = successfulcount + tempstatus.succeeds;
				shareerrorcount = shareerrorcount + tempstatus.fails;
				break;
			case SMBC_PRINTER_SHARE:
				type = "PRINTER_SHARE";
				break;
			case SMBC_COMMS_SHARE:
				type = "COMMS_SHARE";
				break;
			case SMBC_IPC_SHARE:
				type = "IPC_SHARE";
				break;
			case SMBC_DIR:
				type = "DIR";
				//Same thing as above, if its a directory we'll want to recurse as well. 
				tempstatus = browse(ctx, fullpath, outfh, maxdepth, depth + 1);
				successfulcount = successfulcount + tempstatus.succeeds;
				shareerrorcount = shareerrorcount + tempstatus.fails;
				break;
			case SMBC_FILE:
				type = "FILE";
				break;
			case SMBC_LINK:
				type = "LINK";
				break;
			default: 
				type = "UNKNOWN";
				break;
		}

		//Get the "dos_attr.mode" extended attribute which is the file permissions.
		ret = smbc_getFunctionGetxattr(ctx)(ctx, fullpath, "system.dos_attr.mode", acl, sizeof(acl));

		//If we got a return of less than 0 that means it failed.  If we were trying to 
		//get info for IPC$ we only care about access denied, otherwise it throws tons of weird stuff.
		if (ret < 0 && errno != 13 && strcmp(dirent->name, "IPC$") != 0) {
			shareerrorcount++;
		} else {
			successfulcount++;
		}

		//The ACL is returned as a string pointer, but we need to convert it to a long so we can 
		//do binary comparison on the settings. 
		aclvalue = strtol(acl, NULL, 16);

		//If the error was 13, that means we got an access denied. 
		if (errno == 13) 
			permission = "ACCESS DENIED";
		//Next check the binary to see if we've only got read only permissions. 
		else if (aclvalue & SMBC_DOS_MODE_READONLY)
			permission = "READ";
		else 
		//Otherwise we have write permissions. 
			permission = "WRITE";

		//Also, check to see if the hidden flag is set, if so lets show it. 
		if (aclvalue & SMBC_DOS_MODE_HIDDEN)
			hidden = 'X';
		else
			hidden = ' ';

		//Finally lets print the output all nicely to our file. 
		fprintf(outfh, "%s,%s,%s,%s,%s,%c\n", host, share, object, type, permission, hidden);
	}

	//Try to close the directory that we had opened.  If it failed, it'll return > 0.
	if(smbc_getFunctionClosedir(ctx)(ctx, fd) > 0) {
		returnstatus.code = errno;
		returnstatus.message = strerror(errno);
		returnstatus.succeeds = successfulcount;
		returnstatus.fails = shareerrorcount;

	//If successful, then we'll need to determine if we had any failures on some of the sub objects.
	} else {
		//We got some info, but not all.  Prep the response for the user.
		if(shareerrorcount > 0) {
			snprintf(buf, sizeof(buf), "Error on %d items. Got %d items.", shareerrorcount, successfulcount);
			returnstatus.code = 0;
			returnstatus.message = buf;
			returnstatus.succeeds = successfulcount;
			returnstatus.fails = shareerrorcount;

		//We got everything we wanted, so lets let them know that too.
		} else {
			snprintf(buf, sizeof(buf), "Gathered information on %d items.", successfulcount);
			returnstatus.code = -1;
			returnstatus.message = buf;
			returnstatus.succeeds = successfulcount;
			returnstatus.fails = shareerrorcount;
		}
	}

	//Finally, we're done, lets return to the user. 
	return returnstatus;
}
