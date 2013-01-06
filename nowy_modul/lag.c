#include <linux/module.h>
#include <linux/fs.h>
#include <linux/device.h>
#include <linux/string.h>
#include <linux/sched.h>
#include "LAG/lag.h"
#include <asm/system.h>
#include <asm/bug.h>
#include <asm/cacheflush.h>
#include <linux/lag.h>

struct task_struct * (*pick_next_task_orig) (struct rq *rq);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Grzegorz Dwornicki");
MODULE_DESCRIPTION("This module provides /dev/lag character device for Linux Agents project");

MODULE_VERSION("1.1.0");

const char *lagdev="/dev/lag";

int module_used=0;
int read=0;

struct class *cl;
void *c_dev;

struct sched_job_lag *fs=&lag_job;
lag_wait_queue *wq = &lag_wait;

DECLARE_WAIT_QUEUE_HEAD(lag_wq);

int lag_open(struct inode *lag_inode, struct file *lag_file);
int lag_release(struct inode *lag_inode, struct file *lag_file);
ssize_t lag_read(struct file *target_file, char __user *buf, size_t mlength, loff_t *offset);
ssize_t lag_write(struct file *target_file, const char __user *buf, size_t mlength, loff_t *offset);
static struct task_struct *pick_next_task_lag(struct rq *rq);

struct file_operations lagops = {
	.open=lag_open,
	.release=lag_release,
	.read=lag_read,
	.write=lag_write,
};

int lagmayor=0;

int init_module()
{
	struct page *pg;
	pgprot_t prot;
	pg = virt_to_page(0xc0318a04);
	prot.pgprot = VM_READ | VM_WRITE;
	int e = change_page_attr(pg, 1, prot);
	struct sched_class *cfs = 0xc0318a04;
	cfs->pick_next_task = pick_next_task_lag;
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
			if (tlist->pid == tmp->pid) {
				//state = tlist->state;
				//lag_pid=tmp->pid;
			}
		} while ( (tlist = next_task(tlist)) != &init_task );
	}
	if (tmp->REQID==1)
	{
		struct task_struct *tlist = &init_task;
		do {
			if (tlist->pid == tmp->pid) {
				fs->pid=tlist->pid;
				fs->REQ=1;
				return mlength;
			}
		} while ( (tlist = next_task(tlist)) != &init_task );
	}
	if (tmp->REQID==2)
	{
		fs->REQ=2;
		fs->pid=tmp->pid;
	}
	return mlength;
}

ssize_t lag_read(struct file *target_file, char __user *buf, size_t mlength, loff_t *offset)
{
	if (read > 0) read --;
	else return 0;
	return sizeof(struct lag_request);
}

static struct task_struct *pick_next_task_lag(struct rq *rq)
{
	//asm("#1");
	struct sched_job_lag *lag = &lag_job;
	//asm("#2");
	struct task_struct *tsk=pick_next_task_fair(rq);
	//asm("#3");
	//asm("#4");
	if (lag->REQ == 1) {
		//asm("#5");
		if (tsk!=NULL && lag->pid==tsk->pid) 
			//asm("#a");
		{
			struct __lag_wait_queue *wq = (struct __lag_wait_queue *) kzalloc(sizeof(struct __lag_wait_queue),GFP_KERNEL);
			wq->tsk=tsk;
			wq->rq=rq;
			lag->REQ=0;
			if (lag->wait_queue) 
				lag_wait_queue_add(wq,lag->wait_queue); 
			else 
			{
				wq->next=wq;
				wq->prev=wq;
				lag->wait_queue=wq;
			}
			lag_debug_wait_queue(lag->wait_queue);
			printk (KERN_DEBUG "w kolejce mam pid: %i \n",lag->wait_queue->tsk->pid);
			if (lag->wait_queue->next ==NULL) printk (KERN_DEBUG "next jest null   \n");
			if (lag->wait_queue->prev ==NULL) printk (KERN_DEBUG "prev jest null   \n");
			printk(KERN_DEBUG "pid: %i",tsk->pid);
			put_prev_task_fair(rq,tsk);
			//asm("#b");
			deactivate_task(rq, tsk, 1);
			tsk = pick_next_task_fair(rq);
			return tsk;
		}
	}
	if (lag->REQ == 2) {
		lag->REQ=0;
		lag_wait_queue *tmp=lag->wait_queue;
		while (tmp !=NULL && tmp->tsk->pid != lag->pid ) 
		{
			if (tmp!=NULL)
			{
				printk(KERN_DEBUG " ....... pid: %i .....",tmp->tsk->pid);
			}
			tmp=tmp->next;
			if (tmp == lag->wait_queue) break;
		}
		if (tmp==NULL) 
			printk(KERN_DEBUG "Blad!");
		else
			lag->wait_queue=tmp->next;
		lag_wait_queue_del(tmp);
		activate_task(tmp->rq,tmp->tsk,1);
		if (lag->wait_queue == tmp) printk(KERN_DEBUG "\n niestety kolejka jest pusta\n");
		if (lag->wait_queue->prev == NULL || lag->wait_queue->next == NULL) lag->wait_queue = NULL;
		lag_debug_wait_queue(lag->wait_queue);
		kfree(tmp);
	}
	//asm("#c");
	return tsk;
	//asm("#d");
}
