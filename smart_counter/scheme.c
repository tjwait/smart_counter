#include "scheme.h"


struct scheme  scheme_globle[MAX_SCHEMES] = {0};//最多放10个方案，哪个称重板绑定那种方案由称重板对应属性决定

int STOCK_KIND = 0;
int MAX_BUY = 0;

/*
*	功能：生成一个解决方案
*	参数：
*	说明：此函数应该是被用户调用,每一个称重板对应一个解决方案
		  该函数是根据当前数据库中保存的称重板上的商品信息计算解决方案并可选择是否将解决方案存储至数据库中
		  该函数不涉及到系统中的scheme_global全局变量的任何操作

	注：由于目前方案的生成功能放置在服务器上，因此该函数不会在柜体系统中执行
*/

int Scheme_Create(char * scheme_id , char * scheme_name , int error_value , int save )
{
	
	struct scheme scheme;
	memset(&scheme, 0, sizeof(struct scheme));
	scheme.scheme_id = (char *)malloc( strlen(scheme_id) + 1);
	strcpy(scheme.scheme_id, scheme_id);
	scheme.scheme_name = (char *)malloc(strlen(scheme_name) + 1);
	strcpy(scheme.scheme_name, scheme_name);
	scheme.interval = error_value * 2;
	//scheme.error_count = 0;
	//scheme.error_per = 0;
	//scheme.scheme_count = 0;
	scheme.schemes = NULL;//方案链表
	
	
	STOCK_KIND = CharNum_To_Double(counter->max_kind);//设定称重板最多能够放置的产品种类，这个值是数据库里面的
	MAX_BUY = CharNum_To_Double(counter->max_buy);

	int i = 0;
	struct number number[200] = { 0 };//一个柜子的一个称重板最多也不会放置200个产品
	struct Board_Info * bip = board_info;
	while (bip != NULL)//遍历所有称重板
	{
		struct Items * item_p = bip->items;
		while (item_p != NULL)
		{
			//将number的每一个元素同该称重板的item链表元素对应上
			number[i].addr = item_p;
			number[i++].num = 0;//表示该类产品拿走的数量，在方案算法中是用哪个，此处是初始化为0
			item_p = item_p->next;
		}
		STOCK_KIND = STOCK_KIND > i ? i : STOCK_KIND;//对实际放置的产品种类数量同STOCK_KIND进行比较，如果实际放置的种类小于STOCK_KIND则以
													 //实际放置的数量为准，注：如果放置的种类数量大于设定值会怎样还未分析，应该是按照数据库
													 //录入顺序对于超出STOCK_KIND数量的产品不予计算

		//代码到达此处代表某一个称重板的item链表同number数组一一对应完毕，可以开始某一个称重板的算法
		for (int buy_num = 1; buy_num < MAX_BUY; buy_num++)//若MAX_BUY数量大于实际该称重板产品库存数量，则实际会按照产品库存数量计算方案
		{
			test_new(&scheme/*这个链表为最终结果*/, bip->items,
				buy_num /*单次消费者能够购买多少种商品*/, 0/*该值为循环递归调用深度，深度值等于商品种类,首次调用该函数时永远为0*/,
				&number/*这个值为递归调用时记录之前所有层拿取商品数量的，为一个工具类数值，函数调用完毕后不使用*/);

		}

		//某一个称重板方案已经计算完毕，计算方案的错误百分比，错误是指某两个方案重量的间距小于称重板设定的误差
		struct scheme_node * node_p_head = scheme.schemes;//指向方案结构链表内的方案节点地址
		struct scheme_node * node_p_tail = node_p_head->next;
		double between_count = 0;
		//由于没有排序，因此不能按照欧排序的方案进行处理
		while (node_p_head != NULL)
		{
			node_p_tail = node_p_head->next;

			while (node_p_tail != NULL)
			{
				between_count++;
				if (abs(node_p_head->schemem_node_weight - node_p_tail->schemem_node_weight) <= scheme.interval)
				{
					scheme.error_count += 1;
				}
				node_p_tail = node_p_tail->next;
			}

			node_p_head = node_p_head->next;
		}

		//注意异常比例这个值不是用异常的值除以总的方案格式，而是除以每一个值同其他所有值比较的总次数，重复的不计算
		scheme.error_per = (scheme.error_count / between_count) * 100;
		//存入数据库
		if(save)
		{
			SQL_INSERT_INTO_Scheme(&scheme);
		}

		bip = bip->next;
	}
	
	//释放资源
	/*
	struct scheme * scheme_p = &scheme;
	if (scheme_p != NULL)
	{
		
		struct scheme_node * scheme_node_p_next = scheme_p->schemes;
		struct scheme_node * scheme_node_before = scheme_p->schemes;
		while (scheme_node_p_next != NULL)
		{

			struct scheme_node_items_list_node * scheme_node_items_list_node_p_next = scheme_node_p_next->items_list_node;
			struct scheme_node_items_list_node * scheme_node_items_list_node_p_before = scheme_node_p_next->items_list_node;
			while (scheme_node_items_list_node_p_next != NULL)
			{
				scheme_node_items_list_node_p_before = scheme_node_items_list_node_p_next;
				scheme_node_items_list_node_p_next = scheme_node_items_list_node_p_next->next;
				free(scheme_node_items_list_node_p_before);

			}

			scheme_node_before = scheme_node_p_next;
			scheme_node_p_next = scheme_node_p_next->next;
			free(scheme_node_before);
		}
		//free(scheme_p); 不需要此句，scheme是一个局部变量，函数退出后该变量就会失效,但其内部的编号和名称两个属性是通过malloc申请的，需要单独释放
		free(scheme_p->scheme_id);
		free(scheme_p->scheme_name);
	}
	*/
	Free_scheme(&scheme);//此处释放的是在该函数内数申请的scheme的结构体，同全局方案数组没有关系

}



