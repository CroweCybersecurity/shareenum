#include <unistd.h> //Unix standard libraries
#include <string.h> //C "string" library

/* Data type that includes the result for every object that is identified 
 * within our recursive scanning.
 * host   -> The hostname or IP of our target
 * share  -> The name of the share
 * object -> The full path in the share to the object
 * type   -> The type of object (printer, file share, etc.)
 * acl    -> string containing the permissions
 */
typedef struct smbresult {
	char host[256];
	char share[256];
	char *object;
	uint type;
	long acl;
} objectresult

/* So we're typically going to have a large number of results, we'll
 * keep a linked list of our results so we can have a dynamic number.
 * There are also several associated functions for management of the 
 * list.  
 * data -> The smbresult that contains our current data
 * next -> A link to the next item in the linked list
 */
typedef struct smbresultlist {
	smbresult      data;
	smbresultlist* next;
} smbresultlist

/* This function will add a smbresult to the beginning of our
 * linked list.  In the event there isn't anything, it will 
 * create the list.  
 * PARAMETERS: 
 *   head -> A pointer to the first item in our linked list
 *   data -> The smbresult that we want to add
 * RETURN (void): None
 */
void smbresultlist_push(smbresultlist** head, smbresult data);

/* This function will pull the first item off the linked list,
 * reset the head, and free the item.  Useful to iterate through
 * items on our list when we're done.
 * PARAMETERS: 
 *   head -> A pointer to the first item in our linked list
 *   data -> The smbresult that we're gathering to use.  This
 *           will be freed and won't be on the list.
 * RETURN (int): 0 if we failed and 1 if we succeeded.
 */
int smbresultlist_pop(smbresultlist** head, smbresult *data);

/* Takes two lists of smbresults and merges them together in no 
 * particular order. 
 * PARAMETERS: 
 *   dst -> A pointer to the first item it the destination list.
 *   src -> A pointer to the first item in the source list.
 * RETURN (int): 0 if we failed and 1 if we succeeded.
 */
int smbresultlist_merge(smbresultlist** dst, smbresultlist* src);

/* Given a head pointer to the list, this will return the 
 * number of items currently on it.  
 * PARAMETERS: 
 *   head -> A pointer to the first item in our linked list
 * RETURN (uint): The number of items on our list
 */
uint smbresultlist_length(smbresultlist* head);

/* Given a pointer to the head of the list, this will iterate
 * through the entire list and destroy each item, freeing as
 * it goes.  This will destroy the list.
 * PARAMETERS: 
 *   head -> A pointer to the first item in our linked list
 * RETURN (void): None
 */
void smbresultlist_freeall(smbresultlist* head);