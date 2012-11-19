#include <linux/wait.h>

#define GET_PROCESS_STATE 0
#define SET_PROCESS_STATE 1
#define GET_PROCESS_CONTEXT 2
#define SET_PROCESS_CONTEXT 3

#define __wait_event_interruptible_process(wq, condition, task,ret)                  \
do {                                                                    \
	DEFINE_WAIT(__wait);                                            \
                                                                        \
	for (;;) {                                                      \
		prepare_to_wait(&wq, &__wait, TASK_INTERRUPTIBLE);      \
		if (condition)                                          \
			break;                                          \
		if (!signal_pending(task)) {                         \
			schedule();                                     \
			continue;                                       \
		}                                                       \
		ret = -ERESTARTSYS;                                     \
		break;                                                  \
	}                                                               \
	finish_wait(&wq, &__wait);                                      \
} while (0)

#define wait_event_interruptible_process(wq, condition, task)                         \
({                                                                      \
	int __ret = 0;                                                  \
	if (!(condition))                                               \
		__wait_event_interruptible_process(wq, condition, task,__ret);       \
	__ret;                                                          \
})

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
