#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <pthread.h>
#include <string.h>
#include <stdlib.h>



 typedef struct sockaddr *sockaddrp;
 int sockfd;

 void *recv_other(void *arg)
 {
     char buf[255]= {};
     while(1)
     {
         int ret = recv(sockfd,buf,sizeof(buf),0);
         if(0 > ret)
         {
             perror("recv");
             exit(1);
         }
         printf("%s\n",buf);
     }
 }




 int main(int argc,char **argv)
 {
     if(2 != argc)
     {
         perror("參數錯誤");
         return -1;
     }

     //建立socket對象
     sockfd = socket(AF_INET,SOCK_STREAM,0);
     if(0 > sockfd)
     {
         perror("socket");
         return -1;
     }

     //準備連接地址
	 struct sockaddr_in addr = {AF_INET};
     addr.sin_port = htons(atoi(argv[1]));
     addr.sin_addr.s_addr = INADDR_ANY;

     socklen_t addr_len = sizeof(addr);


     //連接
     int ret = connect(sockfd,(sockaddrp)&addr,addr_len);
     if(0 > ret)
     {
         perror("connect");
         return -1;
     }

     
     char buf[255] = {};
     char name[255] = {};
	 char user_name[255] = {};//別人的
	 //發送名字
     printf("請輸入名字：");
     scanf("%s",name);
     ret = send(sockfd,name,strlen(name),0);
     if(0 > ret)
     {
         perror("connect");
         return -1;
     }

      //創建接收子線程
     pthread_t tid;
     ret = pthread_create(&tid,NULL,recv_other,NULL);

     if(0 > ret)
     {
         perror("pthread_create");
         return -1;
     }
     //循環發送
     while(1)
     {
         //printf("%s:",name);
         //scanf("%s",buf);
		 fgets(buf,255,stdin);
         int ret = send(sockfd,buf,strlen(buf),0);
         if(0 > ret)
         {
             perror("send");
             return -1;
         }
		 
         //輸入quit退出
         if(0 == strcmp("quit\n",buf))
         {
             printf("%s ,您已退出聊天室\n",name);
             return 0;
         }
		 
         //指定使用者傳送訊息
		 if(0 == strcmp("to:\n",buf))
		 {
			 printf("傳送訊息，請輸入對方名稱:");
			 scanf("%s" ,user_name); 
			 int ret = send(sockfd,user_name,strlen(user_name),0);
			 printf("請輸入對話:");
			 //可以循環發送
			 while(1)
			 {
				 fgets(buf,255,stdin);
				 if(0 == strcmp("bye\n",buf)) break;
			     int ret = send(sockfd,buf,strlen(buf),0);
			 
			 }
			 
		 }
		 
		 //指定使用者傳送檔案
		 else if(0 ==strcmp("send:\n",buf))
		 {
			 printf("傳送檔案，請輸入對方名稱:");
			 scanf("%s" ,user_name); 
			 int ret = send(sockfd,user_name,strlen(user_name),0);
			 printf("請發送檔案:");
			 //fgets(buf,255,stdin);
			 char file[20] = {};
			 scanf("%s",file);
		     if(0 == strcmp("bye",file)) break;
			 int ret2 = send(sockfd,file,strlen(file),0);
			 
			 
			 
		 }
		 

     }

 }
