#include "func.h"

struct sockaddr_in  single_addr, mutil_addr, board_addr;  //创建发送数据的结构体
socklen_t addrlen = sizeof(single_addr);	
char LOCAL_IP[20], MUTIL_IP[20], BOARD_IP[20];  //本机IP,组播IP,广播IP
int SINGLE_PORT, MUTIL_PORT, BOARD_PORT, NETWORK_PORT;  //本机端口,组播端口,广播端口,网络端口

/*发送单播数据*/
void Send_Single(int sockfd, int h_fd)
{
	char IP[32], buf[1024], h_buf[1024];

	printf("输入对方的IP:");
	memset(IP, 0, sizeof(IP));
	scanf("%s", IP);
	if(!IS_N_addr(IP))
	{
		printf("输入的单播IP不正确！退出发送！\n");
		return;
	}
	single_addr.sin_addr.s_addr = inet_addr(IP);

	while(getchar() != '\n');	//清除输入缓冲区
	while(1)
	{
		printf("发送单播信息<%s>:", IP);
		memset(buf, 0, sizeof(buf));
		fgets(buf, sizeof(buf), stdin); strtok(buf, "\n");
		if(strcmp("\n", buf) == 0)
		{
			continue;
		}
		if(strcmp("#EXIT", buf) == 0 || strcmp("#exit", buf) == 0)
		{
			break;
		}
		if(sendto(sockfd, buf, sizeof(buf), 0,(struct sockaddr *)&single_addr, addrlen) < 0)
		{
			perror("send Single failed");
		}

		if(strncmp("#FILE:", buf, 6) == 0 || strncmp("#file:", buf, 6) == 0)
		{
			/************获取文件路径****************/
			char file_path[32] ;
			memset(file_path, 0, sizeof(file_path));
			if(strncmp("#FILE:", buf, 6) == 0)
				sscanf(buf, "#FILE:%s", file_path);
			else
				sscanf(buf, "#file:%s", file_path);
			/***************************************/
			memset(h_buf, 0, sizeof(h_buf));
			sprintf(h_buf, "<单播信息>I-->IP<%s>：%s\n", IP, buf);
			int i =0;
			while(h_buf[i] != '\n') i++;
			write(h_fd, h_buf, i+1);
			Send_File(IP, file_path);   //发送文件	
			while(getchar() != '\n');		//清除输入缓冲区
		}		
		else
		{
			memset(h_buf, 0, sizeof(h_buf));
			sprintf(h_buf, "<单播信息>I-->IP<%s>：%s\n", IP, buf);
			int i =0;
			while(h_buf[i] != '\n') i++;
			write(h_fd, h_buf, i+1);
		}
	}
}
//接收到相应的广播数据作出单播应答
void Send_Reply(int sockfd, char *IP, char *buf)
{
	int i = 0;
	while(buf[i] != '!') i++;
	if(strcmp("#ON_LINE!", buf) == 0 || strcmp("#DIS_ON_LINE!", buf) == 0)
	{
		single_addr.sin_addr.s_addr = inet_addr(IP);
		sendto(sockfd, buf, i+1, 0,(struct sockaddr *)&single_addr, addrlen);
	}
	else
	{
		//mutil_addr.sin_addr.s_addr = inet_addr(IP);
		sendto(sockfd, buf, i+1, 0,(struct sockaddr *)&mutil_addr, addrlen);
	}
	
}

