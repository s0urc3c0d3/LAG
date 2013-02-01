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

struct __lag_task_struct = {
	long state;
	atomic_t usage;
	unsigned int flags;
	unsigned int ptrace;
	int lock_depth;
	int oncpu;
	int prio, static_prio, normal_prio;
	unsigned int rt_priority;
	int sched_class; //0 - CFS, 1 - RT, 2 - IDLE
	struct sched_entity se;
	struct sched_rt_entity rt;

	unsigned char fpu_counter;
	s8 oomkilladj; /* OOM kill score adjustment (bit shift). */
	unsigned int btrace_seq; //to pole zalezy od  CONFIG_BLK_DEV_IO_TRACE
	unsigned int policy;
	cpumask_t cpus_allowed;

	// ponizsze 2 pola zaleza od CONFIG_PREEMPT_RCU
	int rcu_read_lock_nesting;
	int rcu_flipctr_idx;
	
	// struct sched_info sched_info; zalezy od defined(CONFIG_SCHEDSTATS) || defined(CONFIG_TASK_DELAY_ACCT) i jest do zbadania

	// mm_struct bedzie podawanie w innych strukturach

	//struct linux_binfmt *binfmt; - to pole pomijam bo obraz binarny musi byc dostepny w systemie w sposob nie zalezny ze wzgledu na SWAP
	int exit_state;
	int exit_code, exit_signal;
	int pdeath_signal;  /*  The signal sent when the parent dies  */
	
	unsigned int personality;
	unsigned did_exec:1;
	pid_t pid; //PIDy kopiujemy poniewaz moze uda sie uzyc starych
	pid_t tgid;

	unsigned long stack_canary; //zalezy od CONFIG_CC_STACKPROTECTOR

	//struktury rodzicow pomijam poniewaz prototyp LAG nie musi dbac o relacje rodzinne ale ma poprostu dzialac. Rodzicem bedzie tutaj init

	//struktury ptrace tez pomijam bo przenoszenie debugera to strzal w stope...pomijam tekze sekcje CONFIG_X86_PTRACE_BTS

	//nastepne 2 stuktury to pids typu pid_link oraz thread_roup typu list_Head. O ile pierwsza trzeba odtworzyc od 0 to druga umieszczam zakomentowana bo wymaga wyjasnien do czego sluzy...
	//struct list_head thread_group;

	// vfork_done - mozna olac bo raczej process nie bedzie w trakcie vfork (pozatym struktura completion zawiera wait_queue_headt_t
	// nie kopiuje set_child_tid oraz clear_child_tid - trzeba zbadac jaka wartosc maja defaultowo

	//cputime_t to zwykly unsigned long
	unsigned long utime, stime, utimescaled, stimescaled;
	unsigned long gtime;
	unsigned long prev_utime, prev_stime;
	unsigned long nvcsw, nivcsw;
	
	
};

typedef struct __lag_task_struct lag_task_struct;
