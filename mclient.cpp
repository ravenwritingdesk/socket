#include<iostream>
using namespace std;
#include <unistd.h>
#include <arpa/inet.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<signal.h>
#include<cstring>
#include<cstdlib>
#include<stdlib.h>     
#define BUFMAX 1024
#define SERVER_PORT 8000
#define BUFFER_SIZE 1024
#define FILE_NAME_MAX_SIZE 512
 //消息传递结构
struct packet
{
	int len;
	char name[16];
	char buf[BUFMAX];
};
 //错误提示
void test(pid_t pid,int num)
{
	if(num<=0)
     {
          if(num==0)
            {
                 cout<<"连接中断  read进程关闭"<<endl;
                 kill(pid,SIGUSR1);
                 exit(0);
             }
            else 
             {
                 cout<<"num<0错误"<<endl;
                 exit(0);
              }
      }
 
}
 
 
 //错误提示
void func(int flag)
{
	if(flag==SIGUSR1)
	{
		cout<<"连接中断 write进程中断"<<endl;
		exit(0);
	}
}
 
 
 //read函数模块
 ssize_t readn(int fd, void *buf, size_t count)
	{
		ssize_t nleft=count;
		ssize_t nread;
		char *bufp=(char *)buf;
	
		while(nleft>0)//逐一读取
		{
			if( (nread=read(fd,bufp,nleft))<=0 )
			{
				cout<<"*****nead"<<nread<<endl;
				return 0;
			}
			nleft-=nread;
			bufp+=nread;
 
		}
		return count;//返回count
	}
 
 
 //write函数模块
