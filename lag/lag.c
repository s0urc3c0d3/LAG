/* This file contains structures and variables used by LAG framework */


#include <linux/module.h>
#include <linux/sched.h>
#include <linux/lag.h>

lag_wait_queue lag_wait = {
	.next = NULL,
	.tsk = NULL,
	.rq = NULL,
	.prev = NULL,
};
EXPORT_SYMBOL(lag_wait);

struct sched_job_lag lag_job = {
	.REQ = 0,
	.wait_queue = NULL,
	.pid = 0,
};

EXPORT_SYMBOL(lag_job);

void lag_wait_queue_add(lag_wait_queue *ent, lag_wait_queue *list)
{
	lag_wait_queue *list_next = list->prev;
	list_next->prev = ent;
	ent->next=list_next;
	ent->prev=list;
	list->next=ent;
}

void lag_wait_queue_del(lag_wait_queue *ent)
{
	ent->next->prev = ent->prev;
	ent->prev->next = ent->next;
	ent->next=NULL;
	ent->prev=NULL;
}

void lag_debug_wait_queue(lag_wait_queue *ent)
{
//asm("#1");
	printk(KERN_DEBUG "DEBUG WAIT QUEUE START \n");
//asm("#2");
	if (ent == NULL)
//asm("#3");
	printk(KERN_DEBUG "kolejka pusta \n");
	else
	{
//asm("#4");
		int ent_pid=ent->tsk->pid;
//asm("#5");
		lag_wait_queue *tmp=ent;
//asm("#6");
		do {
//asm("#7");
			printk(KERN_DEBUG "entry pid %i \n",tmp->tsk->pid);
//asm("#8");
			if (tmp->next == NULL) printk(KERN_DEBUG "entry next is null! \n"); else printk(KERN_DEBUG "entry next pid %i \n",tmp->next->tsk->pid);
//asm("#9");
			if (tmp->prev == NULL) printk(KERN_DEBUG "entry prev is null! \n"); else printk(KERN_DEBUG "entry prev pid %i \n",tmp->prev->tsk->pid);
//asm("#0");
			tmp=tmp->next;
//asm("#a");
		}
		while (tmp != NULL && tmp->tsk->pid != ent_pid);
//asm("#c");
	}
//asm("#d");
	printk(KERN_DEBUG "DEBUG WAIT QUEUE STOP \n");
}