/*发送组播数据*/
void Send_Mutil(int sockfd, int h_fd, Node *list_mutil)
{
	//组播IP-->224.0.0.1 ~ 239.255.255.255
	printf("请输入您要加入的组播IP：");
	memset(MUTIL_IP, 0, sizeof(MUTIL_IP));
	scanf("%s", MUTIL_IP);
	if(!IS_N_addr(MUTIL_IP))
	{
		printf("输入的组播IP不正确！退出发送！\n");
		return;
	}
	mutil_addr.sin_addr.s_addr = inet_addr(MUTIL_IP);	//组播IP
	//自己加入多播组
	struct ip_mreq  mutil_ip;
	socklen_t optlen = sizeof(mutil_ip);
	
	inet_pton(AF_INET, MUTIL_IP, &mutil_ip.imr_multiaddr);	//组播地址IP
	inet_pton(AF_INET, LOCAL_IP, &mutil_ip.imr_interface);	//主机地址IP
	
	if(setsockopt(sockfd, IPPROTO_IP, IP_ADD_MEMBERSHIP,&mutil_ip, optlen) < 0)		//加入多播组
	{
		perror("IP_ADD_MEMBERSHIP failed");
		return;
	}	
	//创建并开启组播数据接收的线程
	pthread_t tid;
	Arg arg;
	arg.head = list_mutil;
	arg.history = h_fd; 
	pthread_create(&tid, NULL, Mutil_Recv, (void *)&arg);		//组播接收线程

	//循环发送组播数据
	char buf[1024], h_buf[1024];

	int cmd;
	system("clear");
	printf("===============================================================\n");
	printf("                       组播界面                        \n");
	printf("===============================================================\n");
	while(getchar() != '\n');	//清除输入缓冲区
	Send_Reply(sockfd, MUTIL_IP, "#ON_MUTIL!");
	while(1)
	{	
		printf("组播指令输入（0,退出 1,发送数据 2,获取在线成员）：\n");
		scanf("%d", &cmd); getchar();   //取走字符 '\n'
		switch(cmd)
		{
			case 0:
				pthread_cancel(tid);
				pthread_join(tid, NULL);
				Send_Reply(sockfd, MUTIL_IP, "#DIS_ON_MUTIL!");
				return;
			case 1:
				while(1)
				{
					printf("发送组播信息<%s>:", MUTIL_IP);
					memset(buf, 0, sizeof(buf));
					fgets(buf, sizeof(buf), stdin); strtok(buf, "\n");
					if(strcmp("\n", buf) == 0)
					{
						continue;
					}
					if(strcmp("#EXIT", buf) == 0 || strcmp("#exit", buf) == 0)
					{
						break;
					}
					if(sendto(sockfd, buf, sizeof(buf), 0,(struct sockaddr *)&mutil_addr, addrlen) < 0)
					{
						perror("send Mutil failed");
					}

					if(strncmp("#FILE:", buf, 6) == 0 || strncmp("#file:", buf, 6) == 0)
					{
						/************获取文件路径****************/
						char file_path[32];
						memset(file_path, 0, sizeof(file_path));
						if(strncmp("#FILE:", buf, 6) == 0)
							sscanf(buf, "#FILE:%s", file_path);
						else
							sscanf(buf, "#file:%s", file_path);
						/***************************************/
						memset(h_buf, 0, sizeof(h_buf));
						sprintf(h_buf, "<组播信息>I-->IP<%s>：%s\n", MUTIL_IP, buf);
						int i =0;
						while(h_buf[i] != '\n') i++;
						write(h_fd, h_buf, i+1);
						Node *q = list_mutil->next;
						while(q != NULL)
						{
							Send_File(q->ip, file_path);   //发送文件
						}
					}
					//当输入数据为"#EXIT"或"#exit"时结束发送组播数据
					else if(strcmp("#EXIT", buf) == 0 || strcmp("#exit", buf) == 0)
					{
						break;
					}
					else
					{
						memset(h_buf, 0, sizeof(h_buf));
						sprintf(h_buf, "<组播信息>I-->MUTIL_IP<%s>：%s\n", MUTIL_IP, buf);
						int i = 0;
						while(h_buf[i] != '\n') i++;
						write(h_fd, h_buf, i+1);
					}
				}
				break;
			case 2:
				List_Display(list_mutil);
				break;
			default:
				printf("输入的指令有误!请重新输入!\n");
				break;
		}
		
	}
}
/*发送广播数据*/
void Send_Board(int sockfd, int h_fd, Node *list)
{
	char buf[1024], h_buf[1024];
	printf("广播信息:");
	memset(buf, 0, sizeof(buf));
	fgets(buf, sizeof(buf), stdin); strtok(buf, "\n");
	if(strcmp("\n", buf) == 0)
	{
		return;
	}
	sendto(sockfd, buf, sizeof(buf), 0,(struct sockaddr *)&board_addr, addrlen);
	if(strncmp("#FILE:", buf, 5) == 0)
	{
		/************获取文件路基****************/
		char file_path[32];
		memset(file_path, 0, sizeof(file_path));
		sscanf(buf, "#FILE:%s\n", file_path);
		/***************************************/
		memset(h_buf, 0, sizeof(h_buf));
		sprintf(h_buf, "<广播信息>I-->BOARD_IP<%s>：%s\n", BOARD_IP, buf);

		int i = 0;
		while(h_buf[i] != '\n') i++;
		write(h_fd, h_buf, i+1);

		Node *p = list->next; 
		while(p != NULL)
		{
			Send_File(p->ip, file_path);   //发送文件
			p = p->next;
		}
	}
	else
	{
		memset(h_buf, 0, sizeof(h_buf));
		sprintf(h_buf, "<广播信息>I-->BOARD_IP<%s>：%s\n", BOARD_IP, buf);

		int i = 0;
		while(h_buf[i] != '\n') i++;
		write(h_fd, h_buf, i+1);
	}
	
}
/*发送一次广播数据*/
void Send_once_Board(int sockfd, int h_fd, char *buf)
{
	char h_buf[1024];
	memset(h_buf, 0, sizeof(h_buf));
	sprintf(h_buf, "<广播信息>I-->BOARD_IP<%s>：%s\n", BOARD_IP, buf);
	int i = 0;
	while(h_buf[i] != '\n') i++;
	write(h_fd, h_buf, i+1);
	i = 0;
	while(buf[i] != '!') i++;
	sendto(sockfd, buf, i+1, 0,(struct sockaddr *)&board_addr, addrlen);
}
/*发送文件*/
int Send_File(char *IP, char *file_path)
{
	//1,判断文件是否存在,如果存在,那就获取文件信息 (文件大小) -- stat
	if( access(file_path, F_OK) )
	{
		printf("发送的文件不存在!\n");
		return -1;
	}
	struct stat buf;
	stat(file_path, &buf);
	printf("发送的文件:%s, 大小:%ld\n", file_path, buf.st_size);
	
	//2,获取套接字
	int sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if(-1 == sockfd)
	{
		perror("socket failed");
		return -1;
	}
	
	//3,连接到接收方
	struct sockaddr_in  recv_addr;
	socklen_t addrlen = sizeof(recv_addr);
	
	recv_addr.sin_family = AF_INET;
	recv_addr.sin_port =  htons(NETWORK_PORT);	//把字符串的端口转换成网络字节序端口号
	recv_addr.sin_addr.s_addr = inet_addr(IP);

	if(connect(sockfd,  (struct sockaddr *)&recv_addr, addrlen) < 0 )
	{
		perror("connect failed");
		return -1;
	}

	//4,发送第一个数据包  文件名:文件大小
	char MsgBuf[1024];
	memset(MsgBuf, 0, sizeof(MsgBuf));

	if(strstr(file_path, "/"))
		sprintf(MsgBuf, "%ld:%s", buf.st_size, strrchr(file_path, '/')+1);
	else
		sprintf(MsgBuf, "%ld:%s", buf.st_size, file_path);
	send(sockfd, MsgBuf, sizeof(MsgBuf), 0);

	//5,循环发送数据 --> 循环读取文件内容 --> 循环发送 --> 发送完毕,结束
	FILE *fp = fopen(file_path, "r");
	if(NULL == fp)
	{
		perror("fopen failed");
		return 0;
	}
	size_t ret;
	size_t agg = 0, s = 0;
	printf("文件开始发送...\n");
	while(1)
	{
		ret = fread(MsgBuf, 1, sizeof(MsgBuf), fp);	//读取文件信息
		send(sockfd, MsgBuf, ret, 0);				//读取的文件信息发送给对方
		agg += ret;
		if(buf.st_size != 0 && s != agg*100/buf.st_size)
		{
			printf("文件已传输 %ld%%\n", s);
			s = agg*100/buf.st_size;
		}
		if(ret <= 0)
		{
			printf("文件已传输 100%%\n");
			break;
		}
	}
	printf("文件发送完成！\n");
	//6,关闭套接字,关闭文件描述符
	fclose(fp);
	close(sockfd);
}

