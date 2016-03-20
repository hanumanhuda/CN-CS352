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
//fifo program
int main(int argc, char const *argv[])
{
	int k1=ftok(".",3);
	int shmid=shmget(k1,30,0777|IPC_CREAT);
	if(shmid==-1)
	{
		printf("shmget error\n");
		exit(1);
	}
	char *fifoname;
	fifoname=shmat(shmid,NULL,0);
	if(fifoname==(char*)-1)
	{
		printf("shmat error\n");
		exit(1);
	}
	int fd=open(fifoname,O_WRONLY);
	 //printf("Fifo started\n");
	char *buf="This is  by fifo\n";
	while(1)
	{
		sleep(10);
		//printf("Popen%d\n",i);
		write(fd,buf,strlen(buf));
		
	}
	return 0;
}