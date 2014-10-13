#include <string.h>
#include <errno.h>
#include <asm/segment.h>

char msg[24]; //23个字符 +'\0' = 24

int sys_iam(const char * name)
/***
function：将name的内容拷贝到msg,name的长度不超过23个字符
return：拷贝的字符数。如果name的字符个数超过了23,则返回“­-1”,并置errno为EINVAL。
****/
{
	int i;
	//临时存储 输入字符串 操作失败时不影响msg
	char tmp[30];
	for(i=0; i<30; i++)
	{
		//从用户态内存取得数据
		tmp[i] = get_fs_byte(name+i);
		if(tmp[i] == '\0') break;  //字符串结束
	}
	//printk(tmp);
	i=0;
	while(i<30&&tmp[i]!='\0') i++;
	int len = i;
	// int len = strlen(tmp);
	//字符长度大于23个
	if(len > 23)
	{
		// printk("String too long!\n");
		return -(EINVAL);  //置errno为EINVAL  返回“­-1”  具体见_syscalln宏展开
	}
	strcpy(msg,tmp);
	//printk(tmp);
	return i;
}

int sys_whoami(char* name, unsigned int size)
/***
function:将msg拷贝到name指向的用户地址空间中,确保不会对name越界访存(name的大小由size说明)
return: 拷贝的字符数。如果size小于需要的空间,则返回“­-1”,并置errno为EINVAL。
****/
{ 	
	//msg的长度大于 size
	int len = 0;
	for(;msg[len]!='\0';len++);
	if(len > size)
	{
		return -(EINVAL);
	}
	int i = 0;
	//把msg 输出至 name
	for(i=0; i<size; i++)
	{
		put_fs_byte(msg[i],name+i);
		if(msg[i] == '\0') break; //字符串结束
	}
	return i;
}
