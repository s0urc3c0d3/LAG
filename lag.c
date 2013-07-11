#include <linux/module.h>
#include <linux/fs.h>
#include <linux/device.h>
#include <linux/string.h>
#include <linux/sched.h>
//#include <asm/system.h>
#include <asm/bug.h>
#include <asm/cacheflush.h>
#include <linux/slab.h>
#include "lag.h"

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
//lag_wait_queue *wq = &lag_wait;

int lag_open(struct inode *lag_inode, struct file *lag_file);
int lag_release(struct inode *lag_inode, struct file *lag_file);
ssize_t lag_read(struct file *target_file, char __user *buf, size_t mlength, loff_t *offset);
ssize_t lag_write(struct file *target_file, const char __user *buf, size_t mlength, loff_t *offset);

void (*activate_task)(struct rq *rq, struct task_struct *p, int flags);
void (*deactivate_task)(struct rq *rq, struct task_struct *p, int flags);

//lag_task_struct lag_ts;

struct file_operations lagops = {
	.open=lag_open,
	.release=lag_release,
	.read=lag_read,
	.write=lag_write,
};

int lagmayor=0;

int init_module()
{
	lagmayor = register_chrdev(250,lagdev, &lagops);
	if (lagmayor > -1 ) printk (KERN_DEBUG "lag device created %d",lagmayor);
		else printk(KERN_DEBUG "lag device error");
	cl = class_create(THIS_MODULE, "lag");
	if (cl==NULL)
	{
		printk(KERN_DEBUG "\n class error");	
	}
	c_dev = device_create(cl, NULL, MKDEV(250,1), NULL, "lag");
	if (c_dev==NULL)
	{
		printk(KERN_DEBUG "\n c_dev error");	
	}
	fs->isFreezing=1;
	deactivate_task=0xc0144764; 
	activate_task=0xc0144eb4;
	return 0;
}

void cleanup_module()
{
	device_destroy(cl,MKDEV(250,1));
	class_destroy(cl);
	unregister_chrdev(250 , lagdev);
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
		fs->isFreezing=0;
		fs->tmp=fs->wait_queue;
		do
		{
			activate_task(fs->tmp->rq,fs->tmp->tsk,1);
			fs->tmp=fs->tmp->next;
		} while(fs->tmp != fs->wait_queue);
	}
	if (tmp->REQID==1)
	{	
		fs->isFreezing=1;
		fs->tmp=fs->wait_queue;
		do
		{
			deactivate_task(fs->tmp->rq,fs->tmp->tsk,1);
			fs->tmp=fs->tmp->next;
		} while(fs->tmp != fs->wait_queue);
	}
	if (tmp->REQID==2)
	{
		struct task_struct *tlist = &init_task;
		do {
			if (tlist->pid == tmp->pid) {
				fs->pid=tlist->pid;
				fs->REQ=1;
				fs->tmp=kzalloc(sizeof(lag_wait_queue),GFP_KERNEL);
				return mlength;
			}
		} while ( (tlist = next_task(tlist)) != &init_task );
	}
	if (tmp->REQID==3)
	{
		fs->REQ=2;
		fs->pid=tmp->pid;
	}
	if (tmp->REQID==4) //zwroc liste procesow w kolejce
	{
		
	}
	return mlength;
}

ssize_t lag_read(struct file *target_file, char __user *buf, size_t mlength, loff_t *offset)
{
	if (read > 0) read --;
	else return 0;
	return sizeof(struct lag_request);
}
