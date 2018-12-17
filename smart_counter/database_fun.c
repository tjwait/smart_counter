
#include "database_fun.h"

#pragma comment(lib, "libmysql.lib")

struct counter_info * counter;
struct Board_Info * board_info;//称重板信息头指针

MYSQL * mysql = NULL;

static int finish_with_error()

{

	fprintf(stderr, "%s\n", mysql_error(mysql));

	mysql_close(mysql);

	return	-1;

}

//数据库初始化函数，使用之前先调用该函数
int init_db()
{
	//初始化mysql对象
	mysql = mysql_init(NULL);

	if (mysql == NULL)
	{
		fprintf(stderr, "%s\n", mysql_error(mysql));
		return -1;
	}

	mysql_options(mysql, MYSQL_SET_CHARSET_NAME, "gbk");//若不增加此句，则输出中文为问号，

														//连接数据库获取信息
	if (NULL == mysql_real_connect(mysql, "localhost", "root", "4567324", "smart_sales_counter", 3306, NULL, 0))
	//if (NULL == mysql_real_connect(mysql, "192.168.1.122", "root", "123456", "smart_sales_counter", 3306, NULL, 0))
	//if (NULL == mysql_real_connect(mysql, "localhost", "root", "123456", "smart_sales_counter", 3306, NULL, 0))
	{
		finish_with_error(mysql);
	}

	return DB_SUCCESS;

}

//关闭数据库，经测试这个关闭函数在数据库对象未初始化直接关闭没有异常，但是如果初始化后连接了数据库，然后关闭，再次关闭的时候就会报错
int db_close()
{
	//目前这个函数有一个问题，在关闭前应该判断下是否mysql有连接，但是还不知道如何判断
	mysql_close(mysql);
	return DB_SUCCESS;
}

/*
*	说明：获取柜子的信息
*	参数：无
*	注意：
*/
int Get_Counter_Info()
{
	if (counter != NULL)
	{
		//柜子信息结构体有数据
		free(counter);
	}

	counter = (struct counter_info *)malloc(sizeof(struct counter_info));

	//连接数据库获取信息
	//if (NULL == mysql_real_connect(mysql, "localhost", "root", "4567324", "smart_sales_counter", 3306, NULL, 0))
	//{
		//finish_with_error(mysql);
	//}

	char  query_sql[1024] = "select * from smart_sales_counter.counter_info";

	if (mysql_query(mysql, query_sql))
	{
		finish_with_error(mysql);
	}

	MYSQL_RES * result;//保存结果的指针

	result = mysql_store_result(mysql);//获取语句执行结果

	if (NULL == result)
	{
		finish_with_error(mysql);
	}

	int num_fields = mysql_num_fields(result);//获取结果的表中有多少列
	int num_row = mysql_num_rows(result);//获取行数，正常情况下应该只有1行

	if (num_row > 1) { printf("counter info rows greater than 1\r\n"); }//counter_info 这张表的记录数量不应该多于一条，否则按照目前逻辑，应该只保留最后一行的记录

	MYSQL_ROW row;//遍历结果的各个行用的变量
	
	while ((row = mysql_fetch_row(result)))
	{
		counter->sn = (char *)malloc(strlen(row[0]) + 1);
		strcpy(counter->sn, row[0]);
		counter->name = (char *)malloc(strlen(row[1]) + 1);
		strcpy(counter->name, row[1]);
		counter->info = (char *)malloc(strlen(row[2]) + 1);
		strcpy(counter->info, row[2]);
		counter->installation_date = (char *)malloc(strlen(row[3]) + 1);
		strcpy(counter->installation_date, row[3]);
		counter->manger = (char *)malloc(strlen(row[4]) + 1);
		strcpy(counter->manger, row[4]);
		counter->server_ip = (char *)malloc(strlen(row[5]) + 1);
		strcpy(counter->server_ip, row[5]);
		counter->server_port = (char *)malloc(strlen(row[6]) + 1);
		strcpy(counter->server_port, row[6]);
		counter->exchange_name = (char *)malloc(strlen(row[7]) + 1);
		strcpy(counter->exchange_name, row[7]);
		counter->queue_name = (char *)malloc(strlen(row[8]) + 1);
		strcpy(counter->queue_name, row[8]);
		counter->routingkey = (char *)malloc(strlen(row[9]) + 1);
		strcpy(counter->routingkey, row[9]);
		counter->com_port = (char *)malloc(strlen(row[10]) + 1);
		strcpy(counter->com_port , row[10]);
		counter->com_port_T = (char *)malloc(strlen(row[11]) + 1);
		strcpy(counter->com_port_T, row[11]);
		counter->channel = (char *)malloc(strlen(row[12]) + 1);
		strcpy(counter->channel, row[12]);
		counter->mq_name = (char *)malloc(strlen(row[13]) + 1);
		strcpy(counter->mq_name, row[13]);
		counter->mq_pw = (char *)malloc(strlen(row[14]) + 1);
		strcpy(counter->mq_pw, row[14]);
		counter->locker_id = (char *)malloc(strlen(row[15]) + 1);
		strcpy(counter->locker_id, row[15]);
		counter->max_kind = (char *)malloc(strlen(row[16]) + 1);
		strcpy(counter->max_kind, row[16]);
		counter->max_buy = (char *)malloc(strlen(row[17]) + 1);
		strcpy(counter->max_buy, row[17]);
		counter->error_value = (char *)malloc(strlen(row[18]) + 1);
		strcpy(counter->error_value, row[18]);

		counter->boards_num = 0;//目前暂未使用
		counter->locker_stat = -1;
		counter->IsBusy = -1;
		//printf("%s ", row[i] ? row[i] : "NULL");
	}
	mysql_free_result(result);//释放结果资源
							  
							  
	//db_close();

	return DB_SUCCESS;

}


