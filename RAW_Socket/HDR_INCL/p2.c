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
#include <sys/un.h>
#include <fcntl.h>
#include <assert.h>
#include <netinet/ip.h>
#define BUF_SIZE 1024
int main(int argc, char const *argv[])
{
	int rsfd;
	rsfd=socket(AF_INET,SOCK_RAW,146);
	if (rsfd==-1)
	{
		printf("socket Error %d\n",errno );
		exit(1);
	}
	int addr_len=sizeof(struct sockaddr_in);
	struct sockaddr_in addr,s_addr,d_addr;
	addr.sin_port=0;//htons(atoi(argv[1]));
	addr.sin_family=AF_INET;
	addr.sin_addr.s_addr=inet_addr("127.0.0.1");
	int ret,a=1;
	ret=setsockopt(rsfd,IPPROTO_IP, IP_HDRINCL,&a,sizeof(a));
	if (ret==-1)
	{
		printf("Bind Error %d\n",errno );
		exit(1);
	}
	struct iphdr *iph;
	unsigned int iphlen;
	 char buffer[BUF_SIZE], buf[128];
	while(1)
	{
		/*char *buf=(char*)malloc(BUF_SIZE);
		char *buffer=(char*)malloc(BUF_SIZE);*/
		printf("Enter : \n");
		memset(buf,0,255);
		read(0,buf,255);
		memset(buffer, 0 , BUF_SIZE);
		iph=(struct iphdr*)buffer;
		iph->version=4;
		iph->ihl=5;
		iph->tos=0;
		iph->frag_off=0;
		iph->ttl=8;
		iph->protocol=146;
		iph->saddr=inet_addr("127.0.11.1");
		iph->daddr=inet_addr("127.0.0.1");
		strcpy(buffer+sizeof(struct iphdr),buf);
		iph->tot_len=htons(sizeof(struct iphdr)+strlen(buf));
		ret=sendto(rsfd,buffer,sizeof(buffer),0,(struct sockaddr*)&addr,addr_len);
		if(ret<0)
		{
			printf("Send Error %d\n",errno );
			exit(1);
		}
		
	}
	return 0;
}