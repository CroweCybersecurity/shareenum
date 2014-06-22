#include "smb.h"

browseresult runtarget(char * target, int maxdepth) {
	SMBCCTX *       context;
	char            buf[256];
	browseresult    res;

	//Try to create a context, if it's null that means we failed, so let the user know.
	if((context = create_context()) == NULL) {
		res.code = 1;
		res.message = "Unable to create samba context.\n";

		return res;
	}

	//Check to see if the target has smb:// in front, if not add it.
	if(strncmp("smb://", target, 6) != 0) {
		snprintf(buf, sizeof(buf), "smb://%s", target);
	} else {
		strncpy(buf, target, strlen(target) + 1);
	}

	//Browse to our file and get the goods
	res = browse(context, buf, maxdepth, 0);

	//Delete our context, so there's less segfaults.
	delete_context(context);
	return res;
}

static browseresult browse(SMBCCTX *ctx, char * path, int maxdepth, int depth) { 
	SMBCFILE *              fd;
	struct smbc_dirent *    dirent;

	char                    fullpath[2560] = "";
	char                    acl[1024] = "";
	long                    aclvalue;

	browseresult			thisstatus;
	browseresult            subresults;

	//Try and get a directory listing of the object we just opened.
	//This could be a workgroup, server, share, or directory and
	//we'll get the full listing.  If it doesn't work, return our error.
	//Errors will happen a lot in normal usage due to access denied.
	if ((fd = smbc_getFunctionOpendir(ctx)(ctx, path)) == NULL) {
		thisstatus.code = errno;
		thisstatus.message = strerror(errno);
		return thisstatus;
	}

	//Get the current entity of the directory item we're working on.
	while ((dirent = smbc_getFunctionReaddir(ctx)(ctx, fd)) != NULL) {
		smbresult           thisresult;
		//Check to see if what we're working on is blank, or one of the 
		//special directory characters. If so, skip them.
		if(strcmp(dirent->name, "") == 0) continue;
		if(strcmp(dirent->name, ".") == 0) continue;
		if(strcmp(dirent->name, "..") == 0) continue;

		//Create the full path for this object by concating it with the 
		//parent path.
		sprintf(fullpath, "%s/%s", path, dirent->name);

		//Parse out the various parts of the path for pretty output.
		parsesmburl(fullpath, thisresult.host, thisresult.share, thisresult.object);

		//Set the type so we have it
		thisresult.type = dirent->smbc_type;

		//Get the "dos_attr.mode" extended attribute which is the file permissions.
		smbc_getFunctionGetxattr(ctx)(ctx, fullpath, "system.dos_attr.mode", acl, sizeof(acl));
		if(errno == 13) {
			thisresult.acl = -1;
		} else {
			//The ACL is returned as a string pointer, but we need to convert it to a long so we can 
			//do binary comparison on the settings eventually.
			thisresult.acl = strtol(acl, NULL, 16);
		}

		smbresultlist_push(&thisstatus.results, thisresult);

		//If we have a directory or share we want to recurse to our max depth
		if(depth < maxdepth) {
			switch (dirent->smbc_type) {
				case SMBC_FILE_SHARE:
				case SMBC_DIR:
					subresults = browse(ctx, fullpath, maxdepth, depth++);
					smbresultlist_merge(&thisstatus.results, &subresults.results);
			}
		}
	}


	//Try to close the directory that we had opened.  If it failed, it'll return > 0.
	if(smbc_getFunctionClosedir(ctx)(ctx, fd) > 0) {
		thisstatus.code = errno;
		thisstatus.message = strerror(errno);
		thisstatus.results = NULL;

	//If successful, then we'll need to determine if we had any failures on some of the sub objects.
	} else {
		thisstatus.code = 0;
		thisstatus.message = "We successfully got some results";
	}

	//Finally, we're done, lets return to the user. 
	return thisstatus;
}

void parsesmburl(char * url, char * host, char * share, char * object) {
	char         buf[2048];
	char *       token;
	char *       last;
	const char   sep[2] = "/";

	strcpy(host, "");
	strcpy(share, "");
	strcpy(object, "");

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
	host[strlen(token) + 1] = '\0';

	//Get the second token, it should be our share. 
	token = strtok(NULL, sep);
	if(token == NULL)  {
		return;
	}

	//Set it to the share pointer. 
	strncpy(share, token, strlen(token)+1);
	share[strlen(token) + 1] = '\0';

	//Now loop through the rest of the tokens and make the 
	//file path.  Setting it to the directory pointerj
	while (token != NULL) {
		token = strtok(NULL, sep);
		if(token != NULL)
			sprintf(object, "%s/%s", object, token);
	}
}

char * parsetype(uint type) {
		//We need to translate the type to something readable for our output
		switch(type) {
			case SMBC_WORKGROUP:
				return "WORKGROUP";
			case SMBC_SERVER:
				return "SERVER";
			case SMBC_FILE_SHARE:
				return "FILE_SHARE";
			case SMBC_PRINTER_SHARE:
				return "PRINTER_SHARE";
			case SMBC_COMMS_SHARE:
				return "COMMS_SHARE";
			case SMBC_IPC_SHARE:
				return "IPC_SHARE";
			case SMBC_DIR:
				return "DIR";
			case SMBC_FILE:
				return "FILE";
			case SMBC_LINK:
				return "LINK";
			default: 
				return "UNKNOWN";
		}
}

char * parseacccess(long acl) {
	//Next check the binary to see if we've only got read only permissions. 
	if (acl & SMBC_DOS_MODE_READONLY)
		return "READ";
	else 
	//Otherwise we have write permissions. 
		return "WRITE";
}

uint parsehidden(long acl) {
	//Check to see if the hidden flag is set, if so lets return it
	if (acl & SMBC_DOS_MODE_HIDDEN)
		return 1;
	else
		return 0;
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

	//Create the Samba context struct , if it didn't work quit. 
	if((ctx = smbc_new_context()) == NULL)
		return NULL;

#ifdef DEBUG
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
	smbc_setTimeout(ctx, 200);
	//If we got a hash at the command line, let the context know we're
	//giving it a hash
	smbc_setOptionUseNTHash(ctx, gPassIsHash);

	//Initialize our context with the options that we have set or null on fail.
	if(smbc_init_context(ctx) == NULL) {
		smbc_free_context(ctx, 1);
		return NULL;
	}

	return ctx;
}

static void delete_context(SMBCCTX* ctx) {
	//Trying to fix the error:  no talloc stackframe at ../source3/libsmb/cliconnect.c:2637, leaking memory
	TALLOC_CTX *frame = talloc_stackframe();

	//First we need to purge the cache of servers the context has.
	//This should also free all the used memory allocations.
	smbc_getFunctionPurgeCachedServers(ctx)(ctx);

	//We're done with the frame, free it up now.
	TALLOC_FREE(frame);

	//Next we need to free the context itself, and free all the 
	//memory it used.
	smbc_free_context(ctx, 1);
}