/*
*	说明：获取称重板信息，并且在函数内部完成链表结构的初始化
*	参数：无
*	注意：
*/
int Get_Board_Info()
{
	if (board_info != NULL)
	{
		//柜子信息结构体有数据
		free(board_info);
	}

	board_info = (struct Board_Info *)malloc(sizeof(struct Board_Info));
	memset(board_info, 0, sizeof(struct Board_Info));
	struct Board_Info * board_info_rare = board_info;//临时队尾指针

	//连接数据库获取信息
	//if (NULL == mysql_real_connect(mysql, "localhost", "root", "4567324", "smart_sales_counter", 3306, NULL, 0))
	//{
		//finish_with_error(mysql);
	//}

	char  query_sql[1024] = "select * from smart_sales_counter.board";

	if (mysql_query(mysql, query_sql))
	{
		finish_with_error(mysql);
	}

	MYSQL_RES * result;//保存结果的指针

	result = mysql_store_result(mysql);//获取语句执行结果

	if (NULL == result)
	{
		finish_with_error(mysql);
	}

	int num_fields = mysql_num_fields(result);//获取结果的表中有多少列
	int num_row = mysql_num_rows(result);

	MYSQL_ROW row;//遍历结果的各个行用的变量

	int first_node = 1;
	while ((row = mysql_fetch_row(result)))
	{
		if (first_node == 1)
		{
			board_info_rare->id = row[0] != NULL ? (char *)malloc(strlen(row[0]) + 1) : NULL;//(char *)malloc(strlen(row[0]) + 1);
			if (row[0] != NULL)
			{
				strcpy(board_info_rare->id, row[0]);
			}

			board_info_rare->name = row[1] != NULL ? (char *)malloc(strlen(row[1]) + 1) : NULL;//(char *)malloc(strlen(row[1]) + 1);
			if (row[1] != NULL)
			{
				strcpy(board_info_rare->name, row[1]);
			}

			board_info_rare->type = row[2] != NULL ? (char *)malloc(strlen(row[2]) + 1) : NULL;//(char *)malloc(strlen(row[2]) + 1);
			if(row[2] != NULL)
			{
				strcpy(board_info_rare->type, row[2]);
			}

			board_info_rare->scheme_id = (row[3] != NULL ? (char *)malloc(strlen(row[3]) + 1) : NULL);
			if (row[3] != NULL)
			{
				strcpy(board_info_rare->scheme_id, row[3]);
			}

			board_info_rare->Board_Stat = BOARD_STAT_UNKNOW;
			board_info_rare->items = NULL;
			board_info_rare->board_items_weight_all = 0xFFFF;//无效值
			board_info_rare->board_items_weight_all_after_close_door = 0;//该值每次使用后都清零
			board_info_rare->IsBasicValue = 0;//未去皮
			board_info_rare->IsBasicValueSave = 0;//去皮未保存
			board_info_rare->ISNeedCur = ISNEEDCUR_NO;
			board_info_rare->ISCurvatureValue = 0;//校准
			board_info_rare->ISCurvatureValueSave = 0;//校准保存
			memset(&board_info_rare->Curvature, 0, sizeof(board_info_rare->Curvature));

			first_node = 0;
		}
		else
		{
			board_info_rare->next = (struct Board_Info *)malloc(sizeof(struct Board_Info));
			memset(board_info_rare->next, 0, sizeof(struct Board_Info));
			if (board_info_rare->next != NULL)//若不成功，则不会进入此if语句内，若失败则应退出程序
			{
				board_info_rare = board_info_rare->next;

				board_info_rare->id = row[0] != NULL ? (char *)malloc(strlen(row[0]) + 1) : NULL;//(char *)malloc(strlen(row[0]) + 1);
				if (row[0] != NULL)
				{
					strcpy(board_info_rare->id, row[0]);
				}

				board_info_rare->name = row[1] != NULL ? (char *)malloc(strlen(row[1]) + 1) : NULL;//(char *)malloc(strlen(row[1]) + 1);
				if (row[1] != NULL)
				{
					strcpy(board_info_rare->name, row[1]);
				}

				board_info_rare->type = row[2] != NULL ? (char *)malloc(strlen(row[2]) + 1) : NULL;//(char *)malloc(strlen(row[2]) + 1);
				if (row[2] != NULL)
				{
					strcpy(board_info_rare->type, row[2]);
				}

				board_info_rare->scheme_id = (row[3] != NULL ? (char *)malloc(strlen(row[3]) + 1) : NULL);
				if (row[3] != NULL)
				{
					strcpy(board_info_rare->scheme_id, row[3]);
				}


				board_info_rare->Board_Stat = BOARD_STAT_UNKNOW;
				board_info_rare->board_items_weight_all = 0xFFFF;//无效值
				board_info_rare->board_items_weight_all_after_close_door = 0;//该值每次使用后都清零
				board_info_rare->IsBasicValue = 0;//未校准
				board_info_rare->IsBasicValueSave = 0;//去皮未保存
				board_info_rare->ISNeedCur = ISNEEDCUR_NO;
				board_info_rare->ISCurvatureValue = 0;//校准
				board_info_rare->ISCurvatureValueSave = 0;//校准保存
				memset(&board_info_rare->Curvature, 0, sizeof(board_info_rare->Curvature));

			}

		}
	}
	board_info_rare->next = NULL;//最后一个元素指针指向空
	mysql_free_result(result);//释放结果资源
	
	//db_close();

	return DB_SUCCESS;

}

