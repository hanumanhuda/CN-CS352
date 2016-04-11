#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <assert.h>
#include <arpa/inet.h>
#include <string.h>
#include <pthread.h>
#define BUF_SIZE 255
void *service(void *arg)
{
    int *t=(int*)arg;
    int fd=*t;
    while(1)
    {
        printf("Enter : \n");
        char *buf=(char*)malloc(BUF_SIZE);
        read(0,buf,BUF_SIZE);
        send(fd,buf,strlen(buf),0);
        int ret=recv(fd,buf,BUF_SIZE,0);
        if(ret==0)
            break;
        printf("Result : %s",buf);
        sleep(1);
    }

}

int main(int argc,char *argv[])
{    
    if(argc!=2)
    {
        printf("Pass 1 port  numbers \n");
        exit(1);
    }
    int sfd,a=1;
    assert((sfd=socket(AF_INET,SOCK_STREAM,0))!=-1);
    assert(setsockopt(sfd,SOL_SOCKET,SO_REUSEPORT,&a,sizeof(a))!=-1);

   // struct sockaddr_in caddr;
   // caddr.sin_port=htons(atoi(argv[1]));
   // caddr.sin_family=AF_INET;
   // caddr.sin_addr.s_addr=inet_addr("127.0.0.1");
    int add_len=sizeof(struct sockaddr_in);

    
   // assert(bind(sfd,(struct sockaddr*)&caddr,add_len)!=-1);
   // assert(listen(sfd,3)!=-1);
    struct sockaddr_in saddr;   
    saddr.sin_port=htons(atoi(argv[1]));
    saddr.sin_family=AF_INET;
    saddr.sin_addr.s_addr=inet_addr("127.0.0.1");
    int cfd;
    assert((cfd=socket(AF_INET,SOCK_STREAM,0))!=-1);
    assert(setsockopt(cfd,SOL_SOCKET,SO_REUSEPORT,&a,sizeof(a))!=-1);
    assert(connect(cfd,(struct sockaddr*)&saddr,add_len)!=-1);

    assert(getsockname(cfd,(struct sockaddr*)&saddr,&add_len)!=-1);    
    //saddr.sin_addr.s_addr=inet_addr("127.0.0.1");
    assert(bind(sfd,(struct sockaddr*)&saddr,add_len)!=-1);
    assert(listen(sfd,3)!=-1);

    int nsfd;
    char *buf=(char*)malloc(BUF_SIZE);
    recv(cfd,buf,BUF_SIZE,0);
    if(strcmp(buf,"1")!=0)
    {    
        struct sockaddr_in c1addr;   
        c1addr.sin_port=htons(atoi(buf));
        c1addr.sin_family=AF_INET;
        c1addr.sin_addr.s_addr=inet_addr("127.0.0.1");
        int cfd1;
        assert((cfd1=socket(AF_INET,SOCK_STREAM,0))!=-1);
        assert(connect(cfd1,(struct sockaddr*)&c1addr,add_len)!=-1);
        send(cfd1,"request",strlen("request")+1,0);
        recv(cfd1,buf,BUF_SIZE,0);
        if(strcmp(buf,"per")!=0)
        {   
            printf("Permission is not given by client\n");
            while(1);
        }
        send(cfd,"per",strlen("per"),0);
        recv(cfd,buf,BUF_SIZE,0);
    }
   // send(cfd,argv[1],strlen(argv[1]),0);
    pthread_t tid;
    struct sockaddr_in taddr;
    pthread_create(&tid,NULL,&service,(void*)&cfd);
    while(1)
    {
        assert((nsfd=accept(sfd,(struct sockaddr*)&taddr,&add_len))!=-1);
        recv(nsfd,buf,BUF_SIZE,0);
        if(strcmp(buf,"request")==0)
        {
            printf("Request by other client and permission to him\n");
            send(nsfd,"per",strlen("per")+1,0);
        }
    }
}
