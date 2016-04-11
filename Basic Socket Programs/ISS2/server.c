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
#include <arpa/inet.h>
#include <unistd.h>
#define BUF_SIZE 255
#define No_of_Ports 4
struct SERVERS
{
	int port;
	char protocol[10];
	char service[30];
	int listen;
};
struct SERVERS S[No_of_Ports];
int sfd[No_of_Ports];
void init()
{
	int i;
	FILE *fp=fopen("port.txt","r");
	char buf[BUF_SIZE];
	for(i=0;i<No_of_Ports;i++)
	{
		int x;
		fscanf(fp,"%d",&S[i].port);
		fscanf(fp,"%s",S[i].protocol);
		fscanf(fp,"%s",S[i].service);
		fscanf(fp,"%d",&S[i].listen);
		//printf("%s %s %d\n",S[i].protocol,S[i].service,S[i].listen );
	}
}
int *cnt;
void get_shm()
{
	int k=ftok(".",1);
	int shmid=shmget(k,sizeof(int)*No_of_Ports,0777|IPC_CREAT);
	if (shmid==-1)
	{
		printf("Shmid Error : %d\n",errno);
		exit(1);
	}
	cnt=(int*)shmat(shmid,NULL,0);
	if(cnt==(int*)-1)
	{
		printf("Shmat Error : %d\n",errno);
		exit(1);	
	}
	int i;
	for (i = 0; i < No_of_Ports; ++i)
	{
		cnt[i]=S[i].listen;
	}
}
void print_status()
{
	int i;
	for (i = 0; i < No_of_Ports; ++i)
	{
		printf("Port %d : ",S[i].port);
		if (cnt[i]==S[i].listen)
		{
			printf("Listening\n");
		}
		else
		{
			printf("Connected to %d clients\n",S[i].listen-cnt[i]);
		}
	}
}
int main(int argc, char const *argv[])
{
	init();
	get_shm();
	int ret,i;
	for(i=0;i<No_of_Ports;i++)
	{
		sfd[i]=socket(AF_INET,SOCK_STREAM,0);
		if (sfd[i]==-1)
		{
			printf("Socket Error : %d %d\n",i, errno);
			exit(1);
		}

		struct sockaddr_in saddr;
		memset(&saddr, 0, sizeof(struct sockaddr_in));
		saddr.sin_family=AF_INET;
		saddr.sin_port=htons(S[i].port);
		saddr.sin_addr.s_addr=inet_addr("127.0.0.1");
		int addr_len=sizeof(struct sockaddr_in);

		ret=bind(sfd[i],(struct sockaddr*)&saddr,sizeof(saddr));	
		if (ret==-1)
		{
			printf("Bind Error : %d %d\n", i,errno);
			exit(1);
		}

		ret=listen(sfd[i],S[i].listen);
		if (ret==-1)
		{
			printf("Listen Error : %d %d\n",i, errno);
			exit(1);
		}
	}
	int caddr_len;
	i=0;
	printf("Port : 8889 Service :Capital to Small.\n");
	printf("Port : 8888 Service :Small to Capital.\n");
	printf("Port : 8887 Service :Reverse the String.\n");
	printf("Port : 8886 Service :Check For palindrome.\n");
	while(1)
	{
		if(i==0)
			print_status();
		struct timeval tv;
	    tv.tv_sec = 1;
	    tv.tv_usec = 0;
	    fd_set tr;
	    FD_ZERO(&tr);
	    FD_SET(sfd[i],&tr);
	    int res=select(sfd[i]+1,&tr,NULL,NULL,&tv);
	    if(res>0&&FD_ISSET(sfd[i],&tr))
	    {
			struct sockaddr_in caddr;
			memset(&caddr, 0, sizeof(struct sockaddr_in));
			int nsfd=accept(sfd[i],(struct sockaddr*)&caddr,&caddr_len);
			if(cnt[i]==0)
			{
				send(nsfd,"-1",strlen("-1"),0);
				close(nsfd);
			}
			else
			{
				send(nsfd,"1",strlen("1"),0);
				cnt[i]--;
				if(nsfd==-1)
				{
					printf("Accept Error : %d\n", errno);
					exit(1);
				}	
				printf("Connection Accepted one more Clinet On Port Number : %d\n", S[i].port);
				int c;
				c=fork();
				if(c==0)
				{
					dup2(nsfd,0);
					dup2(nsfd,1);
					execl(S[i].service,S[i].service,NULL);
				}
				else
				{
					//close(nsfd);
				}
			}
		}
		i=(i+1)%No_of_Ports;
	}
	return 0;
}