/*
*	说明：获取销售商品信息
*	参数：无
*	注意：这个函数每次执行都会先清空所有同board关联的item链表然后重新获取，考虑到柜子内的商品种类数量有限，因此采取每次更新items表后
*		  都将board所关联的items链表清空然后在重新建立也是可行的
*		  数据算法为，将逐个获取items表中的各个商品，并且将每一个商品同所有称重板进行比对，找到属于某一个称重板的编号
*		  然后将此商品连接至该称重板数据结构中的items链表的后面
*		 
*		增加板子的items链表中的商品的销售类型必须同称重板的销售类型一致的判定
*		
*		*每一个items记录都有一个总重量，这个值是表示这类商品的总重量，如果称重板的计价模式为1，则按照目前的 逻辑可认为总重量就是称重板的总重量
*			但如果计价模式为2，则该记录的总重量仅代表这类商品的，并不能代表是称重板的总重量，因此在称重板信息的数据结构中有一个是称重板总重量的属性
*			该属性是表达的是称重板的总重量，并且已经考虑了计价模式为1或者为2的情况
*/
int Get_Item_Info()
{
	//board_info = (struct Board_Info *)malloc(sizeof(struct Board_Info));

	Free_Item_Info();//先释放所有称重板关联的items链表内存
	
	struct Board_Info * board_info_p = board_info;
	struct Items * items_p = NULL;

	char  query_sql[1024] = "select * from smart_sales_counter.items";

	if (mysql_query(mysql, query_sql))
	{
		finish_with_error(mysql);
	}

	MYSQL_RES * result;//保存结果的指针

	result = mysql_store_result(mysql);//获取语句执行结果

	if (NULL == result)
	{
		finish_with_error(mysql);
	}

	//int num_fields = mysql_num_fields(result);//获取结果的表中有多少列
	//int num_row = mysql_num_rows(result);//获取行数

	MYSQL_ROW row;//遍历结果的各个行用的变量

	//int first_node = 1;
	while ((row = mysql_fetch_row(result)))
	{
		//先找到这个商品所属的称重板，此处并未对无法找到匹配的称重板items数据的处理
		while (board_info_p != NULL)
		{
			if ((strcmp(board_info_p->id, row[3]) == 0)  && (strcmp(board_info_p->type, row[6]) == 0) )//该items同所绑定的称重板编号和销售类型必须一致才能绑定至该称重板的items链表中
			{
				//代码达到此处，代表找到目标的称重板
				//开始寻找此称重板items链表的最后一个元素，并且添加
				items_p = board_info_p->items;
				if (items_p == NULL)
				{
					//称重板货物信息链表结构的首个节点，添加首个元素后跳出while循环
					items_p = (struct Items *)malloc(sizeof(struct Items));

					items_p->item_id = row[1] != NULL ? (char *)malloc(strlen(row[1]) + 1) : NULL;
					strcpy(items_p->item_id, row[1]);

					items_p->barcode = row[2] != NULL ? (char *)malloc(strlen(row[2]) + 1) : NULL;
					strcpy(items_p->barcode, row[2]);


					items_p->board_id = row[3] != NULL ? (char *)malloc(strlen(row[3]) + 1) : NULL;
					strcpy(items_p->board_id, row[3]);

					items_p->name = row[4] != NULL ? (char *)malloc(strlen(row[4]) + 1) : NULL;
					strcpy(items_p->name, row[4]);

					items_p->supplier = row[5] != NULL ? (char *)malloc(strlen(row[5]) + 1) : NULL;
					strcpy(items_p->supplier, row[5]);

					items_p->saletype = row[6] != NULL ? (char *)malloc(strlen(row[6]) + 1) : NULL;
					strcpy(items_p->saletype, row[6]);

					items_p->ind_count = row[7] != NULL ?  (char *)malloc(strlen(row[7]) + 1) : NULL;
					strcpy(items_p->ind_count, row[7]);

					items_p->ind_weight = row[8] != NULL ? (char *)malloc(strlen(row[8]) + 1) : NULL;
					strcpy(items_p->ind_weight, row[8]);

					items_p->ind_price = row[9] != NULL ? (char *)malloc(strlen(row[9]) + 1) : NULL;
					strcpy(items_p->ind_price, row[9]);

					items_p->weight_sum = row[10] != NULL ? (char *)malloc(strlen(row[10]) + 1) : NULL;
					strcpy(items_p->weight_sum, row[10]);

					items_p->weight_price = row[11] != NULL ? (char *)malloc(strlen(row[11]) + 1) : NULL;
					strcpy(items_p->weight_price, row[11]);

					items_p->next = NULL;

					board_info_p->items = items_p;

					break;//此处退出的是某一个称重板items链表轮询的循环，退出后开始执行items数据库表中的下一条数据的初始化
				}
				else 
				{
					//代码到达此处代表此称重板items链表中已经有数据，需要找到最后一条
					while (items_p->next != NULL) { items_p = items_p->next; }
					//代码到达此处代表已经找到item链表的最后一个元素了
					items_p->next = (struct Items *)malloc(sizeof(struct Items));
					items_p = items_p->next;

					items_p->item_id = row[1] != NULL ? (char *)malloc(strlen(row[1]) + 1) : NULL;
					strcpy(items_p->item_id, row[1]);

					items_p->barcode = row[2] != NULL ? (char *)malloc(strlen(row[2]) + 1) : NULL;
					strcpy(items_p->barcode, row[2]);


					items_p->board_id = row[3] != NULL ? (char *)malloc(strlen(row[3]) + 1) : NULL;
					strcpy(items_p->board_id, row[3]);

					items_p->name = row[4] != NULL ? (char *)malloc(strlen(row[4]) + 1) : NULL;
					strcpy(items_p->name, row[4]);

					items_p->supplier = row[5] != NULL ? (char *)malloc(strlen(row[5]) + 1) : NULL;
					strcpy(items_p->supplier, row[5]);

					items_p->saletype = row[6] != NULL ? (char *)malloc(strlen(row[6]) + 1) : NULL;
					strcpy(items_p->saletype, row[6]);

					items_p->ind_count = row[7] != NULL ? (char *)malloc(strlen(row[7]) + 1) : NULL;
					strcpy(items_p->ind_count, row[7]);

					items_p->ind_weight = row[8] != NULL ? (char *)malloc(strlen(row[8]) + 1) : NULL;
					strcpy(items_p->ind_weight, row[8]);

					items_p->ind_price = row[9] != NULL ? (char *)malloc(strlen(row[9]) + 1) : NULL;
					strcpy(items_p->ind_price, row[9]);

					items_p->weight_sum = row[10] != NULL ? (char *)malloc(strlen(row[10]) + 1) : NULL;
					strcpy(items_p->weight_sum, row[10]);

					items_p->weight_price = row[11] != NULL ? (char *)malloc(strlen(row[11]) + 1) : NULL;
					strcpy(items_p->weight_price, row[11]);
					
					items_p->next = NULL;

					break;//此处退出的是某一个称重板items链表轮询的循环，退出后开始执行items数据库表中的下一条数据的初始化

				}
				
			}//轮询某一层称重板的items链表

			board_info_p = board_info_p->next;
		}
		board_info_p = board_info;//每次都是从头开始寻找
		
	}
	mysql_free_result(result);//释放结果资源

	return DB_SUCCESS;

}


/*
*	说明：释放货物链表
*	参数：无
*	注意：
*/
void Free_Item_Info()
{
	struct Board_Info * board_info_p = board_info;
	struct Items * items_p = NULL;
	struct Items * items_n = NULL;
	while (board_info_p != NULL)
	{
		items_p = items_n = board_info_p->items;
		while (items_p != NULL)
		{
			items_p = items_p->next;
			free(items_n->barcode);
			free(items_n->board_id);
			free(items_n->ind_count);
			free(items_n->ind_price);
			free(items_n->ind_weight);
			free(items_n->item_id);
			free(items_n->name);
			free(items_n->saletype);
			free(items_n->supplier);
			free(items_n->weight_price);
			free(items_n->weight_sum);
			items_n->next = NULL;
			free(items_n);
			items_n = items_p;
		}
		board_info_p->items = NULL;
		board_info_p = board_info_p->next;
	}

}


/*
*	功能：统计柜子内所有称重板上所有的货物重量
*	参数：
*	说明：#这个统计是指将称重板上所有的item链表中的产品重量进行相加所得，并且将该值存入柜子信息的数据结构体中；
*		  #这个函数一定要在执行Get_Item_Info()函数之后才可以执行；
*		  #这个函数是将从数据库里面某一个板子上面的商品总重量进行相加，因此该函数不会失败，暂不做异常判定
*		  #这个函数已经考虑了不同计价模式总重量计算的差异
*/
void Get_Boards_Items_Weight()
{
	struct Board_Info * board_info_p = board_info;
	struct Items * items_p = NULL;
	double weight_buf = 0;

	while (board_info_p != NULL)
	{
		items_p = board_info_p->items;
		weight_buf = 0;
		while (items_p != NULL)
		{
			//若销售模式为按重量销售，则每一个称重板只能放置一种商品，若是按照个数销售则每一个称重板可以放置不同类型的多种货物	
			weight_buf = weight_buf + CharNum_To_Double(items_p->weight_sum);//weight_sum这个值应该在商品上架的时候就由系统计算好
			items_p = items_p->next;
		}
		board_info_p->board_items_weight_all = weight_buf;//赋值全部重量
		board_info_p = board_info_p->next;
	}
	printf("称重板货物重量统计完毕!\r\n");

}


