
#include "database_fun.h"

#pragma comment(lib, "libmysql.lib")

struct counter_info * counter;
struct Board_Info * board_info;//���ذ���Ϣͷָ��

MYSQL * mysql = NULL;

static int finish_with_error()

{
	LogWrite(ERR, "%s", mysql_error(mysql));
	//fprintf(stderr, "%s\n", mysql_error(mysql));
	//mysql_close(mysql);

	return	-1;

}

//���ݿ��ʼ��������ʹ��֮ǰ�ȵ��øú���
int init_db()
{
	//��ʼ��mysql����
	mysql = mysql_init(NULL);

	if (mysql == NULL)
	{
		//fprintf(stderr, "%s\n", mysql_error(mysql));
		LogWrite(ERR, "%s", mysql_error(mysql));
		//exit(-1);
		return DB_FAILURE;
	}

	mysql_options(mysql, MYSQL_SET_CHARSET_NAME, "gbk");//�������Ӵ˾䣬���������Ϊ�ʺţ�

	//�������ݿ��ȡ��Ϣ


	if (NULL == mysql_real_connect(mysql, "localhost", "root", "4567324", "smart_sales_counter", 3306, NULL, 0))
		//if (NULL == mysql_real_connect(mysql, "192.168.1.122", "root", "123456", "smart_sales_counter", 3306, NULL, 0))
		//if (NULL == mysql_real_connect(mysql, "localhost", "root", "123456", "smart_sales_counter", 3306, NULL, 0))
	{
		finish_with_error(mysql);
		mysql_close(mysql);
		return DB_FAILURE;
	}

	return DB_SUCCESS;
}

//�ر����ݿ⣬����������رպ��������ݿ����δ��ʼ��ֱ�ӹر�û���쳣�����������ʼ�������������ݿ⣬Ȼ��رգ��ٴιرյ�ʱ��ͻᱨ��
int db_close()
{
	//Ŀǰ���������һ�����⣬�ڹر�ǰӦ���ж����Ƿ�mysql�����ӣ����ǻ���֪������ж�
	mysql_close(mysql);
	return DB_SUCCESS;
}

/*
*	˵������ȡ���ӵ���Ϣ
*	��������
*	ע�⣺
*/
int Get_Counter_Info()
{
	if (counter != NULL)
	{
		//������Ϣ�ṹ��������
		free(counter);
	}

	counter = (struct counter_info *)malloc(sizeof(struct counter_info));
	if (counter == NULL)
	{
		LogWrite(ERR, "%s", "counter malloc failed ");
		return DB_FAILURE;
	}
	memset(counter, 0, sizeof(struct counter_info));
	char  query_sql[1024] = "select * from smart_sales_counter.counter_info";

	if (mysql_query(mysql, query_sql))
	{
		finish_with_error(mysql);
		return DB_FAILURE;
	}

	MYSQL_RES * result;//��������ָ��

	result = mysql_store_result(mysql);//��ȡ���ִ�н��

	if (NULL == result)
	{
		finish_with_error(mysql);
		return DB_FAILURE;
	}

	int num_fields = mysql_num_fields(result);//��ȡ����ı����ж�����
	int num_row = mysql_num_rows(result);//��ȡ���������������Ӧ��ֻ��1��

	if (num_row > 1) 
	{ 
		//������Ӽ�¼��Ϣ����1������ֻ��ȡ��һ����������
		LogWrite(WARN, "%s", "counter info rows greater than 1 ");
		//printf("counter info rows greater than 1\r\n"); 
	}

	MYSQL_ROW row;//��������ĸ������õı���
	
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

		counter->boards_num = 0;//Ŀǰ��δʹ��
		counter->locker_stat = -1;
		counter->IsBusy = -1;
		break;//����ж�����¼���˴���ǿ���˳�while
		//printf("%s ", row[i] ? row[i] : "NULL");
	}
	mysql_free_result(result);//�ͷŽ����Դ
							  					  
	//db_close();

	return DB_SUCCESS;

}