/*接收单播信息线程*/
void *Sigle_Recv(void *arg)
{
	pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);		//设置为响应取消
    pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL);			//设置为立即响应
	Arg *arg1 = (Arg *)arg;
	Node *list = arg1->head;
	int h_fd = arg1->history;
	//1,获取一个UDP套接字
	int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
	if(-1 == sockfd)
	{
		perror("sockfd Sigle_Recv failed");
		return NULL;
	}
	
	//2,bind自己的IP和单播端口
	struct sockaddr_in  address_in, send_addr;
	socklen_t addrlen = sizeof(address_in);
	
	address_in.sin_family = AF_INET;
	address_in.sin_port =  htons(SINGLE_PORT);	//绑定单播端口
	//address_in.sin_addr.s_addr = inet_addr(LOCAL_IP);
	address_in.sin_addr.s_addr = htonl(INADDR_ANY);	//本机的IP
	if(bind(sockfd, (struct sockaddr *)&address_in, addrlen) < 0)
	{
		perror("bind Sigle_Recv failed");
		return NULL;
	}
	
	//3,循环接收UDP数据
	char ip_addr[20] = {0};
	char buf[1024], h_buf[1024];
	while(1)
	{
		memset(buf, 0, sizeof(buf));
		recvfrom(sockfd, buf, sizeof(buf), 0, (struct sockaddr *)&send_addr, &addrlen);
		inet_ntop(AF_INET, &(send_addr.sin_addr), ip_addr, sizeof(ip_addr));
		//如果接收到“#ON_LINE!”，将其IP存入链表
		if(strcmp("#ON_LINE!", buf) == 0)
		{
			List_Add_Node_End(list, ip_addr);
		}
		else
		{
			printf("<单播信息>IP<%s>:Port<%d>: %s\n",ip_addr, ntohs(send_addr.sin_port), buf);
			memset(h_buf, 0, sizeof(h_buf));
			sprintf(h_buf, "<单播信息>I<--IP<%s>：%s\n", ip_addr, buf);
			int i = 0;
			while(h_buf[i] != '\n') i++;
			write(h_fd, h_buf, i+1);
		}
	}
	
	close(sockfd);
}