/*
*	功能：对某一个表执行updata命令
*	参数：
*	说明：若更新成功则返回受影响的行数，如果为-1 则为sql执行失败
*/
INT64 SQL_UPDATA1(char * tablename , char * col_name1 , char * col_value1 , char * condition_name , char * condition_value)
{
	char  query_sql[1024] =  "update ";
	strcat(query_sql, tablename);
	strcat(query_sql, " set ");
	strcat(query_sql, col_name1);
	strcat(query_sql, " = '");
	strcat(query_sql, col_value1);
	strcat(query_sql, "' where ");
	strcat(query_sql, condition_name);
	strcat(query_sql, " = '");
	strcat(query_sql, condition_value);
	strcat(query_sql, "'");

	if (mysql_query(mysql, query_sql))//若成功mysql_query函数返回0
	{
		finish_with_error(mysql);
	}

	return mysql_affected_rows(mysql);
}

/*
*	功能：对某一个表执行updata命令
*	参数：
*	说明：#若更新成功则返回受影响的行数，如果为-1 则为sql执行失败
*		  #更新的字段值为数字的，其中更新内容为字符串和数字sql语句不一样，若为数字则value的值无需加''
*/
INT64 SQL_UPDATA2(char * tablename, char * col_name1, char * col_value1, char * condition_name, char * condition_value)
{
	char  query_sql[1024] = "update ";
	strcat(query_sql, tablename);
	strcat(query_sql, " set ");
	strcat(query_sql, col_name1);
	strcat(query_sql, " = ");
	strcat(query_sql, col_value1);
	strcat(query_sql, " where ");
	strcat(query_sql, condition_name);
	strcat(query_sql, " = '");
	strcat(query_sql, condition_value);
	strcat(query_sql, "'");

	if (mysql_query(mysql, query_sql))//若成功mysql_query函数返回0
	{
		finish_with_error(mysql);
	}

	return mysql_affected_rows(mysql);
}


/*
*	功能：对items表执行插入数据
*	参数：
*	说明：返回值为受影响的行数，如果为-1 则为sql执行失败
*/
INT64 SQL_INSERT_INTO_ItemsTable(char * item_id, char * barcode, char * board_id, char * name, char * supplier, char * saletype, \
	char * ind_count, char * ind_weight, char * ind_price, char * weight_sum, char * weight_price)
{
	unsigned char change_code_buf[200] = { 0 };
	char  query_sql[1024] = "insert into smart_sales_counter.items ( item_id , barcode ,  board_id , name , supplier , saletype , ind_count , ind_weight , ind_price , weight_sum , weight_price )values ( '";
	strcat(query_sql, item_id);
	strcat(query_sql, "' , '");
	strcat(query_sql, barcode);
	strcat(query_sql, "' , '");
	strcat(query_sql, board_id);
	strcat(query_sql, "' , '");
	//is_valid_utf8(name, strlen(name));
	UTF8ToGBK(name, change_code_buf, 200);
	strcat(query_sql, change_code_buf);
	strcat(query_sql, "' , '");
	UTF8ToGBK(supplier, change_code_buf, 200);
	strcat(query_sql, change_code_buf); //supplier
	strcat(query_sql, "' , '");
	strcat(query_sql, saletype);
	strcat(query_sql, "' , '");
	strcat(query_sql, ind_count);
	strcat(query_sql, "' , '");
	strcat(query_sql, ind_weight);
	strcat(query_sql, "' , '");
	strcat(query_sql, ind_price);
	strcat(query_sql, "' , '");
	strcat(query_sql, weight_sum);
	strcat(query_sql, "' , '");
	strcat(query_sql, weight_price);
	strcat(query_sql, "' ) ");

	if (mysql_query(mysql, query_sql))//若成功mysql_query函数返回0
	{
		finish_with_error(mysql);
	}

	return mysql_affected_rows(mysql);

}

/*
*	功能：对涉及scheme方案的三个表执行一次性插入，执行insert
*	参数：
*	说明：返回值为受影响的行数，如果为-1 则为sql执行失败
*/
INT64 SQL_INSERT_INTO_Scheme(struct scheme * scheme_p)
{
	unsigned char * buf[20] = { 0 };
	char  query_sql[1024] = "insert into smart_sales_counter.scheme values ( '";
	strcat(query_sql, scheme_p->scheme_id);
	strcat(query_sql, "' , '");
	strcat(query_sql, scheme_p->scheme_name);
	strcat(query_sql, "' , '");
	Int_To_CharArray(scheme_p->interval, buf);
	strcat(query_sql, buf);
	strcat(query_sql, "' , '");
	Int_To_CharArray(scheme_p->scheme_count, buf);
	strcat(query_sql, buf);
	strcat(query_sql, "' , '");
	Int_To_CharArray(scheme_p->error_count, buf);
	strcat(query_sql, buf);
	strcat(query_sql, "' , '");
	Int_To_CharArray(scheme_p->error_per, buf);
	strcat(query_sql, buf);
	strcat(query_sql, "' ) ");

	if (mysql_query(mysql, query_sql))//若成功mysql_query函数返回0
	{
		finish_with_error(mysql);
	}

	if (mysql_affected_rows(mysql) <= 0)//sql执行失败或者是插入失败
	{
		return -1;
	}
	struct scheme_node * scheme_node_p = scheme_p->schemes;
	while (scheme_node_p != NULL)
	{
		memset(query_sql, 0, 1024);
		strcat(query_sql, "insert into smart_sales_counter.scheme_node values ( '");
		//char  query_sql[1024] = "insert into smart_sales_counter.scheme_node values ( '";
		Int_To_CharArray(scheme_node_p->schemem_node_id, buf);
		strcat(query_sql, buf);
		strcat(query_sql, "' , '");
		strcat(query_sql, scheme_node_p->scheme_id);
		strcat(query_sql, "' , '");
		Int_To_CharArray(scheme_node_p->schemem_node_weight, buf);
		strcat(query_sql, buf);
		strcat(query_sql, "' , '");
		Double_To_CharArray(scheme_node_p->schemem_node_price, buf);
		strcat(query_sql, buf);
		strcat(query_sql, "' ) ");

		if (mysql_query(mysql, query_sql))//若成功mysql_query函数返回0
		{
			finish_with_error(mysql);
		}

		if (mysql_affected_rows(mysql) <= 0)//sql执行失败或者是插入失败
		{
			return -1;
		}

		//将该方案节点的产品信息插入数据库
		struct scheme_node_items_list_node * scheme_node_items_list_node_p = scheme_node_p->items_list_node;
		while (scheme_node_items_list_node_p != NULL)
		{
			memset(query_sql, 0, 1024);
			strcat(query_sql, "insert into smart_sales_counter.scheme_node_items_list_node values ( '");
			//char  query_sql[1024] = "insert into smart_sales_counter.scheme_node values ( '";
			Int_To_CharArray(scheme_node_items_list_node_p->scheme_node_items_list_node_id, buf);
			strcat(query_sql, buf);
			strcat(query_sql, "' , '");
			Int_To_CharArray(scheme_node_items_list_node_p->schemem_node_id, buf);
			strcat(query_sql, buf);
			strcat(query_sql, "' , '");
			strcat(query_sql, scheme_node_items_list_node_p->scheme_id);
			strcat(query_sql, "' , '");
			strcat(query_sql, scheme_node_items_list_node_p->item_id);
			strcat(query_sql, "' , '");
			strcat(query_sql, scheme_node_items_list_node_p->item_name);
			strcat(query_sql, "' , '");
			Int_To_CharArray(scheme_node_items_list_node_p->item_num, buf);
			strcat(query_sql, buf);
			strcat(query_sql, "' ) ");

			if (mysql_query(mysql, query_sql))//若成功mysql_query函数返回0
			{
				finish_with_error(mysql);
			}

			if (mysql_affected_rows(mysql) <= 0)//sql执行失败或者是插入失败
			{
				return -1;
			}

			scheme_node_items_list_node_p = scheme_node_items_list_node_p->next;
		}
		scheme_node_p = scheme_node_p->next;
	}

}

