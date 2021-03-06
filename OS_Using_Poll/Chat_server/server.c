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
/*struct pollfd
{
	int fd;
	int events;
	int revents;
};*/
struct pollfd pfd[100];
int nfd=0;
int fd[100];
void handler()
{
    //shared memory to accessing the client fifoname
	int shmid;
	int k3=ftok(".",3);
    shmid=shmget(k3,30,777|IPC_CREAT);
     if (shmid==-1)
    {
        printf("shmget error %d\n",errno);
        exit(1);
    }
    char *fifoname;
    fifoname=shmat(shmid,NULL,0);
   
    if(fifoname==(char*)-1)
    {
    	printf("semat error %d\n",errno);
    }

    pfd[nfd].fd=open(fifoname,O_RDONLY);
    if(pfd[nfd].fd<1)
    	printf("pfd error\n");;
    sprintf(fifoname,"%s%d",fifoname,1);
    fd[nfd]=open(fifoname,O_WRONLY);
    if(fd[nfd]<1)
    	printf("pfd error\n");;
    pfd[nfd].events=POLLIN;
    nfd++;
}
int main(int argc, char const *argv[])
{
    //shared memory to write Process ID
	int pid;
    int k3=ftok(".",1);
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
    (*p)=getpid();
   // printf("%d\n", (*p));
    //ataching handler to SIGUSR1
    signal(SIGUSR1,handler);
    while(1)
    {sleep(2);
    	int res=poll(pfd,nfd,0);
        printf("%d\n",res );
    	if(res>=0)
    	{
    		int i,j;
    		char *msg;
    		msg=(char*)(malloc(BUF_SIZE));
    		for(i=0;i<nfd;i++)
    		{
    			
    			if(pfd[i].revents==1)
    			{
    				char *buf;
    				buf=(char*)(malloc(BUF_SIZE));
    				read(pfd[i].fd,buf,BUF_SIZE);
    				char *info="Sender ";
    				sprintf(msg,"%s%d%s",info,i,buf);
    				
    				msg[strlen(msg)+1]='\0';
    				break;
    			}
    		}
    		if(i<nfd)
    		for(j=0;j<nfd;j++)
    			if(j!=i)
    				write(fd[j],msg,BUF_SIZE);

    	}
    }
	return 0;
}