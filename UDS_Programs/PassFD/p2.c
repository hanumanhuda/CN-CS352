#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <assert.h>
#include <sys/un.h>
#include <fcntl.h>
#include <errno.h>
#define BUF_SIZE 255




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
	if((n=recvmsg(fd,&msg,0))<=0)
	{
		return n;
	}


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

int main(int argc, char const *argv[])
{
	int cusfd;
	assert((cusfd=socket(AF_UNIX,SOCK_STREAM,0))!=-1);

	struct sockaddr_un addr;
	addr.sun_family=AF_UNIX;
	strncpy(addr.sun_path,"./usfd",sizeof(addr.sun_path)-1);
	int addr_len=sizeof(struct sockaddr_uwn);
	assert(connect(cusfd,(struct sockaddr*)&addr,addr_len)!=-1);
	while(1)
	{
		int fd=-1;
		char *buf=(char*)malloc(2);
		int ret=read_fd(cusfd,&fd);
		if(ret==-1)
		{
			printf("recvfd error %d\n",errno );
			exit(1);
		}
		char *msg=(char*)malloc(BUF_SIZE);
		int count;
		printf("Enter No. of char to read : \n");
		scanf("%d",&count);
		read(fd,msg,count);
		printf("Data : %s\n", msg);
		write_fd(cusfd,fd);
	}
	
	return 0;
}