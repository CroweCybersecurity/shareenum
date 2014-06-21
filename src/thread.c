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

#include "thread.h"

static pthread_mutex_t mutex_gather = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t mutex_outfile = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t mutex_context = PTHREAD_MUTEX_INITIALIZER;

static FILE * request_thread_outfh;

static int request_thread_total = 0;
static int request_thread_index = 0;

struct thread_request * requests_head = NULL;
struct thread_request * requests_last = NULL;

void request_thread_prepare(FILE * in, FILE * out) {
	request_thread_total = file_countlines(in);
	request_thread_outfh = out;

#ifdef DEBUG
	fprintf(stdout, "Preparing the thread queue, identified %d lines in file.\n", request_thread_total);
#endif

	char        buf [512];
	int         i = 0;
	int         j = 0;

	while ( fgets ( buf, sizeof(buf), in ) != NULL ) {
		if (buf[strlen(buf) - 1] == '\n') {
			buf[strlen(buf) - 1] = '\0';
		}
		if (buf[strlen(buf) - 1] == '\r') {
			buf[strlen(buf) - 1] = '\0';
		}

		struct thread_request* new_request = (struct thread_request *)malloc(sizeof(struct thread_request *));
		new_request->id = i;
		strcpy(new_request->path, buf);
		new_request->next = 0;

		if(requests_head == NULL) {
#ifdef DEBUG
			fprintf(stdout, "Creating the first item of the list.\n");
#endif
			requests_head = requests_last = new_request;
		} else {
			requests_last->next = new_request;
			requests_last = new_request;
#ifdef DEBUG
			fprintf(stdout, "Adding %s to the queue\n", requests_last->path);
#endif
		}
		i++;
	}
}

struct thread_request * request_thread_get_next(pthread_mutex_t * p_mutex) {
	int                       rc;
	struct thread_request *   next_request;

	rc = pthread_mutex_lock(p_mutex);

	if(request_thread_index <= request_thread_total) {
		next_request = requests_head;
		requests_head = next_request->next;

		if(requests_head == NULL)
			requests_last = NULL;

#ifdef DEBUG
		fprintf(stdout, "Retrieved next target of %s\n", next_request->path);
#endif
		request_thread_index++;
	} else {
		next_request = NULL;
	}

	rc = pthread_mutex_unlock(p_mutex);

	return next_request;
}

void * request_thread_worker(void * arg) {
	int                      rc;
	int                      total_digits = numdigits(request_thread_total);
	smb_result               res;
	thread_data *            data = (thread_data *) arg;
	struct thread_request *  target;


#ifdef DEBUG
	fprintf(stdout, "Thread %d starting up.\n", data->id);
#endif

	while(1) {
		if(request_thread_index <= request_thread_total) {
#ifdef DEBUG
			fprintf(stdout, "Thread %d requesting next target.\n", data->id);
#endif

			target = request_thread_get_next(&mutex_gather);

			if(target) {
#ifdef DEBUG
				fprintf(stdout, "Thread %d running against target %s.\n", data->id, target);
#endif
				res = runhost(target->path, request_thread_outfh, &mutex_context);

				rc = pthread_mutex_lock(&mutex_outfile);
				if(res.code > 0) {
					fprintf(stdout, ANSI_COLOR_BLUE "(%*d/%d) " ANSI_COLOR_CYAN "%-30s" ANSI_COLOR_RESET "[" ANSI_COLOR_RED "!" ANSI_COLOR_RESET "] %s (Code: %d)\n", total_digits, request_thread_index, request_thread_total, target->path, res.message, res.code);
				} else if (res.code == 0) {
					fprintf(stdout, ANSI_COLOR_BLUE "(%*d/%d) " ANSI_COLOR_CYAN "%-30s" ANSI_COLOR_RESET "[" ANSI_COLOR_YELLOW "!" ANSI_COLOR_RESET "] %s\n", total_digits, request_thread_index, request_thread_total, target->path, res.message);
				} else {
					fprintf(stdout, ANSI_COLOR_BLUE "(%*d/%d) " ANSI_COLOR_CYAN "%-30s" ANSI_COLOR_RESET "[" ANSI_COLOR_GREEN "!" ANSI_COLOR_RESET "] %s\n", total_digits, request_thread_index, request_thread_total, target->path, res.message);
				}
				rc = pthread_mutex_unlock(&mutex_outfile);
			}
		} else {
			pthread_exit(NULL);
		}
	}
}

