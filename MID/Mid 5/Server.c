#include <stdio.h>
#include <stdlib.h>
#include <semaphore.h>
#include <signal.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <sys/sem.h>
#include <unistd.h>
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


int nsfd[100];
int *flag;
int cnt=0,ecnt=-1;
void shm_init()
{
	int k=ftok(".",1);
	int shmid=shmget(k,sizeof(int)*100,0777|IPC_CREAT);
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
	for (i = 0; i < 100; ++i)
	{
		flag[i]=0;
	}
}

void handler()
{
	ecnt=(ecnt+1)%cnt;
	if(flag[ecnt]==0)
	{
		flag[ecnt]=1;
		int c=fork();
		if(c==0)
		{
			dup2(nsfd[ecnt],0);
			dup2(nsfd[ecnt],1);
			execl("e","e",NULL);
		}
		
	}
}
int main(int argc, char const *argv[])
{
	if(argc!=3)
	{
		printf("Pass the Port and fifo name\n");
		exit(1);
	}
	//sharing Process Id
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
	*p=getpid();
	signal(SIGUSR1,handler);



	k1=ftok(".",3);
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
	char *fifo=(char*)malloc(BUF_SIZE);
	sprintf(fifo,"%s%s",name,argv[2]);
	int r=mkfifo(fifo,0777);
	if(r<0)
	{
		printf("mkfifo error\n");
		exit(1);
	}
	strcpy(p1,fifo);


	int c=fork();
	if(c==0)
	{
		execl("p2","p2",NULL);
	}
	int fd[2];
	pipe(fd);
	c=fork();

	if(c==0)
	{
		dup2(fd[1],1);
		close(fd[0]);
		execl("p1","p1",NULL);
	}	
	int ffd[4];
	ffd[3]=0;
	close(fd[1]);
	ffd[0]=fd[0];
	ffd[1]=open(fifo,O_RDONLY);
	ffd[2]=fileno(popen("./p3","r"));

	shm_init();

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
		int i;
	while(1)
	{
			sleep(1);
			struct timeval tv;
			tv.tv_sec = 0;
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
				nsfd[cnt]=accept(sfd,(struct sockaddr*)&caddr,&caddr_len);
				if(nsfd[cnt]==-1)
				{
					printf("Accept Error : %d\n", errno);
					exit(1);
				}
				flag[cnt]=0;
				char *buf=(char*)malloc(BUF_SIZE);
				sprintf(buf,"%d",cnt);
				send(nsfd[cnt],buf,strlen(buf),0);	
				cnt++;
			}
			for(i=0;i<4;i++)
			{
				char *msg=(char*)malloc(255);
				strcpy(msg,"");
				FD_ZERO(&tr);
		   		FD_SET(ffd[i],&tr);
			    res=select(ffd[i]+1,&tr,NULL,NULL,&tv);
			    if(res>0&&FD_ISSET(ffd[i],&tr))
			    {
			    	char *buf=(char*)malloc(255);
			    	read(ffd[i],buf,BUF_SIZE);
			    	strcat(msg,buf);
				}	
				if(strcmp(msg,"")!=0)
				for(i=0;i<cnt;i++)
				{
					if(flag[i]==0)
					{
						send(nsfd[i],msg,strlen(msg),0);
					}
				}
			}
	}
	return 0;
}