/* This file contains structures and variables used by LAG framework */


#include <linux/module.h>
#include <linux/sched.h>
#include <linux/lag.h>

struct sched_job_lag lag_job = {
	.REQ = 0,
	.task = NULL,
};

EXPORT_SYMBOL(lag_job);