/*接收组播信息线程*/
void *Mutil_Recv(void *arg)
{
	pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);		//设置为响应取消
    pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL);			//设置为立即响应

	Arg *arg1 = (Arg *)arg;
	Node *list = arg1->head;
	int h_fd = arg1->history;
	//1,获取一个UDP套接字
	int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
	if(-1 == sockfd)
	{
		perror("sockfd Mutil_Recv failed");
		return NULL;
	}
	//2,bind自己的IP和单播端口
	struct sockaddr_in  address_in, send_addr;
	socklen_t addrlen = sizeof(address_in);
	
	address_in.sin_family = AF_INET;
	address_in.sin_port =  htons(MUTIL_PORT);	//绑定组播端口
	//address_in.sin_addr.s_addr = inet_addr(LOCAL_IP);
	address_in.sin_addr.s_addr = htonl(INADDR_ANY);	//本机的IP
	
	if(bind(sockfd, (struct sockaddr *)&address_in, addrlen) < 0)
	{
		perror("bind Mutil_Recv failed");
		return NULL;
	}	
	//3,加入组播
	struct ip_mreq  mutil_ip;
	socklen_t optlen = sizeof(mutil_ip);
	
	inet_pton(AF_INET, MUTIL_IP, &mutil_ip.imr_multiaddr);		//组播地址IP
	inet_pton(AF_INET, LOCAL_IP, &mutil_ip.imr_interface);	//主机地址IP
	
	if(setsockopt(sockfd, IPPROTO_IP, IP_ADD_MEMBERSHIP,&mutil_ip, optlen) < 0)		//加入多播组
	{
		perror("IP_ADD_MEMBERSHIP failed");
		return NULL;
	}
	//4,循环接收UDP数据
	char ip_addr[20] = {0};
	char buf[1024], h_buf[1024];
	while(1)
	{
		memset(buf, 0, sizeof(buf));
		recvfrom(sockfd, buf, sizeof(buf), 0, (struct sockaddr *)&send_addr, &addrlen);
		inet_ntop(AF_INET, &(send_addr.sin_addr), ip_addr, sizeof(ip_addr));
		if(strcmp("#ON_MUTIL!", buf) == 0)
		{
			bool flag = List_Add_Node_End(list, ip_addr);
			if(strcmp(ip_addr, LOCAL_IP) == 0) continue;
			if(flag)
			{
				printf("IP<%s> 已加入组播！\n", ip_addr);
				Send_Reply(sockfd, MUTIL_IP, "#ON_MUTIL!");
			}
		}
		//如果接收到“#DIS_ON_MUTIL!”，将其IP从链表中删除
		else if(strcmp("#DIS_ON_MUTIL!", buf) == 0)
		{
			bool flag = List_Remove(list, ip_addr);
			if(flag)
				printf("IP<%s> 已退出组播！\n", ip_addr);
		}
		else
		{
			printf("<组播信息>IP<%s>:Port<%d>: %s\n",ip_addr, ntohs(send_addr.sin_port), buf);
			memset(h_buf, 0, sizeof(h_buf));
			sprintf(h_buf, "<组播信息>I<--MUTIL_IP<%s>：%s\n", ip_addr, buf);
			int i = 0;
			while(h_buf[i] != '\n') i++;
			write(h_fd, h_buf, i+1);
		}
	}

	close(sockfd);
}

