#include <stdio.h>
#include <stdlib.h>
#include <semaphore.h>
#include <signal.h>
#include <pthread.h>
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
#include <sys/un.h>
#include <netinet/ip.h>
#include <assert.h>
#define BUF_SIZE 1024

int rsfd;
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

int *flag;
int platform_no;
void shm_init()
{
	int k=ftok(".",1);
	int shmid=shmget(k,sizeof(int)*4,0777|IPC_CREAT);
	if (shmid==-1)
	{
		printf("Shmid Error : %d\n",errno);
		exit(1);
	}
	flag=(int*)shmat(shmid,NULL,0);
	if(flag==(int*)-1)
	{
		printf("Shmat Error : %d\n",errno);
		exit(1);	
	}
}
void *display_announce(void *arg)
{
	socklen_t addr_len=sizeof(struct sockaddr_in);
	struct sockaddr_in addr;
	struct iphdr *iph;
	unsigned int iphlen;
	while(1)
	{
		char buf[BUF_SIZE],buf1[BUF_SIZE];
		memset(buf,0,128);
		int ret=recvfrom(rsfd,buf,BUF_SIZE,0,(struct sockaddr*)&addr,&addr_len);
		if(ret<0)
		{
			printf("Recv Error %d\n",errno );
			exit(1);
		}
		iph=(struct iphdr*)buf;
		iphlen=iph->ihl*4;
		sprintf(buf1,"Announcement/Adver. at Platform %d  : %s\n",platform_no+1,buf+iphlen);
		write(1,buf1,strlen(buf1));
	}
}
void init_Raw_Socket()
{
	rsfd=socket(AF_INET,SOCK_RAW,146);
	if (rsfd==-1)
	{
		printf("socket Error %d\n",errno );
		exit(1);
	}
	/*int a=1;
	int ret;
	ret=setsockopt(rsfd,IPPROTO_IP, IP_HDRINCL,&a,sizeof(a));
	if (ret==-1)
	{
		printf("Bind Error %d\n",errno );
		exit(1);
	}*/
}

int main(int argc, char const *argv[])
{
	int cusfd=init_UDSCLI();
	init_Raw_Socket();
	char *buf=(char*)malloc(255);
	shm_init();
	int i;
	for(i=0;i<3;i++)
		if(flag[i]==-1)
		{
			flag[i]=0;
			break;
		}
	platform_no=i;
	printf("Platform %d started\n",platform_no );
	
	int pid=flag[3];
	pthread_t tid;
	pthread_create(&tid,NULL,&display_announce,NULL);
	while(1)
	{
		int nsfd;
		read_fd(cusfd,&nsfd);
		char *buf1=(char*)malloc(255);
		int ret=recv(nsfd,buf1,BUF_SIZE,0);
		if(ret<0)
		{
			printf("Recv Error %d\n",errno);
		}
		printf("Compartment Details : %s\n",buf1 );
		memset(buf1,0,255);
		sprintf(buf1,"%d",platform_no);
		send(nsfd,buf1,strlen(buf1),0);
		sleep(10);
		flag[platform_no]=0;
		send(nsfd,"Left",4,0);
		kill(pid,SIGUSR1);
	}
	return 0;
}