/*
*	功能：删除某一个表的某些数据，按照编号删除
*	参数：
*	说明：返回值为受影响的行数，如果为-1 则为sql执行失败
*/
INT64 SQL_DELETE(char * tablename, char * condition_name, char * condition_value)
{
	char  query_sql[1024] = "delete from ";
	//"delete from smart_sales_counter.items where ";
	strcat(query_sql, tablename);
	strcat(query_sql, " where ");
	strcat(query_sql, condition_name);
	strcat(query_sql, " = '");
	strcat(query_sql, condition_value);
	strcat(query_sql, "'");

	if (mysql_query(mysql, query_sql))
	{
		finish_with_error(mysql);
	}

	if (mysql_affected_rows(mysql) <= 0)//sql执行失败或者是删除失败
	{
		return -1;
	}
}


/*
*	功能：从某一个表中查找相同项目
*	参数：
*	说明：返回值为查找到的记录数目
*/
INT64 SQL_SELECT(char * tablename, char * condition_name, char * condition_value)
{
	unsigned char change_code_buf[200] = { 0 };
	char  query_sql[1024] = "select * from ";
	strcat(query_sql, tablename);
	strcat(query_sql, " where ");
	strcat(query_sql, condition_name);
	strcat(query_sql, " = '");
	UTF8ToGBK(condition_value, change_code_buf, 200);
	strcat(query_sql, change_code_buf);
	strcat(query_sql, "'");

	if (mysql_query(mysql, query_sql))
	{
		finish_with_error(mysql);
	}
	MYSQL_RES * result;//保存结果的指针
	result = mysql_store_result(mysql);//获取语句执行结果
									   //INT64 i =  mysql_affected_rows(mysql);
	INT64 num_row = 0;
	if (result != NULL)
	{
		num_row = mysql_num_rows(result);//获取行数，正常情况下应该只有1行
	}

	mysql_free_result(result);//释放结果资源
	return num_row;

}


/*
*	功能: 执行一条服务器传送过来的sql select 语句
*	参数：
*	说明：返回值为受影响的行数，如果为-1 则为sql执行失败
*/

char * Procedure_SQL_Select(JSON_Object * json_object)
{
	
	JSON_Value *root_value = json_value_init_object();//初始化了一个value，其类型为object，value为json数据的一个类型，不同的value类型可以嵌套
	JSON_Object *root_object = json_value_get_object(root_value);//获取value中的object的地址

	JSON_Value *sub_value = json_value_init_array();
	JSON_Array *sub_array = json_value_get_array(sub_value);

	time_t timep;
	time(&timep);
	json_object_set_string(root_object, "devid", counter->sn);
	json_object_set_string(root_object, "cmdid", "SQL_Select");
	json_object_dotset_string(root_object, "Result.Res", "0");//该命令总是成功
	json_object_dotset_string(root_object, "Result.Date", ctime(&timep));

	char * sqlstr = json_object_get_string(json_object, "SqlStr");
	if (sqlstr != NULL)
	{

		//考虑执行的sql语句可能有中文，因此统一转换成为gbk码再执行
		unsigned char sql_gbk[500] = { 0 };
		UTF8ToGBK(sqlstr, sql_gbk, 500);

		if (mysql_query(mysql, sql_gbk))
		{
			finish_with_error(mysql);
		}

		MYSQL_RES * result;//保存结果的指针

		result = mysql_store_result(mysql);//获取语句执行结果

		if (NULL == result)
		{
			finish_with_error(mysql);
		}

		int num_fields = mysql_num_fields(result);//获取结果的表中有多少列
		int num_row = mysql_num_rows(result);//获取行数，正常情况下应该只有1行

		//if (num_row > 1) { printf("counter info rows greater than 1\r\n"); }//counter_info 这张表的记录数量不应该多于一条，否则按照目前逻辑，应该只保留最后一行的记录


		JSON_Value *sub_sub_value[1024];//此处定义一个大缓存
		JSON_Array *sub_sub_array = NULL;
		int sub_sub_value_pos = 0;//使用的value的位置
		MYSQL_ROW row;//遍历结果的各个行用的变量

		char utf8_buf[200] = { 0 };
		while ((row = mysql_fetch_row(result)))
		{
			sub_sub_value[sub_sub_value_pos++] = json_value_init_array();//
			sub_sub_array = json_value_get_array(sub_sub_value[sub_sub_value_pos - 1]);
			for (int i = 0; i < num_fields; i++)//遍历各个列
			{
				//若数据为中文 编码应该为GBK因此无法正常添加至json，需要转码
				if (is_valid_utf8(row[i] != NULL ? row[i] : NULL, strlen(row[i])) == 0)//若为0，则代表数组不是utf8编码
				{
					memset(utf8_buf, 0, 200);
					GBKToUTF8(row[i], utf8_buf, 200);
					json_array_append_string(sub_sub_array, utf8_buf);
				}
				else
				{
					json_array_append_string(sub_sub_array, row[i] != NULL ? row[i] : NULL);
				}
				
			}

			json_array_append_value(sub_array, sub_sub_value[sub_sub_value_pos - 1]);

		}

		json_object_dotset_value(root_object, "Result.Data", sub_value);
		//将json字符序列化处理，并深拷贝至一个字符串中，函数返回该字符串，
		char * json_string = (char *)malloc(json_serialization_size(root_value));//json_serialization_size这个函数的返回值已经考虑了将原字符串长度进行扩展，因为还要增加一些额外的字符，如换行、反斜杠、大括号等
		json_serialize_to_buffer(root_value, json_string, json_serialization_size(root_value));
		//释放json资源
		json_value_free(root_value);//只需释放根资源，其内部关联的所有资源会被递归释放

		mysql_free_result(result);//释放结果资源

		return json_string;//改指针会随着函数跳出而失效，但指向的该内存必须在外部显式释放



	}


}

