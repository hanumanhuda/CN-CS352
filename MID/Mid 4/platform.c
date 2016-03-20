#include <stdio.h>
#include <stdlib.h>
#include <semaphore.h>
#include <signal.h>
#include <pthread.h>
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
int platform_no;
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
}
void *display_announce(void *arg)
{
	while(1)
	{
		char *buf=(char*)malloc(255);
		//scanf("%s",buf);
		read(0,buf,BUF_SIZE);
		char *buf1=(char*)malloc(255);
		sprintf(buf1,"Announcement at Platform %d  : %s\n",platform_no+1,buf);
		write(1,buf1,strlen(buf1));
	}
}
int main(int argc, char const *argv[])
{
	char *buf=(char*)malloc(255);
	read(0,buf,BUF_SIZE);
	platform_no=atoi(buf);
	shm_init();
	int pid=flag[3];
	int sfd=socket(AF_INET,SOCK_STREAM,0);
	if (sfd==-1)
	{
		printf("Socket Error : %d\n", errno);
		exit(1);
	}

	struct sockaddr_in saddr;
	memset(&saddr, 0, sizeof(struct sockaddr_in));
	saddr.sin_family=AF_INET;
	saddr.sin_port=htons(port[platform_no]);
		saddr.sin_addr.s_addr=inet_addr("127.0.0.1");
		int addr_len=sizeof(struct sockaddr_in);

		 int ret=bind(sfd,(struct sockaddr*)&saddr,sizeof(saddr));	
		if (ret==-1)
		{
			printf("Bind Error : %d\n", errno);
			exit(1);
		}

		ret=listen(sfd,10);
		if (ret==-1)
		{
			printf("Listen Error : %d\n", errno);
			exit(1);
		}
		pthread_t tid;
		pthread_create(&tid,NULL,&display_announce,NULL);
		while(1)
		{
			sleep(1);
		    struct sockaddr_in caddr;
		    int caddr_len=sizeof(saddr);
			memset(&caddr, 0, sizeof(struct sockaddr_in));
			int nsfd=accept(sfd,(struct sockaddr*)&caddr,&caddr_len);
			if(nsfd==-1)
			{
				printf("Accept Error : %d\n", errno);
				exit(1);
			}
			char *buf1=(char*)malloc(255);
			recv(nsfd,buf1,BUF_SIZE,0);
			printf("Compartment Details : %s\n",buf1 );
			sleep(50);
			flag[platform_no]=0;
			send(nsfd,"Left",4,0);
			kill(pid,SIGUSR1);
		}

	return 0;
}
