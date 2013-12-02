/* NOT CURRENTLY USED IN THIS PROGRAM
 * The Samba libraries are non-reentrant and are not thread safe,
 * meaning that if you try and use more than 1 context at a time,
 * it defines system level globals and really shits the bed.
 *
 * After writing all of this lovely threading code, testing did not
 * go well at all... so, all of this code is useless for the time
 * being. It is worth looking into forking as Samba may work there,
 * but its fast enough as is now.
 */

#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <string.h>

#include "smb.h"
#include "helpers.h"
#include "colors.h"


struct thread_request {
	int id;
	char path[1024];
	struct thread_request * next;
};

typedef struct thread_data {
	int id;
} thread_data;

void request_thread_prepare(FILE * in, FILE * out);

struct thread_request * request_thread_get_next(pthread_mutex_t * p_mutex);

void * request_thread_worker(void * data);