/*
*	功能: 执行一条服务器传送过来的sql 的更新 语句
*	参数：
*	说明：注此处指的updata是指更新的意思，而不是sql updata 语句，该函数处理包括增删改三个类型的sql语句
*/

char * Procedure_SQL_Updata(JSON_Object * json_object)
{

	JSON_Value *root_value = json_value_init_object();//初始化了一个value，其类型为object，value为json数据的一个类型，不同的value类型可以嵌套
	JSON_Object *root_object = json_value_get_object(root_value);//获取value中的object的地址

	time_t timep;
	time(&timep);
	json_object_set_string(root_object, "devid", counter->sn);
	json_object_set_string(root_object, "cmdid", "SQL_Updata");
	json_object_dotset_string(root_object, "Result.Res", "0");//该命令总是成功,即命令只要拼写没有错误就一定执行成功，但是该命令的结果不一定成功，
	json_object_dotset_string(root_object, "Result.Date", ctime(&timep));

	char * sqlstr = json_object_get_string(json_object, "SqlStr");
	if (sqlstr != NULL)
	{
		//考虑执行的sql语句可能有中文，因此统一转换成为gbk码再执行
		unsigned char sql_gbk[500] = { 0 };
		UTF8ToGBK(sqlstr, sql_gbk, 500);

		if (mysql_query(mysql, sql_gbk))
		{
			finish_with_error(mysql);
		}

		INT64 affected_rows_num = mysql_affected_rows(mysql);

		unsigned char num_buf[100] = { 0 };
		Int_To_CharArray(affected_rows_num, num_buf);//注意这个转关若更新受影响的结果比较多，大于int能表达的上限，此处可能会有问题

		json_object_dotset_string(root_object, "Result.AffRows", num_buf);
		//将json字符序列化处理，并深拷贝至一个字符串中，函数返回该字符串，
		char * json_string = (char *)malloc(json_serialization_size(root_value));//json_serialization_size这个函数的返回值已经考虑了将原字符串长度进行扩展，因为还要增加一些额外的字符，如换行、反斜杠、大括号等
		json_serialize_to_buffer(root_value, json_string, json_serialization_size(root_value));
		//释放json资源
		json_value_free(root_value);//只需释放根资源，其内部关联的所有资源会被递归释放

		return json_string;//改指针会随着函数跳出而失效，但指向的该内存必须在外部显式释放


	}


}

/*
*	功能: 根据输入的方案编号和重量，查找结果
*	参数：
			[in]scheme_id 方案编号
			[in]weight_buf 称得的重量变化，有正负号
			[out]weight_buf_fix 根据方案分析所得的应该该称重板减去多少重量，传入的weight_buf为测量值，weight_buf_fix为准确没有误差的值，该值为返回值
			[out]sub_sub_value 返回的json数组的值，该值为一个json_value 数组，每一个元素都是一个数组，对应一个item信息
			[out]函数返回json数组的子集个数
*	说明：注此处指的updata是指更新的意思，而不是sql updata 语句，该函数处理包括增删改三个类型的sql语句
*/

