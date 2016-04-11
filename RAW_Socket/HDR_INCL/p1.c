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
int main()
{
	int rsfd;
	rsfd=socket(AF_INET,SOCK_RAW,146);
	if (rsfd==-1)
	{
		printf("socket Error %d\n",errno );
		exit(1);
	}
	int a=1;
	socklen_t addr_len=sizeof(struct sockaddr_in);
	struct sockaddr_in addr,s_addr,d_addr;
	int ret;
	ret=setsockopt(rsfd,IPPROTO_IP, IP_HDRINCL,&a,sizeof(a));
	if (ret==-1)
	{
		printf("Bind Error %d\n",errno );
		exit(1);
	}
	struct iphdr *iph;
	unsigned int iphlen;
	while(1)
	{
		char *buf=(char*)malloc(128);
		char *buffer=(char*)malloc(BUF_SIZE);
		memset(buf,0,128);
		ret=recvfrom(rsfd,buf,BUF_SIZE,0,(struct sockaddr*)&addr,&addr_len);
		if(ret<0)
		{
			printf("Recv Error %d\n",errno );
			exit(1);
		}
		iph=(struct iphdr*)buf;
		memset(&s_addr,0,sizeof(s_addr));
		memset(&d_addr,0,sizeof(d_addr));
		s_addr.sin_addr.s_addr=iph->saddr;
		d_addr.sin_addr.s_addr=iph->daddr;
		iphlen=iph->ihl*4;
		printf("------------- IP Header ------------\n");
		printf("|%4d|%4d|%8d|%16d|\n", (unsigned int)iph->version, (unsigned int)iph->ihl, (unsigned int)iph->tos, ntohs(iph->tot_len));
		printf("------------------------------------\n");
		printf("|%13d|R|D|M|%13d|\n", ntohs(iph->id), (unsigned int)iph->frag_off);
		printf("------------------------------------\n");
		printf("|%8d|%8d|%16d|\n", (unsigned int)iph->ttl, (unsigned int)iph->protocol, ntohs(iph->check));
		printf("------------------------------------\n");
		printf("%s\n", inet_ntoa(s_addr.sin_addr));
		printf("------------------------------------\n");
		printf("%s\n", inet_ntoa(d_addr.sin_addr));
		printf("------------------------------------\n");
		strcpy(buffer, buf+iphlen);
		printf("\tMessage :  %s\n", buffer);
	}
	return 0;
}