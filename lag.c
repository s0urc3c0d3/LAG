#include <linux/module.h>
#include <linux/fs.h>
#include <linux/device.h>
#include <linux/string.h>
#include <linux/sched.h>
#include "lag.h"
#include <asm/system.h>
#include <linux/lag.h>
//#include <../kernel/sched.c>

/*#define wait_event_target(wq, condition, target)                        \
do {                                                                    \
        if (condition)                                                  \
                break;                                                  \
        __wait_event_target(wq, condition, target);                     \
} while (0)

#define __wait_event_target(wq, condition, target)                      \
do {                                                                    \
        DEFINE_WAIT(__wait);                                            \
                                                                        \
        for (;;) {                                                      \
                prepare_to_wait_target(&wq, &__wait, TASK_UNINTERRUPTIBLE,target);    \
                if (condition)                                          \
                        break;                                          \
        }                                                               \
        finish_wait(&wq, &__wait);                                      \
} while (0)


#define set_target_state(target,state_value)          \
        set_mb(target->state, (state_value))

void prepare_to_wait_target(wait_queue_head_t *q, wait_queue_t *wait, int state, struct task_struct *target)
{
        unsigned long flags;

        wait->flags &= ~WQ_FLAG_EXCLUSIVE;
//        spin_lock_irqsave(&q->lock, flags);
        if (list_empty(&wait->task_list))
                list_add(q, target);
        set_target_state(target,state);
//        spin_unlock_irqrestore(&q->lock, flags);
}
EXPORT_SYMBOL(prepare_to_wait_target);

*/

struct task_struct * (*pick_next_task_orig) (struct rq *rq);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Grzegorz Dwornicki");
MODULE_DESCRIPTION("This module provides /dev/lag character device for Linux Agents project");

MODULE_VERSION("1.1.0");

const char *lagdev="/dev/lag";

int read=0;
int lag_pid=-2;
int state=-2;
int block=1;

int module_used=0;

struct class *cl;
void *c_dev;

struct sched_job_lag *fs=&lag_job;

DECLARE_WAIT_QUEUE_HEAD(lag_wq);

int lag_open(struct inode *lag_inode, struct file *lag_file);
int lag_release(struct inode *lag_inode, struct file *lag_file);
ssize_t lag_read(struct file *target_file, char __user *buf, size_t mlength, loff_t *offset);
ssize_t lag_write(struct file *target_file, const char __user *buf, size_t mlength, loff_t *offset);

struct file_operations lagops = {
	.open=lag_open,
	.release=lag_release,
	.read=lag_read,
	.write=lag_write,
};

int lagmayor=0;

struct task_struct * pick_next_task_lag(struct rq *rq)
{
	printk(KERN_DEBUG "print LAG");
	return pick_next_task_orig(rq);
}

int init_module()
{
	lagmayor = register_chrdev(250,lagdev, &lagops);
	if (lagmayor > -1 ) printk (KERN_DEBUG "lag device created %d",lagmayor);
		else printk(KERN_DEBUG "lag device error");
	cl = class_create(THIS_MODULE, "lag");
	if (IS_ERR(cl))
	{
		
	}
	c_dev = device_create(cl, NULL, MKDEV(250,1), NULL, "lag");
	if (IS_ERR(c_dev))
	{

	}
	init_waitqueue_head(&lag_wq);
	return 0;
}

void cleanup_module()
{
	//device_destroy(c_dev,MKDEV(251,1));
	//class_destroy(cl);
	unregister_chrdev(lagmayor, lagdev);
}

// FILE_OPERATIONS

int lag_open(struct inode *lag_inode, struct file *lag_file)
{
	if (module_used > 0) return -1;
	try_module_get(THIS_MODULE);
	module_used++;
	read=1;
	return 0;
}

int lag_release(struct inode *lag_inode, struct file *lag_file)
{
	module_used=0;
	module_put(THIS_MODULE);
	return 0;
}

ssize_t lag_write(struct file *target_file, const char __user *buf, size_t mlength, loff_t *offset)
{
	struct lag_request *tmp;
	tmp = (struct lag_request *) buf;
	if (tmp->REQID==0)
	{
		struct task_struct *tlist = &init_task;
		do {
			printk(KERN_DEBUG "patrze na proces %i tmp_pid %i",tlist->pid,tmp->pid);
			if (tlist->pid == tmp->pid) {
				state = tlist->state;
				lag_pid=tmp->pid;
				printk(KERN_DEBUG "proces %i ",tlist->pid);
			}
		} while ( (tlist = next_task(tlist)) != &init_task );
	}
	if (tmp->REQID==1)
	{
		struct task_struct *tlist = &init_task;
		do {
			printk(KERN_DEBUG "patrze na proces %i tmp_pid %i",tlist->pid,tmp->pid);
			if (tlist->pid == tmp->pid) {
				printk(KERN_DEBUG "pid: %i",tlist->pid);
				printk(KERN_DEBUG "pid: %i",current->pid);
				fs->REQ=1;
				fs->task=tlist;
				schedule();
				printk(KERN_DEBUG "pid: %i",current->pid);
				wait_event(lag_wq, block==1);
				fs->REQ=0;
				schedule();
				state = tlist->state;
				lag_pid=tmp->pid;
				printk(KERN_DEBUG "proces %i ma status %i",tlist->pid,tlist->state);
				return mlength;
			}
		} while ( (tlist = next_task(tlist)) != &init_task );
	}
	if (tmp->REQID==2)
	{
		block=0;
	}
	return mlength;
}

ssize_t lag_read(struct file *target_file, char __user *buf, size_t mlength, loff_t *offset)
{
	if (read > 0) read --;
	else return 0;
	struct lag_request *tmp = (struct lag_proces_state *) buf;
	tmp->pid=lag_pid;
	tmp->status = state;
	return sizeof(struct lag_request);
}