int Scheme_Parse(char * scheme_id , double weight_buf , double * weight_buf_fix , double * price ,  JSON_Value **sub_sub_value)
{

	char  query_sql[1024] = "select * from smart_sales_counter.scheme where scheme_id = '";
	strcat(query_sql, scheme_id);
	strcat(query_sql, "'");
	if (mysql_query(mysql, query_sql))
	{
		finish_with_error(mysql);
	}
	MYSQL_RES * result;//保存结果的指针
	result = mysql_store_result(mysql);//获取语句执行结果
									   //INT64 i =  mysql_affected_rows(mysql);
	int num_row = 0;
	if (result == NULL)
	{
		finish_with_error(mysql);
	}
	else
	{
		num_row = mysql_num_rows(result);//获取行数，正常情况下应该只有1行
		mysql_free_result(result);//释放结果资源
		if (num_row == 0)
		{
			return 1;//方案编号不存在
		}
	}

	unsigned char change_code[200] = { 0 };
	memset(query_sql, 0, 1024);
	strcpy(query_sql, "select * from smart_sales_counter.scheme_node where scheme_id = '");
	strcat(query_sql, scheme_id);
	strcat(query_sql, "' AND node_weight >= '");
	Int_To_CharArray(abs(weight_buf) - CharNum_To_Double(counter->error_value) , change_code);
	strcat(query_sql, change_code);
	strcat(query_sql, "' AND node_weight <= '");
	Int_To_CharArray(abs(weight_buf) + CharNum_To_Double(counter->error_value), change_code);
	strcat(query_sql, change_code);
	strcat(query_sql, "'");

	if (mysql_query(mysql, query_sql))
	{
		finish_with_error(mysql);
	}

	result = mysql_store_result(mysql);//获取语句执行结果
									   //INT64 i =  mysql_affected_rows(mysql);
	num_row = 0;
	if (result == NULL)
	{
		finish_with_error(mysql);
	}
	else
	{
		num_row = mysql_num_rows(result);//获取行数，正常情况下应该只有1行
										 //mysql_free_result(result);//释放结果资源
		if (num_row != 1)
		{
			mysql_free_result(result);//释放结果资源
			return 2;//重量区间不存在，即没有落在该区域的方案节点
		}
		else
		{
			//只有一个记录的时候
			MYSQL_ROW row;
			row = mysql_fetch_row(result);//因为只有一行，所以此处直接获取无需遍历
			//将此节点的重量赋值给称得的重量的修正值
			*weight_buf_fix = CharNum_To_Double(row[2]);
			*price = CharNum_To_Double(row[3]);

			memset(query_sql, 0, 1024);
			strcpy(query_sql, "select * from smart_sales_counter.scheme_node_items_list_node where scheme_id = '");
			strcat(query_sql, scheme_id);
			strcat(query_sql, "' AND scheme_node_id = '");
			strcat(query_sql, row[0]);
			strcat(query_sql, "'");

			if (mysql_query(mysql, query_sql))
			{
				finish_with_error(mysql);
			}
			mysql_free_result(result);//释放结果资源
			result = mysql_store_result(mysql);//获取语句执行结果

			int num_fields = mysql_num_fields(result);//获取结果的表中有多少列

			JSON_Array *sub_sub_array = NULL;
			char utf8_buf[200] = { 0 };
			int sub_sub_value_pos = 0;//使用的value的位置
			while ((row = mysql_fetch_row(result)))
			{
				sub_sub_value[sub_sub_value_pos++] = json_value_init_array();//
				sub_sub_array = json_value_get_array(sub_sub_value[sub_sub_value_pos - 1]);
				for (int i = 3; i < num_fields; i++)//遍历各个列
				{
					memset(utf8_buf, 0, 200);
					//若数据为中文 编码应该为GBK因此无法正常添加至json，需要转码
					if (is_valid_utf8(row[i] != NULL ? row[i] : NULL, strlen(row[i])) == 0)//若为0，则代表数组不是utf8编码
					{
						if (i == 5 && weight_buf < 0)//i = 5 为本类商品一共有几个的数据
						{
							//代码应该永远不会进入这个部分，因为数字不是GBK编码
							Int_To_CharArray(0 - CharNum_To_Double(row[i]) , change_code);//将商品拿走的数量增加一个负号，以表示是取走货物，此方式只在P&P命令下，在销售模式下不一定需要，还不确定
							GBKToUTF8(change_code, utf8_buf, 200);
							json_array_append_string(sub_sub_array, utf8_buf);
						}
						else
						{
							GBKToUTF8(row[i], utf8_buf, 200);
							json_array_append_string(sub_sub_array, utf8_buf);
						}
						
					}
					else
					{
						if (i == 5 && weight_buf < 0)//i = 5 为本类商品一共有几个的数据
						{
							Int_To_CharArray(0 - CharNum_To_Double(row[i]), change_code);//将商品拿走的数量增加一个负号，以表示是取走货物，此方式只在P&P命令下，在销售模式下不一定需要，还不确定
							json_array_append_string(sub_sub_array, change_code != NULL ? change_code : NULL);
						}
						else
						{
							GBKToUTF8(row[i], utf8_buf, 200);
							json_array_append_string(sub_sub_array, row[i] != NULL ? row[i] : NULL);
						}
						
					}
					//每个方案的一个产品信息更新完毕后，统一更新该产品所对应的items的表格中的数据，包括单品数量和该类商品的总重
					if (i == num_fields - 1)//更新每一个items的库存量，和此种货物的总重量，注意计价模式为2的时候，不但要更新称重板总重量，还要更新每一个商品的库存量
					{
						//向返回数据添加完某一类商品信息后，额外增加一个总价的信息值子数组的最后一个元素
						Double_To_CharArray(CharNum_To_Double(row[5]) * CharNum_To_Double(row[6]), change_code);
						json_array_append_string(sub_sub_array, change_code);
						//要根据weight_buf的符号判断货物是放入还是取出
						Int_To_CharArray(*weight_buf_fix, change_code);
						if (weight_buf > 0)
						{
							//为放入
							memset(utf8_buf, 0, 200);
							strcpy(utf8_buf, " ind_count + ");//此处使用utf8_buf数组是为了不再申请新的缓冲区
							strcat(utf8_buf, row[5]);
							strcat(utf8_buf, "  ");
							//注意以下sql语句用法实现了数据直接在原值的基础之上进行增加
							SQL_UPDATA2("smart_sales_counter.items", "ind_count", utf8_buf, "item_id", row[3]);

							memset(utf8_buf, 0, 200);
							strcpy(utf8_buf, " weight_sum + ");//此处使用utf8_buf数组是为了不再申请新的缓冲区
							strcat(utf8_buf, change_code);
							strcat(utf8_buf, "  ");
							SQL_UPDATA2("smart_sales_counter.items", "weight_sum", utf8_buf, "item_id", row[3]);

						}
						else
						{
							//为取出
							memset(utf8_buf, 0, 200);
							strcpy(utf8_buf, " ind_count - ");//此处使用utf8_buf数组是为了不再申请新的缓冲区
							strcat(utf8_buf, row[5]);
							strcat(utf8_buf, "  ");
							//注意以下sql语句用法实现了数据直接在原值的基础之上进行增加
							SQL_UPDATA2("smart_sales_counter.items", "ind_count", utf8_buf, "item_id", row[3]);

							memset(utf8_buf, 0, 200);
							strcpy(utf8_buf, " weight_sum - ");//此处使用utf8_buf数组是为了不再申请新的缓冲区
							strcat(utf8_buf, change_code);
							strcat(utf8_buf, "  ");
							SQL_UPDATA2("smart_sales_counter.items", "weight_sum", utf8_buf , "item_id", row[3]);

						}
						
					}

				}


			}
			return sub_sub_value_pos;
		}

	}


}