/*接收广播信息线程*/
void *Board_Recv(void *arg)
{
	pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);		//设置为响应取消
    pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL);			//设置为立即响应
	Arg *arg1 = (Arg *)arg;
	Node *list = arg1->head;
	int h_fd = arg1->history;
	//1,获取一个UDP套接字
	int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
	if(-1 == sockfd)
	{
		perror("sockfd Board_Recv failed");
		return NULL;
	}
	
	//2,bind自己的IP和单播端口
	struct sockaddr_in  address_in, send_addr;
	socklen_t addrlen = sizeof(address_in);
	
	address_in.sin_family = AF_INET;
	address_in.sin_port =  htons(BOARD_PORT);	//绑定广播端口
	//address_in.sin_addr.s_addr = inet_addr(LOCAL_IP);
	address_in.sin_addr.s_addr = htonl(INADDR_ANY);	//本机的IP
	
	if(bind(sockfd, (struct sockaddr *)&address_in, addrlen) < 0)
	{
		perror("bind Board_Recv failed");
		return NULL;
	}	
	
	//3,循环接收UDP数据
	char ip_addr[20] = {0};
	char buf[1024], h_buf[1024];
	while(1)
	{
		memset(buf, 0, sizeof(buf));
		recvfrom(sockfd, buf, sizeof(buf), 0, (struct sockaddr *)&send_addr, &addrlen);
		inet_ntop(AF_INET, &(send_addr.sin_addr), ip_addr, sizeof(ip_addr));
		//如果接收到广播数据“ON_LINE!”，将其IP存入链表，并向其IP发送单播数据“ON_LINE!”
		if(strcmp("#ON_LINE!", buf) == 0)
		{
			bool flag = List_Add_Node_End(list, ip_addr);
			if(strcmp(ip_addr, LOCAL_IP) == 0) continue;
			if(flag)
			{
				printf("IP<%s> 已上线！\n", ip_addr);
				Send_Reply(sockfd, ip_addr, "#ON_LINE!");
			}
		}
		//如果接收到“#DIS_ON_LINE!”，将其IP从链表中删除
		else if(strcmp("#DIS_ON_LINE!", buf) == 0)
		{
			bool flag = List_Remove(list, ip_addr);
			if(flag)
				printf("IP<%s> 已下线！\n", ip_addr);
		}
		else
		{
			printf("<广播信息>IP<%s>:Port<%d>: %s\n",ip_addr, ntohs(send_addr.sin_port), buf);
			memset(h_buf, 0, sizeof(h_buf));
			sprintf(h_buf, "<广播信息>I<--BOARD_IP<%s>：%s\n", ip_addr, buf);
			int i = 0;
			while(h_buf[i] != '\n') i++;
			write(h_fd, h_buf, i+1);
		}
	}
	
	close(sockfd);
}
/*接收文件*/
void *Recv_File(void *arg)
{
	pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);		//设置为响应取消
    pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL);			//设置为立即响应
	//1,获取TCP套接字  -- socket()
	int sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if(-1 == sockfd)
	{
		perror("socket Recv_File failed");
		return NULL;
	}
	
	//2,绑定自己的IP和端口号	-- bind()
	struct sockaddr_in  server_addr, client_addr;
	socklen_t addrlen = sizeof(server_addr);
	
	server_addr.sin_family = AF_INET;
	server_addr.sin_port =  htons(NETWORK_PORT);	//把字符串的端口转换成网络字节序端口号
	//server_addr.sin_addr.s_addr = inet_addr(LOCAL_IP);
	server_addr.sin_addr.s_addr = htonl(INADDR_ANY);	//本机的IP
	
	if(bind(sockfd, (struct sockaddr *)&server_addr, addrlen) < 0)
	{
		perror("bind Recv_File failed");
		return NULL;
	}
	while(1)
	{	
		//3,设置监听套接字		-- listen()
		listen(sockfd, 1);
		
		int talkfd;
		long filesize, recvsize = 0;
		size_t ret;
		char filename[32] = {0};
		char ip_addr[20] = {0};
		char MsgBuf[1024] = {0};
		char path[64] = {0};
		
		talkfd = accept(sockfd, (struct sockaddr *)&client_addr, &addrlen);
		inet_ntop(AF_INET, &(client_addr.sin_addr), ip_addr, sizeof(ip_addr));
		printf("发送方IP[%s], port[%d]\n", ip_addr, ntohs(client_addr.sin_port));	
		//5,接收第一个数据包 --> 解析文件名,文件大小
		recv(talkfd, MsgBuf, sizeof(MsgBuf), 0);
		sscanf(MsgBuf, "%ld:%s", &filesize, filename);
		printf("接收的文件名:%s, 文件大小:%ld\n",filename, filesize);

		// printf("请输入文件保存的路径：");
		// scanf("%s", path);
		// while(getchar() != '\n');
		// strcat(path, filename);

		sprintf(path, "./%s", filename);
		
		//6,在本地创建一个同名文件 --> 打开文件
		FILE *fp = fopen(path, "w");
		if(NULL == fp)
		{
			perror("fopen failed");
			return NULL;
		}
		printf("正在接收文件...\n");
		//7,循环接收数据,记录接收的数据总大小 --> 如果接收的总数据大小大于等于文件大小 --> 结束
		size_t s = 0;
		while(1)
		{
			memset(MsgBuf, 0, sizeof(MsgBuf));
			ret = recv(talkfd, MsgBuf, sizeof(MsgBuf), 0);
			fwrite(MsgBuf, 1, ret, fp);
			recvsize += ret;	//当前接收的文件大小
			if(s != recvsize*100/filesize)
			{
				printf("文件已接收 %ld%%\n", s);
				s = recvsize*100/filesize;
			}
			if(recvsize >= filesize)
			{
				printf("文件已接收 100%%\n");
				break;
			}
		}
		printf("接收文件完成！\n");
		//8,关闭套接字,关闭文件描述符
		fclose(fp);
		close(talkfd);
	}
	close(sockfd);
}

