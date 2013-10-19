#include <linux/module.h>
#include <linux/fs.h>
#include <linux/device.h>
#include <linux/cdev.h>
#include <linux/string.h>
#include <linux/sched.h>
#include <linux/percpu.h>
#include "lag.h"
#include <linux/slab.h>
#include <asm/cacheflush.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Grzegorz Dwornicki");
MODULE_DESCRIPTION("This module provides /dev/lag character device for Linux Agents project");
MODULE_VERSION("1.1.0");

struct rq *runqueues;

#define task_rq(p) (&per_cpu(runqueues, (task_cpu(p))))

const char *lagdev="/dev/lag";

int module_used=0;
int read=0;

static dev_t first;
static struct class *cl;
static struct cdev c_dev;

struct sched_job_lag *fs;

int lag_open(struct inode *lag_inode, struct file *lag_file);
int lag_release(struct inode *lag_inode, struct file *lag_file);
ssize_t lag_read(struct file *target_file, char __user *buf, size_t mlength, loff_t *offset);
ssize_t lag_write(struct file *target_file, const char __user *buf, size_t mlength, loff_t *offset);
static struct task_struct* pick_next_task_lag(struct rq *rq);
static struct task_struct* (*pick_next_task_fair)(struct rq *rq);
static struct task_struct* (*put_prev_task_fair)(struct rq *rq, struct task_struct *prev);
void (*deactivate_task)(struct rq *rq, struct task_struct *p, int sleep);
void (*activate_task)(struct rq *rq, struct task_struct *p, int sleep);
struct sched_class *cfs;

struct file_operations lagops = {
	.open=lag_open,
	.release=lag_release,
	.read=lag_read,
	.write=lag_write,
};

int lagmayor=0;
int freezing=1;

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
        .tmp = NULL,
};



int make_rw(unsigned long long address)
{  
   unsigned int level;
   pte_t *pte = lookup_address(address,&level);
   if(pte->pte &~ _PAGE_RW)
      pte->pte |= _PAGE_RW;
   return 0;
}

int make_ro(unsigned long long address)
{
   unsigned int level;
   pte_t *pte = lookup_address(address, &level);
   pte->pte = pte->pte &~ _PAGE_RW;
   return 0;
}

int init_module()
{
	fs=(struct sched_job_lag *)kzalloc(sizeof(struct sched_job_lag),GFP_KERNEL);
	make_rw(0xc03a98e4);
	make_rw(0xc052fd00);
	runqueues=0xc052fd00;
	cfs = (void *)0xc03a98e4;
	deactivate_task = (void *)0xc0144764;
	activate_task = (void *)0xc0144eb4;
	pick_next_task_fair = cfs->pick_next_task;
	put_prev_task_fair = cfs->put_prev_task;
	cfs->pick_next_task = pick_next_task_lag;
	if (alloc_chrdev_region(&first, 0, 1, "lag") < 0)
	{	
		printk(KERN_DEBUG "lag device error");
		return -1;
	}
	KERN_DEBUG "lag device created";
	if ((cl = class_create(THIS_MODULE, "lag")) == NULL)
	{
		unregister_chrdev_region(first, 1);
		return -1;
	}
	if (device_create(cl, NULL, first, NULL, "lag") == NULL)
	{
		class_destroy(cl);
		unregister_chrdev_region(first, 1);
		return -1;
	}
	cdev_init(&c_dev, &lagops);
	if (cdev_add(&c_dev, first, 1) == -1)
	{
		device_destroy(cl, first);
		class_destroy(cl);
		unregister_chrdev_region(first, 1);
		return -1;
	}
	return 0;
}

