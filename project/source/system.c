#include <stdio.h>
#include "func.h"

int main(int argc, char *argv[])
{
	while(!IP_Addr_PORT_Init());  //初始化IP及端口号
	
	Node *list = List_Create();  //创建一条单向链表，用来存储在线成员IP
	Node *list_mutil = List_Create();  //创建一条单向链表，用来存储组播在线成员IP
	
	int h_fd = open(HISTORY_PATH, O_RDWR | O_TRUNC | O_CREAT);  //清空打开历史记录文件
	if(-1 == h_fd)
	{
		perror("open history failed");
	}
	
	Arg arg1;
	arg1.head = list;
	arg1.history = h_fd; 
	//1,创建三条线程,接收单播,组播,广播
	pthread_t  tid[3];
	pthread_create(&tid[0], NULL, Sigle_Recv, (void *)&arg1);		//单播接收
	pthread_create(&tid[1], NULL, Board_Recv, (void *)&arg1);		//广播接收
	pthread_create(&tid[2], NULL, Recv_File, NULL);	     //文件接收

	//2,初始化主线程UDP套接字 --> 使能广播.
	int sockfd = UDP_Socket_Init();
	
	/*  发送一条广播数据 “ON_LINE!”  */
	Send_once_Board(sockfd, h_fd, "#ON_LINE!");

	//3,循环发送数据 --> 选择发送的数据类型 --> 单播,组播,广播 ...
	int cmd;
	int i = 0;
	Main_Init();
	while(1)
	{
		printf("主界面指令输入（0,退出 1,发送单播 2,发送组播 3,发送广播 4,获取在线成员 5,历史记录）：\n");
		scanf("%d", &cmd);
		if(cmd == 0)
		{
			pthread_cancel(tid[0]);
			pthread_cancel(tid[1]);
			pthread_cancel(tid[2]);
			pthread_join(tid[0], NULL);
			pthread_join(tid[1], NULL);
			pthread_join(tid[2], NULL);
			//发送一条广播数据 “#DIS_ON_LINE!”
			Send_once_Board(sockfd, h_fd, "#DIS_ON_LINE!");
			break;
		}
		while(getchar() != '\n');
		switch(cmd)
		{
			case 1:		//发送单播
				Send_Single(sockfd, h_fd);
				i = 0;
				break;
		
			case 2:		//发送组播			
				Send_Mutil(sockfd, h_fd, list_mutil);
				i = 0;
				break;	
				
			case 3:		//发送广播
				Send_Board(sockfd, h_fd, list);
				i = 0;
				break;

			case 4:		//获取在线成员
				List_Display(list);
				i = 0;
				break;	

			case 5:		//获取历史记录
				 Show_History();
				i = 0;
				break;

			default:
				printf("输入的数据有误!请重新输入!  %d\n", i+1);
				i++;
				if(i == 5)
				{
					printf("多次错误输入，程序运行结束！！！\n");
					exit(1);
				}
				break;
		}
	}

	List_Destroy(list);
	close(h_fd);

	return 0;
}














