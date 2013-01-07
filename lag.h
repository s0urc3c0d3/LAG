#include <linux/sched.h>
#include <linux/wait.h>

#define GET_PROCESS_STATE 0
#define SET_PROCESS_STATE 1
#define GET_PROCESS_CONTEXT 2
#define SET_PROCESS_CONTEXT 3

struct lag_request {
	int REQID;
	int pid;
	int status;
};

struct lag_reply {
	void *data;
};

// short structure to do the job faster besides task_struct


// We do not need process context because tast_struct does the job!

struct lag_request_list {
	struct lag_request_list *next;
	struct lag_request_list *prev;
	struct lag_request *node;
};
/* This file contains structures and variables used by LAG framework */



struct __lag_wait_queue {
	struct task_struct *tsk;
	struct rq *rq;
	struct __lag_wait_queue *next;
	struct __lag_wait_queue *prev;

};

typedef struct __lag_wait_queue lag_wait_queue;

struct sched_job_lag {
	int REQ;
	lag_wait_queue *wait_queue;
	int pid;
};

extern lag_wait_queue lag_wait;

extern struct sched_job_lag lag_job;

// Functions definitions

/* Add a proces to LAG wait queue
 * this is copied list_add - I need to be sure that this structure
 * acts as it should and i did not want to waste time on bugs
 * because I use standard list */

extern void lag_wait_queue_add(lag_wait_queue *ent, lag_wait_queue *list); 

// delete process from list
extern void lag_wait_queue_del(lag_wait_queue *ent);

extern void lag_debug_wait_queue(lag_wait_queue *ent);
