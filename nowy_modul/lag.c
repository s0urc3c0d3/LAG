#include <linux/module.h>
#include <linux/fs.h>
#include <linux/device.h>
#include <linux/string.h>
#include <linux/sched.h>
#include "lag.h"
#include <linux/slab.h>
//#include <asm/system.h>
//#include <asm/bug.h>
#include <asm/cacheflush.h>

//#define GPF_DISABLE write_cr0(read_cr0() & (~ 0x10000))
//#define GPF_ENABLE write_cr0(read_cr0() | 0x10000)

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
//      .rq = NULL,
//      .tsk = NULL,
        .tmp = NULL,
        .isFreezing = 0,
};



int make_rw(unsigned long long address)
{  
   unsigned int level;
   pte_t *pte = lookup_address(address,&level);
   if(pte->pte &~ _PAGE_RW)
      pte->pte |= _PAGE_RW;
   return 0;
}

int make_ro(unsigned long address)
{
   unsigned int level;
   pte_t *pte = lookup_address(address, &level);
   pte->pte = pte->pte &~ _PAGE_RW;
   return 0;
}
/*
ffffffff80228c76 t activate_task
ffffffff80228f71 t deactivate_task
ffffffff80225639 t change_page_attr_set_clr
*/

int init_module()
{
	struct sched_job_lag *lag = &lag_job;
	make_rw(0xffffffff81405410);
	//make_rw(0x80471750);
	cfs = (void *)0xffffffff81405410;
	deactivate_task = (void *)0xffffffff81059a8e;
	activate_task = (void *)0xffffffff81059a6a;
	pick_next_task_fair = cfs->pick_next_task;
	put_prev_task_fair = cfs->put_prev_task;
	cfs->pick_next_task = pick_next_task_lag;
	lag->tmp = kzalloc(sizeof(lag_wait_queue), GFP_KERNEL);
	lagmayor = register_chrdev(250,lagdev, &lagops);
	if (lagmayor > -1 ) printk (KERN_DEBUG "lag device created %d",lagmayor);
		else printk(KERN_DEBUG "lag device error");
	/*cl = class_create(THIS_MODULE, "lag");
	if (IS_ERR(cl))
	{
		
	}
	c_dev = device_create(cl, NULL, MKDEV(250,1), NULL, "lag");
	if (IS_ERR(c_dev))
	{

	}*/
	init_waitqueue_head(&lag_wq);
	return 0;
}

void cleanup_module()
{
	//make_rw(0x80471750);
	cfs->pick_next_task = pick_next_task_fair;
	make_ro(0xffffffff81405410);
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
        struct sched_job_lag *lag = &lag_job;
        struct task_struct *tsk=pick_next_task_fair(rq);
        if (lag->REQ == 1) {
                if (tsk!=NULL && lag->pid==tsk->pid)
                {
                        //lag_wait_queue *wq;
			lag->REQ=0;
			printk (KERN_DEBUG "debug");
			
                        lag->tmp->tsk=tsk;
                        lag->tmp->rq=rq;
                        if (lag->wait_queue)
                                lag_wait_queue_add(lag->tmp,lag->wait_queue);
                        else
                        {
                                lag->tmp->next=lag->tmp;
                                lag->tmp->prev=lag->tmp;
                                lag->wait_queue=lag->tmp;
                        }
                        lag_debug_wait_queue(lag->wait_queue);
                        printk (KERN_DEBUG "w kolejce mam pid: %i \n",lag->wait_queue->tsk->pid);
                        if (lag->wait_queue->next ==NULL) printk (KERN_DEBUG "next jest null   \n");
                        if (lag->wait_queue->prev ==NULL) printk (KERN_DEBUG "prev jest null   \n");
                        
			put_prev_task_fair(rq,tsk);
        //asm("#b");
			//if (lag->isFreezing == 1)
                        //{
				deactivate_task(rq, tsk, 1);
                        	tsk = pick_next_task_fair(rq);
			//}
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
                       
			//if (lag->isFreezing == 1)
			//{
				activate_task(tmp->rq,tmp->tsk,1);
			//}
                        
			if (lag->wait_queue == tmp) printk(KERN_DEBUG "\n niestety kolejka jest pusta\n");
                        if (lag->wait_queue->prev == NULL || lag->wait_queue->next == NULL) lag->wait_queue = NULL;
                        lag_debug_wait_queue(lag->wait_queue);
                        kfree(tmp);
        }
        //asm("#c");
        return tsk;
        //asm("#d");
}







EXPORT_SYMBOL(lag_job);

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
