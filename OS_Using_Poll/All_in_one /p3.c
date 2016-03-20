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
char msg[BUF_SIZE*2];
char *fifo;
void handler()
{
	//printf("handler called\n");
	int fd=open(fifo,O_WRONLY);//printf("Message %s\t%s\n",fifo,msg);
	if(fd<0)
		printf("fd error\n");
	write(fd,msg,strlen(msg));
	close(fd);
	strcpy(msg,"");
}
int main(int argc, char const *argv[])
{
	if(argc!=2)
	{
		printf("Pass the fifo name\n");
		exit(1);
	}
	//sharing Process Id
	int k1=ftok(".",1);
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
	*p=getpid();
	signal(SIGUSR1,handler);

	k1=ftok("..",3);
	shmid=shmget(k1,30,0777|IPC_CREAT);
	if(shmid==-1)
	{
		printf("shmget 1error\n");
		exit(1);
	}
	
	char *p1=shmat(shmid,NULL,0);
	if(p1==(char*)-1)
	{
		printf("shmat error\n");
		exit(1);
	}
	char *name="/tmp/";
	fifo=(char*)malloc(BUF_SIZE);
	sprintf(fifo,"%s%s",name,argv[1]);
	int r=mkfifo(fifo,0777);
	if(r<0)
	{
		printf("mkfifo error\n");
		exit(1);
	}
	strcpy(p1,fifo);

	//getting fifo name
	struct pollfd pfd[5];
	k1=ftok(".",2);
	shmid=shmget(k1,30,0777|IPC_CREAT);
	if(shmid==-1)
	{
		printf("shmget error\n");
		exit(1);
	}
	char *p2;
	p2=shmat(shmid,NULL,0);
	if(p2==(char*)-1)
	{
		printf("shmat error\n");
		exit(1);
	}
	int c=0;
	int fd[2];
	pipe(fd);
	c=fork();
	strcpy(msg,"");
	if(c>0)
	{
		int nfd=4;
		pfd[0].fd=0;
		pfd[1].fd=open(p2,O_RDONLY);
		pfd[0].events=POLLIN;
		pfd[1].events=POLLIN;
		FILE *f=popen("./po","r");
		pfd[2].fd=fileno(f);
		pfd[2].events=POLLIN;
		close(fd[1]);
		pfd[3].fd=fd[0];
		pfd[3].events=POLLIN;

		while(1)
		{
			sleep(3);
			int res=poll(pfd,nfd,0);
			if(res>0)
			{
				int i;//printf("inside Poll\n");
				for(i=0;i<nfd;i++)
				{
					if(pfd[i].revents==1)
					{
						char *buf=(char*)malloc(BUF_SIZE);
						read(pfd[i].fd,buf,BUF_SIZE);//printf("inside event %d\t%s\n", i,buf);
						strcat(msg,"\t");
						strcat(msg,buf);
					}
				}
			}
			
			
		}
	}
	else
	{
		close(fd[0]);
		while(1)
		{
			sleep(10);
			char *m="This is from pipe";
			write(fd[1],m,strlen(m));
		}
	}
	return 0;
}