#include<iostream>
#include<stdio.h>
using namespace std;
#include<cstdlib>
#include <unistd.h>
#include<cstring>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<signal.h>
#include<sys/shm.h>
#include<unistd.h>
#define BUFMAX 1024
#define FILENAME "record.txt"
#define SERVER_PORT 8000
#define LENGTH_OF_LISTEN_QUEUE 20
#define BUFFER_SIZE 1024
#define FILE_NAME_MAX_SIZE 512

//packet用来消息传递
struct packet
{
	int len;
	char name[16];		
	char buf[BUFMAX];
};
 
//进程退出提示 
void func(int flag)
{
	cout<<"write进程退出"<<endl;
	exit(0);
}

//输出提示信息 
void test(pid_t pid,int num)
{
	if(num<=0)
    {
		  if(num==0)
		 {
   			     cout<<"read进程退出"<<endl;
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
 
 //读取buffer内容函数模块
 ssize_t readn(int fd, void  *buf, size_t count)
	{
		ssize_t nleft=count;
		ssize_t nread;
		char *bufp=(char *)buf;
	
		while(nleft>0)
		{
			if( (nread=read(fd,bufp,nleft))<=0 )
			{
					cout<<"连接中断"<<endl;
					return 0;
			}
			nleft-=nread;
			bufp+=nread;
 
		}
		return count;
	}
 
 
 //写入buffer内容函数模块
ssize_t writen(int fd, void *buf, size_t count)
        {
                ssize_t nleft=count;
                ssize_t nwrite;
                char *bufp=(char *)buf;
 
                while(nleft>0)
                {
                        nwrite=write(fd,bufp,nleft);
                        nleft-=nwrite;
                        bufp+=nwrite;
                }
                return count;
        }
 
 //管理员模块，可查看clients留下的日志
void root_usr(int connectfd)  // root 用户
{
	int ret,len;
	packet pack;
	memset(pack.name,0,16);
	while(1)
	{
	strcpy(pack.buf,
	"**********欢迎 Root 管理员**********\n\n        1 . 查看聊天记录\n        2 . 删除聊天记录\n        3 .  退出  \n\n*****************************");
	len=strlen(pack.buf)+1;
	pack.len=htonl(len);
	writen(connectfd,&pack,len+20);
		ret=read(connectfd,&pack,4);
		if(ret<=0)
		return;
		len=ntohl(pack.len);
		ret=read(connectfd,&pack.name,len+16);
		if(ret<=0)
		return;
        if( strlen(pack.buf)!=1 )
		{cout<<pack.buf<<" ***"<<strlen(pack.buf)<<endl; return ;}
 
		switch(pack.buf[0])
		{
			case  '1' :
				   {
					 FILE *fp=fopen(FILENAME,"rb");
					 fseek(fp,0,SEEK_END);
					 int size=ftell(fp);
					 fseek(fp,0,SEEK_SET);
				         char * data=new char[size];
					 int ret=fread(data,size,1,fp);
					 if(ret!=1)
					  cout<<"size  不等于  ret"<<endl;
 
					 strcpy(pack.buf,"*********消息记录******\n\n");
					 len=strlen(pack.buf)+1;
					 pack.len=htonl(len);
					 writen(connectfd,&pack,len+20);
					 writen(connectfd,data,size);
				         strcpy(pack.buf,"***********完***********\n");
                                         len=strlen(pack.buf)+1;
					  pack.len=htonl(len);
					 writen(connectfd,&pack,len+20);
					 delete [] data;
 
				   }break;
 
			case '2':
				   {
				   		FILE *fp=fopen(FILENAME,"w");
						fclose(fp);
				   }
			default :
				   return ;
 
		}
 
	}
 
}
void menu()
{

	cout<<"--------------------------MENU--------------------------"<<endl;
	cout<<endl;
	cout<<"                      1 创建聊天室                       "<<endl;
	cout<<"                      2 开启文件上传                    "<<endl;
	cout<<"                      3 退出                            "<<endl;
	cout<<endl;
	cout<<"--------------------------------------------------------"<<endl;
}
//主函数
void chatroom()
{
	cout<<"运行程序"<<endl;
	FILE *fp;
	fp=fopen(FILENAME,"rb");//打开聊天记录文件
	if(fp==NULL)
		fp=fopen(FILENAME,"wb");
	fclose(fp);//关闭聊天记录文件
 
	int shmid;
	void * shm;
	struct packet * pack;
	shmid = shmget((key_t)1234, sizeof(struct packet), 0666 | IPC_CREAT);//创建共享内存
	if (shmid ==-1)
	{
			cout<<"共享内存创建失败"<<endl;
	        exit(0);
    }
	shm = shmat(shmid,  (void*)0, 0);
	memset(shm,0,sizeof(packet));//共享内存格式化
	if(shm!=(void *)-1)
		cout<<"共享内存格式化成功"<<endl;
	pack=(struct packet *)shmat(shmid,  (void*)0, 0);
	if(pack==(void *)-1)
	     cout<<"shmat失败"<<endl;
 
	
	char objname[16]={0};
	int num,nlen;
	pid_t pid;
	int listenfd,connectfd;
	struct sockaddr_in servaddr;//初始化socket
	memset(&servaddr,0,sizeof(servaddr));
	servaddr.sin_family=AF_INET;
	servaddr.sin_addr.s_addr=htonl(INADDR_ANY);
	servaddr.sin_port=htons(3389);
	if( (listenfd=socket(AF_INET,SOCK_STREAM,0))==-1 )
		cout<<"socket错误"<<endl;
	if( (bind(listenfd,(struct sockaddr *)&servaddr,sizeof(servaddr)))==-1 )//绑定
		cout<<"bind错误"<<endl;
	if( listen(listenfd,10)==-1 )//监听
		cout<<"listen 错误"<<endl;
	while(1)
	{
		if( (connectfd=accept(listenfd,NULL,NULL))==-1)
			cout<<"accept错误"<<endl;
		else		
		cout<<"连接成功"<<endl;
		pid=fork();
		if(pid==0)	//子进程 负责连接
		{		
			close(listenfd);
			read(connectfd,objname,16);		// 读取对方名字
			if( !(strcmp(objname,"root")&&strcmp(objname,"Root") ))
			{

			   cout<<objname<<" 已进入房间"<<endl;
			   root_usr(connectfd);
			   cout<<objname<<" 断开连接"<<endl;
			   exit(0);
			}
			cout<<objname<<" 已进入聊天室"<<endl;
			pid=fork();
			if(pid>0)  //父进程 接受数据
			{
				pack=(struct packet *)shmat(shmid,  (void*)0, 0);
			    if(pack==(void *)-1)
				      cout<<"shmat失败"<<endl;
 
				while(1)	
				{	
					num=readn(connectfd,pack,4);
					test(pid,num);
					nlen=ntohl(pack->len);
					num=readn(connectfd,pack->name,nlen+16);
                                        test(pid,num);
 
					cout<<pack->name<<" : "<<pack->buf<<endl;
					 fp=fopen(FILENAME,"ab");
				     if(fp==NULL)
					 cout<<"历史记录txt打开失败"<<endl;
					 if(fwrite(pack,nlen+20,1,fp)==0)
					   cout<<"fwrite错误"<<endl;
					fclose(fp);
 
				}
			}
			if(pid==0) //子进程 发送数据
			{
				signal(SIGUSR1,func);
				char name_tmp[16]={0};
				char buf_tmp[1024]={0};
				 pack=(struct packet *)shmat(shmid,  (void*)0, 0);
				 if(pack==(void *)-1)
				        cout<<"shmat失败"<<endl;
				while(1)
				{
					while(1)
					{
					    sleep(0.1);
						if( strcmp(objname,pack->name)!=0 )
						{
							 if(  strcmp(name_tmp,pack->name)||strcmp(buf_tmp,pack->buf) ) 
							  {
									break;
						          }
						}
					}
					strcpy(name_tmp,pack->name);
					strcpy(buf_tmp,pack->buf) ;
					nlen=strlen(pack->buf)+1;	//  有'\0'
					int zz=write(connectfd,pack,nlen+20);
					if(zz<=0)
					cout<<"************write 失败 ***"<<endl;
				}
			}		
			else
				cout<<"2pid错误"<<endl;
			exit(0);
			
		
			
		}
		else if(pid>1)	//父进程 继续监听
			close(connectfd);
		else
		{
			cout<<"1pid错误"<<endl;
			exit(0);
		}
	}
}
void load()
{
	cout<<"sorry,暂未完善，敬请等待"<<endl;
	    // 声明并初始化一个服务器端的socket地址结构
    struct sockaddr_in server_addr;
    bzero(&server_addr, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htons(INADDR_ANY);
    server_addr.sin_port = htons(SERVER_PORT);
 
    // 创建socket，若成功，返回socket描述符
    int server_socket_fd = socket(PF_INET, SOCK_STREAM, 0);
    if(server_socket_fd < 0)
    {
        perror("Create Socket Failed:");
        exit(1);
    }
    int opt = 1;
    setsockopt(server_socket_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
 
    // 绑定socket和socket地址结构
    if(-1 == (bind(server_socket_fd, (struct sockaddr*)&server_addr, sizeof(server_addr))))
    {
        perror("Server Bind Failed:");
        exit(1);
    }
    
    // socket监听
    if(-1 == (listen(server_socket_fd, LENGTH_OF_LISTEN_QUEUE)))
    {
        perror("Server Listen Failed:");
        exit(1);
    }
 
    while(1)
    {
        // 定义客户端的socket地址结构
        struct sockaddr_in client_addr;
        socklen_t client_addr_length = sizeof(client_addr);
 
        // 接受连接请求，返回一个新的socket(描述符)，这个新socket用于同连接的客户端通信
        // accept函数会把连接到的客户端信息写到client_addr中
        int new_server_socket_fd = accept(server_socket_fd, (struct sockaddr*)&client_addr, &client_addr_length);
        if(new_server_socket_fd < 0)
        {
            perror("Server Accept Failed:");
            break;
        }
 
        // recv函数接收数据到缓冲区buffer中
        char buffer[BUFFER_SIZE];
        bzero(buffer, BUFFER_SIZE);
        if(recv(new_server_socket_fd, buffer, BUFFER_SIZE, 0) < 0)
        {
            perror("Server Recieve Data Failed:");
            break;
        }
 
        // 然后从buffer(缓冲区)拷贝到file_name中
        char file_name[FILE_NAME_MAX_SIZE+1];
        bzero(file_name, FILE_NAME_MAX_SIZE+1);
        strncpy(file_name, buffer, strlen(buffer)>FILE_NAME_MAX_SIZE?FILE_NAME_MAX_SIZE:strlen(buffer));
        printf("%s\n", file_name);
 
        // 打开文件并读取文件数据
        FILE *fp = fopen(file_name, "r");
        if(NULL == fp)
        {
            printf("File:%s Not Found\n", file_name);
        }
        else
        {
            bzero(buffer, BUFFER_SIZE);
            int length = 0;
            // 每读取一段数据，便将其发送给客户端，循环直到文件读完为止
            while((length = fread(buffer, sizeof(char), BUFFER_SIZE, fp)) > 0)
            {
                if(send(new_server_socket_fd, buffer, length, 0) < 0)
                {
                    printf("Send File:%s Failed./n", file_name);
                    break;
                }
                bzero(buffer, BUFFER_SIZE);
            }
 
            // 关闭文件
            fclose(fp);
            printf("File:%s Transfer Successful!\n", file_name);
        }
        // 关闭与客户端的连接
        close(new_server_socket_fd);
    }
    // 关闭监听用的socket
    close(server_socket_fd);

}
int main()
{   
   	int a;


    while(1){
		menu();
		cout<<"option: ";
  	    cin>>a;
        switch (a)
		{
		case 1:chatroom();
			break;
		case 2:load();
			break;
		case 3:
		    cout<<"程序退出"<<endl;
		   return 0;
		default:
			cout<<"sorry,worry selection"<<endl;
			break;
		}
	}

	
	return 0;
}


