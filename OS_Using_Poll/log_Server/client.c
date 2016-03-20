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
#include <errno.h>
#define BUF_SIZE 255

void *function1(void *arg)
{
	//reading from server
	char *fifoname=(char*)arg;
	int fd=open(fifoname,O_RDONLY);
	char buf[BUF_SIZE];
	while(1)
	{sleep(1);
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
	//writing to the server
	char *fifoname=(char*)arg;
	int fd=open(fifoname,O_WRONLY);
	char buf[BUF_SIZE];
	
			scanf("%s",buf);
			write(fd,buf,strlen(buf));

}
int main(int argc, char const *argv[])
{
	if(argc!=3)
	{
		printf("pass the fifoname and type of request\n");
		exit(1);
	}
	int shmid;
	int k3=ftok("/temp",3);
    shmid=shmget(k3,30,0777|IPC_CREAT);
    if(shmid==-1)
    {
    	printf("shmid error\n");
    	exit(1);
    }
    char *fifoname;
    fifoname=shmat(shmid,NULL,0);
   
    if(fifoname==(char*)-1)
    {
    	printf("semat error %d\n",errno);
    	exit(1);
    }
    //printf("%s\n",fifoname);
    int fd=open(fifoname,O_WRONLY);

	char buf[BUF_SIZE];
	strcpy(buf,"s");
	strcat(buf,argv[2]);
	strcat(buf,"/");
	char *name="/tmp/";
	char num[20];

	char fifoname1[255],fifoname2[255];
	sprintf(fifoname1,"%s%s",name,argv[1]);
	
	sprintf(fifoname2,"%s%s%d",name,argv[1],1);
	int res=mkfifo(fifoname1,0666);
	
	if(res<0)
	{
		printf("mkfifo error %d\n",errno);
		exit(1);
	}
	
	res=mkfifo(fifoname2,0666);
	strcat(buf,fifoname1);
	//writing Fifoname to shared area

	//printf("%s\n",buf );
	write(fd,buf,BUF_SIZE);
	pthread_t pid1,pid2;
    pthread_create(&pid1,NULL,&function1,(void*)fifoname2);
    pthread_create(&pid2,NULL,&function2,(void*)fifoname1);
    pthread_join(pid1,NULL);
    pthread_join(pid2,NULL);
	shmdt(fifoname);
	return 0;
}