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
/*struct pollfd
{
	int fd
	int events;
	int revents;
};
*/
int main(int argc, char const *argv[])
{
	if(argc!=3)
	{
		printf("pass the 2 fifonames\n");
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
    	printf("semat error\n");
    	exit(1);
    }
	//strcpy(fifoname,"/tmp/mk1difsi");
	sprintf(fifoname,"/tmp/%s",argv[1]);
    int res=mkfifo(fifoname,0777);
        //printf("%s\n", fifoname);
    if(res<0)
    {
    	printf("mkfifo 1 error\n");
    	exit(1);
    }
    struct pollfd fds[1];
    fds[0].fd=open(fifoname,O_RDONLY);
    fds[0].events=POLLIN;


    char sfifo[BUF_SIZE];
   	sprintf(sfifo,"/tmp/%s",argv[2]);
   	res=mkfifo(sfifo,0666);
   	if(res<0)
   	{
   		printf("mkfifo error %d \n",errno );
   		exit(1);
   	}
   	int sfd=open(sfifo,O_RDONLY|O_NONBLOCK);
   	if(sfd<0)
   	{
   		printf("Open Error %d\n",errno );
   		exit(1);
   	}
	while(1)    
	{
		sleep(1);
		char buf[BUF_SIZE];
		res=poll(fds,1,0);
		//printf("%d %d\n",res,fds[0].revents);
		if(res>=0&&fds[0].revents==1)
		{
			//printf("started\n");
			read(fds[0].fd,buf,BUF_SIZE);
			char msg[BUF_SIZE];
			char service[BUF_SIZE];
			int i,j=0;;
			
			for(i=0;i<strlen(buf);i++)
				if(buf[i]=='/')
					break;
				else
					service[j++]=buf[i];
			service[j]=0;
			j=0;
			i++;
			while(i<strlen(buf))
			{
				msg[j++]=buf[i++];
			}
			msg[j]='\0';
			int c=0;
			int fd[2];
			//char *service1="o2";
			pipe(fd);
			c=fork();
			if(c>0)
			{
				close(fd[0]);
				strcat(msg,":");
				strcat(msg,sfifo);
				write(fd[1],msg,strlen(msg)+1);
			}
			else
			{
				close(fd[1]);	
				dup2(fd[0],0);
				//printf("%s %s\n",service,msg );
				execl(service,service,NULL)	;
			}
		}

		char *m;
		m=(char*)malloc(BUF_SIZE);
		read(sfd,m,BUF_SIZE);
		if(strlen(m)>0)
			printf("log : %s\n",m);
		m=NULL;
	}
	shmdt(fifoname);
	return 0;
}