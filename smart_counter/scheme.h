#pragma once

#ifdef	__cplusplus
extern "C" {
#endif

#include <string.h>
#include <stdio.h>
#include <assert.h>
#include "global.h"
#include "sys.h"
#include "database_fun.h"

#define MAX_SCHEMES 10//方案数组中的数量

//方案结构体，此结构体应该还包括各个商品子信息（还不能确定，先在此记录）,该结构体在生成后通过编号同board进行关联，一个board只能关联一个scheme
struct scheme
{
	char * scheme_id;
	char * scheme_name;
	int interval;//间隔，即不同解决方案之间的重量差距最小应该为多少，此值应该为允许误差值的两倍
	int scheme_count;//一共有多少个解决方案，所有解决方案组成一个链表，链表的每一个节点都对应一个方案
	int error_count;
	int error_per;//错误的百分比
	struct scheme_node * schemes;
	int Isused;//在全局方案数组中使用，表示该节点是否被使用,在方案生成函数中不使用，0为未使用，1为使用
};

struct scheme_node
{
	int schemem_node_id;
	char * scheme_id;//所属的解决方案编号
	//char * schemem_node_name;
	int schemem_node_weight;//本方案的重量，该值是要被查找的
	double  schemem_node_price;
	struct scheme_node * next;
	struct scheme_node_items_list_node * items_list_node;

};

struct scheme_node_items_list_node
{
	int  scheme_node_items_list_node_id;
	int  schemem_node_id;
	char *  scheme_id;
	char * item_id;
	char * item_name;
	int item_num;
	struct scheme_node_items_list_node * next;
};


//算法使用结构体
struct number
{
	struct Items * addr;
	int num;
};


int Scheme_Create(char * scheme_id, char * scheme_name, int error_value, int save);
void Free_scheme(struct scheme * scheme_p);
void Free_Scheme_Arrya_Node(char * scheme_id, int delete_db/*是否删除该方案数据中保存的信息*/);
//int Scheme_Create()

#ifdef	__cplusplus
}
#endif




