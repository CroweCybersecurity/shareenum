#include "smbresult.h"

void smbresultlist_push(smbresultlist** head, smbresult data) {
	smbresultlist* newnode = malloc(sizeof(smbresultlist));

	newnode->data = data;
	newnode->next = *head;
	*head = newnode;
}

int smbresultlist_pop(smbresultlist** head, smbresult* data) {
	smbresultlist* tmp;
	tmp = *head;

	if(tmp == NULL) {
		return 0;
	}

	*head = tmp->next;
	*data = tmp->data;

	free(tmp);

	return 1;
}

uint smbresultlist_length(smbresultlist* head) {
	uint len = 0;

	smbresultlist* cur;
	for(cur = head; cur != NULL; cur = cur->next) {
		len++;
	}

	return len;
}

void smbresultlist_freeall(smbresultlist* head) {
	smbresultlist* tmp;

	while(head != NULL) {
		tmp = head;
		head = head->next;
		free(tmp);
	}
}