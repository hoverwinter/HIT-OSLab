// int shmget(char * name);
// void * shmat(int shmid);
#include <asm/segment.h>
#include <linux/kernel.h>
#include <unistd.h>
#include <string.h>
#include <linux/sched.h>

#define SHM_COUNT 20
#define SHM_NAME_SIZE 20

struct struct_shm_tables
{
	int occupied;
	char name[SHM_NAME_SIZE];
	long addr;
} shm_tables[SHM_COUNT];

int find_shm_location(char *name)
{
	int i;
	for(i=0; i<SHM_COUNT; i++)
	{
		if(!strcmp(name,shm_tables[i].name) && shm_tables[i].occupied ==1)
		{
			return i;
		}
	}
	return -1;
}

int sys_shmget(char * name)
{
	int i,shmid;
	char tmp[SHM_NAME_SIZE];
	for(i=0; i<SHM_NAME_SIZE; i++)
	{
		tmp[i] = get_fs_byte(name+i);
		if(tmp[i] == '\0') break;
	}
	shmid = find_shm_location(tmp);
	if( shmid != -1) 
	{
		return shmid;
	}
	for(i=0; i<SHM_COUNT; i++)
	{
		if(shm_tables[i].occupied == 0)
		{
			strcpy(shm_tables[i].name,tmp);
			shm_tables[i].occupied = 1;
			shm_tables[i].addr = get_free_page();
			return i;
		}
	}
	printk("SHM Number limited!\n");
	return -1;
}

void * sys_shmat(int shmid)
{
	if(shm_tables[shmid].occupied != 1)
	{
		printk("SHM not exists!\n");
		return -1;
	}
	put_page(shm_tables[shmid].addr,current->brk + current->start_code);
	return (void*)current->brk;
}
