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



int cnt=0;
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


int  init_UDSSER()
{
	int usfd;
	assert((usfd=socket(AF_UNIX,SOCK_STREAM,0))!=-1);

	struct sockaddr_un addr;
	addr.sun_family=AF_UNIX;
	strncpy(addr.sun_path,"./usfd",sizeof(addr.sun_path)-1);
	int addr_len=sizeof(struct sockaddr_un);
	unlink("./usfd");
	assert(bind(usfd,(struct sockaddr*)&addr,addr_len)!=-1);
	assert(listen(usfd,3)!=-1);
	return usfd;
}
void accept_UDS(int usfd,int nusfd[],int n)
{
	struct sockaddr_un addr;
	int addr_len=sizeof(struct sockaddr_un);
	int i;
	for(i=0;i<n;i++)
	{
		assert((nusfd[i]=accept(usfd,(struct sockaddr*)&addr,&addr_len))!=-1);
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
		cnt++;
		int i;
		for(i=0;i<strlen(buf);i++)
		if(buf[i]>=97&&buf[i]<=122)
			buf[i]=buf[i]-32;
		send(fd,buf,strlen(buf),0);
		sleep(1);
	}


}

int main(int argc, char const *argv[])
{
	if (argc!=2)
	{
		printf("Pass the main server Port\n");
		exit(1);
	}

	int usfd=init_UDSSER();
	int nusfd[1];
	int pid=fork();
	if(pid==0)
	{
		execl("bs","bs",NULL);
	}
	printf("%d\n",pid );
	accept_UDS(usfd,nusfd,1);

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
	int nsfd[100];
	pthread_t t[100];
	write_fd(nusfd[0],sfd);
	while(1)
	{	
		sleep(1);
		struct sockaddr_in caddr;
		memset(&caddr, 0, sizeof(struct sockaddr_in));
		nsfd[i]=accept(sfd,(struct sockaddr*)&caddr,&caddr_len);
		if(nsfd[i]==-1)
		{
			printf("Accept Error : %d\n", errno);
			exit(1);
		}
		write_fd(nusfd[0],nsfd[i]);
		pthread_create(&t[i],NULL,&service,(void*)&nsfd[i]);
		i++;
		if(cnt>5)
			break;
	}
	kill(pid,SIGUSR1);
	return 0;
}