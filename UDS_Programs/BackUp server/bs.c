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
#define BUF_SIZE 255

int flag=0;
ssize_t write_fd(int fd,int sendfd)
{

	char *buf=(char*)malloc(2);
	int nbytes=2;
	void *ptr=buf;
	struct msghdr msg;
	struct iovec iov[1];
	union{
		struct cmsghdr cm;
		char control[CMSG_SPACE(sizeof(int))];
	} control_un;
	struct cmsghdr *cmptr;

	msg.msg_control=control_un.control;
	msg.msg_controllen=sizeof(control_un.control);

	cmptr=CMSG_FIRSTHDR(&msg);
	cmptr->cmsg_len=CMSG_LEN(sizeof(int));
	cmptr->cmsg_level=SOL_SOCKET;
	cmptr->cmsg_type=SCM_RIGHTS;
	*((int *)CMSG_DATA(cmptr))=sendfd;

	msg.msg_name=NULL;
	msg.msg_namelen=0;

	iov[0].iov_base=ptr;
	iov[0].iov_len=nbytes;
	msg.msg_iov=iov;
	msg.msg_iovlen=1;
	sendmsg(fd,&msg,0);
}

ssize_t read_fd(int fd,int *recvfd)
{

	char *buf=(char*)malloc(2);
	int nbytes=2;
	void *ptr=buf;
	struct msghdr msg;
	struct iovec iov[1];
	union{
		struct cmsghdr cm;
		char control[CMSG_SPACE(sizeof(int))];
	} control_un;
	struct cmsghdr *cmptr;

	msg.msg_control=control_un.control;
	msg.msg_controllen=sizeof(control_un.control);
	int n;

	msg.msg_name=NULL;
	msg.msg_namelen=0;

	iov[0].iov_base=ptr;
	iov[0].iov_len=nbytes;
	msg.msg_iov=iov;
	msg.msg_iovlen=1;
	if((n=recvmsg(fd,&msg,0))<0)
	{
		return n;
	}

	//printf("Middle read\n");
	if((cmptr=CMSG_FIRSTHDR(&msg))!=NULL && cmptr->cmsg_len==CMSG_LEN(sizeof(int)))
	{
		if(cmptr->cmsg_level!=SOL_SOCKET)
		{
			perror("control level !=SOL_SOCKET");
			exit(1);
		}
		if(cmptr->cmsg_type!=SCM_RIGHTS)
		{
			perror("control type !=SCM_RIGHTS");
			exit(1);	
		}
		*recvfd=*((int*)CMSG_DATA(cmptr));
	}
}

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
			close(fd);
			break;
		}
		int i;
		for(i=0;i<strlen(buf);i++)
		if(buf[i]>=97&&buf[i]<=122)
			buf[i]=buf[i]-32;
		send(fd,buf,strlen(buf),0);
		sleep(1);
	}


}

int init_UDSCLI()
{
	int cusfd;
	assert((cusfd=socket(AF_UNIX,SOCK_STREAM,0))!=-1);

	struct sockaddr_un addr;
	addr.sun_family=AF_UNIX;
	strncpy(addr.sun_path,"./usfd",sizeof(addr.sun_path)-1);
	int addr_len=sizeof(struct sockaddr_un);
	assert(connect(cusfd,(struct sockaddr*)&addr,addr_len)!=-1);
	return cusfd;
}			
void handle()
{
	printf("Signal is hanled by process\n");
	flag=1;
}
int main(int argc, char const *argv[])
{

	printf("Back-UP Servers started\n");
	signal(SIGUSR1,handle);
	int cusfd=init_UDSCLI();
	int nsfd[100];
	pthread_t t[100];

	int i=0;
	while(flag==0)
	{
		sleep(1);
		read_fd(cusfd,&nsfd[i]);		
		i++;
	}
	int j=0,sfd=nsfd[0];
	for(j=1;j<i;j++)
	{
		pthread_create(&t[j],NULL,&service,(void*)&nsfd[j]);
	}
	int caddr_len=sizeof(struct sockaddr_in);
	while(1)
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
	}
	return 0;
}
