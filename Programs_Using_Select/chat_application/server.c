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
#include <sys/select.h>
#include <sys/time.h>
#define BUF_SIZE 255
/*struct pollfd
{
	int fd;
	int events;
	int revents;
};*/
int pfd[100];
int mx[100]={0};
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

    pfd[nfd]=open(fifoname,O_RDONLY);
    mx[nfd]=pfd[nfd]+1;
    if(pfd[nfd]<1)
    	printf("pfd error\n");;
    sprintf(fifoname,"%s%d",fifoname,1);
    fd[nfd]=open(fifoname,O_WRONLY);
    if(fd[nfd]<1)
    	printf("pfd error\n");
    //FD_SET(pfd[nfd],&rd[nfd]);
    //FD_ZERO(&rd[nfd]);
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
    int i=0,j;
    while(nfd==0);
    printf("start\n");
    while(1)
    {
        struct timeval tv;
        tv.tv_sec = 1;
        tv.tv_usec = 0;
        fd_set tr;
        FD_ZERO(&tr);
        FD_SET(pfd[i],&tr);
        
        int res=select(mx[i],&tr,NULL,NULL,&tv);
        
        //read(pfd[i],buf,BUF_SIZE);
        //printf("in while %d %d %d\n",res,mx[i],i);
        if(res>0&&FD_ISSET(pfd[i],&tr))
        {//printf("in if \n");
    		char *msg;
    		msg=(char*)(malloc(BUF_SIZE));
    		char *buf;
            buf=(char*)(malloc(BUF_SIZE));
    		read(pfd[i],buf,BUF_SIZE);
    		char *info="Sender ";
    		sprintf(msg,"%s%d%s",info,i,buf);
    		msg[strlen(msg)+1]='\0';
    		
    		for(j=0;j<nfd;j++)
    			if(j!=i)
    				write(fd[j],msg,BUF_SIZE);
            FD_ZERO(&tr);
        }
        FD_CLR(pfd[i],&tr);
    	i=(i+1)%nfd;//sleep(2);
    }
	return 0;
}