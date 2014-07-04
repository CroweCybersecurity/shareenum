#include <unistd.h> //Unix standard libraries
#include <string.h> //C "string" library
#include <stdlib.h> //GNU Standard Lib

/* Data type that includes the result for every object that is identified 
 * within our recursive scanning.
 * host       - The hostname or IP of our target
 * share      - The name of the share
 * object     - The full path in the share to the object
 * type       - The type of object (printer, file share, etc.)
 * acl        - string containing the permissions
 * statuscode - The result of pulling object data, 0 for success and >0 for smb error codes, <0 for our error codes.
 */
typedef struct smbresult {
	char* host;
	char* share;
	char* object;
	int   type;
	long  acl;
	int   statuscode;
} smbresult;

/* Create an empty smbresult with all of the strings pointing to empty strings ("")
 * and all of the other items set to 0.
 * PARAMETERS: None
 * RETURN (smbresult): A pointer to an empty SMB result
 */
smbresult* createSMBResultEmpty();

/* Create a smbresult with all of the methods and variables set.
 * PARAMETERS: 
 *   host       - Hostname
 *   share      - Share name
 *   object     - Full path to the object
 *   type       - Type of object
 *   acl        - ACL for the object
 *   statuscode - The return code for the object, 0 for success and >0 as error codes
 * RETURN (smbresult): A pointer to the smbresult
 */
smbresult* createSMBResult(char* host, char* share, char* object, int type, long acl, int statuscode);

/* So we're typically going to have a large number of results, we'll
 * keep a linked list of our results so we can have a dynamic number.
 * There are also several associated functions for management of the 
 * list.  
 * data - The smbresult that contains our current data
 * next - A link to the next item in the linked list
 */
typedef struct smbresultlist {
	smbresult*            data;
	struct smbresultlist* next;
} smbresultlist;

/* This function will add a smbresult to the beginning of our
 * linked list.  In the event there isn't anything, it will 
 * create the list.  
 * PARAMETERS: 
 *   head - A pointer to the first item in our linked list
 *   data - The smbresult that we want to add
 * RETURN (void): None
 */
void smbresultlist_push(smbresultlist** head, smbresult *data);

/* This function will pull the first item off the linked list,
 * reset the head, and free the item.  Useful to iterate through
 * items on our list when we're done.
 * PARAMETERS: 
 *   head - A pointer to the first item in our linked list
 *   data - The smbresult that we're gathering to use.  This
 *           will be freed and won't be on the list.
 * RETURN (int): 0 if we failed and 1 if we succeeded.
 */
int smbresultlist_pop(smbresultlist** head, smbresult **data);

/* Takes two lists of smbresults and merges them together in no 
 * particular order. 
 * PARAMETERS: 
 *   dst - A pointer to the first item it the destination list.
 *   src - A pointer to the first item in the source list.
 * RETURN (int): 0 if we failed and 1 if we succeeded.
 */
uint smbresultlist_merge(smbresultlist** dst, smbresultlist** src);

/* Given a head pointer to the list, this will return the 
 * number of items currently on it.  
 * PARAMETERS: 
 *   head - A pointer to the first item in our linked list
 * RETURN (uint): The number of items on our list
 */
uint smbresultlist_length(smbresultlist* head);

/* Given a pointer to the head of the list, this will iterate
 * through the entire list and destroy each item, freeing as
 * it goes.  This will destroy the list.
 * PARAMETERS: 
 *   head - A pointer to the first item in our linked list
 * RETURN (void): None
 */
void smbresultlist_freeall(smbresultlist* head);
