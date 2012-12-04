/* This file contains structures and variables used by LAG framework */


#include <linux/sched.h>


struct __lag_wait_queue {
	struct task_struct *tsk;
	struct __lag_wait_queue *next;
	struct __lag_wait_queue *prev;

};
struct sched_job_lag {
	int REQ;
	struct task_struct *task;
	struct task_struct *curr;
	int tpid;
	int cpid;
};

typedef struct __lag_wait_queue lag_wait_queue;
extern lag_wait_queue lag_wait;

extern struct sched_job_lag lag_job;

// Functions definitions

/* Add a proces to LAG wait queue
 * this is copied list_add - I need to be sure that this structure
 * acts as it should and i did not want to waste time on bugs
 * because I use standard list */

extern void lag_wait_queue_add(lag_wait_queue *list, lag_wait_queue *ent);

// delete process from list
extern void lag_wait_queue_del(lag_wait_queue *ent);