bool IP_Addr_PORT_Init(void)
{
	system("clear");
	printf("**********************************************************************\n");
	printf("\t\t\tIP地址初始化\n");
	printf("请输入本机IP<LOCAL_IP>: "); scanf("%s", LOCAL_IP);
	if(!IS_N_addr(LOCAL_IP))
	{
		printf("错误的IP输入！！！\n");
		sleep(1);
		return false;
	}
	/********************初始化广播IP********************/
	int a, b, c, d;
	sscanf(LOCAL_IP, "%d.%d.%d.%d", &a, &b, &c, &d); d = 255;
	sprintf(BOARD_IP, "%d.%d.%d.%d", a, b, c, d);
	/***************************************************/
	printf("\t\t\t端口号初始化\n");
	printf("请输入单播端口号<SINGLE_PORT>: "); scanf("%d", &SINGLE_PORT);
	printf("请输入组播端口号<MUTIL_PORT>: "); scanf("%d", &MUTIL_PORT);
	printf("请输入广播端口号<BOARD_PORT>: "); scanf("%d", &BOARD_PORT);
	printf("请输入网络端口号<NETWORK_PORT>: "); scanf("%d", &NETWORK_PORT);
	printf("初始化IP和端口号已完成！\n");
	printf("\tLOCAL_IP:%s\tBOARD_IP:%s\n", LOCAL_IP, BOARD_IP);
	printf("\tSINGLE_PORT:%d\tMUTIL_PORT:%d\n", SINGLE_PORT, MUTIL_PORT);
	printf("\tBOARD_PORT:%d\tNETWORK_PORT:%d\n", BOARD_PORT, NETWORK_PORT);
	/************************判断是否确定初始化**************************/
	while(1)
	{
		int i;
		printf("是否确定该初始化？<0,YES\t1,NO> 请输入："); scanf("%d", &i);
		if(i == 0)
			break;
		else if(i == 1)
			return false;
		else
			printf("输入错误的指令！请重新输入！\n");
	}
	/******************************************************************/
	single_addr.sin_family = AF_INET;
	single_addr.sin_port =  htons(SINGLE_PORT);	//单播端口号
		
	mutil_addr.sin_family = AF_INET;
	mutil_addr.sin_port =  htons(MUTIL_PORT);	//组播端口号
	
	board_addr.sin_family = AF_INET;
	board_addr.sin_port =  htons(BOARD_PORT);	//广播播端口号
	board_addr.sin_addr.s_addr = inet_addr(BOARD_IP);	//广播IP	

	printf("即将进入主界面......\n");
	sleep(2);
	return true;
}


