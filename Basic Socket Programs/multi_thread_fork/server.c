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

int main(int argc, char const *argv[])
{
	if (argc!=2)
	{
		printf("Pass the main server Port\n");
		exit(1);
	}
	init();
	int ret,i;
	int sfd=socket(AF_INET,SOCK_STREAM,0);
	if (sfd==-1)
	{
		printf("Socket Error : %d\n", errno);
		exit(1);
	}

	struct sockaddr_in saddr;
	memset(&saddr, 0, sizeof(struct sockaddr_in));
	saddr.sin_family=AF_INET;
	saddr.sin_port=htons(atoi(argv[1]));
	saddr.sin_addr.s_addr=INADDR_ANY;//inet_addr("127.0.0.1");
	int addr_len=sizeof(struct sockaddr_in);

	ret=bind(sfd,(struct sockaddr*)&saddr,sizeof(saddr));	
	if (ret==-1)
	{
		printf("Bind Error : %d\n",errno);
		exit(1);
	}

	ret=listen(sfd,5);
	if (ret==-1)
	{
		printf("Listen Error : %d\n", errno);
		exit(1);
	}
	int caddr_len=sizeof(struct sockaddr_in);
	i=0;
	int st[No_of_Ports]={0};
	printf("Service 1 : Capital to Small.\n");
	printf("Service 2 : Small to Capital.\n");
	printf("Service 3 : Reverse the String.\n");
	printf("Service 4 : Check For palindrome.\n");
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
			int nsfd=accept(sfd,(struct sockaddr*)&caddr,&caddr_len);
			if(nsfd==-1)
			{
				printf("Accept Error : %d\n", errno);
				exit(1);
			}
			char *type;
			type=(char*)malloc(10);
			recv(nsfd,type,10,0);
			i=atoi(type)-1;
			printf("%d\n",i );
			if(i>=0&&i<No_of_Ports)
			{			
				//printf("Connection Accepted one more Clinet On Port Number : %d\n", S[i].port);
				if(st[i]==0)
				{
					st[i]=1;
					int c;
					c=fork();
					if(c==0)
					{
						close(nsfd);
						execl(S[i].service,S[i].service,NULL);
					}
				}
				sprintf(type,"%d",S[i].port);
				send(nsfd,type,strlen(type),0);
			}
			else
			{
				send(nsfd,"This type service is not available",strlen("This type service is not available"),0);
			}
		}
	}
	return 0;
}