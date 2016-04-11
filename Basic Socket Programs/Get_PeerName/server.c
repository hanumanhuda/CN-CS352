#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <assert.h>
#include <arpa/inet.h>
#include <string.h>
#include <pthread.h>
#define BUF_SIZE 255
int stop=0;
char *buf;
int cnt=0,curr_cfd;
int curr_fd;
void *service(void *arg)
{
    int *t=(int*)arg;
    int fd=*t;
    stop=0;
    //recv(fd,buf,BUF_SIZE,0);
    curr_cfd=fd;
    while(1)
    {
        if(stop==1)
            break;
        char *buf1=(char*)malloc(BUF_SIZE);
       if( recv(fd,buf1,BUF_SIZE,0)==-1)
           break;
        int i;
        for(i=0;i<strlen(buf1);i++)
            if(buf1[i]>=97&&buf1[i]<=122)
                buf1[i]=buf1[i]-32;

       if( send(fd,buf1,strlen(buf1),0)==-1)
           break;
        sleep(1);
    }
    cnt--;
    printf("exited\n");
}

int main(int argc,char *argv[])
{

    buf=(char*)malloc(BUF_SIZE);
    if(argc!=2)
    {
        printf("Pass Port number \n");
        exit(1);
    }
    int sfd;
    assert((sfd=socket(AF_INET,SOCK_STREAM,0))!=-1);
    struct sockaddr_in saddr,caddr,curr_addr;
    saddr.sin_port=htons(atoi(argv[1]));
    saddr.sin_family=AF_INET;
    saddr.sin_addr.s_addr=inet_addr("127.0.0.1");
    int add_len=sizeof(struct sockaddr_in);

    assert(bind(sfd,(struct sockaddr*)&saddr,add_len)!=-1);
    assert(listen(sfd,3)!=-1);
    int nsfd[100];
    struct sockaddr_in taddr;
    while(1)
    {
        sleep(1);
        assert((nsfd[cnt]=accept(sfd,(struct sockaddr*)&caddr,&add_len))!=-1);
        if(cnt==0)
        {
            pthread_t tid;
            curr_fd=nsfd[cnt];
            send(nsfd[cnt],"1",strlen("1")+1,0);
            pthread_create(&tid,NULL,&service,(void*)&nsfd[cnt]);
            cnt++;
        }
        else 
        {
            assert(getpeername(curr_fd,(struct sockaddr*)&taddr,&add_len)!=-1);
            sprintf(buf,"%d",ntohs(taddr.sin_port));
            printf("Port number%s\n",buf);
            send(nsfd[cnt],buf,strlen(buf),0);
            char *msg=(char*)malloc(BUF_SIZE);

            recv(nsfd[cnt],msg,BUF_SIZE,0);printf("pre per %s\n",msg);
            send(nsfd[cnt],"1",strlen("1"),0);
            if(strcmp(msg,"per")==0)
            {
                pthread_t tid;
                stop=1;
                close(curr_cfd);
                curr_fd=nsfd[cnt];
                sleep(2);printf("started\n");
                pthread_create(&tid,NULL,&service,(void*)&nsfd[cnt]);
            }
            cnt++;
        }
    }
}
