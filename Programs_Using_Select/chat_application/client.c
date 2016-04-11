#include <stdio.h>
#include <stdlib.h>
#include <semaphore.h>
#include <signal.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <sys/sem.h>
#include <sys/ipc.h>
#include <poll.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#define BUF_SIZE 255

void *function1(void *arg)
{
	//reading from server
	char *fifoname=(char*)arg;
	int fd=open(fifoname,O_RDONLY);
	char buf[BUF_SIZE];
	while(1)
	{
			read(fd,buf,BUF_SIZE);
			if(strlen(buf)>0)
			{
				printf("%s\n", buf);
			}

	}
}
void *function2(void *arg)
{
	//writing to the server
	char *fifoname=(char*)arg;
	int fd=open(fifoname,O_WRONLY);
	char buf[BUF_SIZE];
	while(1)
	{
			scanf("%s",buf);
			write(fd,buf,strlen(buf));
	}
}
int main(int argc, char const *argv[])
{
	if(argc!=2)
	{
		printf("Pass the FIfo name \n");
		exit(1);
	}
	
	char *name="/tmp/";
	char num[20];
	char fifoname[255],fifoname2[255];
	sprintf(fifoname,"%s%s",name,argv[1]);
	sprintf(fifoname2,"%s%s%d",name,argv[1],1);

	int res=mkfifo(fifoname,0666);
	if(res<0)
	{
		printf("mkfifo error %d\n",errno);
		exit(1);
	}
	res=mkfifo(fifoname2,0666);
	if(res<0)
	{
		printf("mkfifo error %d\n",errno);
		exit(1);
	}

	//writing Fifoname to shared area
	int shmid;
	int k3=ftok(".",3);
    shmid=shmget(k3,30,0777|IPC_CREAT);
    if (shmid==-1)
    {
    	printf("shmget error %d\n",errno);
    	exit(1);
    }

    char *fifoname1;
    fifoname1=shmat(shmid,NULL,0);
    if(fifoname1==(char*)-1)
    {
    	printf("semat error %d\n",errno);
    	exit(1);
    }
    strcpy(fifoname1,fifoname);

    pthread_t pid1,pid2;
    pthread_create(&pid1,NULL,&function1,(void*)fifoname2);
    pthread_create(&pid2,NULL,&function2,(void*)fifoname);

    //accessing the process ID of the server
    int pid;
     k3=ftok(".",1);
    pid=shmget(k3,sizeof(int),0777|IPC_CREAT);
    if (pid==-1)
    {
    	printf("shmget error %d\n",errno);
    	exit(1);
    }
    int *p;
    p=shmat(pid,NULL,0);
   
    if(p==(int*)-1)
    {
    	printf("semat error %d\n",errno);
    	exit(1);
    }

    //printf("%d\n",(*p) );
    kill((*p),SIGUSR1);printf("end\n");
    pthread_join(pid1,NULL);
    pthread_join(pid2,NULL);

	return 0;
}