/*
*	˵������ȡ���ذ���Ϣ�������ں����ڲ��������ṹ�ĳ�ʼ��
*	��������
*	ע�⣺
*/
int Get_Board_Info()
{
	if (board_info != NULL)
	{
		//������Ϣ�ṹ��������
		free(board_info);
	}

	board_info = (struct Board_Info *)malloc(sizeof(struct Board_Info));
	if (counter == NULL)
	{
		LogWrite(ERR, "%s", "board_info malloc failed ");
		return DB_FAILURE;
	}
	memset(board_info, 0, sizeof(struct Board_Info));
	struct Board_Info * board_info_rare = board_info;//��ʱ��βָ��

	char  query_sql[1024] = "select * from smart_sales_counter.board";

	if (mysql_query(mysql, query_sql))
	{
		finish_with_error(mysql);
		return DB_FAILURE;
	}

	MYSQL_RES * result;//��������ָ��

	result = mysql_store_result(mysql);//��ȡ���ִ�н��

	if (NULL == result)
	{
		finish_with_error(mysql);
		return DB_FAILURE;
	}

	int num_fields = mysql_num_fields(result);//��ȡ����ı����ж�����
	int num_row = mysql_num_rows(result);

	MYSQL_ROW row;//��������ĸ������õı���

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
			board_info_rare->board_items_weight_all = 0xFFFF;//��Чֵ
			board_info_rare->board_items_weight_all_after_close_door = 0;//��ֵÿ��ʹ�ú�����
			board_info_rare->IsBasicValue = 0;//δȥƤ
			board_info_rare->IsBasicValueSave = 0;//ȥƤδ����
			board_info_rare->ISNeedCur = ISNEEDCUR_NO;
			board_info_rare->ISCurvatureValue = 0;//У׼
			board_info_rare->ISCurvatureValueSave = 0;//У׼����
			memset(&board_info_rare->Curvature, 0, sizeof(board_info_rare->Curvature));

			first_node = 0;
		}
		else
		{
			board_info_rare->next = (struct Board_Info *)malloc(sizeof(struct Board_Info));
			memset(board_info_rare->next, 0, sizeof(struct Board_Info));
			if (board_info_rare->next != NULL)//�����ɹ����򲻻�����if����ڣ���ʧ����Ӧ�˳�����
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
				board_info_rare->board_items_weight_all = 0xFFFF;//��Чֵ
				board_info_rare->board_items_weight_all_after_close_door = 0;//��ֵÿ��ʹ�ú�����
				board_info_rare->IsBasicValue = 0;//δУ׼
				board_info_rare->IsBasicValueSave = 0;//ȥƤδ����
				board_info_rare->ISNeedCur = ISNEEDCUR_NO;
				board_info_rare->ISCurvatureValue = 0;//У׼
				board_info_rare->ISCurvatureValueSave = 0;//У׼����
				memset(&board_info_rare->Curvature, 0, sizeof(board_info_rare->Curvature));

			}

		}
	}
	board_info_rare->next = NULL;//���һ��Ԫ��ָ��ָ���
	mysql_free_result(result);//�ͷŽ����Դ
	
	//db_close();

	return DB_SUCCESS;

}

/*
*	˵������ȡ������Ʒ��Ϣ
*	��������
*	ע�⣺�������ÿ��ִ�ж������������ͬboard������item����Ȼ�����»�ȡ�����ǵ������ڵ���Ʒ�����������ޣ���˲�ȡÿ�θ���items���
*		  ����board��������items�������Ȼ�������½���Ҳ�ǿ��е�
*		  �����㷨Ϊ���������ȡitems���еĸ�����Ʒ�����ҽ�ÿһ����Ʒͬ���г��ذ���бȶԣ��ҵ�����ĳһ�����ذ�ı��
*		  Ȼ�󽫴���Ʒ�������ó��ذ����ݽṹ�е�items����ĺ���
*		 
*		���Ӱ��ӵ�items�����е���Ʒ���������ͱ���ͬ���ذ����������һ�µ��ж�
*		
*		*ÿһ��items��¼����һ�������������ֵ�Ǳ�ʾ������Ʒ����������������ذ�ļƼ�ģʽΪ1������Ŀǰ�� �߼�����Ϊ���������ǳ��ذ��������
*			������Ƽ�ģʽΪ2����ü�¼��������������������Ʒ�ģ������ܴ����ǳ��ذ��������������ڳ��ذ���Ϣ�����ݽṹ����һ���ǳ��ذ�������������
*			�������Ǳ����ǳ��ذ���������������Ѿ������˼Ƽ�ģʽΪ1����Ϊ2�����
*/
int Get_Item_Info()
{
	//board_info = (struct Board_Info *)malloc(sizeof(struct Board_Info));

	Free_Item_Info();//���ͷ����г��ذ������items�����ڴ�
	
	struct Board_Info * board_info_p = board_info;
	struct Items * items_p = NULL;

	char  query_sql[1024] = "select * from smart_sales_counter.items";

	if (mysql_query(mysql, query_sql))
	{
		finish_with_error(mysql);
	}

	MYSQL_RES * result;//��������ָ��

	result = mysql_store_result(mysql);//��ȡ���ִ�н��

	if (NULL == result)
	{
		finish_with_error(mysql);
	}

	//int num_fields = mysql_num_fields(result);//��ȡ����ı����ж�����
	//int num_row = mysql_num_rows(result);//��ȡ����

	MYSQL_ROW row;//��������ĸ������õı���

	//int first_node = 1;
	while ((row = mysql_fetch_row(result)))
	{
		//���ҵ������Ʒ�����ĳ��ذ壬�˴���δ���޷��ҵ�ƥ��ĳ��ذ�items���ݵĴ���
		while (board_info_p != NULL)
		{
			if ((strcmp(board_info_p->id, row[3]) == 0)  && (strcmp(board_info_p->type, row[6]) == 0) )//��itemsͬ���󶨵ĳ��ذ��ź��������ͱ���һ�²��ܰ����ó��ذ��items������
			{
				//����ﵽ�˴��������ҵ�Ŀ��ĳ��ذ�
				//��ʼѰ�Ҵ˳��ذ�items��������һ��Ԫ�أ��������
				items_p = board_info_p->items;
				if (items_p == NULL)
				{
					//���ذ������Ϣ����ṹ���׸��ڵ㣬����׸�Ԫ�غ�����whileѭ��
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

					break;//�˴��˳�����ĳһ�����ذ�items������ѯ��ѭ�����˳���ʼִ��items���ݿ���е���һ�����ݵĳ�ʼ��
				}
				else 
				{
					//���뵽��˴�����˳��ذ�items�������Ѿ������ݣ���Ҫ�ҵ����һ��
					while (items_p->next != NULL) { items_p = items_p->next; }
					//���뵽��˴������Ѿ��ҵ�item��������һ��Ԫ����
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

					break;//�˴��˳�����ĳһ�����ذ�items������ѯ��ѭ�����˳���ʼִ��items���ݿ���е���һ�����ݵĳ�ʼ��

				}
				
			}//��ѯĳһ����ذ��items����

			board_info_p = board_info_p->next;
		}
		board_info_p = board_info;//ÿ�ζ��Ǵ�ͷ��ʼѰ��
		
	}
	mysql_free_result(result);//�ͷŽ����Դ

	return DB_SUCCESS;

}


