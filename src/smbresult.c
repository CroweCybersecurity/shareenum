#include "smbresult.h"

smbresult* createSMBResultEmpty() {
	return createSMBResult("", "", "", 0, "", 0, 0);
}

smbresult* createSMBResult(char *host, char *share, char *object, int type, char *acl, unsigned int mode, int statuscode) {
	smbresult *tmp = malloc(sizeof(smbresult));
	tmp->host = strdup(host);
	tmp->share = strdup(share);
	tmp->object = strdup(object);
	tmp->type = type;
	tmp->acl = acl;
	tmp->mode = mode;
	tmp->statuscode = statuscode;
	return tmp;
}

void smbresultlist_push(smbresultlist **head, smbresult *data) {
	//Here we declare our new node for our list
	smbresultlist *newnode = malloc(sizeof(smbresultlist)); 

	newnode->data = data;      //We take the new data and store it
	newnode->next = *head;     //Then we set the next node to our last one
	*head = newnode;           //And set our current node to the new one	
}

int smbresultlist_pop(smbresultlist **head, smbresult **data) {
	smbresultlist *tmp = NULL;        //Tmp variable to hold our list head
	tmp = *head;               //Set the head to the tmp

	if(tmp == NULL) {          //If tmp is null, we've reached the end.
		return 0;
	}

	*head = tmp->next;         //Otherwise, set the head to the next item
	*data = tmp->data;         //Extract the data to the data pointer

	free(tmp);                 //And free the memory

	return 1;                  //Finally, return that it worked
}

uint smbresultlist_merge(smbresultlist **dst, smbresultlist **src) {
	smbresult *tmp;             //Tmp variable to hold the current data

	//Pop the current one of src and push it onto dst
	while(smbresultlist_pop(src, &tmp) > 0) {
		smbresultlist_push(dst, tmp);
	}
}

uint smbresultlist_length(smbresultlist *head) {
	uint len = 0;              //Tmp variable to hold the length
	smbresultlist *cur;        //Tmp variable to hold the current pointer

	//Loop through each pointer and set cur to the next pointer	
	for(cur = head; cur != NULL; cur = cur->next) {
		len++;                 //Add to our counter
	}

	return len;                //And now we have the length
}

void smbresultlist_freeall(smbresultlist *head) {
	smbresultlist *tmp;        //Tmp variable to hold the head pointer

	while(head != NULL) {      //Loop through each item
		tmp = head;            //Get the current smbresultlist
		head = head->next;     //Set head to the next one
		free(tmp);             //And free the current one
	}
}
