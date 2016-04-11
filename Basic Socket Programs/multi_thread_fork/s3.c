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
#define port 8887
void service(void *arg)
{
	int *t=(int*)(arg);
	int fd=(*t);
	while(1)
	{
		char *buf=(char*)malloc(BUF_SIZE);
		int ret=recv(fd,buf,BUF_SIZE,0);
		if(ret==0)
		{
			break;
		}
		int i=0,j=strlen(buf)-2;
		while(j>i)
		{
			char ch=buf[i];
			buf[i]=buf[j];
			buf[j]=ch;
			i++;
			j--;
		}
		buf[strlen(buf)]='\0';
		send(fd,buf,strlen(buf),0);
	}

}
int main(int argc, char const *argv[])
{
	printf("S4 started\n");
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
	saddr.sin_port=htons(port);
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
	int caddr_len;
	pthread_t t[100];
	int nsfd[100];
	i=0;
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
			nsfd[i]=accept(sfd,(struct sockaddr*)&caddr,&caddr_len);
			if(nsfd[i]==-1)
			{
				printf("Accept Error : %d\n", errno);
				exit(1);
			}
			pthread_create(&t[i],NULL,&service,(void*)&nsfd[i]);
			i++;
		}
	}
	return 0;
}