/*
*	˵�����ͷŻ�������
*	��������
*	ע�⣺
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
*	���ܣ�ͳ�ƹ��������г��ذ������еĻ�������
*	������
*	˵����#���ͳ����ָ�����ذ������е�item�����еĲ�Ʒ��������������ã����ҽ���ֵ���������Ϣ�����ݽṹ���У�
*		  #�������һ��Ҫ��ִ��Get_Item_Info()����֮��ſ���ִ�У�
*		  #��������ǽ������ݿ�����ĳһ�������������Ʒ������������ӣ���˸ú�������ʧ�ܣ��ݲ����쳣�ж�
*		  #��������Ѿ������˲�ͬ�Ƽ�ģʽ����������Ĳ���
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
			//������ģʽΪ���������ۣ���ÿһ�����ذ�ֻ�ܷ���һ����Ʒ�����ǰ��ո���������ÿһ�����ذ���Է��ò�ͬ���͵Ķ��ֻ���	
			weight_buf = weight_buf + CharNum_To_Double(items_p->weight_sum);//weight_sum���ֵӦ������Ʒ�ϼܵ�ʱ�����ϵͳ�����
			items_p = items_p->next;
		}
		board_info_p->board_items_weight_all = weight_buf;//��ֵȫ������
		board_info_p = board_info_p->next;
	}
	printf("���ذ��������ͳ�����!\r\n");

}


/*
*	���ܣ���ĳһ����ִ��updata����
*	������
*	˵���������³ɹ��򷵻���Ӱ������������Ϊ-1 ��Ϊsqlִ��ʧ��
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

	if (mysql_query(mysql, query_sql))//���ɹ�mysql_query��������0
	{
		finish_with_error(mysql);
	}

	return mysql_affected_rows(mysql);
}

/*
*	���ܣ���ĳһ����ִ��updata����
*	������
*	˵����#�����³ɹ��򷵻���Ӱ������������Ϊ-1 ��Ϊsqlִ��ʧ��
*		  #���µ��ֶ�ֵΪ���ֵģ����и�������Ϊ�ַ���������sql��䲻һ������Ϊ������value��ֵ�����''
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

	if (mysql_query(mysql, query_sql))//���ɹ�mysql_query��������0
	{
		finish_with_error(mysql);
	}

	return mysql_affected_rows(mysql);
}


/*
*	���ܣ���items��ִ�в�������
*	������
*	˵��������ֵΪ��Ӱ������������Ϊ-1 ��Ϊsqlִ��ʧ��
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

	if (mysql_query(mysql, query_sql))//���ɹ�mysql_query��������0
	{
		finish_with_error(mysql);
	}

	return mysql_affected_rows(mysql);

}

/*
*	���ܣ���up_messageд����ˮ������
*	������
*	˵��������mq�������������ݵ�ʱ�򣬷���֮ǰ��д�����ݿ⣬Ȼ���ٷ���
*/
INT64 SQL_INSERT_INTO_Up_Message(char * msn , char * message , time_t timep)
{
	char * query_sql = (char *)malloc(strlen(message) + 200);
	memset(query_sql, 0, strlen(message) + 200);
	char * buf = (char *)malloc(strlen(message) + 200);
	memset(buf, 0, strlen(message) + 200);
	//mysql��������ʹ�ú�����λ�ò���Ҫ���� '' ���
	sprintf(buf,"FROM_UNIXTIME(%d)", timep);
	//char  query_sql[1024] = "insert into smart_sales_counter.up_message ( msn , message , date ) values ( '";
	strcat(query_sql, "insert into smart_sales_counter.up_message ( msn , message , date ) values ( '");
	strcat(query_sql, msn);
	strcat(query_sql, "' , '");
	strcat(query_sql, message);
	strcat(query_sql, "' , ");
	//strcat(query_sql, "FROM_UNIXTIME(");
	//strcat(query_sql, ctime(&timep));
	strcat(query_sql, buf);
	strcat(query_sql, " ) ");
	//����ִ�еĽṹ���ܻ������ģ���˴˴���insert ���ͳһתΪGBK����
	memset(buf, 0, strlen(message) + 200);
	UTF8ToGBK(query_sql, buf, strlen(message) + 200);

	if (mysql_query(mysql, buf))//���ɹ�mysql_query��������0
	{
		finish_with_error(mysql);
	}
	
	int res = mysql_affected_rows(mysql);
		//if (mysql_affected_rows(mysql) <= 0)//sqlִ��ʧ�ܻ����ǲ���ʧ��
		//{
		//	return -1;
		//}
	free(query_sql);
	free(buf);
	return res;

}

