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
	int k1=ftok(".",2);
	int shmid=shmget(k1,sizeof(int),0777|IPC_CREAT);
	if(shmid==-1)
	{
		printf("shmget error\n");
		exit(1);
	}
	int *p;
	p=shmat(shmid,NULL,0);
	if(p==(int*)-1)
	{
		printf("shmat error\n");
		exit(1);
	}
	
	while(1)
	{
		sleep(1);
		printf("Enter 1 to send Signal : ");
		int n;
		scanf("%d",&n);
		kill(*p,SIGUSR1);
	}
	return 0;
}