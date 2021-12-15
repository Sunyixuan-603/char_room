#ifndef _FUNC_H_
#define _FUNC_H_

#include <stdio.h>
#include <sys/types.h>          /* See NOTES */
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "list.h"

// #define  SIGLE_PORT	 10001		//单播端口号
// #define  MUTIL_PORT	 10002		//组播端口号
// #define  BOARD_PORT	 10003		//广播端口号
// #define  NETWORK_PORT 10004     //网络端口号

//#define MUTIL_IP  "224.0.0.10"		//组播IP
//#define BOARD_IP  "192.168.11.255"	//广播IP
//#define LOCAL_IP  "192.168.11.52"	//本机IP

#define HISTORY_PATH "history.txt"

typedef struct arg{
		Node *head;
		int history;
	}Arg;



void Send_Single(int sockfd, int h_fd);
void Send_Reply(int sockfd, char *IP, char *buf);
void Send_Mutil(int sockfd, int h_fd, Node *list_mutil);
void Send_Board(int sockfd, int h_fd, Node *list);
void Send_once_Board(int sockfd, int h_fd, char *buf);
int Send_File(char *IP, char *file_path);
void *Sigle_Recv(void *arg);
void *Mutil_Recv(void *arg);
void *Board_Recv(void *arg);
void *Recv_File(void *arg);

bool IP_Addr_PORT_Init(void);
int UDP_Socket_Init(void);
void UDP_Port_IP_Init(void);

void Main_Init();
bool IS_N_addr(char *ip);
void Show_History(void);

#endif



