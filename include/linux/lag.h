/* This file contains structures and variables used by LAG framework */


#include <linux/sched.h>


struct sched_job_lag {
	int REQ;
	struct task_struct *task;
};

extern struct sched_job_lag lag_job;