void cleanup_module()
{
	cdev_del(&c_dev);
	device_destroy(cl, first);
	class_destroy(cl);
	cfs->pick_next_task = pick_next_task_fair;
	make_ro(0xc03a98e4);
	unregister_chrdev_region(first, 1);
	kfree(fs);
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
	if (tmp->REQID==1)
	{
		struct task_struct *tlist = &init_task;
		do {
			if (tlist->pid == tmp->pid) {
				fs->pid=tlist->pid;
				fs->REQ=1;
				fs->tmp = kzalloc(sizeof(lag_wait_queue), GFP_KERNEL);
				return mlength;
			}
		} while ( (tlist = next_task(tlist)) != &init_task );
	}
	if (tmp->REQID==2 || tmp->REQID==3 || tmp->REQID==4)
	{
		fs->REQ=tmp->REQID;
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
        struct task_struct *tsk=pick_next_task_fair(rq);
        if (fs->REQ == 1) {
                if (tsk!=NULL && fs->pid==tsk->pid)
                {
			fs->REQ=0;
			printk (KERN_DEBUG "debug");
			
                        fs->tmp->tsk=tsk;
                        fs->tmp->rq=rq;
                        if (fs->wait_queue)
			{
				printk (KERN_DEBUG "w if_then");
				lag_wait_queue_add(fs->tmp,fs->wait_queue);
			}
                        else
                        {
				printk (KERN_DEBUG "w else");
                                fs->tmp->next=fs->tmp;
                                fs->tmp->prev=fs->tmp;
                                fs->wait_queue=fs->tmp;
                        }
                        lag_debug_wait_queue(fs->wait_queue);
                        printk (KERN_DEBUG "w kolejce mam pid: %i \n",fs->wait_queue->tsk->pid);
                        if (fs->wait_queue->next ==NULL) printk (KERN_DEBUG "next jest null   \n");
                        if (fs->wait_queue->prev ==NULL) printk (KERN_DEBUG "prev jest null   \n");
                        
			put_prev_task_fair(rq,tsk);
			if (freezing == 1) deactivate_task(rq, tsk, 1);
                        tsk = pick_next_task_fair(rq);
                        return tsk;
                }
        }
        if (fs->REQ == 2) {
                fs->REQ=0;
                lag_wait_queue *tmp=fs->wait_queue;
                while (tmp !=NULL && tmp->tsk->pid != fs->pid )
                {
                        if (tmp!=NULL)
                        {
                                printk(KERN_DEBUG " ....... pid: %i .....",tmp->tsk->pid);
                        }
                        tmp=tmp->next;
                        if (tmp == fs->wait_queue) break;
                }
                if (tmp==NULL)
                        printk(KERN_DEBUG "Blad!");
                else {
                        fs->wait_queue=tmp->next;
                        lag_wait_queue_del(tmp);
                       
			if (freezing == 1 )activate_task(tmp->rq, tmp->tsk, 1);
                        
			if (fs->wait_queue == tmp) printk(KERN_DEBUG "\n niestety kolejka jest pusta\n");
                        if (fs->wait_queue->prev == NULL || fs->wait_queue->next == NULL) fs->wait_queue = NULL;
                        lag_debug_wait_queue(fs->wait_queue);
                        kfree(tmp);
		}
        }
	if (fs->REQ == 3)
	{
		fs->REQ = 0;
		if (freezing == 0)
		{
			freezing=1;
			lag_wait_queue *tmp1,*tmp=fs->wait_queue;
			if (tmp == NULL) return tsk;
//			do {
				tmp1=tmp;
				if (tmp->tsk == NULL || tmp->rq == NULL)
				{
					tmp=tmp->next;
					lag_wait_queue_del(tmp1);
//					continue;
					return tsk;
				}
//				tmp->rq=task_rq(tmp->tsk);
				printk(KERN_DEBUG "\ndeactivate\n");
				deactivate_task(tmp->rq, tmp->tsk, 1);
				tmp=tmp->next; //}
				if (tmp == tmp1) printk(KERN_DEBUG "\nrowne\n");
//			while (tmp != fs->wait_queue);
		}
	}
	if (fs->REQ == 4)
	{
		fs->REQ = 0;
		if (freezing == 1)
		{
			freezing=0;
			lag_wait_queue *tmp1,*tmp=fs->wait_queue;
			if (tmp == NULL) return tsk;
			do {
				tmp1=tmp;
				tmp=tmp->next; 
				activate_task(tmp1->rq, tmp1->tsk, 1);
				if (tmp != tmp1)
                        		lag_wait_queue_del(tmp1);
				else fs->wait_queue = NULL;
				kfree(tmp1); }
			while (fs->wait_queue != NULL);
		}
	}
        return tsk;
}

void lag_wait_queue_add(lag_wait_queue *ent, lag_wait_queue *list)
{
        lag_wait_queue *list_next = list->next;
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
        printk(KERN_DEBUG "DEBUG WAIT QUEUE START \n");
        if (ent == NULL)
        printk(KERN_DEBUG "kolejka pusta \n");
        else
        {
                int ent_pid=ent->tsk->pid;
                lag_wait_queue *tmp=ent;
                do {
                        printk(KERN_DEBUG "entry pid %i \n",tmp->tsk->pid);
                        if (tmp->next == NULL) printk(KERN_DEBUG "entry next is null! \n"); else printk(KERN_DEBUG "entry next pid %i \n",tmp->next->tsk->pid);
                        if (tmp->prev == NULL) printk(KERN_DEBUG "entry prev is null! \n"); else printk(KERN_DEBUG "entry prev pid %i \n",tmp->prev->tsk->pid);
                        tmp=tmp->next;
                }
                while (tmp != NULL && tmp->tsk->pid != ent_pid);
        }
        printk(KERN_DEBUG "DEBUG WAIT QUEUE STOP \n");
}

