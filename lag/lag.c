/* This file contains structures and variables used by LAG framework */


#include <linux/module.h>
#include <linux/sched.h>
#include <linux/lag.h>

lag_wait_queue lag_wait = {
	.next = NULL,
	.tsk = NULL,
	.prev = NULL,
};
EXPORT_SYMBOL(lag_wait);

struct sched_job_lag lag_job = {
	.REQ = 0,
	.task = NULL,
	.curr = NULL,
	.tpid=0,
	.cpid=0,
};

EXPORT_SYMBOL(lag_job);

void lag_wait_queue_add(lag_wait_queue *list, lag_wait_queue *ent)
{
	lag_wait_queue *tmp = list->prev;
	list->prev = ent;
        ent->next = list;
        ent->prev = tmp;
        tmp->next = ent;
}

void lag_wait_queue_del(lag_wait_queue *ent)
{
	ent->next = ent->prev;
	ent->prev = ent->next;
}
