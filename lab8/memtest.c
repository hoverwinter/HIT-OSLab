#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define MEM_SIZE 0x20000
#define MAX_TH_NUM 10

unsigned long test_addr[MAX_TH_NUM];
int times;
int tid[MAX_TH_NUM];
int result[MAX_TH_NUM];

int memtest(unsigned long start_addr)
{
	int i,j,flag=1,tmp,counter=0;
	unsigned char * p;
	p = (unsigned char*)start_addr;
	for(i=0;i<times;i++)
	{
		for(j=0;j<MEM_SIZE;j++)
		{
			*(p+j) = 0;
			if(*(p+j) != 0) flag = 0;
		
			*(p+j) = 0xFF;
			if(*(p+j) != 0xFF) flag = 0;
	
			*(p+j) = 0x55;
			if(*(p+j) != 0x55) flag = 0;
	
			*(p+j) = 0xAA;
			if(*(p+j) != 0xAA) flag = 0;

			srand(time(NULL));
			tmp = rand() % 0xff;
			*(p+j) = tmp;
			if(*(p+j) != tmp) flag = 0;
			if(flag)
			{
				counter ++;
			}else
			{
				flag = 1;
			}
		}
	}
	pthread_exit(counter);
	return 0;
}

int main()
{
	int num,i = 0;
	char tmp[MAX_TH_NUM];
	times = 1;
	num = 2;
	printf("Usage:\n"
		"\tgo: start test\n"
		"\tabort: cancel thread\n"
		"\tstatus: show test status\n"
		"\texit: exit the program\n"
		"\tthread n: create n threads to test\n"
		"\ttimes n: each mem unit tests n times\n"
		"Default: 1 times and 2 threads\n"
		);
	fflush(stdout);
	while(1)
	{
		printf(">>>");
		fflush(stdout);
		scanf(" %s",tmp);
		if(strcmp(tmp,"times") == 0)
		{
			scanf(" %d",&times);
			continue;
		}
		if(strcmp(tmp,"thread") == 0)
		{
			scanf(" %d",&num);
			continue;
		}
		if(strcmp(tmp,"exit") == 0)
		{
			exit(0);
		}
		if(strcmp(tmp,"go") == 0)
		{
			for(i=0;i<num;i++)
			{
				test_addr[i] = (unsigned long) malloc(MEM_SIZE*sizeof(unsigned char));
				pthread_create(&tid[i],memtest,test_addr[i]);
			}
			continue;
		}
		if(strcmp(tmp,"status") == 0)
		{
			for(i=0;i<num;i++)
			{
				printf("Thread %d :  ",tid[i]);
				switch(pthread_status(tid[i]))
				{
					case -1: 
						printf("is to be created\n");
						break;
					case 0: 
						printf("is running\n");
						break;
					case 1:
						printf("is waiting\n");
						break;
					case 2:
						printf("is waiting\n");
						break;
					case 4:
						printf("is stopped\n");
						pthread_join(tid[i],&result[i]);
						printf("\tTest Addr: %X Result: %d/%d(OK)\n",test_addr[i],result[i],MEM_SIZE);
						break;
					case 5:
						printf("is canceled\n");
						break;
					default:
						printf("no such thread\n");
						break;
				}
			}
			continue;
		}
		if(strcmp(tmp,"abort") == 0)
		{
			for(i=0;i<num;i++)
			{
				if(tid[i])
				{
					pthread_cancel(tid[i]);
				}
			}
			continue;
		}
	}
	
}