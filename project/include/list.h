#ifndef _LIST_H_
#define _LIST_H_

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

//0,设计链表节点
typedef struct node{
	char ip[32];			//数据域
	struct node *next;	//指针域
}Node;		


Node *List_Create(void);
bool List_Add_Node_End(Node *head, char *data);
void List_Display(Node *head);
bool List_Search(Node *head, char *data);
bool List_Remove(Node *head, char *data);
void List_Destroy(Node *head);

#endif