 #include <stdio.h>
 #include <sys/types.h>
 #include <sys/socket.h>
 #include <pthread.h>
 #include <arpa/inet.h>
 #include <netinet/in.h>
 #include <string.h>
 #include <unistd.h>
 #include <stdlib.h>
 #include <sys/stat.h>
 #include <fcntl.h>
 #include <sys/sendfile.h>
//  /mnt/c/Users/user/desktop
  typedef struct sockaddr *sockaddrp;

  //存儲客戶端地址的結構體數組
  struct sockaddr_in src_addr[50];
  socklen_t src_len = sizeof(src_addr[0]);



  //連接後記錄confd數組
  int confd[50] = {};


  //設置連接人數
  int count = 0;

  char name_list[30][20] = {};
  //int j = 0;

  void *broadcast(void *indexp)
  {
      int index = *(int *)indexp;
      char buf_rcv[255] = {};
      char buf_snd[255] = {};
      //第一次讀取使用者姓名
      char name[20] = {};
	  int j = 0;
	  //char name_list[30][20] = {};
      int ret = recv(confd[index],name,sizeof(name),0);
      if(0 > ret)
      {
          perror("recv");
          close(confd[index]);
          exit(1);
      }
      //上線使用者名單
	  strcpy(name_list[index], name);
	  //name_list[j] = name;
	  //printf("名單:%s",name_list[index]);
	  //printf("j=%d",j);
	  
	  
      while(1)
      {
          bzero(buf_rcv,sizeof(buf_rcv));
          recv(confd[index],buf_rcv,sizeof(buf_rcv),0);

          //判斷是否退出
          if(0 == strcmp("quit\n",buf_rcv))
          {
             sprintf(buf_snd,"%s 已離線",name);
              for(int i = 0;i <= count;i++)
              {
                  if(i == index || 0 == confd[i])
                  {
                      continue;
                  }

                  send(confd[i],buf_snd,strlen(buf_snd),0);
              }
			  strcpy(name_list[index],"null");
              confd[index] = -1;
              pthread_exit(0);

          }
		  
		  //查詢線上使用者
          if(0 == strcmp("onlineUser\n",buf_rcv))
          {
			  //printf("%d",count);
			 //for(int i=0;i<30;i++)
				 //for(j=0;j<20;j++)
					 //printf("%d=%s\n",i,name_list[i]);
				 
             for(int i = 0 ; i < count;i++)
             {
				 if(0 != strcmp("null",name_list[i]))	  
				 {
				    sprintf(buf_snd,"上線的有: %s ",name_list[i]);
                    send(confd[index],buf_snd,strlen(buf_snd),0);
				 }
				 
				 
             }

          }
		  
		  //傳送訊息給指定的使用者
		  else if( 0 == strcmp("to:\n",buf_rcv))
		  {
			  char user_name[20] = {};
			  recv(confd[index],user_name,sizeof(user_name),0);
			  while(1)
			  {
				 bzero(buf_rcv,sizeof(buf_rcv));
			     recv(confd[index],buf_rcv,sizeof(buf_rcv),0);
			     if(0 == strcmp("bye\n",buf_rcv)) break;
				 
			     for(int i = 0 ; i< count ;i++)
			    {
				     if(0 == strcmp(user_name,name_list[i]))
				     {
					       //send(confd[i],buf_snd,strlen(buf_snd),0);
						   //send(confd[i],buf_rcv,strlen(buf_rcv),0);
						   sprintf(buf_snd,"%s:%s",name,buf_rcv);
						   send(confd[i],buf_snd,strlen(buf_snd),0);
				     }
				  
			    }
			  }
              			  	  
		  }
		  
          //傳送檔案給指定的使用者
		  else if( 0 == strcmp("send:\n",buf_rcv))
		  {
			  char user_name[20] = {};
			  char file[20]= {};
			  recv(confd[index],user_name,sizeof(user_name),0);
			  bzero(buf_rcv,sizeof(buf_rcv));		  
			  recv(confd[index],file,sizeof(file),0);
			  if(0 == strcmp("bye",file)) break;
			  
			  for(int i = 0 ;i < count ;i++)
			  {
				  if(0 == strcmp(user_name,name_list[i]))
				  {
					  sprintf(buf_snd,"%s 要傳送檔案給你，你要接收嗎?yes or no",name);
					  send(confd[i],buf_snd,strlen(buf_snd),0);
					  
					  bzero(buf_rcv,sizeof(buf_rcv));
                      recv(confd[i],buf_rcv,sizeof(buf_rcv),0);
					  if(0 == strcmp("yes\n",buf_rcv))
					  {
					  int fdimg = open(file, O_RDONLY);
                      sendfile(confd[i], fdimg, NULL, 9000);
                      close(fdimg);	
					  }
					  else
						  break;
					  
				  }				  				  
		 
			  }
			                			  	  
		  }
		  
			  
          else{
          sprintf(buf_snd,"%s:%s",name,buf_rcv);
          printf("%s\n",buf_snd);
          for(int i = 0;i <= count;i++)
          {
              if(i == index || 0 == confd[i])
              {
                  continue;
              }

              send(confd[i],buf_snd,sizeof(buf_snd),0);
          }
		  }
		  
		  

      }

  }





  int main(int argc,char **argv)
  {
      printf("聊天室伺服器運行中...\n");


      //創建通信對象
      int sockfd = socket(AF_INET,SOCK_STREAM,0);
      if(0 > sockfd)
      {
          perror("socket");
          return -1;
      }
	  
      //準備地址
     struct sockaddr_in addr = {AF_INET};
     addr.sin_port = htons(8080);
     addr.sin_addr.s_addr = INADDR_ANY;

     socklen_t addr_len = sizeof(addr);



     //綁定
     int ret = bind(sockfd,(sockaddrp)&addr,addr_len);
     if(0 > ret)
     {
         perror("bind");
         return -1;
     }


     //設置最大排隊數
     listen(sockfd,50);

     int index = 0;


     while(count <= 50)
     {
         confd[count] = accept(sockfd,(sockaddrp)&src_addr[count],&src_len);
         ++count;
         //保存此次客戶端地址所在下標方便後續傳入
         index = count-1;

         pthread_t tid;
         int ret = pthread_create(&tid,NULL,broadcast,&index);
         if(0 > ret)
         {
             perror("pthread_create");
             return -1;
         }


     }


 }
