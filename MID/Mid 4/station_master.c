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
#include <sys/socket.h>
#include <netinet/in.h>
#define BUF_SIZE 255

int port[]={8801,8802,8803};
//int port[]={8001,8002,8003};
int *flag;
int queue[100],no[100];
int scnt=0,tcnt=0;
int fd[3];
void shm_init()
{
	int k=ftok(".",1);
	int shmid=shmget(k,sizeof(int)*4,0777|IPC_CREAT);
	if (shmid==-1)
	{
		printf("Shmid Error : %d\n",errno);
		exit(1);
	}
	flag=(int*)shmat(shmid,NULL,0);
	if(flag==(int*)-1)
	{
		printf("Shmat Error : %d\n",errno);
		exit(1);	
	}
	int i;
	for (i = 0; i < 3; ++i)
	{
		flag[i]=0;
	}
}
void announce(int st_no,int trn_no)
{
	char *buf=(char*)malloc(255);
	sprintf(buf,"train_no %d is reached at platform %d ...",trn_no,st_no);
	int i;
	for (i = 0; i < 3; ++i)
	{
		write(fd[i],buf,strlen(buf));
	}
}

void handler()
{
	if(scnt<tcnt)
	{
		int i;
		for (i = 0; i < 3; ++i)
		{
			if(flag[i]==0)
				break;
		}
		if(i!=3)
		{
			flag[i]=1;
			char *buf=(char*)malloc(255);
			sprintf(buf,"%d %d",port[i],no[scnt]);
			send(queue[scnt],buf,strlen(buf),0);

			close(queue[scnt]);
			announce(i,no[scnt]);
			scnt++;
		}
	}
}
int main(int argc, char const *argv[])
{
		int train_no=1;
		shm_init();
		flag[3]=getpid();
		if (argc!=2)
		{
			printf("Pass the Port number\n");
			exit(1);
		}
		int sfd=socket(AF_INET,SOCK_STREAM,0);
		if (sfd==-1)
		{
			printf("Socket Error : %d\n",errno);
			exit(1);
		}

		struct sockaddr_in saddr;
		memset(&saddr, 0, sizeof(struct sockaddr_in));
		saddr.sin_family=AF_INET;
		saddr.sin_port=htons(atoi(argv[1]));
		saddr.sin_addr.s_addr=inet_addr("127.0.0.1");
		int addr_len=sizeof(struct sockaddr_in);

		int ret=bind(sfd,(struct sockaddr*)&saddr,sizeof(saddr));	
		if (ret==-1)
		{
			printf("Bind Error : %d\n",errno);
			exit(1);
		}

		ret=listen(sfd,10);
		if (ret==-1)
		{
			printf("Listen Error : %d\n", errno);
			exit(1);
		}

		signal(SIGUSR1,handler);
		fd[0]=fileno(popen("./p1","w"));
		fd[1]=fileno(popen("./p2","w"));
		fd[2]=fileno(popen("./p3","w"));


		int i;
		for (i = 0; i < 3; ++i)
		{
			char *buf=(char*)malloc(255);
			sprintf(buf,"%d",i);
			write(fd[i],buf,strlen(buf));
		}

		while(1)
		{
			struct timeval tv;
			tv.tv_sec = 1;
		    tv.tv_usec = 0;
		    fd_set tr;
		    FD_ZERO(&tr);
		    FD_SET(sfd,&tr);
		    int res=select(sfd+1,&tr,NULL,NULL,&tv);
		    if(res>0&&FD_ISSET(sfd,&tr))
		    {
		    	struct sockaddr_in caddr;
				memset(&caddr, 0, sizeof(struct sockaddr_in));
				int caddr_len=sizeof(saddr);
				int nsfd=accept(sfd,(struct sockaddr*)&caddr,&caddr_len);
				if(nsfd==-1)
				{
					printf("Accept Error : %d\n", errno);
					exit(1);
				}
				int i;
				for (i = 0; i < 3; ++i)
				{
					if(flag[i]==0)
						break;
				}
				if(i!=3)
				{
					flag[i]=1;
					char *buf=(char*)malloc(255);
					sprintf(buf,"%d %d",port[i],train_no);
					send(nsfd,buf,strlen(buf),0);
					close(nsfd);
					announce(i,train_no);
				}
				else
				{
					queue[tcnt]=nsfd;
					no[tcnt]=train_no;
					tcnt++;
					printf("%d is waiting ...\n",train_no );
				}
				train_no++;
		    }
		}
}