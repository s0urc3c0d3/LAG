/* This file contains structures and variables used by LAG framework */


#include <linux/sched.h>

struct sched_job {
	int REQ;
	struct task_struct *task;
};

struct sched_job lag_job = {
	.REQ = 0,
	.task = NULL,
};