/*
*	功能：从数据库获取指定名编号的scheme
*	参数：
*	说明：该函数将根据方案编号初始化一个方案，并放置至柜体的方案数组中
		  该函数将涉及到三个表的数据获取
*/
void Get_Scheme(char * scheme_id)
{

	char  query_sql[1024] = "select * from smart_sales_counter.scheme where scheme_id = '";
	strcat(query_sql, scheme_id);
	strcat(query_sql, "'");
	if (mysql_query(mysql, query_sql))
	{
		finish_with_error(mysql);
	}

	/*
	if (mysql_affected_rows(mysql) <= 0)//sql执行失败或者是查询没有相关方案的编号
	{
		return -1;
	}
	*/

	//在全局方案数组中查找待从数据库读取的方案是否已经引入内存中
	for (int j = 0; j < MAX_SCHEMES; j++)
	{
		if (scheme_globle[j].Isused && strcmp(scheme_globle[j].scheme_id, scheme_id) == 0)
		{
			//方案已经被保存
			return;
		}
	}

	int i = 0;
	for (; i < MAX_SCHEMES; i++)
	{
		if (scheme_globle[i].Isused == 0 )
		{
			//开始获取数据中的指定编号的方案数据
			MYSQL_RES * result;//保存结果的指针

			result = mysql_store_result(mysql);//获取语句执行结果，此时已经对scheme表执行了查询语句了

			if (NULL == result)
			{
				finish_with_error(mysql);
			}

			int num_row = mysql_num_rows(result);//获取行数，正常情况下应该只有1行

			if (num_row > 1) { sys_die("rows greater than 1\r\n"); }//应该只能查询到一条记录

			MYSQL_ROW row;//遍历结果的各个行用的变量

			while ((row = mysql_fetch_row(result)))//此处应该只循环一次
			{
				scheme_globle[i].scheme_id = (char *)malloc(strlen(row[0]) + 1);
				strcpy(scheme_globle[i].scheme_id, row[0]);
				scheme_globle[i].scheme_name = (char *)malloc(strlen(row[1]) + 1);
				strcpy(scheme_globle[i].scheme_name , row[1]);
				scheme_globle[i].interval = CharNum_To_Double(row[2]);
				scheme_globle[i].scheme_count = CharNum_To_Double(row[3]);
				scheme_globle[i].error_count = CharNum_To_Double(row[4]);
				scheme_globle[i].error_per = CharNum_To_Double(row[5]);
				scheme_globle[i].schemes = NULL;
			}
			mysql_free_result(result);//释放结果资源
			scheme_globle[i].Isused == 1;//将该节点标识位已经使用

			//开始填充某一个方案中的各个节点
			memset(query_sql, 0, 1024);
			strcat(query_sql, "select * from smart_sales_counter.scheme_node where scheme_id = '");
			strcat(query_sql, scheme_id);
			strcat(query_sql, "'");

			if (mysql_query(mysql, query_sql))
			{
				finish_with_error(mysql);
			}

			if (mysql_affected_rows(mysql) <= 0)//sql执行失败或者是查询没有相关方案的编号
			{
				return -1;
			}

			result = mysql_store_result(mysql);//获取语句执行结果，此时已经对scheme_node表执行了查询语句了

			if (NULL == result)
			{
				finish_with_error(mysql);
			}

			num_row = mysql_num_rows(result);//获取行数，正常情况下应该只有1行

			//if (num_row > 1) { sys_die("rows greater than 1\r\n"); }//应该只能查询到一条记录

			struct scheme_node * node_p = NULL;
			int head_time = 1;//为使用首个元素而设定的变量
			while ((row = mysql_fetch_row(result)))//此处应该只循环一次
			{
				if (head_time == 1)//为首个节点
				{
					scheme_globle[i].schemes = node_p = (struct scheme_node *)malloc(sizeof(struct scheme_node));
					head_time = 0;
				}
				else
				{
					node_p->next = (struct scheme_node *)malloc(sizeof(struct scheme_node));
					node_p = node_p->next;
				}
				memset(node_p, 0, sizeof(struct scheme_node));
				node_p->schemem_node_id = CharNum_To_Double(row[0]);
				//注此处并未对该元素赋值，而是让该元素指向了schmem的id地址，这样做是因为数据库查询出来的方案节点其隶属一定是该方案编号的
				//因此无需对该值进行赋值，另外在释放资源的时候也较为容易
				node_p->scheme_id  = scheme_globle[i].scheme_id;
				node_p->schemem_node_weight = CharNum_To_Double(row[2]);
				node_p->schemem_node_price = CharNum_To_Double(row[3]);
				node_p->items_list_node = NULL;
				
			}
			mysql_free_result(result);//释放结果资源

			//开始填充各个方案节点中的items_list节点信息
			node_p = scheme_globle[i].schemes;
			for (int j = 0; j < scheme_globle[i].scheme_count; j++)
			{
				memset(query_sql, 0, 1024);
				unsigned char num_buf[10];
				strcat(query_sql, 
						"select * from smart_sales_counter.scheme_node_items_list_node where scheme_id = '");
				strcat(query_sql, scheme_id);
				strcat(query_sql, "' and scheme_node_id = '" );
				Int_To_CharArray(j, num_buf);
				strcat(query_sql, num_buf);
				strcat(query_sql, " '");
				if (mysql_query(mysql, query_sql))
				{
					finish_with_error(mysql);
				}

				if (mysql_affected_rows(mysql) <= 0)//sql执行失败或者是查询没有相关方案的编号
				{
					return -1;
				}

				result = mysql_store_result(mysql);//获取语句执行结果，此时已经对scheme_node表执行了查询语句了

				if (NULL == result)
				{
					finish_with_error(mysql);
				}
				struct scheme_node_items_list_node * scheme_node_items_list_node_p = NULL;
				head_time = 1;
				while ((row = mysql_fetch_row(result)))//此处应该只循环一次
				{
					if (head_time == 1)//为首个节点
					{
						node_p->items_list_node = scheme_node_items_list_node_p = \
										(struct scheme_node_items_list_node *)malloc(sizeof(struct scheme_node_items_list_node));
						
						head_time = 0;
					}
					else
					{
						scheme_node_items_list_node_p->next = \
										(struct scheme_node_items_list_node *)malloc(sizeof(struct scheme_node_items_list_node));
						scheme_node_items_list_node_p = scheme_node_items_list_node_p->next;
					}
					memset(scheme_node_items_list_node_p, 0, sizeof(struct scheme_node_items_list_node));

					scheme_node_items_list_node_p->scheme_node_items_list_node_id = CharNum_To_Double(row[0]);
					scheme_node_items_list_node_p->schemem_node_id = CharNum_To_Double(row[1]);//该编号就是本次循环的次数
					scheme_node_items_list_node_p->scheme_id = scheme_globle[i].scheme_id;
					scheme_node_items_list_node_p->item_id = (char *)malloc(strlen(row[3]) + 1);
					strcpy(scheme_node_items_list_node_p->item_id, row[3]);
					scheme_node_items_list_node_p->item_name = (char *)malloc(strlen(row[4]) + 1);
					strcpy(scheme_node_items_list_node_p->item_name, row[4]);
					scheme_node_items_list_node_p->item_num = CharNum_To_Double(row[5]);


				}

				node_p = node_p->next;//方案节点下移一位
			}


			break;//推出寻找空闲方案位置循环
		}
		


	}//方案数组循环结束
	if (i == MAX_SCHEMES)
	{
		//代码到达此处则代表方案数组里面都满了
	}

	
}


/*
*	功能：删除某一个方案的数据库数据
*	参数：[in]删除方案编号
*	说明：该函数应该不会被外部直接调用
*/
void Free_Scheme_DB(char * scheme_id)
{
	char  query_sql[1024] = "delete from smart_sales_counter.scheme where scheme_id = '";
	strcat(query_sql, scheme_id);
	strcat(query_sql, "'");

	if (mysql_query(mysql, query_sql))
	{
		finish_with_error(mysql);
	}

	if (mysql_affected_rows(mysql) <= 0)//sql执行失败或者是删除失败
	{
		return -1;
	}
}