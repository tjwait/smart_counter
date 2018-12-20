
#ifndef _database_fun_h
#define _database_fun_h

#ifdef	__cplusplus
extern "C" {
#endif

#include <string.h>
#include <stdio.h>
#include <mysql.h>
#include <assert.h>
#include "global.h"
#include "sys.h"
#include "scheme.h"
#include "parson.h"

#define DB_SUCCESS 0
#define DB_FAILURE 1



	struct counter_info
	{
		char * sn;
		char * name;
		char * info;
		char * installation_date;
		char * manger;
		char * server_ip;
		char * server_port;
		char * exchange_name;
		char * queue_name;
		char * routingkey;
		char * com_port;
		char * com_port_T;
		char * channel;
		char * mq_name;
		char * mq_pw;
		char * locker_id;
		char * max_kind;
		char * max_buy;
		char * error_value;
		char locker_stat;//��״̬����ֵΪ��ϵͳ��������ϵͳ��ã�Ŀǰ��δ�洢�����ݿ���,����0ֵ��ʹ�ã���˴����ݵ�Ĭ��ֵΪ-1
		char IsBusy;//�˴�ʵ������һ���̻߳���������ֵΪ-1 ����ӿ���ִ���κδ����������ֵΪ����0�Ĵ������ڴ��ڹ����У��޷�Ӧ������
					//���ֵĿǰû�����ã��Ƿ��б�Ҫʹ�û���Ҫ���о�
		UINT8 boards_num;//�������ذ�����,�������Ŀǰû��ʹ��
	};

#define BOARD_STAT_UNKNOW 0//״̬δ֪�������������Ӳ���֮ǰ�����״̬
#define BOARD_STAT_OK	1//��������
#define BOARD_STAT_ERROR	2//״̬�쳣

#define ISNEEDCUR_NO 0
#define ISNEEDCUR_YES 1

	struct Board_Info
	{
		unsigned char * id;//���ذ��ַ
		char * name;//���ƣ���ֵ����Ϊ�����ݲ�ʹ��
		char * type;//�˳��ذ�Ƽ�����
		char * scheme_id;//���Ƽ�ģʽΪ2ʱ���󶨵ķ������
		struct Board_Info * next;
		int Board_Stat;//�����������
		struct Items * items;
		UINT16 board_items_weight_all;//�ò���ذ������л����������,�����ֽ��㹻�����Ҳ��ܳ��ָ�ֵ
		UINT16 board_items_weight_all_after_close_door;//ÿ�����ۻ����ʱ����ź�������ذ��ϵ�����
		UINT8 IsBasicValue;//�ж��ó��ذ��Ƿ��Ѿ�ȥƤ���
		UINT8 IsBasicValueSave;//ȥƤ�Ƿ�ɹ�����
		UINT8 ISNeedCur;//�Ƿ���ҪУ׼
		UINT8 ISCurvatureValue;//�ж�У׼�Ƿ�ɹ�
		UINT8 ISCurvatureValueSave;//�ж�У׼�Ƿ񱣴�ɹ�
		union {
			float f;
			unsigned char x[4];
		} Curvature;
		//float Curvature;
	};

	//���۵���Ʒ
	struct Items 
	{
		char * item_id;
		char * barcode;//��Ʒ��ͨ�����룬δ���ƻ������̳����������ʹ�ã���Ԥ��
		unsigned char * board_id;
		char * name;
		char * supplier;
		char * saletype;
		char * ind_count;
		char * ind_weight;
		char * ind_price;
		char * weight_sum;
		char * weight_price;
		struct Items * next;
	};

	extern struct counter_info * counter;
	extern struct Board_Info * board_info;//���ذ���Ϣͷָ��

	//���ݿ��ʼ��������ʹ��֮ǰ�ȵ��øú���
	int init_db();
	//��ȡ���ӵ����ݿ���Ϣ
	int Get_Counter_Info();
	//��ȡ��������ĳ��ذ���Ϣ
	int Get_Board_Info();
	//��ȡ�����������Ʒ��Ϣ������������Ʒͬ���ذ�����������ӣ�����ͬ�ĳ��ذ��ڵ�items����ĳ�ʼ��
	int Get_Item_Info();
	//�ͷ�items�����ڴ�
	void Free_Item_Info();
	//�������ݿ���Ϣ��ȡ�������ذ���Ʒ����
	void Get_Boards_Items_Weight();
	//��ȡ������Ϣ
	void Get_Scheme(char * scheme_id);
	//ɾ��ָ����ŷ��������ݿ�����
	void Free_Scheme_DB(char * scheme_id);
	//�ر����ݿ�
	int db_close();


	//SQL��غ���
	INT64 SQL_SELECT(char * tablename, char * condition_name, char * condition_value);
	//���µ�ֵΪ�ַ�����
	INT64 SQL_UPDATA1(char * tablename, char * col_name1, char * col_value1, char * condition_name, char * condition_value);
	//���µ�ֵΪ���ֵģ�ע������Ҳ��ͨ���ַ�������ģ�ֻ��sql��䲻һ��
	INT64 SQL_UPDATA2(char * tablename, char * col_name1, char * col_value1, char * condition_name, char * condition_value);

	INT64 SQL_INSERT_INTO_ItemsTable(char * item_id, char * barcode, char * board_id, char * name, char * supplier, char * saletype, \
		char * ind_count, char * ind_weight, char * ind_price, char * weight_sum, char * weight_price);
	INT64 SQL_INSERT_INTO_Scheme(struct scheme * scheme_p);
	INT64 SQL_DELETE(char * tablename, char * condition_name, char * condition_value);

	INT64 SQL_INSERT_INTO_Up_Message(char * msn, char * message, time_t timep);


	char * Procedure_SQL_Select(JSON_Object * json_object);
	char * Procedure_SQL_Updata(JSON_Object * json_object);

	int Scheme_Parse(char * scheme_id, double weight_buf, double * weight_buf_fix, double * price, JSON_Value **sub_sub_value);

#ifdef	__cplusplus
}
#endif

#endif /* _database_fun_h */
