#define __LIBRARY__
#include <unistd.h>

typedef int (*fn_ptr)();
_syscall3(int,make_thread,fn_ptr,sum,int,ss,int,sp)
_syscall1(void,thread_exit,int,ret)
_syscall2(void,thread_join,int,tid,int*,ret)

int test; 

int sum(int a)
{
test = 2; 
printf("\n\nsum=%d\n\n",a);
thread_exit(123);
return 12;
}

int main()
{
test  =1;
int ret;
int *p = malloc(1024*sizeof(int));
*(p+1023) = 0x20;
ret =    make_thread(sum,p+1022,p+1022);
thread_join(ret,&ret);
printf("ret from thread is:%d\n",ret);
printf("test is:%d\n",test);
return 0;
}
