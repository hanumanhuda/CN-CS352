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


void msend(void *arg)
{
	int *t=(int*)(arg);
	int fd=(*t);
	while(1)
	{
		sleep(1);
		char *buf;
		buf=(char*)malloc(BUF_SIZE);
		read(0,buf,BUF_SIZE);
		send(fd,buf,strlen(buf),0);
	}
}
void mrecv(void *arg)
{
	int *t=(int*)(arg);
	int fd=(*t);
	while(1)
	{
		sleep(1);
		struct timeval tv;
	    tv.tv_sec = 0;
	    tv.tv_usec = 0;
	    fd_set tr;
	    FD_ZERO(&tr);
	    FD_SET(fd,&tr);
	    int res=select(fd+1,&tr,NULL,NULL,&tv);
	    if(res>0&&FD_ISSET(fd,&tr))
	    {
			char *buf;
			buf=(char*)malloc(BUF_SIZE);
			recv(fd,buf,BUF_SIZE,0);
			printf("%s",buf );
		}
	}
}

void chat(int fd)
{
	pthread_t pid1,pid2;
	pthread_create(&pid1,NULL,&msend,(void*)&fd);
	pthread_create(&pid2,NULL,&mrecv,(void*)&fd);
	pthread_join(pid1,NULL);
	pthread_join(pid2,NULL);
}


int main(int argc, char const *argv[])
{
	int ret;
	if (argc!=2)
	{
		printf("Pass Port Number\n");
		exit(1);
	}
	int cfd=socket(AF_INET,SOCK_STREAM,0);
	if (cfd==-1)
	{
		printf("Socket Error : %d\n", errno);
		exit(1);
	}

	struct sockaddr_in caddr;
	memset(&caddr, 0, sizeof(struct sockaddr_in));
	caddr.sin_family=AF_INET;
	caddr.sin_port=htons(atoi(argv[1]));
	caddr.sin_addr.s_addr=INADDR_ANY;//inet_addr("127.0.0.1");
	int addr_len=sizeof(struct sockaddr_in);

	ret=connect(cfd,(struct sockaddr*)&caddr,addr_len);		
	if (ret==-1)
	{
		printf("Connect Error : %d\n", errno);
		exit(1);
	}
	char m[100];
	ret=recv(cfd,m,100,0);
	printf("Enter Group Number(for new group Enter next value of these numbers) : %s\n",m);
	int type;
	scanf("%d",&type);
	char t[10];
	sprintf(t,"%d",type);
	send(cfd,t,strlen(t),0);
	ret=recv(cfd,m,100,0);
	int port=atoi(m);
	if(port<1000)
	{
		printf("%s\n",m);
		exit(1);
	}
	close(cfd);

	sleep(1);
	printf("%d\n",port );
	cfd=socket(AF_INET,SOCK_STREAM,0);
	if (cfd==-1)
	{
		printf("Socket Error : %d\n", errno);
		exit(1);
	}

	memset(&caddr, 0, sizeof(struct sockaddr_in));
	caddr.sin_family=AF_INET;
	caddr.sin_port=htons(port);
	caddr.sin_addr.s_addr=INADDR_ANY;//inet_addr("127.0.0.1");
	addr_len=sizeof(struct sockaddr_in);

	ret=connect(cfd,(struct sockaddr*)&caddr,addr_len);
	if (ret==-1)
	{
		printf("Connect Error : %d\n", errno);
		exit(1);
	}
	chat(cfd);
	while(1)
	{
		sleep(5);
	}
	return 0;
}