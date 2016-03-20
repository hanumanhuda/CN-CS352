#include <stdio.h>
#include <stdlib.h>
#include <semaphore.h>
#include <signal.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <sys/sem.h>
#include <sys/ipc.h>
#include <string.h>
#include <pthread.h>
#include <fcntl.h>
#include <poll.h>
#include <unistd.h>
#define BUF_SIZE 255
int main(int argc, char const *argv[])
{
	if(argc!=2)
	{
		printf("Pass Fifo name \n");
		exit(1);
	}
	int k1=ftok(".",2);
	int shmid=shmget(k1,30,0777|IPC_CREAT);
	if(shmid==-1)
	{
		printf("shmget error\n");
		exit(1);
	}
	char *p;
	p=shmat(shmid,NULL,0);
	if(p==(char*)-1)
	{
		printf("shmat error\n");
		exit(1);
	}
	char *name="/tmp/";
	char fifoname[BUF_SIZE];
	sprintf(fifoname,"%s%s",name,argv[1]);
	int res=mkfifo(fifoname,0777);
	if(res<0)
	{
		printf("mkfifo error\n");
		exit(1);
	}
	strcpy(p,fifoname);
	int fd=open(fifoname,O_WRONLY);
	printf("Fifo started\n");
	char *buf="This is  by fifo";
	while(1)
	{
		sleep(10);
		//printf("Popen%d\n",i);
		write(fd,buf,strlen(buf));
		
	}
	return 0;
}