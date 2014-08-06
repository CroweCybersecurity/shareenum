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
 *   ace  - An access control entry string in the format PRINCIPAL:TYPE/FLAGS/MASK
 * RETURN (size_t): The size of the string we put in buf.  0 if error
 */
size_t smbresult_tocsv(smbresult data, char **buf, char *ace);

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
void parse_smburl(char *url, char **host, char **share, char **object);

/* Parse the type (Dir, File, etc.) of targeted object into something human readable
 * PARAMETERS: 
 *   uint - The smbc_type of the current object
 * RETURN (char *): The human readable name of the type
 */
char * parse_type(int type);

/* Parse the ACL and determine the type of access we have (write, read only, etc.)
 * PARAMETERS: 
 *   ace    - The access control entity string in the format Principle:Type/Flag/Access Mask
 *   print  - Pointer to the string for the user principle
 *   atype  - Pointer to the int that will hold the access mask type
 *   aflags - Pointer to hold the bitmask of ACE flags
 *   amask  - Pointer to hold the access mask
 * RETURN (int): 1 if we were successful, 0 if we failed
 */
int parse_acl(const char * ace, char ** princ, unsigned int * atype, unsigned int * aflags, unsigned int * amask);

/* Parse the access mask into a string that can be output
 * PARAMETERS: 
 *   amask - The bitmask of access flags to parse
 * RETURN (char *): A string containing all of the access values
 */
char * parse_accessmask(unsigned int amask);

/* Parse the ACL to determine if the hidden flag is set
 * PARAMETERS: 
 *   long - An ACL that we have received from Samba, represented as a series of bytes
 * RETURN (char *): A string containing our human readable access
 */
uint parse_hidden(long acl);

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
static void auth_fn(    const char *pServer,
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


/* ACE TYPE CONSTANTS
 * Constants for the type field within ACE strings
 */
#define ACE_TYPE_ALLOW                    0
#define ACE_TYPE_DENY                     1

/* ACE FLAG CONSTANTS
 * Flags within the ACE strings
 */
#define ACE_FLAG_OBJECT_INHERIT           1
#define ACE_FLAG_CONTAINER_INHERIT        2
#define ACE_FLAG_NO_PROPAGATE_INHERIT     4
#define ACE_INHERIT_ONLY                  8
#define ACE_INHERITED                     16

/* ACCESS MASK CONSTANTS
 *
 * Generic access rights 
 *
 * These are required by all object types and are mapped to a set
 * of general or generic standard/object specific access rights
 * which are shown in the in-line comments
 *
 * Bytes 28-31 - Generic Items
 * Bytes 25-27 - Unused / Reserved
 * Byte  24    - SACL 
 */
#define ACCESS_GENERIC_READ               0x80000000 //FILE_READ_ATTRIBUTES && FILE_READ_DATA && FILE_READ_EA && STANDARD_RIGHTS_READ && SYNCHRONIZE
#define ACCESS_GENERIC_WRITE              0x40000000 //FILE_APPEND_DATA && FILE_WRITE_ATTRIBUTES && FILE_WRITE_DATA && FILE_WRITE_EA && STANDARD_RIGHTS_WRITE && SYNCHRONIZE 
#define ACCESS_GENERIC_EXECUTE            0x20000000 //FILE_EXECUTE && FILE_READ_ATTRIBUTES && STANDARD_RIGHTS_EXECUTE && SYNCHRONIZE 
#define ACCESS_GENERIC_ALL                0x10000000 //GENERIC_READ && GENERIC_WRITE && GENERIC_EXECUTE
#define ACCESS_SYSTEM_SECURITY            0x01000000 //Allows access to get or set the SACL for the object

/* Standard access rights 
 *
 * These are the standard rights that all objects will have at a 
 * minimum and map to very high level access, such as modifying 
 * the access or deleting the object
 * 
 * Bytes 16-20 - Used
 * Bytes 21-23 - Appear unused at this point
 */
#define ACCESS_STANDARD_SYNCHRONIZE       0x00100000 //Allow the program to wait until our object is in a signaled state
#define ACCESS_STANDARD_WRITE_OWNER       0x00080000 //The right to change the owner
#define ACCESS_STANDARD_WRITE_DAC         0x00040000 //The right to change the DACL of the object
#define ACCESS_STANDARD_READ_CONTROL      0x00020000 //The right to read the security descriptor
#define ACCESS_STANDARD_DELETE            0x00010000 //The right to delete the object

/* Object specific access rights
 *
 * These are for the specific access rights (in our case) for
 * files and directories which are what we care about.  The 
 * bits are the same for files and directories, so bit 1
 * is READ for files and LIST for directories but is the same
 * bit because the access is the same.  
 *
 * Bytes 0-8  - Used
 * Bytes 9-16 - Appear unused at this point
 */
#define ACCESS_FILE_WRITE_ATTRIBUTES      0x00000100 //BOTH - Write attributes to the file (archive, read only, etc.)
#define ACCESS_FILE_READ_ATTRIBUTES       0x00000080 //BOTH - Read the attributes set ont he file
#define ACCESS_FILE_DELETE_CHILD          0x00000040 //BOTH - Allows the delete of an entire directory tree, including files inside even if they're read only
#define ACCESS_FILE_EXECUTE               0x00000020 //FILE - Run and/or execute the file
#define ACCESS_FILE_TRAVERSE              0x00000020 //DIR  - The right to move into a directory, BYPASS_TRAVERSE_CHECKING user right can bypass this
#define ACCESS_FILE_WRITE_EA              0x00000010 //BOTH - Write extended attributes 
#define ACCESS_FILE_READ_EA               0x00000008 //BOTH - Read extended attributes
#define ACCESS_FILE_APPEND_DATA           0x00000004 //FILE - Allow appending to the file (will not overwrite data)
#define ACCESS_FILE_ADD_SUBDIRECTORY      0x00000004 //DIR  - Allow creating subdirectories
#define ACCESS_FILE_WRITE_DATA            0x00000002 //FILE - Allow writing data into a file
#define ACCESS_FILE_ADD_FILE              0x00000002 //DIR  - Allow adding a file to a directory
#define ACCESS_FILE_READ_DATA             0x00000001 //FILE - Allow opening and reading from a file
#define ACCESS_FILE_LIST_DIRECTORY        0x00000001 //DIR  - Allow reading a file list from a directory

