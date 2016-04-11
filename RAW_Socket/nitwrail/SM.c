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
#include <sys/un.h>
#include <assert.h>
#include <netinet/ip.h>
#include <unistd.h>
#define BUF_SIZE 255

int *flag;
int queue[100],no[100];
int scnt=0,tcnt=0;
int fd[3];
int nusfd[3];
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
	int i;
	for (i = 0; i < 3; ++i)
	{
		flag[i]=-1;
	}
}
void announce(int st_no,int trn_no)
{
	char *buf=(char*)malloc(255);
	sprintf(buf,"train_no %d is reached at platform %d ...",trn_no,st_no);
	int addr_len=sizeof(struct sockaddr_in);
	struct sockaddr_in addr;
	addr.sin_port=0;
	addr.sin_family=AF_INET;
	addr.sin_addr.s_addr=inet_addr("127.0.0.1");
	int ret=sendto(rsfd,buf,BUF_SIZE,0,(struct sockaddr*)&addr,addr_len);
	if(ret<0)
	{
		printf("Send Error %d\n",errno );
		exit(1);
	}
}

void handler()
{
	printf("Handler Called\n");
	if(scnt<tcnt)
	{
		int i;
		for (i = 0; i < 3; ++i)
		{
			if(flag[i]==0)
				break;
		}
		if(i!=3)
		{
			flag[i]=1;
			char *buf=(char*)malloc(255);
			sprintf(buf,"%d",no[scnt]);
			send(queue[scnt],buf,strlen(buf),0);
			write_fd(nusfd[i],queue[scnt]);
			announce(i,no[scnt]);
			scnt++;
		}
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
}
int main(int argc, char const *argv[])
{
	if (argc!=2)
	{
		printf("Pass the main server Port\n");
		exit(1);
	}
	shm_init();
	int usfd=init_UDSSER();
	init_Raw_Socket();
	
	int i;
	for(i=0;i<3;i++)
	{
		int c=fork();
		if(c==0)
		{
			execl("p","p",NULL);
		}
	}
	accept_UDS(usfd,nusfd,3);
	int train_no=1;
	
	flag[3]=getpid();
	int sfd=socket(AF_INET,SOCK_STREAM,0);
	if (sfd==-1)
	{
		printf("Socket Error : %d\n",errno);
		exit(1);
	}

	struct sockaddr_in saddr;
	memset(&saddr, 0, sizeof(struct sockaddr_in));
	saddr.sin_family=AF_INET;
	saddr.sin_port=htons(atoi(argv[1]));
	saddr.sin_addr.s_addr=inet_addr("127.0.0.1");
	int addr_len=sizeof(struct sockaddr_in);

	int ret=bind(sfd,(struct sockaddr*)&saddr,sizeof(saddr));	
	if (ret==-1)
	{
		printf("Bind Error : %d\n",errno);
		exit(1);
	}

	ret=listen(sfd,10);
	if (ret==-1)
	{
		printf("Listen Error : %d\n", errno);
		exit(1);
	}

	signal(SIGUSR1,handler);
	

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
			int caddr_len=sizeof(saddr);
			int nsfd=accept(sfd,(struct sockaddr*)&caddr,&caddr_len);
			if(nsfd==-1)
			{
				printf("Accept Error : %d\n", errno);
				exit(1);
			}
			int i;
			for (i = 0; i < 3; ++i)
			{
				if(flag[i]==0)
					break;
			}
			if(i!=3)
			{
				flag[i]=1;
				char *buf=(char*)malloc(255);
				sprintf(buf,"%d",train_no);
				send(nsfd,buf,strlen(buf),0);
				write_fd(nusfd[i],nsfd);
				announce(i,train_no);
			}
			else
			{
				queue[tcnt]=nsfd;
				no[tcnt]=train_no;
				tcnt++;
				printf("%d is waiting ...\n",train_no );
			}
			train_no++;
	    }
	}
}