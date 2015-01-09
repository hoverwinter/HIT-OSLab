#define   __LIBRARY__
#include <unistd.h>
#include <sys/types.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>

_syscall2(sem_t*,sem_open,const char *,name,unsigned int,value);
_syscall1(int,sem_wait,sem_t*,sem);
_syscall1(int,sem_post,sem_t*,sem);
_syscall1(int,sem_unlink,const char *,name);
_syscall1(void*,shmat,int,shmid);
_syscall1(int,shmget,char*,name);

#define NUMBER 520 /*打出数字总数*/
#define BUFSIZE 10 /*缓冲区大小*/

sem_t   *empty, *full, *mutex;

int main()
{
    int  i,shmid,data;
    int *p;
    int  buf_out = 0; /*从缓冲区读取位置*/
    /*打开信号量*/
    if((mutex = sem_open("carpelamutex",1)) == SEM_FAILED)
    {
        perror("sem_open() error!\n");
        return -1;
    }
    if((empty = sem_open("carpelaempty",10)) == SEM_FAILED)
    {
        perror("sem_open() error!\n");
        return -1;
    }
    if((full = sem_open("carpelafull",0)) == SEM_FAILED)
    {
        perror("sem_open() error!\n");
        return -1;
    }
    shmid = shmget("buffer");
    if(shmid == -1)
    {
        return -1;
    }
    p = (int *)shmat(shmid);

    for( i = 0; i < NUMBER; i++ )
    {
        sem_wait(full);
        sem_wait(mutex);

        data = p[buf_out];
        buf_out = (buf_out + 1) % BUFSIZE;

        sem_post(mutex);
        sem_post(empty);
        /*消费资源*/
        printf("%d:  %d\n",getpid(),data);
        fflush(stdout);
    }

    /*释放信号量*/
    sem_unlink("carpelafull");
    sem_unlink("carpelaempty");
    sem_unlink("carpelamutex");

    return 0;
}
