#include "smb.h"

smbresultlist* runtarget(char *target, int maxdepth) {
	SMBCCTX         *context;
	char            buf[256];
	smbresultlist   *res = NULL;

	//Try to create a context, if it's null that means we failed, so let the user know.
	if((context = create_context()) == NULL) {
		smbresult *tmp = createSMBResultEmpty();
		parse_smburl(target, &tmp->host, &tmp->share, &tmp->object);
		tmp->statuscode = errno;
		smbresultlist_push(&res, tmp);
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

static smbresultlist* browse(SMBCCTX *ctx, char *path, int maxdepth, int depth) { 
	SMBCFILE                *fd;
	struct smbc_dirent      *dirent;

	char                    fullpath[2560] = "";

	char                    acl[1024] = "";
	int                     aclret;

	char			mode[128] = "";
	int			moderet;

	smbresultlist           *thisresults = NULL;
	smbresultlist           *subresults = NULL;


	//Try and get a directory listing of the object we just opened.
	//This could be a workgroup, server, share, or directory and
	//we'll get the full listing.  If it doesn't work, return our error.
	//Errors will happen a lot in normal usage due to access denied.
	if ((fd = smbc_getFunctionOpendir(ctx)(ctx, path)) == NULL) {
		smbresult *tmp = createSMBResultEmpty();
		parse_smburl(path, &tmp->host, &tmp->share, &tmp->object);
		tmp->statuscode = errno;
		smbresultlist_push(&thisresults, tmp);
		return thisresults;
	}

	//Get the current entity of the directory item we're working on.
	while ((dirent = smbc_getFunctionReaddir(ctx)(ctx, fd)) != NULL) {
		smbresult *thisresult = createSMBResultEmpty();

		//Check to see if what we're working on is blank, or one of the 
		//special directory characters. If so, skip them.
		if(strcmp(dirent->name, "") == 0) continue;
		if(strcmp(dirent->name, ".") == 0) continue;
		if(strcmp(dirent->name, "..") == 0) continue;

		//Create the full path for this object by concating it with the 
		//parent path.
		sprintf(fullpath, "%s/%s", path, dirent->name);

		//Parse out the various parts of the path for pretty output.
		parse_smburl(fullpath, &thisresult->host, &thisresult->share, &thisresult->object);

		//Set the type so we have it
		thisresult->type = dirent->smbc_type;

		//Get the "dos_attr.mode" extended attribute which is the file permissions.
		moderet = smbc_getFunctionGetxattr(ctx)(ctx, fullpath, "system.dos_attr.mode", &mode, sizeof(mode));
		if(moderet == -1 && errno == 13) {
			thisresult->mode = -1;
		} else {
			//The ACL is returned as a string pointer, but we need to convert it to a long so we can 
			//do binary comparison on the settings eventually.
			thisresult->mode = strtol(acl, NULL, 16);
		}

		//Get the ACL ACEs for the NTFS permissions.  The + is so we lookup SIDs to names
		aclret = smbc_getFunctionGetxattr(ctx)(ctx, fullpath, "system.nt_sec_desc.acl.*+", acl, sizeof(acl));
		if(aclret < 0) {
			char permerrbuf[100];
			sprintf(permerrbuf, "Unable to pull permissions (%d): %s", errno, strerror(errno));
			thisresult->acl = strdup(permerrbuf);
		} else {
			thisresult->acl = strdup(acl);
		}

		smbresultlist_push(&thisresults, thisresult);

		//If we have a directory or share we want to recurse to our max depth
		if(depth < maxdepth) {
			switch (thisresult->type) {
				case SMBC_FILE_SHARE:
				case SMBC_DIR:
					subresults = browse(ctx, fullpath, maxdepth, depth++);
					smbresultlist_merge(&thisresults, &subresults);
			}
		}
	}


	//Try to close the directory that we had opened.  If it failed, it'll return > 0.
	if(smbc_getFunctionClosedir(ctx)(ctx, fd) > 0) {
		smbresult *tmp = createSMBResultEmpty();
		parse_smburl(path, &tmp->host, &tmp->share, &tmp->object);
		tmp->statuscode = errno;
		smbresultlist_push(&thisresults, tmp);
	}

	//Finally, we're done, lets return to the user. 
	return thisresults;
}

size_t smbresult_tocsv(smbresult data, char **buf, char *ace) {
	if(data.statuscode < 0) {
		return 0;
	}

	//parsehidden returns 0 or 1, so we need a quick if statement
	char hidden = ' ';
	if(parse_hidden(data.mode))
		hidden = 'X';

	//We need to parse the access entry, here are the variables we'll use to hold them
	char * principal = "";
	unsigned int atype = 0;
	unsigned int aflags = 0;
	unsigned int amask = 0;

	//Parse the entry, if we can't then just quit because we got bad data.
	if(ace != NULL) {
		if(parse_acl(ace, &principal, &atype, &aflags, &amask) == 0) {
			return 0;
		}
	}

	//We need to determine the length of our new string
	size_t size = snprintf(NULL, 0, "\"%s\",\"%s\",\"%s\",\"%s\",\"%s\",\"%s\",\"%c\"", 
		data.host, 
		data.share, 
		data.object, 
		parse_type(data.type),
		principal,
		parse_accessmask(amask),
		hidden
	);

	//Otherwise, just a simple sprintf to the buffer the user gave us.
	char *buffer = malloc(size+1);
	snprintf(buffer, size+1, "\"%s\",\"%s\",\"%s\",\"%s\",\"%s\",\"%s\",\"%c\"", 
		data.host, 
		data.share, 
		data.object, 
		parse_type(data.type),
		principal,
		parse_accessmask(amask),
		hidden
	);

	*buf = strdup(buffer);

	free(buffer);

	return size+1;
}

void parse_smburl(char *url, char **host, char **share, char **object) {
	char       buf[2048];
	char       *token;
	char       *last;
	const char sep[2] = "/";

	if(strncmp("smb://", url, 6) == 0) 
		strncpy(buf, url+6, strlen(url) - 5);
	else
		strncpy(buf, url, strlen(url));

	//Tokenize the URL by /, get the first one, it should be our host.
	token = strtok(buf, sep);
	if(token == NULL)
		return;
	*host = strdup(token);

	//Get the second one, it should be our share
	token = strtok(NULL, sep);
	if(token == NULL)
		return;
	*share = strdup(token);

	//Finally, we take the rest of the string and its our object path
	*object = strdup(token + strlen(token) + 1);
}

char* parse_type(int type) {
		//We need to translate the type to something readable for our output
		switch(type) {
			case -1:
				return "ERROR";
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

int parse_acl(const char * ace, char ** princ, unsigned int * i1, unsigned int * i2, unsigned int * i3) {
	unsigned int *atype = i1;
	unsigned int *aflags = i2;
	unsigned int *amask = i3;

	char *p = strchr(ace, ':');
	if(!p) {
		return 0;
	}
	*p = '\0';
	p++;

	*princ = malloc(strlen(ace) + 1);
	if(!*princ) {
		return 0;
	}
	strcpy(*princ, ace);

	if(sscanf(p, "%u/%u/%x", atype, aflags, amask) == 3) {
		return 1;
	} else {
		return 0;
	}
}

char * parse_accessmask(unsigned int acl) {
	char tmpaccess[128] = "";

	if(acl & ACCESS_FILE_READ_DATA)
		strcat(tmpaccess, "READ|");
	if(acl & ACCESS_FILE_WRITE_DATA)
		strcat(tmpaccess, "WRITE|");
	if(acl & ACCESS_FILE_APPEND_DATA)
		strcat(tmpaccess, "APPEND|");
	if(acl & ACCESS_FILE_READ_EA)
		strcat(tmpaccess, "READEA|");
	if(acl & ACCESS_FILE_WRITE_EA)
		strcat(tmpaccess, "WRITEEA|");
	if(acl & ACCESS_FILE_EXECUTE)
		strcat(tmpaccess, "EXECUTE|");
	if(acl & ACCESS_STANDARD_DELETE)
		strcat(tmpaccess, "DELETE|");

	if(tmpaccess[strlen(tmpaccess)-1] == '|') {
		tmpaccess[strlen(tmpaccess)-1] = '\0';
	}

	return strdup(tmpaccess);
}

uint parse_hidden(long acl) {
	//Check to see if the hidden flag is set, if so lets return it
	if (acl & SMBC_DOS_MODE_HIDDEN)
		return 1;
	else
		return 0;
}

static void auth_fn(
	const char      *pServer,
	const char      *pShare,
	char            *pWorkgroup,
	int             maxLenWorkgroup,
	char            *pUsername,
	int             maxLenUsername,
	char            *pPassword,
	int             maxLenPassword) {

	//Get our external globals that we got from the command line
	extern char     *gUsername;
	extern char     *gPassword;

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
	SMBCCTX  		*ctx;
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

static void delete_context(SMBCCTX *ctx) {
	//Trying to fix the error:  no talloc stackframe at ../source3/libsmb/cliconnect.c:2637, leaking memory
	//TALLOC_CTX *frame = talloc_stackframe();

	//First we need to purge the cache of servers the context has.
	//This should also free all the used memory allocations.
	smbc_getFunctionPurgeCachedServers(ctx)(ctx);

	//We're done with the frame, free it up now.
	//TALLOC_FREE(frame);

	//Next we need to free the context itself, and free all the 
	//memory it used.
	smbc_free_context(ctx, 1);
}


