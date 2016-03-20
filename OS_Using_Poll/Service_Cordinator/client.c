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


struct Data
{
	char name[30][30];
	int in;
	int out;
};
void *function1(void *arg)
{
	char *fifoname=(char*)arg;
	int fd=open(fifoname,O_RDONLY);
	char buf[BUF_SIZE];
	while(1)
	{sleep(2);
			read(fd,buf,BUF_SIZE);
			if(strlen(buf)!=0)
			{
				printf("%s\n", buf);
				break;
			}

	}
}
void *function2(void *arg)
{
	char *fifoname=(char*)arg;
	int fd=open(fifoname,O_WRONLY);
	char buf[BUF_SIZE];
	scanf("%s",buf);
	write(fd,buf,strlen(buf));
}
int main(int argc, char const *argv[])
{
	if(argc!=2)
	{
		printf("Pass the FIfo name\n");
		exit(1);
	}
	
	char *name="/tmp/";
	char num[20];

	char fifoname[255],fifoname2[255];
	sprintf(fifoname,"%s%s",name,argv[1]);
	
	sprintf(fifoname2,"%s%s%d",name,argv[1],1);
	int res=mkfifo(fifoname,0666);
	
	if(res<0)
		printf("mkfifo error\n");
	
	res=mkfifo(fifoname2,0666);

	//writing Fifoname to shared area
	int shmid;
	int k3=ftok(".",3);
    shmid=shmget(k3,sizeof(struct Data*),0777|IPC_CREAT);
    struct Data *d;
    d=(struct Data*)shmat(shmid,NULL,0);
   
    if(d==(struct Data*)-1)
    {
    	printf("semat error\n");
    }
    //d->name=(char*)malloc(30);
    strcpy(d->name[d->out],fifoname);
    
   // printf("%d %d %s\n",d->in,d->out,d->name[d->out] );
    d->out++;
    pthread_t pid1,pid2;
    pthread_create(&pid1,NULL,&function1,(void*)fifoname2);
    pthread_create(&pid2,NULL,&function2,(void*)fifoname);

    //accessing the process ID of the server
    int pid;
     k3=ftok(".",6);
    pid=shmget(k3,sizeof(int),0777|IPC_CREAT);
    int *p;
    p=shmat(pid,NULL,0);
   
    if(p==(int*)-1)
    {
    	printf("semat error\n");
    }


    kill((*p),SIGUSR1);
    pthread_join(pid1,NULL);
    pthread_join(pid2,NULL);

	return 0;
}
