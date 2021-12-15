#include "list.h"
//1,创建链表	-->创建一个链表头
Node *List_Create(void)
{
	//1）对链表头结点空间申请 --> malloc
	Node *list = (Node *)malloc(sizeof(Node));
	if ( list == NULL )
	{
		perror("malloc failed ");	//打印出错信息
		return NULL;				//结束函数
	}

	//2）对链表头结点进行赋值
	list->next = NULL;	

	//3）返回链表头结点地址
	return list;	
}

// //2,链表节点插入 （头插法）
// bool List_Add_Node_Head(Node *head, char *data)
// {
// 	//1）给新节点申请堆内存
// 	Node *newnode = (Node *)malloc(sizeof(Node));
// 	if (newnode == NULL)
// 	{
// 		perror("newnode malloc failed");
// 		return false;
// 	}

// 	//2）新节点赋值
// 	int i = 0;
// 	while(data[i] != '\0')
// 	{
// 		newnode->ip[i] = data[i];
// 		i++;
// 	}
// 	newnode->ip[i] = '\0';
// 	newnode->next = NULL;

// 	//3）头插法插入链表
// 	newnode->next = head->next;
// 	head->next = newnode;

// 	return true;
// }
//尾插法
bool List_Add_Node_End(Node *head, char *data)
{
	if(List_Search(head, data))
		return false;
	else
	{
		//1）给新节点申请堆内存
		Node *newnode = (Node *)malloc(sizeof(Node));
		if ( newnode == NULL)
		{
			perror("malloc newnode failed");
			return false;
		}

		//2）给新节点赋值
		int i = 0;
		while(data[i] != '\0')
		{
			newnode->ip[i] = data[i];
			i++;
		}
		newnode->ip[i] = '\0';
		newnode->next = NULL;

		//3）尾插法插入链表
			//找到链表的最后一个节点 p (while(p->next !=NULL)p=p->next)
		Node *p = head;
		while(p->next != NULL)		//当这个循环结束时，p节点就是最后一个节点
		{
			p = p->next;
		}
			//新节点插入到p节点的后面
		p->next = newnode;
		return true;
	}
}


//3,链表数据输出
void List_Display(Node *head)
{
	//1）定义一个指针用来遍历链表
	Node *p = head->next;	//头结点不存放数据
	int i = 1;
	printf("在线成员：\n");
	//2）遍历链表，输出链表中的每一个节点的数据
	while(p != NULL)
	{
		printf("成员%d<%s>\t", i, p->ip);
		p = p->next;		//把p节点下一个节点的地址赋值给p
		if(i%3 == 0)
			printf("\n");
		i++;
	}
	printf("\n");
}

//4，链表节点查找
bool List_Search(Node *head, char *data)
{
	int i = 1;
	Node *p = head->next;		//遍历链表指针
	while(p != NULL)
	{
		if(strcmp(p->ip, data) == 0)	//找到这个节点
		{
			return true;	
		}
		p = p->next;
		i++;
	}
	return false;
}

//5,链表节点删除 ==>删除节点时只能删除一个节点,思考：如何同时删除多个节点？
bool List_Remove(Node *head, char *data)
{
	int i = 1;
	//1）找到需要被删除的节点p和p的前一个节点q
	Node *p = head->next;
	Node *q = head;		//q是p的前一个节点

	while(p != NULL)
	{
		if(strcmp(p->ip, data) == 0)//2）找到之后，删除节点p
		{
			q->next = p->next;	//从逻辑上删除p节点
			free(p);
			return true;
		}
		q = q->next;	//遍历链表
		p = q->next;
		i++;
	}
	//3）如果遍历完链表都没有找到，那就说明链表中没有这个节点
	printf("链表中没有这个节点！\n");
	return false;
}

//7,链表销毁
void List_Destroy(Node *head)
{
	int i = 0;
	Node *p = head;
	Node *q = p->next;

	while(q != NULL)	//释放头结点后面的所有节点
	{
		p = q;
		q = q->next;
		free(p);
		i++;
	}
	free(head);		//最后释放头结点
	printf("成功释放[%d]个节点\n", i);
}



#if 0
/*练习：输入数字，实现添加或者删除*/
int main(int argc, char const *argv[])
{
	int num;
	//1,创建一条链表
	Node *list = List_Create();

	//2,循环从键盘获取整数

	char ip[20] = "192.168.0.100";
	List_Add_Node_End(list, ip);	//尾插法插入数据
	List_Add_Node_End(list, "192.168.0.101");	//尾插法插入数据
	List_Add_Node_End(list, "192.168.0.102");	//尾插法插入数据
	List_Add_Node_End(list, "192.168.0.103");	//尾插法插入数据
	List_Display(list);
	List_Remove(list, "192.168.0.101");
	List_Display(list);
	memset(ip, 0, sizeof(ip));
	fgets(ip, sizeof(ip), stdin);
	int i = 0;
	while(ip[i] != '\n')
		i++;
	ip[i] = '\0';
	List_Remove(list, ip);
	List_Display(list);
	List_Destroy(list);

	return 0;
}
#endif





