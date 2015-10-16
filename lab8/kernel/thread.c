/*
 *  linux/kernel/thread.c
 *
 *  (C) 2014  Hover Winter (carpela@163.com)
 *  https://github.com/traitorousfc
 *  MIT License   Harbin Institute of Technology(HIT)
 *
 */

/*
 * 'thread.c' is the main kernel file. It contains the implement of multi-thread
 * Thread number is limited to 10. And obviously part of POSIX thread!
 * Thread is the basic schedule unit.
 * It takes me 2 days to finish the whole part and memtest.c!
 */

#include <linux/sched.h>
#include <linux/kernel.h>
#include <asm/segment.h>
#include <asm/system.h>
#include <errno.h>

int get_task_nr(long pid,long tid)
{
	int i;
	for(i=0 ; i<NR_TASKS ; i++)
		if(task[i]->pid == pid && task[i]->tid == tid)
			return i;
	return -1;
}

/*参数none: sys_make_thread返回地址*/
int init_tss(int eax,long ebp,long edi,long esi,long gs,long none,
		long ebx,long ecx,long edx,
		long fs,long es,long ds,
		long eip,long cs,long eflags,long esp,long ss)
{
	int i,nr;
	struct file *f;
	struct task_struct * p;
	p = (struct task_struct *) get_free_page();
	if (!p)
		return -EAGAIN;
	nr = find_empty_process();
	task[nr] = p;
	for(i=0;i<NR_THREADS_PER_TASK;i++)
		if(current->thread[i]==NULL)
			break;
	if(i>NR_THREADS_PER_TASK-1)
	{
		printk("BAD BAD: Thread are limited to 10!\n");
		return -1;
	}
	current->thread[i] = p;
	*p = *current;
	p->state = TASK_UNINTERRUPTIBLE;
	p->pid = current->pid;
	p->father = current->father;
	p->counter = p->priority;
	p->signal = 0;
	p->alarm = 0;
	p->leader = 0;		/* process leadership doesn't inherit */
	p->utime = p->stime = 0;
	p->cutime = p->cstime = 0;
	p->start_time = jiffies;
	p->tid = current->tid_num;
	p->tid_num = 1;
	/*tid_num only make effects in Main thread*/
	current->tid_num += 1;
	p->tss.back_link = 0;
	p->tss.esp0 = PAGE_SIZE + (long) p;
	p->tss.ss0 = 0x10;
	p->tss.eip = ebx;
	p->tss.eflags = eflags;
	p->tss.eax = eax;
	p->tss.ecx = ecx;
	p->tss.edx = edx;
	p->tss.ebx = ebx;
	p->tss.esp = ecx;
	p->tss.ebp = ecx;
	p->tss.esi = esi;
	p->tss.edi = edi;
	p->tss.es = es & 0xffff;
	p->tss.cs = cs & 0xffff;
	p->tss.ss = ss & 0xffff;
	p->tss.ds = ds & 0xffff;
	p->tss.fs = fs & 0xffff;
	p->tss.gs = gs & 0xffff;
	p->tss.ldt = current->tss.ldt;
	p->tss.trace_bitmap = 0x80000000;
	p->tss.i387 = current->tss.i387;
	for (i=0; i<NR_OPEN;i++)
		if ((f=p->filp[i]))
			f->f_count++;
	if (current->pwd)
		current->pwd->i_count++;
	if (current->root)
		current->root->i_count++;
	if (current->executable)
		current->executable->i_count++;
	set_tss_desc(gdt+(nr<<1)+FIRST_TSS_ENTRY,&(p->tss));
	set_ldt_desc(gdt+(nr<<1)+FIRST_LDT_ENTRY,&(p->ldt));
	p->state = TASK_RUNNING;
	return p->tid;
}

int sys_thread_cancel(int tid)
{
	cli();
	int nr,i;
	struct task_struct *p;
	if(tid == current->tid)
	{
		printk("BAD BAD: try to cancel self!\n");
		return;
	}
	if(tid == 0)
	{
		printk("BAD BAD: try to cancel Main Thread!\n");
		return -1;
	}
	nr = get_task_nr(current->pid,0);
	for(i=0;i<NR_THREADS_PER_TASK;i++)
	{
		if(task[nr]->thread[i]->tid == tid)
		{
			p = task[nr]->thread[i];
		}
	}
	p->state = THREAD_CANCELED;
	nr = get_task_nr(current->pid,tid);
	task[nr] = NULL;
	// printk("PID:%d\tTID:%d canceled\n",current->pid,tid);
	sti();
	schedule();
	return 0;
}

int sys_thread_exit(int value)
{
	int nr,i;
	if(current->tid == 0)
	{
		panic("Error Error: Main thread(tid=0) should never be here!\n");
	}
	current->state = TASK_STOPPED;
	current->exit_code = value;
	nr = get_task_nr(current->pid,0);
	for(i=0;i<NR_THREADS_PER_TASK;i++)
	{
		if(task[nr]->thread[i]!=NULL)
		{
			if(task[nr]->thread[i]->state == current->tid*10 + TASK_UNINTERRUPTIBLE)
				task[nr]->thread[i]->state = TASK_RUNNING;
		}
	}
	nr = get_task_nr(current->pid,current->tid);
	task[nr] = NULL;
	// printk("PID:%d\tTID:%d exit\n",current->pid,current->tid);
	schedule();
	return 0;
}

int sys_thread_join(int tid, int* value_ptr)
{
	int nr,i;
	struct task_struct *p;
	// nr = get_task_nr(current->pid,tid) this is wrong!
	nr = get_task_nr(current->pid,0);
	for(i=0;i<NR_THREADS_PER_TASK;i++)
	{
		if(task[nr]->thread[i]->tid==tid)
		{
			p = task[nr]->thread[i];
			break;
		}
	}
	if(p == NULL)
	{
		// printk("BAD BAD: try to wait for non-existing thread!\n");
		return -1;
	}
	if(p->state != TASK_STOPPED)
	{
		current->state = tid*10 + TASK_UNINTERRUPTIBLE;
		schedule();
	}
	// printk("exit_code:%d\n",task[nr]->thread[tid-1]->exit_code);
	put_fs_long(p->exit_code,(unsigned long*)value_ptr);
	return 0;
}

int sys_thread_status(int tid)
{
	// return current->state;
	int nr,i;
	struct task_struct *p = NULL;
	nr = get_task_nr(current->pid,0);
	for(i=0;i<NR_THREADS_PER_TASK;i++)
	{
		if(task[nr]->thread[i]->tid==tid)
		{
			p = task[nr]->thread[i];
			break;
		}
	}
	if(p == NULL)
	{
		// printk("BAD BAD: no such thread!\n");
		return -1;
	}
	return p->state;
}

int sys_thread_gettid()
{
	return current->tid;
}