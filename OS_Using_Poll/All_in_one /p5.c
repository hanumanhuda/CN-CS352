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
	
	int k1=ftok("..",3);

	int shmid=shmget(k1,30,0777|IPC_CREAT);
	if(shmid==-1)
	{
		printf("shmget error\n");
		exit(1);
	}
	
	char *p=shmat(shmid,NULL,0);
	if(p==(char*)-1)
	{
		printf("shmat error\n");
		exit(1);
	}//printf("P5 started %s\n",p );
	int fd=open(p,O_RDONLY|O_NONBLOCK);
	if(fd<0)
	{
		printf("fifo open error\n");
		exit(1);
	}
	
	while(1)
	{
		sleep(2);
		char *buf=(char*)malloc(BUF_SIZE);
		read(fd,buf,BUF_SIZE);
		if(strlen(buf)>0)
			printf("%s\n", buf);
	}
	return 0;
}