/*
*	���ܣ����漰scheme������������ִ��һ���Բ��룬ִ��insert
*	������
*	˵��������ֵΪ��Ӱ������������Ϊ-1 ��Ϊsqlִ��ʧ��
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

	if (mysql_query(mysql, query_sql))//���ɹ�mysql_query��������0
	{
		finish_with_error(mysql);
	}

	if (mysql_affected_rows(mysql) <= 0)//sqlִ��ʧ�ܻ����ǲ���ʧ��
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

		if (mysql_query(mysql, query_sql))//���ɹ�mysql_query��������0
		{
			finish_with_error(mysql);
		}

		if (mysql_affected_rows(mysql) <= 0)//sqlִ��ʧ�ܻ����ǲ���ʧ��
		{
			return -1;
		}

		//���÷����ڵ�Ĳ�Ʒ��Ϣ�������ݿ�
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

			if (mysql_query(mysql, query_sql))//���ɹ�mysql_query��������0
			{
				finish_with_error(mysql);
			}

			if (mysql_affected_rows(mysql) <= 0)//sqlִ��ʧ�ܻ����ǲ���ʧ��
			{
				return -1;
			}

			scheme_node_items_list_node_p = scheme_node_items_list_node_p->next;
		}
		scheme_node_p = scheme_node_p->next;
	}

}

/*
*	���ܣ�ɾ��ĳһ�����ĳЩ���ݣ����ձ��ɾ��
*	������
*	˵��������ֵΪ��Ӱ������������Ϊ-1 ��Ϊsqlִ��ʧ��
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

	if (mysql_affected_rows(mysql) <= 0)//sqlִ��ʧ�ܻ�����ɾ��ʧ��
	{
		return -1;
	}
}


/*
*	���ܣ���ĳһ�����в�����ͬ��Ŀ
*	������
*	˵��������ֵΪ���ҵ��ļ�¼��Ŀ
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
	MYSQL_RES * result;//��������ָ��
	result = mysql_store_result(mysql);//��ȡ���ִ�н��
									   //INT64 i =  mysql_affected_rows(mysql);
	INT64 num_row = 0;
	if (result != NULL)
	{
		num_row = mysql_num_rows(result);//��ȡ���������������Ӧ��ֻ��1��
	}

	mysql_free_result(result);//�ͷŽ����Դ
	return num_row;

}

/*
*	���ܣ���up_message���л�ȡĳһ����¼��message����
*	������
*	˵��������ֵΪ���ҵ���message����ָ�룬��������Ҫ��������ʽ�ͷ���Դ
*/
char * Get_up_message_message(char * tablename, char * condition_name, char * condition_value)
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
	MYSQL_RES * result;//��������ָ��
	result = mysql_store_result(mysql);//��ȡ���ִ�н��

	INT64 num_row = 0;
	if (result != NULL)
	{
		num_row = mysql_num_rows(result);//��ȡ���������������Ӧ��ֻ��1��
		if (num_row == 0)
		{
			return NULL;
		}
		MYSQL_ROW row = mysql_fetch_row(result);//���۲�ѯ���˶����м�¼��ֻ����һ����¼����
		unsigned char * message_buf = (char *)malloc(strlen(row[2])+200);
		memset(message_buf, 0, strlen(row[2]) + 200);
		GBKToUTF8((unsigned char * )row[2], message_buf, strlen(row[2]) + 200);
		//strcpy(message_buf, row[2]);//��ȡmessage����
		mysql_free_result(result);//�ͷŽ����Դ

		return message_buf;
	}
	else
	{
		return NULL;
	}

}


/*
*	����: ִ��һ�����������͹�����sql select ���
*	������
*	˵��������ֵΪ��Ӱ������������Ϊ-1 ��Ϊsqlִ��ʧ��
*/