/*主线程UDP套接字初始化*/
int UDP_Socket_Init(void)
{
	//获取UDP套接字
	int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
	if(-1 == sockfd)
	{
		perror("sockfd sigle_recv failed");
		return -1;
	}	


	//使能广播属性
	int on = 1;
	setsockopt(sockfd, SOL_SOCKET, SO_BROADCAST, &on, sizeof(on));	

	//返回初始化的UDP套接字
	return sockfd;
}

/*主界面初始化*/
void Main_Init()
{
	system("clear");
	printf("**********************************************************************\n");
	printf("                            主功能窗口                            \n");
	printf("0,退出 1,发送单播 2,发送组播 3,发送广播 4,在线成员 5,历史记录\n");
	printf("特定命令： \n");
	printf("\t“#FILE:+文件路径”或“#file:+文件路径”\t表示传输文件\n");
	printf("\t“#EXIT”或“#exit”\t表示退出当前聊天\n");
	printf("**********************************************************************\n");
}



bool IS_N_addr(char *ip)
{
	int i = 0, j = 0;
	int a, b, c, d;
	while(ip[i] != '\0') 
	{
		if(ip[i] != '.' && ip[i]<'0' && ip[i]>'9')
			return false;
		i++;
	}
	sscanf(ip, "%d.%d.%d.%d", &a, &b, &c, &d);
	if(a>=0 && a<=255 && b>=0 && b<=255 && c>=0 && c<=255 && d>=0 && d<=255)
		return true;
	else
		return false;
}

void Show_History(void)
{
	system("clear");
	printf("**********************************************************************\n");
	printf("                           历史记录窗口                            \n");
	printf("\t0,退出\t1,查看所有历史记录\t2,查找指定消息 \n");
	printf("**********************************************************************\n");
	int cmd;
	while(1)
	{
		char buf[128], order[128];
		printf("历史记录指令输入(0,退出 1,查看所有历史记录 2,查找指定消息):\n");
		scanf("%d", &cmd); getchar();
		switch(cmd)
		{
			case 0:
				return;
			case 1:
				system("clear");
				printf("**********************************************************************\n");
				printf("历史记录：\n");
				printf("======================================================================\n");
				system("cat history.txt");
				printf("**********************************************************************\n");
				break;
			case 2:
				system("clear");
				printf("**********************************************************************\n");
				printf("请输入要查找的内容："); memset(buf, 0, sizeof(buf));
				fgets(buf, sizeof(buf), stdin); strtok(buf, "\n");
				memset(order, 0, sizeof(order));
				sprintf(order, "grep -n \"%s\" history.txt", buf);
				printf("**********************************************************************\n");
				printf("查找结果：\n");
				printf("======================================================================\n");
				system(order);
				printf("**********************************************************************\n");
				break;
			default:
				printf("输入的指令有误!请重新输入!\n");
				break;
		}
	}
}