int test_new(struct scheme * scheme_list/*这个链表为最终结果*/, struct Items * items_list,
	int max_buy /*单次消费者能够购买多少种商品*/, int depth/*该值为循环递归调用深度，深度值等于商品种类,首次调用该函数时永远为0*/,
	struct number * number/*这个值为递归调用时记录之前所有层拿取商品数量的，为一个工具类数值，函数调用完毕后不使用*/)
{
	int num_buf = 0;
	int max = CharNum_To_Double(items_list->ind_count) > max_buy ? max_buy : CharNum_To_Double(items_list->ind_count);//设定当前层（当前种类商品最多能够购买几个）

	if (depth == STOCK_KIND - 1)
	{
		for (int k = 0; k < depth; k++)
		{
			//printf("number are : %d\r\n", number[k]);
			num_buf += number[k].num;
		}

		for (int i = 0; i <= max; i++)
		{
			if (num_buf + i == max_buy)
			{
				number[depth].num = i;//number 此时记录了所有能够满足最大购买量的组合，每一个元素位置对应items_list位置，每一个元素的值代表拿取此货物的数量
								  //因此number数组的长度一定等于商品的种类之和，其内部各个元素之和一定等于MAX_BUY

				scheme_list->scheme_count = scheme_list->scheme_count + 1;//方案节点数量加1
				struct scheme_node * node_p = scheme_list->schemes;//指向方案结构链表内的方案节点地址
				if (node_p == NULL)
				{
					//代码到达此处代表链表为空
					scheme_list->schemes = node_p = (struct scheme_node *)malloc(sizeof(struct scheme_node));
					memset(node_p, 0, sizeof(struct scheme_node));
					node_p->scheme_id = scheme_list->scheme_id;
					node_p->schemem_node_id = 0;//首个节点
					node_p->next = NULL;
					node_p->items_list_node = NULL;//这个是某一个解决方案链表中的所有产品链表信息，该链表内每一个节点都是组成这个方案的产品信息和数量

					for (int k = 0; k <= depth; k++)//depth编号是items_list的每一个元素编号，而且无需减1，因此此循环k的值要一直取到depth
					{
						if (number[k].num > 0)//在本类商品中拿取了货物
						{
							node_p->schemem_node_weight += number[k].num * CharNum_To_Double(number[k].addr->ind_weight);
							node_p->schemem_node_price += number[k].num * CharNum_To_Double(number[k].addr->ind_price);
							struct scheme_node_items_list_node * list_node_p = node_p->items_list_node;
							if (list_node_p == NULL)
							{
								//代码到达此处代表为本方案item链表中首个元素
								node_p->items_list_node = list_node_p = (struct scheme_node_items_list_node *)malloc(sizeof(struct scheme_node_items_list_node));
								list_node_p->item_id = (char *)malloc(strlen(number[k].addr->item_id) + 1);
								strcpy(list_node_p->item_id, number[k].addr->item_id);
								list_node_p->item_name = (char *)malloc(strlen(number[k].addr->name) + 1);
								strcpy(list_node_p->item_name, number[k].addr->name);
								//list_node_p->item_id = number[k].addr->item_id;
								//list_node_p->item_name = number[k].addr->name;
								list_node_p->item_num = number[k].num;
								list_node_p->schemem_node_id = node_p->schemem_node_id;//这节点所属某一个方案节点编号
								list_node_p->scheme_id = scheme_list->scheme_id;//这个节点所属总方案编号
								list_node_p->scheme_node_items_list_node_id = 0;//该节点编号
								list_node_p->next = NULL;
							}
							else
							{
								while (list_node_p->next != NULL) { list_node_p = list_node_p->next; }//寻找最后一个元素
								list_node_p->next = (struct scheme_node_items_list_node *)malloc(sizeof(struct scheme_node_items_list_node));
								list_node_p->next->scheme_node_items_list_node_id = list_node_p->scheme_node_items_list_node_id + 1;
								list_node_p = list_node_p->next;
								list_node_p->item_id = (char *)malloc(strlen(number[k].addr->item_id) + 1);
								strcpy(list_node_p->item_id, number[k].addr->item_id);
								list_node_p->item_name = (char *)malloc(strlen(number[k].addr->name) + 1);
								strcpy(list_node_p->item_name, number[k].addr->name);
								//list_node_p->item_id = number[k].addr->item_id;
								//list_node_p->item_name = number[k].addr->name;
								list_node_p->item_num = number[k].num;
								list_node_p->schemem_node_id = node_p->schemem_node_id;
								list_node_p->scheme_id = scheme_list->scheme_id;//这个节点所属总方案编号
								list_node_p->next = NULL;

							}


						}
						
					}

				}
				else
				{
					//链表不为空
					while (node_p->next != NULL) { node_p = node_p->next; }//寻找到链表的最后一个元素
																		   //代码到达此处代表链表为空
					node_p->next = (struct scheme_node *)malloc(sizeof(struct scheme_node));
					memset(node_p->next, 0, sizeof(struct scheme_node));
					node_p->next->schemem_node_id = node_p->schemem_node_id + 1;
					node_p = node_p->next;
					node_p->scheme_id = scheme_list->scheme_id;
					//node_p->schemem_node_id = 0;//首个节点
					node_p->next = NULL;
					node_p->items_list_node = NULL;

					for (int k = 0; k <= depth; k++)//depth编号是items_list的每一个元素编号，而且无需减1，因此此循环k的值要一直取到depth
					{
						if (number[k].num > 0)//在本类商品中拿取了货物
						{
							node_p->schemem_node_weight += number[k].num * CharNum_To_Double(number[k].addr->ind_weight);
							node_p->schemem_node_price += number[k].num * CharNum_To_Double(number[k].addr->ind_price);
							struct scheme_node_items_list_node * list_node_p = node_p->items_list_node;
							if (list_node_p == NULL)
							{
								//代码到达此处代表为本方案item链表中首个元素
								node_p->items_list_node = list_node_p = (struct scheme_node_items_list_node *)malloc(sizeof(struct scheme_node_items_list_node));
								list_node_p->item_id = (char *)malloc(strlen(number[k].addr->item_id) + 1);
								strcpy(list_node_p->item_id, number[k].addr->item_id);
								list_node_p->item_name = (char *)malloc(strlen(number[k].addr->name) + 1);
								strcpy(list_node_p->item_name, number[k].addr->name);
								//list_node_p->item_id = number[k].addr->item_id;
								//list_node_p->item_name = number[k].addr->name;
								list_node_p->item_num = number[k].num;
								list_node_p->schemem_node_id = node_p->schemem_node_id;
								list_node_p->scheme_node_items_list_node_id = 0;
								list_node_p->scheme_id = scheme_list->scheme_id;//这个节点所属总方案编号
								list_node_p->next = NULL;
							}
							else
							{
								while (list_node_p->next != NULL) { list_node_p = list_node_p->next; }//寻找最后一个元素
								list_node_p->next = (struct scheme_node_items_list_node *)malloc(sizeof(struct scheme_node_items_list_node));
								list_node_p->next->scheme_node_items_list_node_id = list_node_p->scheme_node_items_list_node_id + 1;
								list_node_p = list_node_p->next;
								list_node_p->item_id = (char *)malloc(strlen(number[k].addr->item_id) + 1);
								strcpy(list_node_p->item_id, number[k].addr->item_id);
								list_node_p->item_name = (char *)malloc(strlen(number[k].addr->name) + 1);
								strcpy(list_node_p->item_name, number[k].addr->name);
								//list_node_p->item_id = number[k].addr->item_id;
								//list_node_p->item_name = number[k].addr->name;
								list_node_p->item_num = number[k].num;
								list_node_p->schemem_node_id = node_p->schemem_node_id;
								list_node_p->scheme_id = scheme_list->scheme_id;//这个节点所属总方案编号
								list_node_p->next = NULL;

							}


						}

					}

				}

				/* 测试函数*/

				for (int k = 0; k <= depth; k++)
				{
					printf("Stoct  %s   number are  %d \t", number[k].addr->name , number[k].num);
				}
				printf("\r\n");

			}

		}
	}
	else
	{
		for (int i = 0; i <= max; i++)//循环调用，计算每一种商品拿取的数量从0至能够购买的最多之间所有的可能，其中最后一类商品
									  //代码不会到达此处，最后一件商品其实质上补足作用，即若截止最后一件商品之前所拿取的商品
									  //数量不足能够购买的最大数量，则用最后一件商品的数量不足，如果最后一件商品无法补足，则增
									  //前一个商品数量，或者是更前一个商品，以此类推，直至结束
		{
			number[depth].num = i;
			test_new(scheme_list, items_list->next, max_buy, depth + 1, number);
			//test(number, max, depth + 1);
		}
	}

}