char * Procedure_SQL_Select(JSON_Object * json_object)
{
	
	//JSON_Value *root_value = json_value_init_object();//��ʼ����һ��value��������Ϊobject��valueΪjson���ݵ�һ�����ͣ���ͬ��value���Ϳ���Ƕ��
	//JSON_Object *root_object = json_value_get_object(root_value);//��ȡvalue�е�object�ĵ�ַ

	JSON_Value *sub_value = json_value_init_array();
	JSON_Array *sub_array = json_value_get_array(sub_value);

	//time_t timep;
	//time(&timep);
	//json_object_set_string(root_object, "devid", counter->sn);
	//json_object_set_string(root_object, "cmdid", "SQL_Select");
	//json_object_dotset_string(root_object, "Result.Res", "0");//���������ǳɹ�
	//json_object_dotset_string(root_object, "Result.Date", ctime(&timep));

	char * sqlstr = json_object_get_string(json_object, "SqlStr");
	if (sqlstr != NULL)
	{

		//����ִ�е�sql�����������ģ����ͳһת����Ϊgbk����ִ��
		unsigned char sql_gbk[500] = { 0 };
		UTF8ToGBK(sqlstr, sql_gbk, 500);
		LogWrite(INFO, "%s", sql_gbk);
		if (mysql_query(mysql, sql_gbk))
		{
			finish_with_error(mysql);
		}

		MYSQL_RES * result;//��������ָ��

		result = mysql_store_result(mysql);//��ȡ���ִ�н��

		if (NULL == result)
		{
			finish_with_error(mysql);
		}

		int num_fields = mysql_num_fields(result);//��ȡ����ı����ж�����
		int num_row = mysql_num_rows(result);//��ȡ���������������Ӧ��ֻ��1��

		//if (num_row > 1) { printf("counter info rows greater than 1\r\n"); }//counter_info ���ű�ļ�¼������Ӧ�ö���һ����������Ŀǰ�߼���Ӧ��ֻ�������һ�еļ�¼


		JSON_Value *sub_sub_value[1024];//�˴�����һ���󻺴�
		JSON_Array *sub_sub_array = NULL;
		int sub_sub_value_pos = 0;//ʹ�õ�value��λ��
		MYSQL_ROW row;//��������ĸ������õı���

		char utf8_buf[200] = { 0 };
		while ((row = mysql_fetch_row(result)))
		{
			sub_sub_value[sub_sub_value_pos++] = json_value_init_array();//
			sub_sub_array = json_value_get_array(sub_sub_value[sub_sub_value_pos - 1]);
			for (int i = 0; i < num_fields; i++)//����������
			{
				//������Ϊ���� ����Ӧ��ΪGBK����޷����������json����Ҫת��
				if (is_valid_utf8(row[i] != NULL ? row[i] : NULL, strlen(row[i])) == 0)//��Ϊ0����������鲻��utf8����
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

		return Procedure_Answer_Message(json_object_get_string(json_object, "MSN"), "SQL_Select", 0, sub_value);//�������������ʧ�ܣ����res�̶�Ϊ0

		//json_object_dotset_value(root_object, "Result.Data", sub_value);
		//��json�ַ����л������������һ���ַ����У��������ظ��ַ�����
		//char * json_string = (char *)malloc(json_serialization_size(root_value));//json_serialization_size��������ķ���ֵ�Ѿ������˽�ԭ�ַ������Ƚ�����չ����Ϊ��Ҫ����һЩ������ַ����绻�С���б�ܡ������ŵ�
		//json_serialize_to_buffer(root_value, json_string, json_serialization_size(root_value));
		//�ͷ�json��Դ
		//json_value_free(root_value);//ֻ���ͷŸ���Դ�����ڲ�������������Դ�ᱻ�ݹ��ͷ�

		//mysql_free_result(result);//�ͷŽ����Դ

		//return json_string;//��ָ������ź���������ʧЧ����ָ��ĸ��ڴ�������ⲿ��ʽ�ͷ�



	}


}

/*
*	����: ִ��һ�����������͹�����sql �ĸ��� ���
*	������
*	˵����ע�˴�ָ��updata��ָ���µ���˼��������sql updata ��䣬�ú������������ɾ���������͵�sql���
*/

char * Procedure_SQL_Updata(JSON_Object * json_object)
{

	//JSON_Value *root_value = json_value_init_object();//��ʼ����һ��value��������Ϊobject��valueΪjson���ݵ�һ�����ͣ���ͬ��value���Ϳ���Ƕ��
	//JSON_Object *root_object = json_value_get_object(root_value);//��ȡvalue�е�object�ĵ�ַ

	JSON_Value *sub_value = json_value_init_array();
	JSON_Array *sub_array = json_value_get_array(sub_value);

	//time_t timep;
	//time(&timep);
	//json_object_set_string(root_object, "devid", counter->sn);
	//json_object_set_string(root_object, "cmdid", "SQL_Updata");
	//json_object_dotset_string(root_object, "Result.Res", "0");//���������ǳɹ�,������ֻҪƴдû�д����һ��ִ�гɹ������Ǹ�����Ľ����һ���ɹ���
	//json_object_dotset_string(root_object, "Result.Date", ctime(&timep));

	char * sqlstr = json_object_get_string(json_object, "SqlStr");
	if (sqlstr != NULL)
	{
		//����ִ�е�sql�����������ģ����ͳһת����Ϊgbk����ִ��
		unsigned char sql_gbk[500] = { 0 };
		UTF8ToGBK(sqlstr, sql_gbk, 500);
		LogWrite(INFO, "%s", sql_gbk);
		if (mysql_query(mysql, sql_gbk))
		{
			finish_with_error(mysql);
		}

		INT64 affected_rows_num = mysql_affected_rows(mysql);

		unsigned char num_buf[100] = { 0 };
		Int_To_CharArray(affected_rows_num, num_buf);//ע�����ת����������Ӱ��Ľ���Ƚ϶࣬����int�ܱ������ޣ��˴����ܻ�������
		json_array_append_string(sub_array, num_buf);
		return Procedure_Answer_Message(json_object_get_string(json_object, "MSN"), "SQL_Updata", 0, sub_value);//�������������ʧ�ܣ����res�̶�Ϊ0

		//json_object_dotset_string(root_object, "Result.AffRows", num_buf);
		//��json�ַ����л������������һ���ַ����У��������ظ��ַ�����
		//char * json_string = (char *)malloc(json_serialization_size(root_value));//json_serialization_size��������ķ���ֵ�Ѿ������˽�ԭ�ַ������Ƚ�����չ����Ϊ��Ҫ����һЩ������ַ����绻�С���б�ܡ������ŵ�
		//json_serialize_to_buffer(root_value, json_string, json_serialization_size(root_value));
		//�ͷ�json��Դ
		//json_value_free(root_value);//ֻ���ͷŸ���Դ�����ڲ�������������Դ�ᱻ�ݹ��ͷ�

		//return json_string;//��ָ������ź���������ʧЧ����ָ��ĸ��ڴ�������ⲿ��ʽ�ͷ�

	}


}

/*
*	����: ��������ķ�����ź����������ҽ��
*	������
			[in]scheme_id �������
			[in]weight_buf �Ƶõ������仯����������
			[out]weight_buf_fix ���ݷ����������õ�Ӧ�øó��ذ��ȥ���������������weight_bufΪ����ֵ��weight_buf_fixΪ׼ȷû������ֵ����ֵΪ����ֵ
			[out]sub_sub_value ���ص�json�����ֵ����ֵΪһ��json_value ���飬ÿһ��Ԫ�ض���һ�����飬��Ӧһ��item��Ϣ
			[out]��������json������Ӽ�����
*	˵����ע�˴�ָ��updata��ָ���µ���˼��������sql updata ��䣬�ú������������ɾ���������͵�sql���
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
	MYSQL_RES * result;//��������ָ��
	result = mysql_store_result(mysql);//��ȡ���ִ�н��
									   //INT64 i =  mysql_affected_rows(mysql);
	int num_row = 0;
	if (result == NULL)
	{
		finish_with_error(mysql);
	}
	else
	{
		num_row = mysql_num_rows(result);//��ȡ���������������Ӧ��ֻ��1��
		mysql_free_result(result);//�ͷŽ����Դ
		if (num_row == 0)
		{
			return 1;//������Ų�����
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

	result = mysql_store_result(mysql);//��ȡ���ִ�н��
									   //INT64 i =  mysql_affected_rows(mysql);
	num_row = 0;
	if (result == NULL)
	{
		finish_with_error(mysql);
	}
	else
	{
		num_row = mysql_num_rows(result);//��ȡ���������������Ӧ��ֻ��1��
										 //mysql_free_result(result);//�ͷŽ����Դ
		if (num_row != 1)
		{
			mysql_free_result(result);//�ͷŽ����Դ
			return 2;//�������䲻���ڣ���û�����ڸ�����ķ����ڵ�
		}
		else
		{
			//ֻ��һ����¼��ʱ��
			MYSQL_ROW row;
			row = mysql_fetch_row(result);//��Ϊֻ��һ�У����Դ˴�ֱ�ӻ�ȡ�������
			//���˽ڵ��������ֵ���Ƶõ�����������ֵ
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
			mysql_free_result(result);//�ͷŽ����Դ
			result = mysql_store_result(mysql);//��ȡ���ִ�н��

			int num_fields = mysql_num_fields(result);//��ȡ����ı����ж�����

			JSON_Array *sub_sub_array = NULL;
			char utf8_buf[200] = { 0 };
			int sub_sub_value_pos = 0;//ʹ�õ�value��λ��
			while ((row = mysql_fetch_row(result)))
			{
				sub_sub_value[sub_sub_value_pos++] = json_value_init_array();//
				sub_sub_array = json_value_get_array(sub_sub_value[sub_sub_value_pos - 1]);
				for (int i = 3; i < num_fields; i++)//����������
				{
					memset(utf8_buf, 0, 200);
					//������Ϊ���� ����Ӧ��ΪGBK����޷����������json����Ҫת��
					if (is_valid_utf8(row[i] != NULL ? row[i] : NULL, strlen(row[i])) == 0)//��Ϊ0����������鲻��utf8����
					{
						if (i == 5 && weight_buf < 0)//i = 5 Ϊ������Ʒһ���м���������
						{
							//����Ӧ����Զ�������������֣���Ϊ���ֲ���GBK����
							Int_To_CharArray(0 - CharNum_To_Double(row[i]) , change_code);//����Ʒ���ߵ���������һ�����ţ��Ա�ʾ��ȡ�߻���˷�ʽֻ��P&P�����£�������ģʽ�²�һ����Ҫ������ȷ��
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
						if (i == 5 && weight_buf < 0)//i = 5 Ϊ������Ʒһ���м���������
						{
							Int_To_CharArray(0 - CharNum_To_Double(row[i]), change_code);//����Ʒ���ߵ���������һ�����ţ��Ա�ʾ��ȡ�߻���˷�ʽֻ��P&P�����£�������ģʽ�²�һ����Ҫ������ȷ��
							json_array_append_string(sub_sub_array, change_code != NULL ? change_code : NULL);
						}
						else
						{
							GBKToUTF8(row[i], utf8_buf, 200);
							json_array_append_string(sub_sub_array, row[i] != NULL ? row[i] : NULL);
						}
						
					}
					//ÿ��������һ����Ʒ��Ϣ������Ϻ�ͳһ���¸ò�Ʒ����Ӧ��items�ı���е����ݣ�������Ʒ�����͸�����Ʒ������
					if (i == num_fields - 1)//����ÿһ��items�Ŀ�������ʹ��ֻ������������ע��Ƽ�ģʽΪ2��ʱ�򣬲���Ҫ���³��ذ�����������Ҫ����ÿһ����Ʒ�Ŀ����
					{
						//�򷵻����������ĳһ����Ʒ��Ϣ�󣬶�������һ���ܼ۵���Ϣֵ����������һ��Ԫ��
						Double_To_CharArray(CharNum_To_Double(row[5]) * CharNum_To_Double(row[6]), change_code);
						json_array_append_string(sub_sub_array, change_code);
						//Ҫ����weight_buf�ķ����жϻ����Ƿ��뻹��ȡ��
						Int_To_CharArray(*weight_buf_fix, change_code);
						if (weight_buf > 0)
						{
							//Ϊ����
							memset(utf8_buf, 0, 200);
							strcpy(utf8_buf, " ind_count + ");//�˴�ʹ��utf8_buf������Ϊ�˲��������µĻ�����
							strcat(utf8_buf, row[5]);
							strcat(utf8_buf, "  ");
							//ע������sql����÷�ʵ��������ֱ����ԭֵ�Ļ���֮�Ͻ�������
							SQL_UPDATA2("smart_sales_counter.items", "ind_count", utf8_buf, "item_id", row[3]);

							memset(utf8_buf, 0, 200);
							strcpy(utf8_buf, " weight_sum + ");//�˴�ʹ��utf8_buf������Ϊ�˲��������µĻ�����
							strcat(utf8_buf, change_code);
							strcat(utf8_buf, "  ");
							SQL_UPDATA2("smart_sales_counter.items", "weight_sum", utf8_buf, "item_id", row[3]);

						}
						else
						{
							//Ϊȡ��
							memset(utf8_buf, 0, 200);
							strcpy(utf8_buf, " ind_count - ");//�˴�ʹ��utf8_buf������Ϊ�˲��������µĻ�����
							strcat(utf8_buf, row[5]);
							strcat(utf8_buf, "  ");
							//ע������sql����÷�ʵ��������ֱ����ԭֵ�Ļ���֮�Ͻ�������
							SQL_UPDATA2("smart_sales_counter.items", "ind_count", utf8_buf, "item_id", row[3]);

							memset(utf8_buf, 0, 200);
							strcpy(utf8_buf, " weight_sum - ");//�˴�ʹ��utf8_buf������Ϊ�˲��������µĻ�����
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
*	���ܣ������ݿ��ȡָ������ŵ�scheme
*	������
*	˵�����ú��������ݷ�����ų�ʼ��һ��������������������ķ���������
		  �ú������漰������������ݻ�ȡ
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
	if (mysql_affected_rows(mysql) <= 0)//sqlִ��ʧ�ܻ����ǲ�ѯû����ط����ı��
	{
		return -1;
	}
	*/

	//��ȫ�ַ��������в��Ҵ������ݿ��ȡ�ķ����Ƿ��Ѿ������ڴ���
	for (int j = 0; j < MAX_SCHEMES; j++)
	{
		if (scheme_globle[j].Isused && strcmp(scheme_globle[j].scheme_id, scheme_id) == 0)
		{
			//�����Ѿ�������
			return;
		}
	}

	int i = 0;
	for (; i < MAX_SCHEMES; i++)
	{
		if (scheme_globle[i].Isused == 0 )
		{
			//��ʼ��ȡ�����е�ָ����ŵķ�������
			MYSQL_RES * result;//��������ָ��

			result = mysql_store_result(mysql);//��ȡ���ִ�н������ʱ�Ѿ���scheme��ִ���˲�ѯ�����

			if (NULL == result)
			{
				finish_with_error(mysql);
			}

			int num_row = mysql_num_rows(result);//��ȡ���������������Ӧ��ֻ��1��

			if (num_row > 1) { sys_die("rows greater than 1\r\n"); }//Ӧ��ֻ�ܲ�ѯ��һ����¼

			MYSQL_ROW row;//��������ĸ������õı���

			while ((row = mysql_fetch_row(result)))//�˴�Ӧ��ֻѭ��һ��
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
			mysql_free_result(result);//�ͷŽ����Դ
			scheme_globle[i].Isused == 1;//���ýڵ��ʶλ�Ѿ�ʹ��

			//��ʼ���ĳһ�������еĸ����ڵ�
			memset(query_sql, 0, 1024);
			strcat(query_sql, "select * from smart_sales_counter.scheme_node where scheme_id = '");
			strcat(query_sql, scheme_id);
			strcat(query_sql, "'");

			if (mysql_query(mysql, query_sql))
			{
				finish_with_error(mysql);
			}

			if (mysql_affected_rows(mysql) <= 0)//sqlִ��ʧ�ܻ����ǲ�ѯû����ط����ı��
			{
				return -1;
			}

			result = mysql_store_result(mysql);//��ȡ���ִ�н������ʱ�Ѿ���scheme_node��ִ���˲�ѯ�����

			if (NULL == result)
			{
				finish_with_error(mysql);
			}

			num_row = mysql_num_rows(result);//��ȡ���������������Ӧ��ֻ��1��

			//if (num_row > 1) { sys_die("rows greater than 1\r\n"); }//Ӧ��ֻ�ܲ�ѯ��һ����¼

			struct scheme_node * node_p = NULL;
			int head_time = 1;//Ϊʹ���׸�Ԫ�ض��趨�ı���
			while ((row = mysql_fetch_row(result)))//�˴�Ӧ��ֻѭ��һ��
			{
				if (head_time == 1)//Ϊ�׸��ڵ�
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
				//ע�˴���δ�Ը�Ԫ�ظ�ֵ�������ø�Ԫ��ָ����schmem��id��ַ������������Ϊ���ݿ��ѯ�����ķ����ڵ�������һ���Ǹ÷�����ŵ�
				//�������Ը�ֵ���и�ֵ���������ͷ���Դ��ʱ��Ҳ��Ϊ����
				node_p->scheme_id  = scheme_globle[i].scheme_id;
				node_p->schemem_node_weight = CharNum_To_Double(row[2]);
				node_p->schemem_node_price = CharNum_To_Double(row[3]);
				node_p->items_list_node = NULL;
				
			}
			mysql_free_result(result);//�ͷŽ����Դ

			//��ʼ�����������ڵ��е�items_list�ڵ���Ϣ
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

				if (mysql_affected_rows(mysql) <= 0)//sqlִ��ʧ�ܻ����ǲ�ѯû����ط����ı��
				{
					return -1;
				}

				result = mysql_store_result(mysql);//��ȡ���ִ�н������ʱ�Ѿ���scheme_node��ִ���˲�ѯ�����

				if (NULL == result)
				{
					finish_with_error(mysql);
				}
				struct scheme_node_items_list_node * scheme_node_items_list_node_p = NULL;
				head_time = 1;
				while ((row = mysql_fetch_row(result)))//�˴�Ӧ��ֻѭ��һ��
				{
					if (head_time == 1)//Ϊ�׸��ڵ�
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
					scheme_node_items_list_node_p->schemem_node_id = CharNum_To_Double(row[1]);//�ñ�ž��Ǳ���ѭ���Ĵ���
					scheme_node_items_list_node_p->scheme_id = scheme_globle[i].scheme_id;
					scheme_node_items_list_node_p->item_id = (char *)malloc(strlen(row[3]) + 1);
					strcpy(scheme_node_items_list_node_p->item_id, row[3]);
					scheme_node_items_list_node_p->item_name = (char *)malloc(strlen(row[4]) + 1);
					strcpy(scheme_node_items_list_node_p->item_name, row[4]);
					scheme_node_items_list_node_p->item_num = CharNum_To_Double(row[5]);


				}

				node_p = node_p->next;//�����ڵ�����һλ
			}


			break;//�Ƴ�Ѱ�ҿ��з���λ��ѭ��
		}
		


	}//��������ѭ������
	if (i == MAX_SCHEMES)
	{
		//���뵽��˴���������������涼����
	}

	
}


/*
*	���ܣ�ɾ��ĳһ�����������ݿ�����
*	������[in]ɾ���������
*	˵�����ú���Ӧ�ò��ᱻ�ⲿֱ�ӵ���
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

	if (mysql_affected_rows(mysql) <= 0)//sqlִ��ʧ�ܻ�����ɾ��ʧ��
	{
		return -1;
	}
}