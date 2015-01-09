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

#define NUMBER 520 /*打出数字总数*/
#define CHILD 5 /*消费者进程数*/
#define BUFSIZE 10 /*缓冲区大小*/

sem_t   *empty, *full, *mutex;
int fno; /*文件描述符*/

int main()
{
    int  i,j,k;
    int  data;
    pid_t p;
    int  buf_out = 0; /*从缓冲区读取位置*/
    int  buf_in = 0; /*写入缓冲区位置*/
    /*打开信号量*/
    if((mutex = sem_open("carmutex",1)) == SEM_FAILED)
    {
        perror("sem_open() error!\n");
        return -1;
    }
    if((empty = sem_open("carempty",10)) == SEM_FAILED)
    {
        perror("sem_open() error!\n");
        return -1;
    }
    if((full = sem_open("carfull",0)) == SEM_FAILED)
    {
        perror("sem_open() error!\n");
        return -1;
    }
    fno = open("buffer.dat",O_CREAT|O_RDWR|O_TRUNC,0666);
    /* 将待读取位置存入buffer后,以便 子进程 之间通信 */
    lseek(fno,10*sizeof(int),SEEK_SET);
    write(fno,(char *)&buf_out,sizeof(int));
    /*生产者进程*/
    if((p=fork())==0)
    {
        for( i = 0 ; i < NUMBER; i++)
        {
            sem_wait(empty);
            sem_wait(mutex);
            /*写入一个字符*/
            lseek(fno, buf_in*sizeof(int), SEEK_SET); 
            write(fno,(char *)&i,sizeof(int));  
            buf_in = ( buf_in + 1)% BUFSIZE;

            sem_post(mutex);
            sem_post(full);
        }
        return 0;
    }else if(p < 0)
    {
        perror("Fail to fork!\n");
        return -1;
    }

    for( j = 0; j < CHILD ; j++ )
    {
        if((p=fork())==0)
        {
            for( k = 0; k < NUMBER/CHILD; k++ )
            {
                sem_wait(full);
                sem_wait(mutex);
                /*获得读取位置*/
                lseek(fno,10*sizeof(int),SEEK_SET);
                read(fno,(char *)&buf_out,sizeof(int));
                /*读取数据*/
                lseek(fno,buf_out*sizeof(int),SEEK_SET);
                read(fno,(char *)&data,sizeof(int));
                /*写入读取位置*/
                buf_out = (buf_out + 1) % BUFSIZE;
                lseek(fno,10*sizeof(int),SEEK_SET);
                write(fno,(char *)&buf_out,sizeof(int));

                sem_post(mutex);
                sem_post(empty);
                /*消费资源*/
                printf("%d:  %d\n",getpid(),data);
                fflush(stdout);
            }
           return 0;
        }else if(p<0)
        {
            perror("Fail to fork!\n");
            return -1;
        }
    }
    wait(NULL);
    /*释放信号量*/
    sem_unlink("carfull");
    sem_unlink("carempty");
    sem_unlink("carmutex");
    /*释放资源*/
    close(fno);
    return 0;
}