/*
*	功能：释放方案方案数组中的某一个方案整体
*	参数：
*	说明：在系统中方案是一个全局数组变量，不是链表结构，因此对于scheme指针不需要释放，但其中内部的编号和名称需要释放，另对要释放的节点
		  数值应该赋予默认值（还不知道赋值多少合适）
*/
void Free_scheme(struct scheme * scheme_p)
{
	if (scheme_p != NULL)
	{

		struct scheme_node * scheme_node_p_next = scheme_p->schemes;
		struct scheme_node * scheme_node_before = scheme_p->schemes;
		while (scheme_node_p_next != NULL)
		{

			struct scheme_node_items_list_node * scheme_node_items_list_node_p_next = scheme_node_p_next->items_list_node;
			struct scheme_node_items_list_node * scheme_node_items_list_node_p_before = scheme_node_p_next->items_list_node;
			while (scheme_node_items_list_node_p_next != NULL)
			{
				scheme_node_items_list_node_p_before = scheme_node_items_list_node_p_next;
				scheme_node_items_list_node_p_next = scheme_node_items_list_node_p_next->next;
				free(scheme_node_items_list_node_p_before->item_id);//这个变量是从items表格中拷贝过来的，可以free
				free(scheme_node_items_list_node_p_before->item_name);//这个变量是从items表格中拷贝过来的，可以free
				free(scheme_node_items_list_node_p_before);
				scheme_node_items_list_node_p_before = NULL;
			}

			scheme_node_before = scheme_node_p_next;
			scheme_node_p_next = scheme_node_p_next->next;
			free(scheme_node_before);
			scheme_node_before = NULL;
		}

		free(scheme_p->scheme_id);
		free(scheme_p->scheme_name);
		memset(scheme_p, 0, sizeof(struct scheme));
		scheme_p->scheme_id = NULL;
		scheme_p->scheme_name = NULL;
		scheme_p->schemes = NULL;
	}
	//将该节点的各个值赋初始化值，暂未编写
	scheme_p->Isused = 0;
}


/*
*	功能：从方案数组中删除指定方案，并且删除数据库中相关数据
*	参数：[in]删除方案编号
[in]是否连同删除该方案保存在数据库中的数据，0不删除 1删除
*	说明：
*/
void Free_Scheme_Arrya_Node(char * scheme_id, int delete_db/*是否删除该方案数据中保存的信息*/)
{
	for (int i = 0; i < MAX_SCHEMES; i++)
	{
		if (scheme_globle[i].scheme_id != NULL && strcmp(scheme_globle[i].scheme_id, scheme_id) == 0)
		{
			Free_scheme(&scheme_globle[i]);
			if (delete_db)
			{
				Free_Scheme_DB(scheme_id);
			}
			return;
		}
	}
}