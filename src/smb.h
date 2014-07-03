#include <unistd.h> //Unix standard libraries
#include <dirent.h> //Directory entities
#include <errno.h> //Error numbers and information
#include <string.h> //C "string" library
#include <stdlib.h> //GNU standard library
#include <stdbool.h> //Enable C99 booleans for the talloc stuff in samba
#include <libsmbclient.h> //Samba client headers
#include <util/talloc_stack.h> //Samba TALLOC stack
#include "smbresult.h"

/* Run a check against a host.  This creates a Samba context, browses to the path
 * creates a result object, and then clears the context after its finished.  
 *
 * PARAMETERS:
 *   char * - Pointer to a string containing the full path to our target.
 *   int    - How deep into the structure should we go.
 *
 * RETURN (smb_result): The result of our run of this host.
 */
smbresultlist* runtarget(char *target, int maxdepth);

/* Takes a smbresult object and converts it to a string formatted in CSV
 * formatting including field terminators in the format: 
 * "host","share","object","type","permissions","hidden"
 * PARAMETERS: 
 *   data - The smbresult containing the data to format.
 *   buf  - Pointer to the string buffer where we should put the output.
 * RETURN (void): None
 */
void smbresult_tocsv(smbresult data, char *buf);

/* This is the function that browses a system and attempts to list all of the shares.
 *
 * PARAMETERS:
 *   SMBCCTX *  - Pointer to the samba context that should be used during browse
 *   char *     - The path to browse.  Should be something like "smb://IP -or- HOST/.
 *                If you give it a share it'll list the directories but can also do
 *                printers, files, directories, etc.
 *   int        - How many levels down should be go.
 *   int        - What is the current depth that we're at.
 *
 * RETURN (smb_result): The result of our run on this host.  Will be aggregated if
 *                recursion is in use.
 */
static smbresultlist* browse(SMBCCTX *ctx, char *path, int maxdepth, int depth);

/* Parse out a smb uri string into its various components.  Should typically be in 
 * the format of: smb://TARGET/SHARE/DIRECTORY/FILE
 *
 * PARAMETERS:
 *   char * - The current URL including all of the info we're splitting up
 *   char * - The host or IP that we're operating against.
 *   char * - The name of the share, printer, etc. that we've found.
 *   char * - The full path of the current object that we're browsing.
 * RETURN (void): None
 */
void parsesmburl(char *url, char **host, char **share, char **object);

/* Parse the type (Dir, File, etc.) of targeted object into something human readable
 * PARAMETERS: 
 *   uint - The smbc_type of the current object
 * RETURN (char *): The human readable name of the type
 */
char * parsetype(int type);

/* Parse the ACL and determine the type of access we have (write, read only, etc.)
 * PARAMETERS: 
 *   long - An ACL that we have received from Samba, represented as a series of bytes
 * RETURN (char *): A string containing our human readable access
 */
char * parseaccess(long acl);

/* Parse the ACL to determine if the hidden flag is set
 * PARAMETERS: 
 *   long - An ACL that we have received from Samba, represented as a series of bytes
 * RETURN (char *): A string containing our human readable access
 */
uint parsehidden(long acl);

/* The authentication function that will be passed into the Samba context.  Whenever
 * it needs to authenticate it will call this function to get the data we need.
 * The variables are the username, server info, share, etc. that we may authenticate
 * with and are set by reference for samba.
 *
 * PARAMETERS:
 *   These parameters are defined and set by the Samba libraries and cannot be changed. 
 *   They are mostly self explanatory and represent pointers to the various buffers
 *   that need to have information added for authentication.
 */
static void auth_fn(
			const char *pServer,
			const char *pShare,
			char *pWorkgroup,
			int maxLenWorkgroup,
			char *pUsername,
			int maxLenUsername,
			char *pPassword,
			int maxLenPassword);

/* Create a Samba context for us to handle all of our connections with. This will pull
 * several options from extern global variables set in the main function from the
 * command line.
 *
 * PARAMETERS: None
 *
 * RETURN (SMBCCTX *): Pointer to the samba context we have created.
 */
static SMBCCTX* create_context(void);

/* This will delete the a Samba context, clear the set of cached hosts that were stored
 * in memory, and then free the context's used memory. 
 *
 * PARAMETERS: 
 *   SMBCCTX * - Pointer to the Samba context that should be freed. 
 *
 * RETURN (void): None
 */
static void delete_context(SMBCCTX *ctx);

