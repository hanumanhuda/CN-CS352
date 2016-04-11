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
#define BUF_SIZE 255
int main(int argc, char const *argv[])
{
	int ret;
	if (argc!=2)
	{
		printf("Pass Port Number\n");
		exit(1);
	}
	int sfd=socket(AF_INET,SOCK_DGRAM,0);
	if (sfd==-1)
	{
		printf("Socket Error : %d\n", errno);
		exit(1);
	}

	struct sockaddr_in saddr,caddr;
	memset(&saddr, 0, sizeof(struct sockaddr_in));
	memset(&caddr, 0, sizeof(struct sockaddr_in));
	saddr.sin_family=AF_INET;
	saddr.sin_port=htons(atoi(argv[1]));
	saddr.sin_addr.s_addr=inet_addr("127.0.0.1");
	int addr_len=sizeof(struct sockaddr_in);

	ret=bind(sfd,(struct sockaddr*)&saddr,addr_len);	
	if (ret==-1)
	{
		printf("Bind Error : %d\n", errno);
		exit(1);
	}

	char *buf;
	int caddr_len=sizeof(struct sockaddr_in);
	while(1)
	{
		buf=(char*)malloc(BUF_SIZE);
		ret=recvfrom(sfd,buf,BUF_SIZE,0,(struct sockaddr*)&caddr,&caddr_len);
		if(ret==0)
			break;
		int i;
		for(i=0;i<strlen(buf);i++)
		if(buf[i]>=97&&buf[i]<=122)
			buf[i]=buf[i]-32;

		sendto(sfd,buf,strlen(buf),0,(struct sockaddr*)&caddr,caddr_len);
		sleep(1);
	}

	return 0;
}