ssize_t writen(int fd, void *buf, size_t count)
        {
                ssize_t nleft=count;
                ssize_t nwrite;
                char *bufp=(char *)buf;
		cout<<"**************"<<endl;
                while(nleft>0)//逐一读取
                {
                        if( (nwrite=write(fd,bufp,nleft))<=0 )
                        {
                              cout<<"连接中断"<<endl;
                              return -1;
                        }
                        nleft-=nwrite;
                        bufp+=nwrite;
 
                }
                return count;//返回count
        }
 
 //菜单设计
 void menu()
 {

	cout<<"--------------------------MENU--------------------------"<<endl;
	cout<<endl;
	cout<<"                      1 进入聊天室                       "<<endl;
	cout<<"                      2 上传下载文件                     "<<endl;
	cout<<"                      3 退出                            "<<endl;
	cout<<endl;
	cout<<"--------------------------------------------------------"<<endl;

 }
 //聊天室程序代码部分
 void chat()
 {
	cout<<"正在进入聊天室"<<endl;//提示信息
	struct packet pack;
	memset(&pack ,0,sizeof(pack));//内存初始化
	int num,nlen,x;
	pid_t pid;
	int connectfd;
	struct sockaddr_in servaddr;//socket初始化
	memset(&servaddr,0,sizeof(servaddr));
	servaddr.sin_family=AF_INET;
	servaddr.sin_addr.s_addr=inet_addr("193.112.240.154");//服务器ip，这里用的是我的腾讯云
	servaddr.sin_port=htons(3389);
	if( (connectfd=socket(AF_INET,SOCK_STREAM,0))==-1 )
		cout<<"socker错误"<<endl;
 
 
	if( connect(connectfd,(struct sockaddr*)&servaddr,sizeof(servaddr))==-1 )	
	cout<<"连接失败"<<endl;//错误提示
	else
	cout<<"连接成功"<<endl;
	cout<<endl<<endl<<"欢迎进入聊天室!"<<endl;
	cout<<"请输入你的名字： "<<endl;//输入姓名
	cin>>pack.name;
 
 
	pid=fork();//创建子进程，以实现多用户随时接入
	if(pid>0)//父亲进程，用于read
	{
		while(1)//while循环实现反复调用
		{
			num=readn(connectfd,&pack,4);
			test(pid,num);
			nlen=ntohl(pack.len);
			num=readn(connectfd,pack.name,nlen+16);
			test(pid,num);
 
			cout<<pack.name<<" : "<<pack.buf<<endl;;
			memset(&pack,0,strlen(pack.buf)+20);
		}
	}
	else if(pid==0)//子进程，用于write
	{
		write(connectfd,pack.name,strlen(pack.name));
		signal(SIGUSR1,func);
		while(1)//while循环实现反复调用
		{
			cin>>pack.buf; 
			nlen=strlen(pack.buf)+1;	//  '\0'
			pack.len=htonl(nlen);//packlen
			write(connectfd,&pack,nlen+20);
			//memset(pack.buf,0,nlen);
		}
	}
	else
	cout<<"pid错误"<<endl;//特殊错误处理

 }
 void snake(){
	 // 声明并初始化一个客户端的socket地址结构
    struct sockaddr_in client_addr;
    bzero(&client_addr, sizeof(client_addr));
    client_addr.sin_family = AF_INET;
    client_addr.sin_addr.s_addr = htons(INADDR_ANY);
    client_addr.sin_port = htons(0);
 
    // 创建socket，若成功，返回socket描述符
    int client_socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if(client_socket_fd < 0)
    {
        perror("Create Socket Failed:");
        exit(1);
    }
 
    // 绑定客户端的socket和客户端的socket地址结构 非必需
    if(-1 == (bind(client_socket_fd, (struct sockaddr*)&client_addr, sizeof(client_addr))))
    {
        perror("Client Bind Failed:");
        exit(1);
    }
 
    // 声明一个服务器端的socket地址结构，并用服务器那边的IP地址及端口对其进行初始化，用于后面的连接
    struct sockaddr_in server_addr;
    bzero(&server_addr, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    if(inet_pton(AF_INET, "193.112.240.154", &server_addr.sin_addr) == 0)
    {
        perror("Server IP Address Error:");
        exit(1);
    }
    server_addr.sin_port = htons(SERVER_PORT);
    socklen_t server_addr_length = sizeof(server_addr);
 
    // 向服务器发起连接，连接成功后client_socket_fd代表了客户端和服务器的一个socket连接
    if(connect(client_socket_fd, (struct sockaddr*)&server_addr, server_addr_length) < 0)
    {
        perror("Can Not Connect To Server IP:");
        exit(0);
    }
 
    // 输入文件名 并放到缓冲区buffer中等待发送
    char file_name[FILE_NAME_MAX_SIZE+1];
    bzero(file_name, FILE_NAME_MAX_SIZE+1);
    printf("Please Input File Name On Server:\t");
    scanf("%s", file_name);
 
    char buffer[BUFFER_SIZE];
    bzero(buffer, BUFFER_SIZE);
    strncpy(buffer, file_name, strlen(file_name)>BUFFER_SIZE?BUFFER_SIZE:strlen(file_name));
    
    // 向服务器发送buffer中的数据
    if(send(client_socket_fd, buffer, BUFFER_SIZE, 0) < 0)
    {
        perror("Send File Name Failed:");
        exit(1);
    }
 
    // 打开文件，准备写入
    FILE *fp = fopen(file_name, "w");
    if(NULL == fp)
    {
        printf("File:\t%s Can Not Open To Write\n", file_name);
        exit(1);
    }
 
    // 从服务器接收数据到buffer中
    // 每接收一段数据，便将其写入文件中，循环直到文件接收完并写完为止
    bzero(buffer, BUFFER_SIZE);
    int length = 0;
    while((length = recv(client_socket_fd, buffer, BUFFER_SIZE, 0)) > 0)
    {
        if(fwrite(buffer, sizeof(char), length, fp) < length)
        {
            printf("File:\t%s Write Failed\n", file_name);
            break;
        }
        bzero(buffer, BUFFER_SIZE);
    }
 
    // 接收成功后，关闭文件，关闭socket
    printf("Receive File:\t%s From Server IP Successful!\n", file_name);
    fclose(fp);
    close(client_socket_fd);

 }

int main()
{
	int a;//option


    while(1){//while使程序可以反复调用
		menu();
		cout<<"option: ";
  	    cin>>a;
        switch (a)
		{
		case 1:chat();//聊天室
			break;
		case 2:snake();//下载文件
			break;
		case 3:
		    cout<<"程序退出"<<endl;//退出
		    return 0;
		default:
			cout<<"sorry,worry selection"<<endl;//输入错误处理
			break;
		}
	}

	
 
 
	return 0;
}


