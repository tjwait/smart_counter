
#include "sys.h"
#include "math.h"

#include "stdio.h"
#include "windows.h"

struct Sys_Tem sys_tem;

/*
*	���ܣ�ϵͳ������ʼ��
*	������
*	˵����ϵͳ������ʼ��������������Ϊ
			1�����ݿ����ӳ�ʼ��
			2����ȡ���ӻ�����Ϣ����������ṹ����
			3����ȡ���ذ������Ϣ�����������Ӧ�ṹ����
			4���������ӳ�ʼ��
			5�����Գ��ذ��Ƿ���������
			6�����Թ������ӷ�������·�Ƿ�����

*/
void Init_System(void)
{
	LogWrite(INFO, "%s", "Init DataBase Start!");
	int recon = 0;
	while ( (init_db() == DB_FAILURE) && (recon < 4) )
	{
		recon++;
	}
	if (recon >= 4)
	{
		LogWrite(ERR, "%s", "Init DataBase Failed!");
		exit(-1);//���ݿ�����ʧ��ʱ���ش������ֱ���˳�����
	}
	else
	{
		LogWrite(INFO, "%s", "Init DataBase SUCCESS!");
	}
	//init_db();

	LogWrite(INFO, "%s", "Get Counter Info Start");
	recon = 0;
	while ((Get_Counter_Info() == DB_FAILURE) && (recon < 4))
	{
		recon++;
	}
	if (recon >= 4)
	{
		LogWrite(ERR, "%s", "Init Counter Failed!");
		exit(-1);//��ȡ������Ϣ����Ϊ���ش������ֱ���˳�����
	}
	else
	{
		LogWrite(INFO, "%s", "Init Counter SUCCESS!");
	}

	LogWrite(INFO, "%s", "Get Board Info Start");
	recon = 0;
	while ((Get_Board_Info() == DB_FAILURE) && (recon < 4))
	{
		recon++;
	}
	if (recon >= 4)
	{
		LogWrite(ERR, "%s", "Init Board Failed!");
		exit(-1);//��ȡ���ذ���Ϣ����Ϊ���ش������ֱ���˳�����
	}
	else
	{
		LogWrite(INFO, "%s", "Init Board SUCCESS!");
	}

	LogWrite(INFO, "%s", "Init Serial Start");
	recon = 0;
	init_serial_port( &hCom_C, counter->com_port, 9600);
	init_serial_port(& hCom_Tem, counter->com_port_T, 9600);
}



/*
*	���ܣ�ϵͳ�쳣���
*	������
*	˵����
*/
//���������һ���ɱ������������Ҫ������־������ڴ˷���һ��
void sys_die(const char *fmt, ...) {
	va_list ap;//�����һ��char * 
	va_start(ap, fmt);//�˺��ǽ�apָ��ʡ�Ժ�"..."ǰ�����һ�������ĵ�ַ��������Ϊfmt
	vfprintf(stderr, fmt, ap);//�˺����ǽ�fmt�������ap����Ҫ���͵�stderr���У�stderr��Ϊϵͳ������쳣�������fmt�������Ϊһ���ַ�����ap������Ϊ��
							  //��fmt�ַ����и�ʽ������Ĳ������硰aa%d��������ap��Ҫ��%d����Ӧ��ֵ�ģ��������Ϊap�ڵ���va_startʱָ����fmt������ʵ��
							  //ʹ��ʱ�Ǵӵڶ���Ԫ�ؿ�ʼʹ�õ�
	va_end(ap);//�ͷ�ap��Դ
	fprintf(stderr, "\n");//���stderr����Ϣ
	exit(1);//�˳�ϵͳ
}

/*
*	���ܣ����������ӵĳ��ذ��Ƿ�����
*	������
*	˵�������쳣���˳�����

*/
void Board_Ready(void)
{
	struct Board_Info * p = board_info;
	//�ڷ�������ָ��ǰ�Ƚ�����״̬������Ϊunknow״̬
	while (p != NULL)
	{
		p->Board_Stat = BOARD_STAT_UNKNOW;

		p = p->next;
	}
	p = board_info;
	char send_buf[6];
	send_buf[0] = 0x24;
	send_buf[1] = 0x00;//���ֵ�ǵ�ַλ�ߣ�һ����ѭ����Ҫ����
	send_buf[2] = 0x00;//���ֵ�ǵ�ַλ�ͣ�һ����ѭ����Ҫ����
	send_buf[3] = 0x06;
	send_buf[5] = 0x23;
	while (p != NULL)
	{
		send_buf[1] = ((ASCII_To_byte(p->id[0]) << 4) & 0xf0) + ASCII_To_byte(p->id[1]);
		send_buf[2] = ((ASCII_To_byte(p->id[2]) << 4) & 0xf0) + ASCII_To_byte(p->id[3]);
		send_buf[4] = Sum_Check(&send_buf[1], 3);
		WriteChar(hCom_C, send_buf, 6);
		Sleep(1000);

		p = p->next;
	}
	
	//Sleep(5000);//��������ȫ��������Ϻ󣬵ȴ�����ʱ���Ƿ���ʻ���ȷ��
	//��ʱ�ж����г��ذ��״̬����Ӧ����δ���ӵ����
	p = board_info;
	while (p != NULL)
	{
		if (p->Board_Stat != BOARD_STAT_OK)
		{
			//���ذ������쳣
			sys_die("Board Ready");
		}
		printf("���ذ� %s ����������\r\n" , p->id);
		p = p->next;
	}
	printf("���ذ����Ӽ��������\r\n");

}

/*
*	���ܣ�ȥƤУ׼���ұ���
*	������[in] ȥƤ����
*	˵���������ڵ���ʱ��board_info�ṹ���е�IsBasicVale�趨Ϊ0����ΪδȥƤ��Ȼ��ִ��ȥƤ�����ȴ�һ��ʱ����ж��Ƿ�ȥƤ�ɹ�
*/
int Board_Basic_Value_Set(int times)
{
	struct Board_Info * p = board_info;
	int boards_num = 0;
	printf("�ȴ�ȥƤ���");
	while (p != NULL)
	{
		p->IsBasicValue = 0;//�����Ƿ��Ѿ�ȥ��Ƥ���趨ΪδȥƤ״̬
		p->IsBasicValueSave = 0;//ȥƤδ����
		for (int i = 0; i < times; i++)
		{
			printf("...");
			Send_CMD(&hCom_C, p->id, BOARD_CMD_BASIC_VALUE, NULL, 0, 1 , 0);
			Sleep(5000);
		}
		printf("...");
		Send_CMD(&hCom_C, p->id, BOARD_CMD_BASIC_VALUE_SAVE, NULL, 0, 1 , 0);
		Sleep(1000);
		if (p->IsBasicValue == 0 || p->IsBasicValueSave == 0)
		{
			//ȥƤʧ��
			printf("\r\nȥƤʧ�ܣ���\r\n");
			return 1;
		}

		p = p->next;
	}
	printf("  ȥƤ���ұ���ɹ���\r\n");
	return 0;

}

/*
*	���ܣ�ȥƤУ׼���ұ���,�������������json����
*	������[in] ȥƤ����
*	˵���������ڵ���ʱ��board_info�ṹ���е�IsBasicVale�趨Ϊ0����ΪδȥƤ��Ȼ��ִ��ȥƤ�����ȴ�һ��ʱ����ж��Ƿ�ȥƤ�ɹ�
*			����У׼��������ֵ���жϣ���Ϊ0�����ɹ�����Ϊ1�����ʧ��
*		  #ע��������λ�����͹�����У׼�����������г��ذ�ִ��У׼�����˸ú����ķ���ֵӦ�������еĳ��ذ��Ƿ��Ѿ�ȥƤ
*			������Ӧ������������һ�����ذ�ȥƤʧ�ܣ�������ʧ�ܣ�������data���з���ÿһ�����ذ�ȥƤ�Ľ��
*/
char * Board_Basic_Value_Set_With_ACK()
{
	char change_code[100];
	int return_value = Board_Basic_Value_Set(2);//ȥƤ�Ĵ������������δ���⿪��
	JSON_Value *root_value = json_value_init_object();//��ʼ����һ��value��������Ϊobject��valueΪjson���ݵ�һ�����ͣ���ͬ��value���Ϳ���Ƕ��
	JSON_Object *root_object = json_value_get_object(root_value);//��ȡvalue�е�object�ĵ�ַ

	JSON_Value *sub_value = json_value_init_array();
	JSON_Array *sub_array = json_value_get_array(sub_value);

	time_t timep;
	time(&timep);
	//printf("%s", ctime(&timep));
	json_object_set_string(root_object, "devid", counter->sn);
	json_object_set_string(root_object, "cmdid", "Basic_Value");
	Int_To_CharArray(return_value, change_code);
	json_object_dotset_string(root_object, "Result.Res", change_code);
	json_object_dotset_string(root_object, "Result.Date", ctime(&timep));
	//json_object_set_string(root_object, "Counter-SN", counter->sn);
	json_object_dotset_value(root_object, "Result.Data", "NULL");//û�з�������
	char * json_string = (char *)malloc(json_serialization_size(root_value));//json_serialization_size��������ķ���ֵ�Ѿ������˽�ԭ�ַ������Ƚ�����չ����Ϊ��Ҫ����һЩ������ַ����绻�С���б�ܡ������ŵ�
	json_serialize_to_buffer(root_value, json_string, json_serialization_size(root_value));
	//�ͷ�json��Դ
	json_value_free(root_value);//ֻ���ͷŸ���Դ�����ڲ�������������Դ�ᱻ�ݹ��ͷ�

	return json_string;//��ָ������ź���������ʧЧ����ָ��ĸ��ڴ�������ⲿ��ʽ�ͷ�

}


/*
*	���ܣ�����У׼
*	������[in] У׼����
		  [in] �Ƿ񱣴�����ֵ 0 ������  1����
*	˵����Ŀǰδ�Է���ֵ�����κδ���
*
*/
void Board_Curavture_Value_Set( int times , int save)
{
	
	struct Board_Info * p = board_info;
	
	if (!sys_tem.IsCheck)
	{
		//�¶�û�б仯����У׼
		printf("���һ�β����¶�Ϊ %d ���϶� �� ϵͳ��ʼ�¶�Ϊ %d ���϶ȣ�����У׼\r\n", sys_tem.Tem_Cur, sys_tem.Tem);
		return;
	}
	else
	{
		//�¶ȱ仯��ʱ������У׼Ҫ���Ƚ��¶Ƚṹ�����ݽ��е���
		sys_tem.Tem = sys_tem.Tem_Cur;
		sys_tem.Time = 0;
		sys_tem.IsCheck = 0;
	}

	while (p != NULL)
	{
		if (p->board_items_weight_all < 1000)//���У׼�Ļ�������С��1000�ˣ���ִ��У׼���Ƿ���ʻ���Ҫ����
		{
			//printf("У׼�������-- %d  ������ԭ�����ʲ���\r\n", p->board_items_weight_all);
			printf("���ذ� %s �������  %d , �޷�У׼������ԭ�����ʲ���", p->id, p->board_items_weight_all);
			p = p->next;
			continue;
		}

		p->ISCurvatureValue = 0;//���ó�δУ׼״̬
		p->ISCurvatureValueSave = 0;//���ó�δ����״̬
		for (int i = 0; i < times; i++)
		{
			Send_CMD(&hCom_C, p->id, BOARD_CMD_CURVATURE, (unsigned char *)&p->board_items_weight_all, 2, 1, 1);
			printf("...");
			Sleep(5000);
			if (p->ISCurvatureValue == 0)
			{
				//У׼ʧ��
				printf("\r\nУ׼ʧ�ܣ�����������˳�����\r\n");
				getchar(1);
				sys_die("\r\nУ׼ʧ�ܣ���\r\n");
			}
		}
		if (save == 1)
		{
			Send_CMD(&hCom_C, p->id, BOARD_CMD_CURVATURE_SAVE, (unsigned char *)&p->board_items_weight_all, 2, 1, 1);
			printf("...");
			Sleep(1000);
			if (p->ISCurvatureValueSave == 0)
			{
				//У׼ʧ��
				printf("\r\nУ׼����ʧ�ܣ�����������˳�����\r\n");
				getchar(1);
				sys_die("\r\nУ׼����ʧ�ܣ���\r\n");
			}
		}
		printf("���ذ� %s У׼��ɣ�\r\n", p->id);
		p = p->next;

	}

	/*
	if ( p->board_items_weight_all < 1000)//���У׼�Ļ�������С��1000�ˣ���ִ��У׼���Ƿ���ʻ���Ҫ����
	{
		printf("У׼�������-- %d  ������ԭ�����ʲ���\r\n", p->board_items_weight_all);
		return;
	}
	if (!sys_tem.IsCheck)
	{
		//�¶�û�б仯����У׼
		printf("���һ�β����¶�Ϊ %d ���϶� �� ϵͳ��ʼ�¶�Ϊ %d ���϶ȣ�����У׼\r\n" ,sys_tem.Tem_Cur , sys_tem.Tem);
		return;
	}
	else
	{
		//�¶ȱ仯��ʱ������У׼Ҫ���Ƚ��¶Ƚṹ�����ݽ��е���
		sys_tem.Tem = sys_tem.Tem_Cur;
		sys_tem.Time = 0;
		sys_tem.IsCheck = 0;
	}
	printf("�ȴ����ذ�  %s  У׼���" , p->id);

		p->ISCurvatureValue = 0;//���ó�δУ׼״̬
		p->ISCurvatureValueSave = 0;//���ó�δ����״̬
		for (int i = 0; i < times; i++)
		{
			Send_CMD(&hCom_C, p->id, BOARD_CMD_CURVATURE, (unsigned char *)&p->board_items_weight_all, 2, 1 , 1);
			printf("...");
			Sleep(5000);
			if (p->ISCurvatureValue == 0)
			{
				//У׼ʧ��
				printf("\r\nУ׼ʧ�ܣ�����������˳�����\r\n");
				getchar(1);
				sys_die("\r\nУ׼ʧ�ܣ���\r\n");
			}
		}
		if (save == 1)
		{
			Send_CMD(&hCom_C, p->id, BOARD_CMD_CURVATURE_SAVE, (unsigned char *)&p->board_items_weight_all, 2, 1 , 1);
			printf("...");
			Sleep(1000);
			if (p->ISCurvatureValueSave == 0)
			{
				//У׼ʧ��
				printf("\r\nУ׼����ʧ�ܣ�����������˳�����\r\n");
				getchar(1);
				sys_die("\r\nУ׼����ʧ�ܣ���\r\n");
			}
		}

	printf("���ذ� %s У׼��ɣ�\r\n" , p->id);
	*/
}

/*
*	���ܣ���ȡ���ذ嵱ǰ����
*	������
*	˵����Ŀǰδ�Է���ֵ�����κδ���
*/
void Board_Get_Weight()
{
	struct Board_Info * p = board_info;
	while (p != NULL)
	{
		p->board_items_weight_all_after_close_door = 0;//��ֵÿ��ʹ��ǰ������
		Send_CMD(&hCom_C, p->id, BOARD_CMD_GET_WEIGHT, NULL, 0, 1, 0);
		Sleep(5000);
		p = p->next;
	}
	printf("��ȡ������ɣ�\r\n");
}

/*
*	���ܣ���ʼ���¶Ƚṹ��
*	������
*	˵����
*/
void Init_Tem()
{
	sys_tem.delay = 60000;
	sys_tem.IsCheck = 0;
	sys_tem.MaxTime = 5;
	sys_tem.Tem = 0;
	sys_tem.Tem_Dis = 5;//���¶�ƫ����5���ڣ�����У׼
	sys_tem.Time = 0;
	sys_tem.Tem_Cur = 0;

	printf("��ȡ��ʼ���¶�\r\n");
	char send_buf[6];
	send_buf[0] = '@';
	send_buf[1] = 0xFF;//���ֵ�ǵ�ַλ�ߣ�һ����ѭ����Ҫ����
	send_buf[2] = 0xFE;//���ֵ�ǵ�ַλ�ͣ�һ����ѭ����Ҫ����
	send_buf[3] = 0xFE;
	send_buf[4] = 0xFB;
	send_buf[5] = '#';
	WriteCharT(hCom_Tem, send_buf, 6);
	Sleep(2000);
	if (Tem[4] != -100)
	{
		printf("��ȡ��ʼ���¶�����  %d ���϶�\r\n" , Tem[4]);
		sys_tem.Tem_Cur = sys_tem.Tem = Tem[4];
	}
	else
	{
		printf("��ȡ��ʼ���¶��쳣\r\n");
	}

}

/*
*	���ܣ���ȡ�����¶�
*	������
*	˵����
*			�����ִ�в����ǣ�ÿָ��ʱ���ȡһ���¶�ֵ�����¶ȱ仯������ֵ���������һ�γ�����¼����û�г�����ֵ����������¼��Ϊ0��ʱ���򳬳�������1
*			�������˴��������ֵ��ʱ�򣬻ᴥ��У׼��ֵ�����ֵ��ִ�г��ص������ʱ�򣬻ᱻ�жϣ��������λ������ڳ���ǰִ������У׼�������У׼��λ
*			��������ֵ�Ĵ������лָ�
*/
void Counter_Get_Tem()
{

	printf("��ʼ���ڼ���¶ȣ�����Ϊ %d ����\r\n" , sys_tem.delay);
	while(1)
	{ 
		Sleep(sys_tem.delay);
		printf("��ȡ�¶�\r\n");
		char send_buf[6];
		send_buf[0] = '@';
		send_buf[1] = 0xFF;//���ֵ�ǵ�ַλ��
		send_buf[2] = 0xFE;//���ֵ�ǵ�ַλ��
		send_buf[3] = 0xFE;
		send_buf[4] = 0xFB;
		send_buf[5] = '#';
		WriteCharT(hCom_Tem, send_buf, 6);
		Sleep(2000);
		if (Tem[0] == '@' &&  Tem[4] != -100 && Tem[6] == '#')//-100�ǽ������������Ĭ��ֵ�����Ϊ-100�������û���յ���Ч����
		{
			sys_tem.Tem_Cur = (signed char)Tem[4];
			printf("��ȡ�¶����� %d \r\n", sys_tem.Tem_Cur);

			if (sys_tem.Time == sys_tem.MaxTime)
			{
				sys_tem.IsCheck = 1;//��Ϊ���ջ���У׼�����лظ�Ϊ0
			}
			else if(sys_tem.Time == 0)
			{
				sys_tem.IsCheck = 0;
			}

			if (abs(sys_tem.Tem - sys_tem.Tem_Cur) > sys_tem.Tem_Dis)//����¶�ƫ����������ֵ,���һ�û������ִ��У׼��ʱ��Ҫ���ʱ��
			{
				if (sys_tem.Time < sys_tem.MaxTime)
				{
					sys_tem.Time++;
				}
				
			}
			else
			{
				if (sys_tem.Time > 0)
				{
					sys_tem.Time -= 1;
				}
			}
		}
		else
		{
			printf("�¶ȴ�����У���쳣\r\n");
		}
		for (int i = 0; i < 7; i++)
		{
			Tem[i] = -100;//0x9C ���ֵ����Ƿ����з����ַ��;���-100 ������޷����ַ��;���156
		}
		//Send_CMD(&hCom_Tem, "FFFE", LOCKER_CMD_TEM, NULL, 0, 2, 0);//id ��Ϊ�̶�

	}
}



/*
*	���ܣ���ȡ�����¶�
*	������
*	˵����
*			�����ִ�в��Ը��º��ǽ�ÿһ�����ذ嶼�趨һ���Ƿ���ҪУ׼���ж�λ��������У׼������ʱ�򣬽��������ذ�У׼λ��λ��Ȼ����µ�ǰ�¶�ֵ��
*			�����лظ��Ĳ��ԣ������¶Ȼָ���ԭʼ�¶�Ҳ����ظ�У׼��λ��ֻ��У׼���ܽ����ذ�У׼λ�ظ�
*/
void Counter_Get_Tem_Ex()
{

	printf("\r\n��ʼ���ڼ���¶ȣ�����Ϊ %d ����\r\n", sys_tem.delay);
	while (1)
	{
		Sleep(sys_tem.delay);
		printf("\r\n��ȡ�¶�\r\n");
		char send_buf[6];
		send_buf[0] = '@';
		send_buf[1] = 0xFF;//���ֵ�ǵ�ַλ��
		send_buf[2] = 0xFE;//���ֵ�ǵ�ַλ��
		send_buf[3] = 0xFE;
		send_buf[4] = 0xFB;
		send_buf[5] = '#';
		WriteCharT(hCom_Tem, send_buf, 6);
		Sleep(2000);
		if (Tem[0] == '@' &&  Tem[4] != -100 && Tem[6] == '#')//-100�ǽ������������Ĭ��ֵ�����Ϊ-100�������û���յ���Ч����
		{
			sys_tem.Tem_Cur = (signed char)Tem[4];
			printf("��ȡ�¶����� %d \r\n", sys_tem.Tem_Cur);

			if (sys_tem.Time == sys_tem.MaxTime)
			{
				//sys_tem.IsCheck = 1;//��Ϊ���ջ���У׼�����лظ�Ϊ0
				struct Board_Info * p = board_info;
				while (p != NULL)
				{
					p->ISNeedCur = ISNEEDCUR_YES;
					p = p->next;
				}
				sys_tem.Tem = sys_tem.Tem_Cur;
				sys_tem.Time = 0;
			}

			if (abs(sys_tem.Tem - sys_tem.Tem_Cur) > sys_tem.Tem_Dis)//����¶�ƫ����������ֵ,���һ�û������ִ��У׼��ʱ��Ҫ���ʱ��
			{
				if (sys_tem.Time < sys_tem.MaxTime)
				{
					sys_tem.Time++;
				}

			}
		}
		else
		{
			printf("�¶ȴ�����У���쳣\r\n");
		}
		for (int i = 0; i < 7; i++)
		{
			Tem[i] = -100;//0x9C ���ֵ����Ƿ����з����ַ��;���-100 ������޷����ַ��;���156
		}
		//Send_CMD(&hCom_Tem, "FFFE", LOCKER_CMD_TEM, NULL, 0, 2, 0);//id ��Ϊ�̶�

	}
}


/*
*	���ܣ�����
*	������[out]	LOCKER_UNLOCK_STATE_OK 0x00//�������������������Ѿ�����
				LOCKER_UNLOCK_STATE_OPEN_ALREADY 0x02//�ڷ��Ϳ���ָ���ʱ�����Ѿ�����
				LOCKER_UNLOCK_STATE_DONOT_OPEN 0x03//���������������ų�ʱ��û�б��򿪣����Զ�������
				LOCKER_UNLOCK_STATE_ERROR 0xFF//�����ش���
				GEN_ERROR_COMMUNICATION_ERROR 0xFE //ͨ�Ŵ��󣬸ô�����һ��ͨ�ô�����ָ������ͬǶ��ʽ�豸ͨ���쳣

*	˵�������������ִ��һ�ο��ţ������Եȴ�һ��ʱ�䣬Ȼ���Ƿ��ŵĽ������
*
*/
int Locker_Open()
{
	/*
	if (counter->locker_stat != 2 || counter->IsBusy != -1)//����û�д��ڹر�״̬,���ߴ�ʱ��������ִ����������
	{
		printf("�޷�ִ�п���\r\n");
		return;
	}
	*/
	//counter->IsBusy = 1;
	counter->locker_stat = -1;//��ֵ-1�������ж��ú����ķ���ֵ
	Send_CMD(&hCom_C, counter->locker_id, LOCKER_CMD_UNLOCK, NULL, 0, 2, 0);
	Sleep(5000);
	Sleep(5000);
	if (counter->locker_stat == -1)//���ǵ��˺�����Ƕ��ʽ����ֵ����һ��bug����˴˴�ֻ�ж��Ƿ��з���ֵ
	{
		printf("δ���յ����ķ������ݣ�\r\n");
		return GEN_ERROR_COMMUNICATION_ERROR;
	}
	return counter->locker_stat;
	//while (counter->locker_stat != 2)//����û�йر�֮ǰ����ѭ��,�˴�Ҫ����ҵ����Ҫ���ӳ�ʱ�����˳���ѭ���Ĺ���
	//{
	//	Locker_Get_Stat();
	//	Sleep(2000);
	//}
	//counter->IsBusy = -1;
	//counter->locker_stat = -1;
}

/*
*	���ܣ�ִ��һ�ο�����
*	������[out]	LOCKER_GET_STATE_OPENING 0x01//�����ڱ���
				LOCKER_GET_STATE_OK 0x02//������ִ����һ�ο��ض���
				LOCKER_GET_STATE_WAITING_OPEN 0x04//�����ڵȴ�����
				LOCKER_GET_STATE_ERROR 0xFF//�����ش���
				GEN_ERROR_COMMUNICATION_ERROR 0xFE //ͨ�Ŵ��󣬸ô�����һ��ͨ�ô�����ָ������ͬǶ��ʽ�豸ͨ���쳣

*	˵����ÿ�εȴ�1���ӣ�Ȼ���ж��ŵ�״̬�������أ����岻ͬ��״̬�߼���δ����ɵ��ô���

*/
int Locker_Open_Closed()
{
	/*
	if (counter->IsBusy != -1)//����û�д��ڹر�״̬,���ߴ�ʱ��������ִ����������
	{
		printf("�޷�ִ�п���\r\n");
		return;
	}
	*/
	int wait_time = 0;
	counter->locker_stat = -1;//��ֵ-1�������ж��ú����ķ���ֵ
	Send_CMD(&hCom_C, counter->locker_id, LOCKER_CMD_UNLOCK, NULL, 0, 2, 0);
	Sleep(5000);
	Sleep(5000);
	if (counter->locker_stat == -1)//���ǵ��˺�����Ƕ��ʽ����ֵ����һ��bug����˴˴�ֻ�ж��Ƿ��з���ֵ
	{
		printf("δ���յ����ķ������ݣ�\r\n");
		return GEN_ERROR_COMMUNICATION_ERROR;
	}

	do 
	{
		Locker_Get_Stat();
		Sleep(2000);
	} while (counter->locker_stat != 2 && ((wait_time++) < 15));

	//while (counter->locker_stat != 2 && ((wait_time ++) < 30))//����û�йر�֮ǰ����ѭ��,�˴�Ҫ����ҵ����Ҫ���ӳ�ʱ�����Ƴ���ѭ���Ĺ���
	//{
	//	Locker_Get_Stat();
	//	Sleep(2000);
	//}
	printf("ִ��һ�ο����ŵĽ�� ��%d\r\n" , counter->locker_stat);
	return counter->locker_stat;
	//counter->IsBusy = -1;
	//counter->locker_stat = -1;
}

/*
*	���ܣ���ȡ��״̬
*	������[out]	LOCKER_GET_STATE_OPENING 0x01//�����ڱ���
				LOCKER_GET_STATE_OK 0x02//������ִ����һ�ο��ض���
				LOCKER_GET_STATE_WAITING_OPEN 0x04//�����ڵȴ�����
				LOCKER_GET_STATE_ERROR 0xFF//�����ش���
				GEN_ERROR_COMMUNICATION_ERROR 0xFE //ͨ�Ŵ��󣬸ô�����һ��ͨ�ô�����ָ������ͬǶ��ʽ�豸ͨ���쳣
*	˵����Ŀǰδ�Է���ֵ�����κδ���

*/
int Locker_Get_Stat()
{
	//counter->IsBusy = 1;
	counter->locker_stat = -1;//��ֵ-1�������ж��ú����ķ���ֵ
	Send_CMD(&hCom_C, counter->locker_id, LOCKER_CMD_STATUS, NULL, 0, 2, 0);
	Sleep(2000);
	printf("��ȡ��״̬��%d \r\n" , counter->locker_stat);
	return counter->locker_stat;
	//counter->IsBusy = -1;
	//printf("������ɣ�\r\n");
}

/*
*	���ܣ���ɹ��������������̣�����������������۵�
*	������[out]���غ������������嵥��ָ���ַ�����û�������򷵻�null
*	˵�����ӿ����������˵�
*			�����˵���ʽ��һ��json������ɼ�onenote�ʼ�
*/
char *  Procedure_Sales()
{
	Get_Item_Info();//���¹��������ذ��items����ṹ,�������Ҳ���Է��ڵ������и��ĵ�ʱ����ִ�У��ݲ����������պ������ٵ���
	Get_Boards_Items_Weight();//��ȡ������ذ��ϻ�������
	//����ǰУ׼
	//struct Board_Info * board_info_p = board_info;
	//while (board_info_p != NULL)
	//{
		Board_Curavture_Value_Set( 2, 1);//У׼�ͱ���
		//board_info_p = board_info_p->next;
	//}
	//���Ų��ȴ�����
	Locker_Open();//Ŀǰ������ʽ�ģ�ֱ������
	Board_Get_Weight();//��ȡÿ�����������л��������

	JSON_Value *root_value = json_value_init_object();//��ʼ����һ��value��������Ϊobject��valueΪjson���ݵ�һ�����ͣ���ͬ��value���Ϳ���Ƕ��
	JSON_Object *root_object = json_value_get_object(root_value);//��ȡvalue�е�object�ĵ�ַ

	JSON_Value *sub_value = json_value_init_array();
	JSON_Array *sub_array = json_value_get_array(sub_value);

	JSON_Value *sub_sub_value[100];//�˴����������ʵ�����ǰ�����������ܹ����õ���Ʒ�������ֵ������ֵ���������ݿ⣬
								   //����޷�ֱ�����ڶ���̶��������飬��˴˴�����һ����Ϊ���ֵ���
	JSON_Array *sub_sub_array = NULL;
	int sub_sub_value_pos = 0;//ʹ�õ�value��λ��
	//JSON_Value *sub_sub_value = json_value_init_array();
	//JSON_Array *sub_sub_array = json_value_get_array(sub_sub_value);
	time_t timep;
	
	//printf("%s", ctime(&timep));
	json_object_set_string(root_object, "devid", counter->sn);
	json_object_set_string(root_object, "cmdid", "Shopping");
	//time(&timep);
	//json_object_set_string(root_object, "Date", ctime(&timep));
	//json_object_set_string(root_object, "Counter-SN", counter->sn);

	int list_items_num = 1;//���ﵥ�е���Ʒ˳���
	int Isale = 0;//����ڽ��������ж��У�����Ʒ�����ۻ�����ȡ���ˣ���ֵ���1���˳�whileѭ����ִ����Ϣ���ͺ���
	//board_info_p = board_info;
	struct Board_Info * board_info_p = board_info;
	while (board_info_p != NULL)//��ѯ���г��ذ�ĳƵõ�����
	{
		/*UINT16*/ double weight_buf =  board_info_p->board_items_weight_all - board_info_p->board_items_weight_all_after_close_door;
		if (abs(weight_buf) >= 0 && abs(weight_buf) <= ERROR_VALUE)//�������仯С�����ֵ����Ϊû�仯��ע���޷���ָ����������ʹ��abs��������
		{
			//weight_buf = 0;//��С��10 ����Ϊû������
			//���뵽��˴�����Ϊ������ذ�û���κλ���䶯
			board_info_p = board_info_p->next;
			continue;
		}
		else if (weight_buf < - ERROR_VALUE)//���ж������Ž����ذ壬�����ղ����Ľ�����Ǹ�ֵ��������ֵ��С�ڸ���������ֵ�Ŷ�
		{
			//�����뵽��˴���������л�����������ﱻ ���õ���������ذ��ϣ����ִ��������Ҫ���������ۻ��ڻ���ȡ�Ż�����
			//��Ϊ���ۻ�����˴�Ӧ��Ϊ�쳣�����Ϊȡ�Ż�����Ӧ��Ϊ������Ҫ�����������򣬲��ҽ����õĻ��������������������ݿ�
		}
		else //��������
		{
			Isale = 1;
			//���������������ĳһ�����ذ����ߵĻ�������ͬ���û�֮ǰ�����������С�ڵ��ڳ������ֵ������Ϊ���ߵľ��ǹ���ĳһ�����ذ�
			//�����еĻ���˴��趨��Ϊ�˱�����ֻ����ò������������ʵ��������500�ˣ�������������������ذ�Ƶ�����Ϊ1�ˣ�����
			//ϵͳ������൱��������499��
			//�˴�������������������ģʽ�������ͳһִ��
			if (abs(weight_buf - board_info_p->board_items_weight_all) <= ERROR_VALUE)
			{
				//������߻��������ͬ���ŵĻ�������ݿ��������ֵΪ��������ڣ�����Ϊ���ߵľ���ȫ�������ݿ�ı궨������
				//��˽����ߵĻ��������趨Ϊ����ǰ���ݿ��е������������ź�������趨Ϊ0
				weight_buf = board_info_p->board_items_weight_all;
				//��������if��䣬�����ߵĻ������������¼���󣬹��ź�ĳƵõ�����ҲҪ���¼���
				board_info_p->board_items_weight_all_after_close_door = 0;
			}


			if ( strcmp(board_info_p->type , "1") == 0)//����������
			{
				unsigned char change_code_buf[200] = { 0 };
				sub_sub_value[sub_sub_value_pos++] = json_value_init_array();//��һ��ʵ����һ��
				sub_sub_array = json_value_get_array(sub_sub_value[sub_sub_value_pos - 1]);
				GBKToUTF8(board_info_p->items->name, change_code_buf, 200);//��Ʒ���ƺ��Ӹ�ʽת��,�˴���Ҫ�����Ʒ���ֲ��Ǻ��ӻ���֪���Ƿ�ᱨ��
				json_array_append_string(sub_sub_array, change_code_buf);//��Ʒ����
				json_array_append_string(sub_sub_array, board_info_p->type);//�Ƽ�����
				json_array_append_string(sub_sub_array, board_info_p->items->weight_price);//�����
				Int_To_CharArray(weight_buf, change_code_buf);
				json_array_append_string(sub_sub_array, change_code_buf);//���������
				Double_To_CharArray(CharNum_To_Double(board_info_p->items->weight_price) * (weight_buf / 500.0)/*�����ػ���ɽ�*/, change_code_buf);
				json_array_append_string(sub_sub_array, change_code_buf);//������¼�ܼۣ��ܼ۲���ָ�������ѵ����ܼ۸�
				json_array_append_value(sub_array, sub_sub_value[sub_sub_value_pos - 1]);
				/*
				unsigned char item_list_num_buf[20] = {0};//�嵥�е���Ʒ���
				unsigned char item_list_buf[100] = { 0 };//�嵥�е���Ʒ��Ϣ
				unsigned char change_code_buf[200] = { 0 };
				Int_To_CharArray(list_items_num++, change_code_buf);
				strcat(item_list_num_buf, change_code_buf);

				GBKToUTF8(board_info_p->items->name, change_code_buf, 200);
				strcat(item_list_buf, change_code_buf);//��Ʒ����
				strcat(item_list_buf, "#");
				strcat(item_list_buf, board_info_p->type);//������ʽ
				strcat(item_list_buf, "#");
				strcat(item_list_buf, board_info_p->items->weight_price);//�����
				strcat(item_list_buf, "#");
				Int_To_CharArray(weight_buf, change_code_buf);
				strcat(item_list_buf, change_code_buf);//��������� ��
				strcat(item_list_buf, "#");
				*/
				//Double_To_CharArray(CharNum_To_Double(board_info_p->items->weight_price) * (weight_buf/500.0)/*�����ػ���ɽ�*/, change_code_buf);
				/*
				strcat(item_list_buf, change_code_buf);//�ܼ۸�Ԫ
				strcat(item_list_buf, "#");
				json_object_set_string(root_object, item_list_num_buf, item_list_buf);
				*/
				//���´�����Ʒ��¼�еĿ������
				Int_To_CharArray(board_info_p->board_items_weight_all_after_close_door, change_code_buf);
				SQL_UPDATA1("smart_sales_counter.items", "weight_sum", change_code_buf \
							, "item_id", board_info_p->items->item_id); 
			}
			else if (strcmp(board_info_p->type, "2") == 0)//����Ʒ��������
			{

			}
		}

		board_info_p = board_info_p->next;
	}
	if (Isale == 1)
	{
		time(&timep);
		json_object_dotset_string(root_object, "Result.Res", "0");//SUCCESS ����Ϊ 0
		json_object_dotset_string(root_object, "Result.Date", ctime(&timep));
		//json_object_set_string(root_object, "Counter-SN", counter->sn);
		json_object_dotset_value(root_object, "Result.Data", sub_value);
		//��json�ַ����л������������һ���ַ����У��������ظ��ַ�����
		char * json_string = (char *)malloc(json_serialization_size(root_value));//json_serialization_size��������ķ���ֵ�Ѿ������˽�ԭ�ַ������Ƚ�����չ����Ϊ��Ҫ����һЩ������ַ����绻�С���б�ܡ������ŵ�
		json_serialize_to_buffer(root_value, json_string, json_serialization_size(root_value));
		//�ͷ�json��Դ
		json_value_free(root_value);//ֻ���ͷŸ���Դ�����ڲ�������������Դ�ᱻ�ݹ��ͷ�

		return json_string;//��ָ������ź���������ʧЧ����ָ��ĸ��ڴ�������ⲿ��ʽ�ͷ�
	}
	else
	{
		//û�л��ﱻ���ۣ��˴����п���Ҫ����һЩ�쳣��Ӧ����Ϣ����ʱֻд��û�л��ﱻ���۵�ʱ���Ӧ��
		time(&timep);
		json_object_dotset_string(root_object, "Result.Res", "SUCCESS");
		json_object_dotset_string(root_object, "Result.Date", ctime(&timep));
		//json_object_set_string(root_object, "Counter-SN", counter->sn);
		json_object_dotset_value(root_object, "Result.Data", "NULL");//����û�л��ﱻ����
																	 //��json�ַ����л������������һ���ַ����У��������ظ��ַ�����
		char * json_string = (char *)malloc(json_serialization_size(root_value));//json_serialization_size��������ķ���ֵ�Ѿ������˽�ԭ�ַ������Ƚ�����չ����Ϊ��Ҫ����һЩ������ַ����绻�С���б�ܡ������ŵ�
		memset(json_string, 0, json_serialization_size(root_value));
		json_serialize_to_buffer(root_value, json_string, json_serialization_size(root_value));
		//�ͷ�json��Դ
		json_value_free(root_value);//ֻ���ͷŸ���Դ�����ڲ�������������Դ�ᱻ�ݹ��ͷ�

		return json_string;//��ָ������ź���������ʧЧ����ָ��ĸ��ڴ�������ⲿ��ʽ�ͷ�
	}
	return NULL;

}

/*
*	���ܣ������ϼܣ�ֻ��һ�����ذ��ṩ��Ʒ�ϼ�
*	������[in] ���յ���json����
*		  [out] ���صĽ��
*	˵�������ݸ�ʽΪ Devid :  ����   Cmdid:??    Data:[  [ ]   [ ]   [ ] �� [ ] ]������data�����������˳��Ӧ��ͬ���ݿ���items˳����ͬ
*         �ϼܵķ�ʽ��
*					 �������ģʽΪ1 ��Ҫ�ж���Ӧ���ذ����Ƿ��Ѿ�����Ʒ��������ϼ�ʧ�ܣ��˴��ж���ͨ��counter�ṹ����items�����Ƿ�Ϊ�����жϣ�
*					 Ҫ�û����¼ܷ�������ϼܣ����û�����жϷ��͹�����data���е��Ӽ������Ƿ�Ϊ1��
*					 �����Ψһ���ϼ�ʧ�ܣ�Ӧ�ò��ܷ����������Ӧ�ÿ��Ƹ��ַ�ʽ�ĳ��֣������û�ֻ���ϼ�һ����Ʒ
*					 ����ϼܶ����Ʒ��������ģʽ��Ϊ1�������е���Ʒ��Ҫ�ϼܵ�Ŀ����ذ���Ӧ����ͬ�������ϼ�ʧ�ܡ�
*					 ģʽ2���ϼܷ�ʽ��δȷ�����˴��ݲ����
*/
char *  Procedure_On_Shelf(JSON_Object * json_object)
{
	int error_code = 0;//0����һ������
	//���ж����յ���json�ַ����е�data���е������Ƿ�Ϸ�
	JSON_Array * sub_array_parse = json_object_get_array(json_object, "Data");
	JSON_Array  * sub_sub_array_parse = NULL;
	if (sub_array_parse != NULL)
	{
		if (json_array_get_count(sub_array_parse) > 1)//����������е����������������1��������ϴ��˶����Ʒ��Ϣ���������ģʽֻ��Ϊ2���������ݴ���
		{
			for (int i = 0; i < json_array_get_count(sub_array_parse); i++)//ÿһ�����͹�������Ʒ��Ϣ�е�����ģʽ������Ϊ������1����
			{
				sub_sub_array_parse = json_array_get_array(sub_array_parse, i);
				//��Ϊ��⵽���������������������������ģʽ����1���Ǿ�Ϊ����
				if (CharNum_To_Double(json_array_get_string(sub_sub_array_parse, 5)) == 1)
				{
					//��Ʒ����ģʽ�趨����
					error_code = 1;
					break;
				}
			}
		}
		if (json_array_get_count(sub_array_parse) > 1 && (!error_code))
		{
			//�������˴����ϴ�����Ʒ��Ϣ������ģʽû�д��������ж�����ϴ�����Ʒ��Ϣ�ж����������е���Ʒ��Ϣ����Ӧ�ĳ��ذ��Ŷ�������ͬ
			sub_sub_array_parse = json_array_get_array(sub_array_parse, 0);//��ȡ��һ��������ĳ��ذ��ţ�����ȶԶ��Ǹ���һ����
			char * board_id = json_array_get_string(sub_sub_array_parse, 2);
			for (int i = 0; i < json_array_get_count(sub_array_parse); i++)//ÿһ�����͹�������Ʒ��Ϣ�еĳ��ذ��ű�����ͬ
			{
				sub_sub_array_parse = json_array_get_array(sub_array_parse, i);
				if (strcmp(board_id, json_array_get_string(sub_sub_array_parse, 2)))//�����ͬ�򲻻����if
				{
					//���ذ�󶨴���
					error_code = 2;
					break;//��һ�������ֱ���˳���ѭ��
				}
			}
		}

	}
	else if(error_code == 0)
	{
		//���������
		error_code = 3;
		
	}

	int isfind = 0;//�ж��Ƿ��ҵ���Ŀ����ذ�
	struct Board_Info * board_info_p = board_info;
	//����������ݵ��߼�Ϊ�������ȴ�json��Ʒ�嵥�������ó���һ�������飨��Ϊһ����Ʒ��Ϣ��¼����Ȼ����������¼�������ĸ����ذ�ģ�
	//���������ѯ���г��ذ��û�ҵ�ƥ��ģ���Ϊ����������������֮ǰ���ж���Ӧ���Ѿ����¼��������ˣ����Ҵ��벻������while֮��
	//�������ģʽΪ2������ܻ���ͬһ�����ذ��ϴ������Ʒ��Ϣ�����ʱ�򲻻���ֶ����Ʒ��Ϣ������һ�����ذ壬��Ϊ֮ǰ�Ѿ����ж���
	//���ֻҪ��һ����Ʒ��Ϣ�����ĳ��ذ壬������ж����Ʒ����Ӧ�ö����ڸó��ذ��
	while (board_info_p != NULL && (!error_code))
	{
		//Ѱ�ҵ�������¼��Ҫ�����ĸ����ذ��
		sub_sub_array_parse = json_array_get_array(sub_array_parse, 0);
		if (!strcmp(board_info_p->id, json_array_get_string(sub_sub_array_parse, 2)))//�ȶ���Ʒ��Ϣ�еĳ��ذ���
		{
			//���뵽��˴������Ѿ��ҵ����ذ�
			isfind = 1;
			if ((CharNum_To_Double(json_array_get_string(sub_sub_array_parse, 5)) == 1) && \
				(!strcmp(board_info_p->type, json_array_get_string(sub_sub_array_parse, 5)))  )//�����������ļ�¼����ģʽΪ1�����Ҹó��ذ������ģʽҲΪ1
			{
				//����ģʽΪ1
				if (board_info_p->items != NULL)
				{
					error_code = 4;//��Ҫ�Ƚ�����Ʒ�¼�
					break;
				}
				//�������ݿ���Ϣ��������Ҫ���³�ʼ��board�е�items����
				if (SQL_INSERT_INTO_ItemsTable(\
					json_array_get_string(sub_sub_array_parse, 0), json_array_get_string(sub_sub_array_parse, 1), \
					json_array_get_string(sub_sub_array_parse, 2), json_array_get_string(sub_sub_array_parse, 3), \
					json_array_get_string(sub_sub_array_parse, 4), json_array_get_string(sub_sub_array_parse, 5), \
					json_array_get_string(sub_sub_array_parse, 6), json_array_get_string(sub_sub_array_parse, 7), \
					json_array_get_string(sub_sub_array_parse, 8), json_array_get_string(sub_sub_array_parse, 9), \
					json_array_get_string(sub_sub_array_parse, 10)) != 1)
				{
					error_code = 5;//��Ʒ���ݿ�������
				}
				else
				{
					//���ݿ�ִ�гɹ�
					Get_Item_Info();//���¹��������ذ��items����ṹ,��Ϊ���ݿ��Ѿ��ı�
				}
				break;
			}
			else if ((CharNum_To_Double(json_array_get_string(sub_sub_array_parse, 5)) == 2) && \
				(!strcmp(board_info_p->type, json_array_get_string(sub_sub_array_parse, 5))) )
			{
				//����ģʽΪ2
				//���Ȳ�ѯҪ���µ�ȫ����Ʒ��Ϣ�Ƿ�ͬ�ó��ذ��Ѿ��ϼܵ���Ʒ��ͬ�ģ���������쳣
				for (int i = 0; i < json_array_get_count(sub_array_parse); i++)
				{
					sub_sub_array_parse = json_array_get_array(sub_array_parse, i);
					//ÿһ�����������Ʒ��ű�����������ذ���û�г��ֹ�
					if (SQL_SELECT("smart_sales_counter.items", "item_id", json_array_get_string(sub_sub_array_parse, 0)) != 0 )
					{
						//��Ʒ�Ѿ��ϼ�
						error_code = 8;
						break;
					}
				}
				//��ʼִ��insert ���
				if (!error_code)
				{
					for (int i = 0; i < json_array_get_count(sub_array_parse); i++)
					{
						sub_sub_array_parse = json_array_get_array(sub_array_parse, i);
						if (SQL_INSERT_INTO_ItemsTable(\
							json_array_get_string(sub_sub_array_parse, 0), json_array_get_string(sub_sub_array_parse, 1), \
							json_array_get_string(sub_sub_array_parse, 2), json_array_get_string(sub_sub_array_parse, 3), \
							json_array_get_string(sub_sub_array_parse, 4), json_array_get_string(sub_sub_array_parse, 5), \
							json_array_get_string(sub_sub_array_parse, 6), json_array_get_string(sub_sub_array_parse, 7), \
							json_array_get_string(sub_sub_array_parse, 8), json_array_get_string(sub_sub_array_parse, 9), \
							json_array_get_string(sub_sub_array_parse, 10)) != 1)
						{
							error_code = 9;//��Ʒ���ݿ�������
							break;
						}
					}
				}
				if (!error_code)
				{
					//���ݿ�ִ�гɹ�
					Get_Item_Info();//���¹��������ذ��items����ṹ,��Ϊ���ݿ��Ѿ��ı�
				}
				break;

			}
			else
			{
				error_code = 6;//��Ʒ����ģʽͬĿ����ذ岻��
				break;
			}

		}
		board_info_p = board_info_p->next;
		
	}
	if (isfind == 0 && error_code == 0)
	{
		//û���ҵ�Ŀ����ذ�
		error_code = 7;
	}
	//if (!error_code)//������˶�û�з�������
	//{
		//���سɹ�����
		time_t timep;
		char * res_num[20];
		JSON_Value *root_value = json_value_init_object();//��ʼ����һ��value��������Ϊobject��valueΪjson���ݵ�һ�����ͣ���ͬ��value���Ϳ���Ƕ��
		JSON_Object *root_object = json_value_get_object(root_value);//��ȡvalue�е�object�ĵ�ַ
		json_object_set_string(root_object, "devid", counter->sn);
		json_object_set_string(root_object, "cmdid", "OnShelf");
		time(&timep);
		Int_To_CharArray(error_code, res_num);
		json_object_dotset_string(root_object, "Result.Res", res_num);
		json_object_dotset_string(root_object, "Result.Date", ctime(&timep));
		//json_object_set_string(root_object, "Counter-SN", counter->sn);
		json_object_dotset_value(root_object, "Result.Data", "NULL");//û������
		char * json_string = (char *)malloc(json_serialization_size(root_value));//json_serialization_size��������ķ���ֵ�Ѿ������˽�ԭ�ַ������Ƚ�����չ����Ϊ��Ҫ����һЩ������ַ����绻�С���б�ܡ������ŵ�
		memset(json_string, 0, json_serialization_size(root_value));
		json_serialize_to_buffer(root_value, json_string, json_serialization_size(root_value));
		//�ͷ�json��Դ
		json_value_free(root_value);//ֻ���ͷŸ���Դ�����ڲ�������������Դ�ᱻ�ݹ��ͷ�

		return json_string;//��ָ������ź���������ʧЧ����ָ��ĸ��ڴ�������ⲿ��ʽ�ͷ�

	//}
	//else
	//{
	//	//���سɹ�����
	//	time_t timep;
	//	char * res_num[20];
	//	JSON_Value *root_value = json_value_init_object();//��ʼ����һ��value��������Ϊobject��valueΪjson���ݵ�һ�����ͣ���ͬ��value���Ϳ���Ƕ��
	//	JSON_Object *root_object = json_value_get_object(root_value);//��ȡvalue�е�object�ĵ�ַ
	//	json_object_set_string(root_object, "devid", counter->sn);
	//	json_object_set_string(root_object, "cmdid", "OnShelf");
	//	time(&timep);
	//	Int_To_CharArray(error_code, res_num);
	//	json_object_dotset_string(root_object, "Result.Res", res_num);
	//	json_object_dotset_string(root_object, "Result.Date", ctime(&timep));
	//	//json_object_set_string(root_object, "Counter-SN", counter->sn);
	//	json_object_dotset_value(root_object, "Result.Data", "NULL");
	//																 //��json�ַ����л������������һ���ַ����У��������ظ��ַ�����
	//	char * json_string = (char *)malloc(json_serialization_size(root_value));//json_serialization_size��������ķ���ֵ�Ѿ������˽�ԭ�ַ������Ƚ�����չ����Ϊ��Ҫ����һЩ������ַ����绻�С���б�ܡ������ŵ�
	//	memset(json_string, 0, json_serialization_size(root_value));
	//	json_serialize_to_buffer(root_value, json_string, json_serialization_size(root_value));
	//	//�ͷ�json��Դ
	//	json_value_free(root_value);//ֻ���ͷŸ���Դ�����ڲ�������������Դ�ᱻ�ݹ��ͷ�

	//	return json_string;//��ָ������ź���������ʧЧ����ָ��ĸ��ڴ�������ⲿ��ʽ�ͷ�
	//}

	//��Դ���ͬ�ı���Ӧ��ʹ��switch caseȥ����ж�Ȼ��������Ӧ�Ĵ�����Ϣ���ҷ��ͣ������ǵ�����һ������ģʽû����ɣ�����ݲ���д�˶δ���

}

/*
*	���ܣ������¼�
*	������[in] ���յ���json����
*		  [out] ���صĽ��
*	˵�������ݸ�ʽΪ Devid :  ����   Cmdid:??    Data:[  [ ]   [ ]   [ ] �� [ ] ]������data�����������˳��Ӧ��ͬ���ݿ���items˳����ͬ
*         �¼ܵķ�ʽ��
*					#�����¼���ָ���ĳһ�����ذ��ϵĻ�������¼ܴ����¼���ɾ���ó��ذ������items�б��е���Ʒ��
*						����ɾ�������������ݿ��еļ�¼
*					#ִ�и�����֮��һ������Ϊ�Ըù��ӵĸó��ذ�ִ���ϼܣ�Ȼ��ִ��һ�β���⿪���ţ����ڲ�������е����󷽿��������й���
*						��Ϊ�¼���Ʒ��Ϣ��items��������¹�������ʱitems������Ϣ��ͬ���ذ���ʵ�ʻ�����Ϣ����������ִ���ϼ��ǿ��Եģ���Ϊ
*						�ϼ�����Ҳ�������Ʒ���г��ز���������Ҫִ��һ�β���⿪���ţ������ڲ�����ͬ���¼ܺ��趨�Ľ��һ�£�Ҳ���Զ�ͬһ��
*						���Ӳ�ͬ���ذ�ִ�ж�����¼������Ȼ����ͳһ���в���⿪���ţ��Ի������ͳһ����
*					#��֮����ִ���˶��ٴ����¼ܣ����¼�Ҳ���ý�����У�������һ��Ҫͨ��һ�β���⿪�������������ӵ����¼�״̬��������Ӳ���
*						����
*					#��Ʒ�¼��ϴ�����Ʒ��Ϣ��ʵ����Ҫ��������Ϣ��ֻ������Ʒ��ź�Ŀ����ذ���Ϣ���У���Ϊͬ�ϼܸ�ʽ��������ٴ�����ģ��˴�
*						�Ծɽ����ϴ�һ����������Ʒ��Ϣ
*/
char *  Procedure_Off_Shelf(JSON_Object * json_object)
{
	int error_code = 0;//0����һ������
	//���ж����յ���json�ַ����е�data���е������Ƿ�Ϸ�
	JSON_Array * sub_array_parse = json_object_get_array(json_object, "Data");
	JSON_Array  * sub_sub_array_parse = NULL;
	if (sub_array_parse != NULL)
	{
		if (json_array_get_count(sub_array_parse) > 1)//����������е����������������1��������ϴ��˶����Ʒ��Ϣ���������ģʽֻ��Ϊ2���������ݴ���
		{
			for (int i = 0; i < json_array_get_count(sub_array_parse); i++)//ÿһ�����͹�������Ʒ��Ϣ�е�����ģʽ������Ϊ������1����
			{
				sub_sub_array_parse = json_array_get_array(sub_array_parse, i);
				//��Ϊ��⵽���������������������������ģʽ����1���Ǿ�Ϊ����
				if (CharNum_To_Double(json_array_get_string(sub_sub_array_parse, 5)) == 1)
				{
					//��Ʒ����ģʽ�趨����
					error_code = 1;
					break;
				}
			}
		}
		if (json_array_get_count(sub_array_parse) > 1 && (!error_code))
		{
			//�������˴����ϴ�����Ʒ��Ϣ������ģʽû�д��������ж�����ϴ�����Ʒ��Ϣ�ж����������е���Ʒ��Ϣ����Ӧ�ĳ��ذ��Ŷ�������ͬ
			sub_sub_array_parse = json_array_get_array(sub_array_parse, 0);//��ȡ��һ��������ĳ��ذ��ţ�����ȶԶ��Ǹ���һ����
			char * board_id = json_array_get_string(sub_sub_array_parse, 2);
			for (int i = 0; i < json_array_get_count(sub_array_parse); i++)//ÿһ�����͹�������Ʒ��Ϣ�еĳ��ذ��ű�����ͬ
			{
				sub_sub_array_parse = json_array_get_array(sub_array_parse, i);
				if (strcmp(board_id, json_array_get_string(sub_sub_array_parse, 2)))//�����ͬ�򲻻����if
				{
					//���ذ�󶨴���
					error_code = 2;
					break;//��һ�������ֱ���˳���ѭ��
				}
			}
		}

	}
	else if (error_code == 0)
	{
		//���������
		error_code = 3;

	}
	
	int isfind = 0;//�ж��Ƿ��ҵ���Ŀ����ذ�
	struct Board_Info * board_info_p = board_info;
	while (board_info_p != NULL && (!error_code))
	{
		//Ѱ�ҵ�Ҫ�¼��Ǹ����ذ��ϵĻ��������ģʽ1����ģʽ2�������м���Ҫ�¼ܵ���Ʒ��Ϣ�����׸�Ԫ�ص�Ŀ����ذ��źͺ��������Ԫ�ص�
		//Ŀ����ذ��ž�����ͬ�ģ��������ͬ��֮ǰ�����ݺϷ��Լ���н������֣����벻����������
		sub_sub_array_parse = json_array_get_array(sub_array_parse, 0);
		if (!strcmp(board_info_p->id, json_array_get_string(sub_sub_array_parse, 2)))//�ȶ���Ʒ��Ϣ�еĳ��ذ���
		{
			//���뵽��˴������Ѿ��ҵ����ذ�
			isfind = 1;//�Ѿ��ҵ���Ӧ�ĳ��ذ�
			//�����������ļ�¼����ģʽΪ1�����Ҹó��ذ������ģʽҲΪ1���������if
			if ((CharNum_To_Double(json_array_get_string(sub_sub_array_parse, 5)) == 1) && \
				(!strcmp(board_info_p->type, json_array_get_string(sub_sub_array_parse, 5))))
			{
				//����ģʽΪ1
				if (board_info_p->items == NULL)
				{
					error_code = 4;//�ó��ذ��ϲ�û����Ʒ�����¼�
					break;
				}
				//Ҫ�¼ܵ���Ʒ��Ϣͬ�ó��ذ��ϵĻ�����Ϣ�Ƿ���ͬ,��Ϊ����ģʽΪ1�����ֻ��ȡitems������׸��ڵ�
				if (strcmp(board_info_p->items->item_id, json_array_get_string(sub_sub_array_parse, 0)))
				{
					//�����������˴���Ҫ�¼ܵ���Ʒ���ͬ�ó��ذ�����Ʒ��Ų�ͬ
					error_code = 5;
					break;
				}
				//�������ݿ���Ϣ��������Ҫ���³�ʼ��board�е�items����
				if (SQL_DELETE("smart_sales_counter.items", "item_id", json_array_get_string(sub_sub_array_parse, 0)) != 1)
				{
					error_code = 6;//��Ʒ���ݿ�������
					break;
				}
				else
				{
					//���ݿ�ִ�гɹ�
					Get_Item_Info();//���¹��������ذ��items����ṹ,��Ϊ���ݿ��Ѿ��ı�
				}
				break;
			}
			else if ((CharNum_To_Double(json_array_get_string(sub_sub_array_parse, 5)) == 2) && \
				(!strcmp(board_info_p->type, json_array_get_string(sub_sub_array_parse, 5))))
			{
				//����ģʽΪ2

			}
			else
			{
				error_code = 7;//��Ʒ����ģʽͬĿ����ذ岻��
				break;
			}

		}
		board_info_p = board_info_p->next;
	}
	if (isfind == 0 && error_code == 0)
	{
		//û���ҵ�Ŀ����ذ�
		error_code = 8;
	}
	if (!error_code)//������˶�û�з�������
	{
		//���سɹ�����
		time_t timep;
		char * res_num[20];
		JSON_Value *root_value = json_value_init_object();//��ʼ����һ��value��������Ϊobject��valueΪjson���ݵ�һ�����ͣ���ͬ��value���Ϳ���Ƕ��
		JSON_Object *root_object = json_value_get_object(root_value);//��ȡvalue�е�object�ĵ�ַ
		json_object_set_string(root_object, "devid", counter->sn);
		json_object_set_string(root_object, "cmdid", "OffShelf");
		time(&timep);
		Int_To_CharArray(error_code, res_num);
		json_object_dotset_string(root_object, "Result.Res", res_num);
		json_object_dotset_string(root_object, "Result.Date", ctime(&timep));
		//json_object_set_string(root_object, "Counter-SN", counter->sn);
		json_object_dotset_value(root_object, "Result.Data", "NULL");//û������
		char * json_string = (char *)malloc(json_serialization_size(root_value));//json_serialization_size��������ķ���ֵ�Ѿ������˽�ԭ�ַ������Ƚ�����չ����Ϊ��Ҫ����һЩ������ַ����绻�С���б�ܡ������ŵ�
		memset(json_string, 0, json_serialization_size(root_value));
		json_serialize_to_buffer(root_value, json_string, json_serialization_size(root_value));
		//�ͷ�json��Դ
		json_value_free(root_value);//ֻ���ͷŸ���Դ�����ڲ�������������Դ�ᱻ�ݹ��ͷ�

		return json_string;//��ָ������ź���������ʧЧ����ָ��ĸ��ڴ�������ⲿ��ʽ�ͷ�

	}
	else
	{
		//���سɹ�����
		time_t timep;
		char * res_num[20];
		JSON_Value *root_value = json_value_init_object();//��ʼ����һ��value��������Ϊobject��valueΪjson���ݵ�һ�����ͣ���ͬ��value���Ϳ���Ƕ��
		JSON_Object *root_object = json_value_get_object(root_value);//��ȡvalue�е�object�ĵ�ַ
		json_object_set_string(root_object, "devid", counter->sn);
		json_object_set_string(root_object, "cmdid", "OffShelf");
		time(&timep);
		Int_To_CharArray(error_code, res_num);
		json_object_dotset_string(root_object, "Result.Res", res_num);
		json_object_dotset_string(root_object, "Result.Date", ctime(&timep));
		//json_object_set_string(root_object, "Counter-SN", counter->sn);
		json_object_dotset_value(root_object, "Result.Data", "NULL");
																	 //��json�ַ����л������������һ���ַ����У��������ظ��ַ�����
		char * json_string = (char *)malloc(json_serialization_size(root_value));//json_serialization_size��������ķ���ֵ�Ѿ������˽�ԭ�ַ������Ƚ�����չ����Ϊ��Ҫ����һЩ������ַ����绻�С���б�ܡ������ŵ�
		memset(json_string, 0, json_serialization_size(root_value));
		json_serialize_to_buffer(root_value, json_string, json_serialization_size(root_value));
		//�ͷ�json��Դ
		json_value_free(root_value);//ֻ���ͷŸ���Դ�����ڲ�������������Դ�ᱻ�ݹ��ͷ�

		return json_string;//��ָ������ź���������ʧЧ����ָ��ĸ��ڴ�������ⲿ��ʽ�ͷ�
	}



}


/*
*	���ܣ����һ��ȡ�Ż�
*	������[in] ȡ�Ż����ͣ�0Ϊ����ȡ�Ż���1Ϊ����⣬ֱ�ӿ����ţ�1λ��������Ʒ���¼�֮��ִ�еĻ���ȡ������
*		  [out]���غ������������嵥��ָ���ַ�����û�������򷵻�null
*	˵�����ӿ���������ȡ�Ż��嵥�������嵥��ʽ��һ��json������ɼ�onenote�ʼ�
*		  ȡ�Ż���������ʽ��һ��������Ʒ������¼�֮��ִ�е�ȡ�Ż�����ʱִֻ�п����ţ������أ�ֻ�ش��Źرյ��������ԭ��Ϊ��
*			#��Ʒ����ϼܺ����ݿ��Ѿ�д�룬���а����ϼ���Ʒ����������ʱ���ذ��ϻ�û�л����˽�����Ҫִ�е��ǿ��Ž�����
*				����ȥ�������ڷ�֮ǰ���ذ�û�л������޷�ִ��У׼������Ҳ���޷�������ȷ����Ʒ����
*			#��Ʒ�¼���ɣ���ʱ���ݿ��Ѿ�ɾ�������ҳ��ذ�items����Ҳ�Ѿ�û�и���Ʒ��¼������ڿ���ǰ���ִ��У׼��������ֵ���쳣
*				��Ϊ��Ʒ���ݿ���Ϣ�Ѿ�ɾ�������ǻ���û��ȡ�ߣ�����������ݿ��������ڿ���ǰУ׼������ذ���ʵ�ʻ�������ͬ���ݿ���
*				��¼��һ�£�����޷���ȡ����
*			#ע�����ǵ���Ʒ���¼ܺ���ʵ�ʻ���Ķ����֮ǰ���ӻ���ֻ�������ݿ���Ϣ��һ�µ��������˴�ʱ����Ӧ���ǳ��������¼�֮����
*				����ָ��һ�Ų����ܣ�������λ�����Ӧ�ÿ��ƴ����̣���֤�����¼���ɺ�ʵ�ʻ���Ķ�֮ǰ���ܽ����û�����������
*/
char *  Procedure_Pick_And_Place(int Type)
{
	JSON_Value *root_value = json_value_init_object();//��ʼ����һ��value��������Ϊobject��valueΪjson���ݵ�һ�����ͣ���ͬ��value���Ϳ���Ƕ��
	JSON_Object *root_object = json_value_get_object(root_value);//��ȡvalue�е�object�ĵ�ַ

	JSON_Value *sub_value = json_value_init_array();
	JSON_Array *sub_array = json_value_get_array(sub_value);

	time_t timep;
	time(&timep);
	json_object_set_string(root_object, "devid", counter->sn);
	if (Type)//ִ��һ�β������Ŀ�����
	{
		Locker_Open();//Ŀǰ������ʽ�ģ�ֱ������
		//���ͳɹ�ִ��һ�ο�����
		//printf("%s", ctime(&timep));
		json_object_set_string(root_object, "cmdid", "Unlock");
		json_object_dotset_string(root_object, "Result.Res", "0");
		json_object_dotset_string(root_object, "Result.Date", ctime(&timep));
		json_object_dotset_value(root_object, "Result.Data", "NULL");
		//��json�ַ����л������������һ���ַ����У��������ظ��ַ�����
		char * json_string = (char *)malloc(json_serialization_size(root_value));//json_serialization_size��������ķ���ֵ�Ѿ������˽�ԭ�ַ������Ƚ�����չ����Ϊ��Ҫ����һЩ������ַ����绻�С���б�ܡ������ŵ�
		json_serialize_to_buffer(root_value, json_string, json_serialization_size(root_value));
		//�ͷ�json��Դ
		json_value_free(root_value);//ֻ���ͷŸ���Դ�����ڲ�������������Դ�ᱻ�ݹ��ͷ�
		return json_string;//��ָ������ź���������ʧЧ����ָ��ĸ��ڴ�������ⲿ��ʽ�ͷ�

	}
	else//ִ��һ��������ȡ�Ż�
	{
		Get_Item_Info();//���¹��������ذ��items����ṹ,�������Ҳ���Է��ڵ������и��ĵ�ʱ����ִ�У��ݲ����������պ������ٵ���
		Get_Boards_Items_Weight();//��ȡ������ذ��ϻ�������
		struct Board_Info * board_info_p = board_info;
		//����ǰУ׼
		//while (board_info_p != NULL)
		//{
			Board_Curavture_Value_Set( 2 , 1);//У׼�ͱ���
			//board_info_p = board_info_p->next;
		//}
		//���Ų��ȴ�����
		Locker_Open();//Ŀǰ������ʽ�ģ�ֱ������
		Board_Get_Weight();//��ȡÿ�����������л��������
		JSON_Value *sub_sub_value[100];//�˴����������ʵ�����ǰ�����������ܹ����õ���Ʒ�������ֵ������ֵ���������ݿ⣬
									   //����޷�ֱ�����ڶ���̶��������飬��˴˴�����һ����Ϊ���ֵ���
		JSON_Array *sub_sub_array = NULL;
		int sub_sub_value_pos = 0;//ʹ�õ�value��λ��
		json_object_set_string(root_object, "cmdid", "P&P");

		int Isale = 0;//����ڽ��������ж��У�����Ʒ��ȡ���ˣ���ֵ���1���˳�whileѭ����ִ����Ϣ���ͺ���
		board_info_p = board_info;
		double weight_buf = 0;
		double weight_buf_fix = 0;//�Ƽ�ģʽ2ʹ�õģ����ջ�ȡ����׼ȷ���������ǳƵ�����������ֵ
		double price = 0;//�Ƽ�ģʽ2ʹ�ã����ջ�ȡ����Ҫ���ķ���
		while (board_info_p != NULL)//��ѯ���г��ذ�ĳƵõ�����
		{
			//weight_bufΪ���γƵõ������仯ֵ������������
			weight_buf = board_info_p->board_items_weight_all - board_info_p->board_items_weight_all_after_close_door;
			//�������仯С�����ֵ����Ϊû�仯��ע���޷���ָ����������ʹ��abs��������
			if (abs(weight_buf) >= 0 && abs(weight_buf) <= ERROR_VALUE)
			{
				//weight_buf = 0;//��С��10 ����Ϊû������
				//���뵽��˴�����Ϊ������ذ�û���κλ���䶯
				board_info_p = board_info_p->next;
				continue;
			}
			Isale = 1;//���뵽��˴�����ó��ذ������б仯
			unsigned char change_code_buf[200] = { 0 };
			if (strcmp(board_info_p->type, "1") == 0)//������������¼ܻ���
			{
				//��������ģʽΪ1�Ļ����ÿ��У׼�Ͳ����������΢С��ƫ��������޷��ж������ۻ�����ȡ�ߵĻ����ʵ�����������ռ�������ֵ
				//���������ۻ����Ƿ���������ۼӡ�����ȡ�߻�����ǿ���ǰ�����ϵĻ�������ͬ���ݿ��е���������С����ȡ�������Ժ��ԣ������
				//���ֵ���������۴������������������
				//���������������ĳһ�����ذ����ߵĻ�������ͬ���û�֮ǰ�����������С�ڵ��ڳ������ֵ������Ϊ���ߵľ��ǹ���ĳһ�����ذ�
				//�����еĻ���˴��趨��Ϊ�˱�����ֻ����ò������������ʵ��������500�ˣ�������������������ذ�Ƶ�����Ϊ1�ˣ�����
				//ϵͳ������൱��������499��
				//�������ֻ�����������ģʽ1�У���Ϊ2���Ǹ��ݲ������Ѱ�����ߵĻ�����ϣ��������ݿ���ٵ�����ϱ�ʶ������������ʵ������
				if (abs(weight_buf - board_info_p->board_items_weight_all) <= ERROR_VALUE)
				{
					//������߻��������ͬ���ŵĻ�������ݿ��������ֵΪ��������ڣ�����Ϊ���ߵľ���ȫ�������ݿ�ı궨������
					//��˽����ߵĻ��������趨Ϊ����ǰ���ݿ��е������������ź�������趨Ϊ0
					weight_buf = board_info_p->board_items_weight_all;
					//��������if��䣬�����ߵĻ������������¼���󣬹��ź�ĳƵõ�����ҲҪ���¼���
					board_info_p->board_items_weight_all_after_close_door = 0;
				}
				//����ȡ��

					sub_sub_value[sub_sub_value_pos++] = json_value_init_array();//
					sub_sub_array = json_value_get_array(sub_sub_value[sub_sub_value_pos - 1]);
					//�˴���ʽת����˵����Ϊ�˱���windows��ӡ���粻�������룬������ݿ�����Ӷ��������õ�ΪGBK�룬board_info->itmes->name����ֶ�
					//�Ǵ����ݿ��ѯ�����ֱ�Ӹ�ֵ�����ģ����Ұ������ģ���������ΪGBK�룬��ʱҪ�����json�ṹ�У�jsonҪ�������utf8���룬���Ҫ����ת��
					GBKToUTF8(board_info_p->items->name, change_code_buf, 200);//��Ʒ���ƺ��Ӹ�ʽת��,�˴���Ҫ�����Ʒ���ֲ��Ǻ��ӻ���֪���Ƿ�ᱨ��--Ӧ�ò��ᱨ��
					json_array_append_string(sub_sub_array, change_code_buf);//��Ʒ����
					json_array_append_string(sub_sub_array, board_info_p->type);//�Ƽ�����
					json_array_append_string(sub_sub_array, board_info_p->items->weight_price);//�����
					//���ŵĵ�����������ȡ����ͬ���ۻ��ڲ�ͬ�����ۻ�������������²�����ֻ����������ź�ȿ���ǰ�������
					//������ۻ�����ȡ���������Ӧ�ò��ᱻ��ֵ�������㷨����ֵ��ʵ�Ǵ����л��ﱻ��������ϣ������ǵ����ǵ�
					//�Ķ�ϰ�ߣ�һ��ֵΪ��ֵ��ʱ�������ӣ���Ϊ����ʱ����ʵ���٣�ͬ�㷨�պ��෴����˴˴��Է��Ž��е�������
					//����Ǳ������˽�weight_buf����Ϊ��ֵ�����Ϊ�����˽�weight_buf����Ϊ��ֵ
					weight_buf = 0 - weight_buf;
					Int_To_CharArray(weight_buf, change_code_buf);
					json_array_append_string(sub_sub_array, change_code_buf);//ȡ�ŵ��������˴�Ӧ���Ǵ������ŵ�
					//����ȡ�߻���ļ۸񣬴˴�Ҫ��������ȥ����ͳһΪ����
					Double_To_CharArray(CharNum_To_Double(board_info_p->items->weight_price) * (abs(weight_buf) / 500.0)/*�����ػ���ɽ�*/, change_code_buf);
					json_array_append_string(sub_sub_array, change_code_buf);//������¼�ܼۣ��ܼ۲���ָ�������ѵ����ܼ۸�
					json_array_append_value(sub_array, sub_sub_value[sub_sub_value_pos - 1]);
					//���´�����Ʒ��¼�еĿ������
					//��Ϊ����ģʽΪ1����˳��ذ���ֻ����һ�ֻ����˴�ʱ���ź�����������ǳ��ذ��϶Զ�Ӧ����Ʒ����
					//������ȡ�Ż�������������ܱȿ���ǰС�����ߣ���Ҳ���ܱȿ���ǰ�󣨷��룩
					Int_To_CharArray(board_info_p->board_items_weight_all_after_close_door, change_code_buf);
					SQL_UPDATA1("smart_sales_counter.items", "weight_sum", change_code_buf \
						, "item_id", board_info_p->items->item_id);
			}
			else if (strcmp(board_info_p->type, "2") == 0)//����Ʒ��������
			{
				//����Ƽ�ģʽΪ2
				//ע�����һ�����������÷�ʽ�����ӵ����ۿ����ǻ��ģʽ��sub_sub_value��data���ڸ��������飬��˲�����ģʽ1����ģʽ2��Ӧ���ǳ����ۼ�sub_sub_value_posֵ
				weight_buf = 0 - weight_buf;//�ı�weight_buf���ţ�����Ϊ���룬����Ϊȡ��
				int sub_sub_value_pos_start = sub_sub_value_pos;
				sub_sub_value_pos += Scheme_Parse(board_info_p->scheme_id, weight_buf, &weight_buf_fix, &price, &sub_sub_value[sub_sub_value_pos]);
				for (int i = sub_sub_value_pos_start; i < sub_sub_value_pos ; i++)
				{
					json_array_append_value(sub_array, sub_sub_value[i]);
				}
				
			}

			board_info_p = board_info_p->next;
		}
		if (Isale == 1)
		{
			time(&timep);
			json_object_dotset_string(root_object, "Result.Res", "0");
			json_object_dotset_string(root_object, "Result.Date", ctime(&timep));
			//json_object_set_string(root_object, "Counter-SN", counter->sn);
			json_object_dotset_value(root_object, "Result.Data", sub_value);
			//��json�ַ����л������������һ���ַ����У��������ظ��ַ�����
			char * json_string = (char *)malloc(json_serialization_size(root_value));//json_serialization_size��������ķ���ֵ�Ѿ������˽�ԭ�ַ������Ƚ�����չ����Ϊ��Ҫ����һЩ������ַ����绻�С���б�ܡ������ŵ�
			json_serialize_to_buffer(root_value, json_string, json_serialization_size(root_value));
			//�ͷ�json��Դ
			json_value_free(root_value);//ֻ���ͷŸ���Դ�����ڲ�������������Դ�ᱻ�ݹ��ͷ�

			return json_string;//��ָ������ź���������ʧЧ����ָ��ĸ��ڴ�������ⲿ��ʽ�ͷ�
		}
		else
		{
			//û�л��ﱻ���ۣ��˴����п���Ҫ����һЩ�쳣��Ӧ����Ϣ����ʱֻд��û�л��ﱻ���۵�ʱ���Ӧ��
			time(&timep);
			json_object_dotset_string(root_object, "Result.Res", "0");
			json_object_dotset_string(root_object, "Result.Date", ctime(&timep));
			//json_object_set_string(root_object, "Counter-SN", counter->sn);
			json_object_dotset_value(root_object, "Result.Data", "NULL");//����û�л��ﱻ����
																		 //��json�ַ����л������������һ���ַ����У��������ظ��ַ�����
			char * json_string = (char *)malloc(json_serialization_size(root_value));//json_serialization_size��������ķ���ֵ�Ѿ������˽�ԭ�ַ������Ƚ�����չ����Ϊ��Ҫ����һЩ������ַ����绻�С���б�ܡ������ŵ�
			memset(json_string, 0, json_serialization_size(root_value));
			json_serialize_to_buffer(root_value, json_string, json_serialization_size(root_value));
			//�ͷ�json��Դ
			json_value_free(root_value);//ֻ���ͷŸ���Դ�����ڲ�������������Դ�ᱻ�ݹ��ͷ�

			return json_string;//��ָ������ź���������ʧЧ����ָ��ĸ��ڴ�������ⲿ��ʽ�ͷ�
		}


	}
	return NULL;
}


/*
*	���ܣ�������յ��Ĵ�������
*	������
*	˵����
*/
void Parse_Usart_Data_Run()
{
	while (1)
	{
		while (REC_BUF_LOCKER != LOCKER_FREE) { Sleep(20); }
		REC_BUF_LOCKER = LOCKER_USED;

		for (int i = 0; i < SERIAL_REC_BUF_NODE_NUM; i++)
		{
			if (srb[i].IsUsed == SERIAL_REC_BUF_NODE_USED)//����ڵ㱻ʹ��
			{
				if (srb[i].len < 6)//���ܺ��������䳤�ȶ�������С��6
				{
					char msg[200];
					LogWrite(ERR, "%s", "Baord Com Rec Message Len Error!");
					//printf("rec len error!\r\n");
					//printf("error msg : %s ", msg);
					HexStringFormatForPrintf(srb[i].data, srb[i].len, msg);
					LogWrite(ERR, "%s", msg);
					//printf("\r\n");
					for (int j = 0; j < srb[i].len; j++)
					{
						srb[i].data[j] = 0x00;
					}
					srb[i].len = 0;
					srb[i].IsUsed = SERIAL_REC_BUF_NODE_FREE;
					continue;
				}
				if ( (srb[i].data[0] != '$' && srb[i].data[0] != '@') || srb[i].data[srb[i].len - 1] != '#' )
				{
					char msg[200];
					//printf("data format error!\r\n");
					//printf("error msg : %s ", msg);
					LogWrite(ERR, "%s", "Baord Com Rec Message Format Error!");
					HexStringFormatForPrintf(srb[i].data, srb[i].len, msg);
					LogWrite(ERR, "%s", msg);
					//printf("\r\n");
					for (int j = 0; j < srb[i].len; j++)
					{
						srb[i].data[j] = 0x00;
					}
					srb[i].len = 0;
					srb[i].IsUsed = SERIAL_REC_BUF_NODE_FREE;
					continue;
				}
				if (Sum_Check(&srb[i].data[1], srb[i].len - 3) != srb[i].data[srb[i].len - 2])
				{
					char msg[200];
					//printf("crc error��\r\n");
					//printf("error msg : %s ", msg);
					LogWrite(ERR, "%s", "Baord Com Rec Message Crc Error!");
					HexStringFormatForPrintf(srb[i].data, srb[i].len, msg);
					LogWrite(ERR, "%s", msg);
					//printf("\r\n");
					for (int j = 0; j < srb[i].len; j++)
					{
						srb[i].data[j] = 0x00;
					}
					srb[i].len = 0;
					srb[i].IsUsed = SERIAL_REC_BUF_NODE_FREE;
					continue;
				}
				//���뵽��˴���������У�����ж��๤������������ʼ��������
				if (srb[i].data[0] == '$')//���ذ巵������
				{
					if (srb[i].data[3] == BOARD_CMD_SHAKE)//���ذ�����ָ��
					{
						struct Board_Info * p = board_info;
						while (p != NULL)//��ѯ���г��ذ壬���ȶ�������Ϣ�Ǹ��Ǹ����ذ�ģ�����Ӧ���ذ�״̬��Ϊ���ֳɹ�
						{
							if ((srb[i].data[1] == ((ASCII_To_byte(p->id[0]) << 4) & 0xf0) + ASCII_To_byte(p->id[1])) &&
								(srb[i].data[2] == ((ASCII_To_byte(p->id[2]) << 4) & 0xf0) + ASCII_To_byte(p->id[3])))
							{
								p->Board_Stat = BOARD_STAT_OK;
								//break;
							}

							p = p->next;
						}
						for (int j = 0; j < srb[i].len; j++)
						{
							srb[i].data[j] = 0x00;
						}
						srb[i].len = 0;
						srb[i].IsUsed = SERIAL_REC_BUF_NODE_FREE;

					}
					else if (srb[i].data[3] == BOARD_CMD_BASIC_VALUE)//���ذ�ȥƤָ��
					{
						struct Board_Info * p = board_info;
						while (p != NULL)//��ѯ���г��ذ壬���ȶ�������Ϣ�Ǹ��Ǹ����ذ�ģ�����Ӧ���ذ�״̬��Ϊ���ֳɹ�
						{
							if ((srb[i].data[1] == ((ASCII_To_byte(p->id[0]) << 4) & 0xf0) + ASCII_To_byte(p->id[1])) &&
								(srb[i].data[2] == ((ASCII_To_byte(p->id[2]) << 4) & 0xf0) + ASCII_To_byte(p->id[3])))
							{
								p->IsBasicValue = 1;//ȥƤ���
								//break;
							}

							p = p->next;
						}
						for (int j = 0; j < srb[i].len; j++)
						{
							srb[i].data[j] = 0x00;
						}
						srb[i].len = 0;
						srb[i].IsUsed = SERIAL_REC_BUF_NODE_FREE;

					}
					else if (srb[i].data[3] == BOARD_CMD_BASIC_VALUE_SAVE)//���ذ�ȥƤ����ָ��
					{
						struct Board_Info * p = board_info;
						while (p != NULL)//��ѯ���г��ذ壬���ȶ�������Ϣ�Ǹ��Ǹ����ذ�ģ�����Ӧ���ذ�״̬��Ϊ���ֳɹ�
						{
							if ((srb[i].data[1] == ((ASCII_To_byte(p->id[0]) << 4) & 0xf0) + ASCII_To_byte(p->id[1])) &&
								(srb[i].data[2] == ((ASCII_To_byte(p->id[2]) << 4) & 0xf0) + ASCII_To_byte(p->id[3])))
							{
								p->IsBasicValueSave = 1;//ȥƤ���
													//break;
							}

							p = p->next;
						}
						for (int j = 0; j < srb[i].len; j++)
						{
							srb[i].data[j] = 0x00;
						}
						srb[i].len = 0;
						srb[i].IsUsed = SERIAL_REC_BUF_NODE_FREE;

					}
					else if (srb[i].data[3] == BOARD_CMD_CURVATURE)//���ذ�У׼����
					{
						struct Board_Info * p = board_info;
						while (p != NULL)//��ѯ���г��ذ壬���ȶ�������Ϣ�Ǹ��Ǹ����ذ�ģ�����Ӧ���ذ�״̬��Ϊ���ֳɹ�
						{
							if ((srb[i].data[1] == ((ASCII_To_byte(p->id[0]) << 4) & 0xf0) + ASCII_To_byte(p->id[1])) &&
								(srb[i].data[2] == ((ASCII_To_byte(p->id[2]) << 4) & 0xf0) + ASCII_To_byte(p->id[3])))
							{
								p->ISCurvatureValue = 1;//У׼�ɹ�
								p->Curvature.x[0] = srb[i].data[4];
								p->Curvature.x[1] = srb[i].data[5];
								p->Curvature.x[2] = srb[i].data[6];
								p->Curvature.x[3] = srb[i].data[7];
							}

							p = p->next;
						}
						for (int j = 0; j < srb[i].len; j++)
						{
							srb[i].data[j] = 0x00;
						}
						srb[i].len = 0;
						srb[i].IsUsed = SERIAL_REC_BUF_NODE_FREE;

					}
					else if (srb[i].data[3] == BOARD_CMD_CURVATURE_SAVE)//���ذ�У׼��������
					{
						struct Board_Info * p = board_info;
						while (p != NULL)//��ѯ���г��ذ壬���ȶ�������Ϣ�Ǹ��Ǹ����ذ�ģ�����Ӧ���ذ�״̬��Ϊ���ֳɹ�
						{
							if ((srb[i].data[1] == ((ASCII_To_byte(p->id[0]) << 4) & 0xf0) + ASCII_To_byte(p->id[1])) &&
								(srb[i].data[2] == ((ASCII_To_byte(p->id[2]) << 4) & 0xf0) + ASCII_To_byte(p->id[3])))
							{
								p->ISCurvatureValueSave = 1;//У׼�ɹ�
								p->Curvature.x[0] = srb[i].data[4];
								p->Curvature.x[1] = srb[i].data[5];
								p->Curvature.x[2] = srb[i].data[6];
								p->Curvature.x[3] = srb[i].data[7];
							}

							p = p->next;
						}
						for (int j = 0; j < srb[i].len; j++)
						{
							srb[i].data[j] = 0x00;
						}
						srb[i].len = 0;
						srb[i].IsUsed = SERIAL_REC_BUF_NODE_FREE;

					}
					else if (srb[i].data[3] == BOARD_CMD_GET_WEIGHT)//�������ذ����������������
					{
						struct Board_Info * p = board_info;
						while (p != NULL)//��ѯ���г��ذ壬���ȶ�������Ϣ�Ǹ��Ǹ����ذ�ģ�����Ӧ���ذ�״̬��Ϊ���ֳɹ�
						{
							if ((srb[i].data[1] == ((ASCII_To_byte(p->id[0]) << 4) & 0xf0) + ASCII_To_byte(p->id[1])) &&
								(srb[i].data[2] == ((ASCII_To_byte(p->id[2]) << 4) & 0xf0) + ASCII_To_byte(p->id[3])))
							{
								p->board_items_weight_all_after_close_door = ((srb[i].data[4] << 8) & 0xff00) + (srb[i].data[5] & 0x00ff);
							}

							p = p->next;
						}
						for (int j = 0; j < srb[i].len; j++)
						{
							srb[i].data[j] = 0x00;
						}
						srb[i].len = 0;
						srb[i].IsUsed = SERIAL_REC_BUF_NODE_FREE;

					}
					else
					{
						//������������δʶ�������
						char msg[200];
						HexStringFormatForPrintf(srb[i].data, srb[i].len, msg);
						printf("board other msg : %s\r\n", msg);
						for (int j = 0; j < srb[i].len; j++)
						{
							srb[i].data[j] = 0x00;
						}
						srb[i].len = 0;
						srb[i].IsUsed = SERIAL_REC_BUF_NODE_FREE;
					}
				}
				else if (srb[i].data[0] == '@')
				{
					if (srb[i].data[3] == LOCKER_CMD_STATUS)//��ȡ��״̬
					{
						struct counter_info * p = counter;
						if (p != NULL)//��ѯ���г��ذ壬���ȶ�������Ϣ�Ǹ��Ǹ����ذ�ģ�����Ӧ���ذ�״̬��Ϊ���ֳɹ�
						{
							if ((srb[i].data[1] == ((ASCII_To_byte(p->locker_id[0]) << 4) & 0xf0) + ASCII_To_byte(p->locker_id[1])) &&
								(srb[i].data[2] == ((ASCII_To_byte(p->locker_id[2]) << 4) & 0xf0) + ASCII_To_byte(p->locker_id[3])))
							{
								
								if (srb[i].data[4] == 0xFF) { printf("locker error\r\n"); }
								else if (srb[i].data[4] == 0x02) { printf("locker closed\r\n"); }
								else if (srb[i].data[4] == 0x01) { printf("locker opening\r\n"); }
								else if (srb[i].data[4] == 0x04) { printf("locker waiting for open or close\r\n"); }
								
								counter->locker_stat = srb[i].data[4];
							}
						}
						for (int j = 0; j < srb[i].len; j++)
						{
							srb[i].data[j] = 0x00;
						}
						srb[i].len = 0;
						srb[i].IsUsed = SERIAL_REC_BUF_NODE_FREE;
					}
					else if (srb[i].data[3] == LOCKER_CMD_UNLOCK)//��ȡִ�п��ź�����ķ�������
					{
						struct counter_info * p = counter;
						if (p != NULL)//��ѯ���г��ذ壬���ȶ�������Ϣ�Ǹ��Ǹ����ذ�ģ�����Ӧ���ذ�״̬��Ϊ���ֳɹ�
						{
							if ((srb[i].data[1] == ((ASCII_To_byte(p->locker_id[0]) << 4) & 0xf0) + ASCII_To_byte(p->locker_id[1])) &&
								(srb[i].data[2] == ((ASCII_To_byte(p->locker_id[2]) << 4) & 0xf0) + ASCII_To_byte(p->locker_id[3])))
							{
								counter->locker_stat = srb[i].data[4];
							}
						}
						for (int j = 0; j < srb[i].len; j++)
						{
							srb[i].data[j] = 0x00;
						}
						srb[i].len = 0;
						srb[i].IsUsed = SERIAL_REC_BUF_NODE_FREE;
					}
					else
					{
						//������������δʶ�������
						char msg[200];
						LogWrite(WARN, "%s", "Baord Com Rec Message UNKNOWN!");
						HexStringFormatForPrintf(srb[i].data, srb[i].len, msg);
						LogWrite(WARN, "%s", msg);
						//printf("locker other msg : %s\r\n", msg);
						for (int j = 0; j < srb[i].len; j++)
						{
							srb[i].data[j] = 0x00;
						}
						srb[i].len = 0;
						srb[i].IsUsed = SERIAL_REC_BUF_NODE_FREE;
					}
					
				}


			}//���ϴ���Ϊif���ܻ�������ĳһ���ڵ㱻ʹ�õ�ʱ��Ž���
		}


		REC_BUF_LOCKER = LOCKER_FREE;
		Sleep(500);
	}

}



/*
*	���ܣ�����ذ��������������
*	������
			[in] id �豸���
			[in] cmd ������
			[in] cmd_data ��������
			[in] cmd_data_len �������ݳ���
			[in] dev_kind Ŀ���豸����  1Ϊ���ذ� 2Ϊ��
			[in] little �Ƿ��cmd_data���д�С��ת��  0Ϊ��ת��  1Ϊת��  ע���˴���ָcmd_data�ķ���˳������С���� �����ɴ���С
*	˵����Ŀǰ����û�з���ֵ�����պ�������
*/

void Send_CMD(HANDLE  * hCom , unsigned char * id , unsigned char CMD , unsigned char * cmd_data , int cmd_data_len , char dev_kind , int little)
{
	unsigned char send_buf[100];
	int send_buf_len = 0;
	if (dev_kind == 1)//����ذ巢������
	{
		send_buf[send_buf_len++] = '$';
	}
	else if (dev_kind == 2)//��������������
	{
		send_buf[send_buf_len++] = '@';
	}
	else
	{
		//�豸���ͱ������
	}
	send_buf[send_buf_len++] = ((ASCII_To_byte(id[0]) << 4) & 0xf0) + ASCII_To_byte(id[1]);
	send_buf[send_buf_len++] = ((ASCII_To_byte(id[2]) << 4) & 0xf0) + ASCII_To_byte(id[3]);
	send_buf[send_buf_len++] = CMD;
	if(little == 0)
	{
		for (int i = 0; i < cmd_data_len; i++)
		{
			send_buf[send_buf_len++] = cmd_data[i];
		}
	}
	else if (little == 1)
	{
		for (int i = cmd_data_len - 1; i >= 0; i--)
		{
			send_buf[send_buf_len++] = cmd_data[i];
		}
	}
	
	send_buf[send_buf_len] = Sum_Check(&send_buf[1], send_buf_len - 1);
	send_buf_len++;
	send_buf[send_buf_len++] = '#';
	WriteChar(* hCom ,send_buf, send_buf_len);

}

/*
*	���ܣ���У��
*	������
*	˵����
*/

unsigned char Sum_Check(unsigned char * data, int len)
{
	char buf = 0x00;
	for (int i = 0; i < len; i++)
	{
		buf += data[i];
	}
	return buf;
}

/*
*	���ܣ�char �ַ����ڲ�����ΪASCII��ʽ��ʮ�������ַ��� AB��Ϊ  0x41  0x42�� ת��λ  byte��ʽ�� ��0xAB
*	������
*	˵����charָ��ı���Ϊ���ֵ�ascii������Ϊ����,
*/

byte ASCII_To_byte(unsigned char buf)
{
	if (buf >= '0' && buf <= '9')
	{
		return (buf - '0');
	}
	else if (buf >= 'A' && buf <= 'F')
	{
		return (buf - 'A' + 10);
	}
	else if (buf >= 'a' && buf <= 'f')
	{
		return (buf - 'a' + 10);
	}
}

/*
*	���ܣ�char �ַ� �����ַ��� ת double��
*	������
*	˵����
*/

double CharNum_To_Double( const unsigned char * data)
{
	char * p = data;
	double value = 0;
	double value_dec = 0;
	int sw = 0;//����С���л�
	int i = 0;
	while (*p != '\0')
	{
		if (*p >= '0' && *p <= '9' && sw == 0)
		{
			value = value * 10 + (*p - '0');
		}
		else if (*p == '.')
		{
			if(sw == 0)
			{
				sw = 1;
			}
			else
			{
				//���ܳ�������С����
				return 0;
			}
			
		}
		else if (*p >= '0' && *p <= '9' && sw == 1)
		{
			value_dec = value_dec * 10 + (*p - '0');
			i++;
		}
		else
		{
			//���뵽��˴������ַ��������з�double������
			return 0;
		}
		p++;
	}

	value = (value + value_dec / pow(10,i));
	return value;
}

/*
*	���ܣ�double ����ת char ����
*	������
*	˵����С���㱣����λ,����λ��������,�����������ã�Ҫ��֤�������ݵ����鳤���㹻�ã��������Զ���ĩβ��ӽ�����ʶ��'\0'
*/

void Double_To_CharArray(double num, unsigned char * data)
{
	unsigned char buf[20];
	unsigned char temp;
	int num_buf = abs(num * 1000);//������������ͳһ�ĳ�����
	if ((num_buf % 10) >= 5) { num_buf += 10; }
	num_buf /= 10;
	int num_len = 1;
	if (num_buf == 0)
	{
		//��������������0,��ֱ�Ӹ�ֵ������
		data[0] = '0';
		data[1] = '.';
		data[2] = '0';
		data[3] = '0';
		data[4] = '\0';
		return;
	}
	while (num_buf / (pow(10, num_len)) >= 1)
	{
		num_len++;
	}
	
	for (int i = 0 ; i < num_len; i++)
	{
		buf[i] = num_buf % 10;
		buf[i] += '0';
		num_buf /= 10;
	}
	//ǰ��˳��ߵ�
	for (int i = 0, j = num_len - 1; i < j; i++, j--)
	{
		temp = buf[i];
		buf[i] = buf[j];
		buf[j] = temp;
	}
	//���˴�������С���㱣����λ�ķ�ʽ��num_len��ʱ����Ϊ3������������1λС����Ϊ������num_len��СӦ��Ϊ4λ
	if (num_len < 3)
	{
		for (int i = 2; i >= 0; i--)
		{
			if (num_len == 0)
			{
				buf[i] = '0';
			}
			else
			{
				buf[i] = buf[--num_len];
			}
			
		}
		num_len = 3;
	}
	/*
	if (num >= 0.1 && num < 1)
	{
		//��������double����С��1����˴���ָ�������ַ�����??��,�����ʺ�ΪС�����������λ���֣���˴˴�Ҫ�ڸ��ַ�����ͷ��һ��0�ַ�
		//����ת������������С�������û������
		for (int i = num_len; i > 0; i--)
		{
			buf[i] = buf[i - 1];
		}
		buf[0] = '0';//���ӿ�ͷ��0�ַ�
	}
	*/
	buf[num_len] = buf[num_len - 1];
	buf[num_len-1] = buf[num_len - 2];
	buf[num_len - 2] = '.';//��ԭ����ĵ����ڶ���λ�������С���㣬�����ַ�������һλ�����ȼ�1
	num_len++;//�������鳤�ȼ�1

	if (num < 0)
	{
		data[0] = '-';//��ԭ��С���㣬�����Ӹ���
		data[num_len + 1] = '\0';
	}
	else
	{
		data[num_len] = '\0';
	}
		
	for (int i = 0; i < num_len; i++)//ע��˴���С����num_len����ΪҪ��һ��С�������Ҫ��һλ
	{
		if (num < 0)
		{
			data[i+1] = buf[i];
		}
		else
		{
			data[i] = buf[i];
		}
	}

}

/*
*	���ܣ���������ת char ����
*	������
*	˵�����������Ϊ���������֣���ֱ��ȥ��С��,�����������ã�Ҫ��֤�������ݵ����鳤���㹻�ã��������Զ���ĩβ��ӽ�����ʶ��'\0'
*/

void Int_To_CharArray(int num, unsigned char * data)
{
	unsigned char buf[20];
	unsigned char temp;
	int num_buf = abs(num);//������������ͬ��ĳ�����
	int num_len = 1;
	while (num_buf / (pow(10, num_len)) >= 1)
	{
		num_len++;
	}

	for (int i = 0; i < num_len; i++)
	{
		buf[i] = num_buf % 10;
		buf[i] += '0';
		num_buf /= 10;
	}
	for (int i = 0, j = num_len - 1; i < j; i++, j--)
	{
		temp = buf[i];
		buf[i] = buf[j];
		buf[j] = temp;
	}
	
	if (num < 0)
	{
		data[0] = '-';//��ԭ��С���㣬�����Ӹ���
		data[num_len + 1] = '\0';
	}
	else
	{
		data[num_len] = '\0';
	}

	for (int i = 0; i < num_len; i++)
	{
		if (num < 0)
		{
			data[i + 1] = buf[i];
		}
		else
		{
			data[i] = buf[i];
		}
	}

}

/*
*	���ܣ�hex�ַ������������ֽ�Ϊһ��hex�����и�ʽ��������byte����
*	������
*	˵����

*/

void HexStringFormatForPrintf(unsigned char * in , int in_len , unsigned char * out)
{
	unsigned char buf[200];
	unsigned char * p = in;
	int k = 0;
	int out_len = 0;

	for (int i = 0; i < in_len; i++)
	{
		if (*p != ' ')
		{
			buf[out_len++] = *p;
		}
		p++;
	}

	buf[out_len] = '\0';

	for (int j = 0; j < out_len; j++)
	{
		/*
		if (buf[j] >= 0x00 && buf[j] <= 0x09)
		{
			out[k++] = buf[j] + '0';
		}
		*/
		/*
		else if (buf[j] >= 0x0A && buf[j] <= 0x0F)
		{
			out[k++] = buf[j] - 10 + 'A';
		}
		*/
		if (buf[j] == '$' || buf[j] == '@' || buf[j] == '#')
		{
			out[k++] = buf[j];
		}
		else
		{
			out[k++] = (buf[j] >> 4 & 0x0f) <= 9 ? (buf[j] >> 4 & 0x0f) + '0' : (buf[j] >> 4 & 0x0f) - 10 + 'A';
			out[k++] = (buf[j] & 0x0f) <= 9 ? (buf[j] & 0x0f) + '0' : (buf[j] & 0x0f) - 10 + 'A';
		}
	}
	out[k] = '\0';
}


/*
*	���ܣ�GBKתutf8
*	������[in] GBK �����ַ���
*		  [out] UTF8 �����ַ���
*		  [in] UTF8 �����ַ������ȣ�ֻҪ��֤�����ܹ��н���������ݼ���
*	˵����
*/
int GBKToUTF8(unsigned char * lpGBKStr, unsigned char * lpUTF8Str, int nUTF8StrLen)
{

	wchar_t * lpUnicodeStr = NULL;

	int nRetLen = 0;

	if (!lpGBKStr) //���GBK�ַ���ΪNULL������˳�
	{
		return 0;
	}

	nRetLen = MultiByteToWideChar(CP_ACP, 0, (char *)lpGBKStr, -1, NULL, NULL); //��ȡת����Unicode���������Ҫ���ַ��ռ䳤��

	lpUnicodeStr = (wchar_t *)malloc(sizeof(wchar_t) * (nRetLen + 1));	//new WCHAR[nRetLen + 1]; //ΪUnicode�ַ����ռ�

	nRetLen = MultiByteToWideChar(CP_ACP, 0, (char *)lpGBKStr, -1, lpUnicodeStr, nRetLen); //ת����Unicode����

	if (!nRetLen) //ת��ʧ��������˳�
	{
		return 0;
	}
	nRetLen = WideCharToMultiByte(CP_UTF8, 0, lpUnicodeStr, -1, NULL, 0, NULL, NULL); //��ȡת����UTF8���������Ҫ���ַ��ռ䳤��

	if (!lpUTF8Str) //���������Ϊ���򷵻�ת������Ҫ�Ŀռ��С
	{
		if (lpUnicodeStr)
		{
			free(lpUnicodeStr);
		}
		return nRetLen;

	}

	if (nUTF8StrLen < nRetLen) //���������������Ȳ������˳�

	{
		if (lpUnicodeStr)
		{
			free(lpUnicodeStr);
		}
		return 0;
	}

	nRetLen = WideCharToMultiByte(CP_UTF8, 0, lpUnicodeStr, -1, (char *)lpUTF8Str, nUTF8StrLen, NULL, NULL); //ת����UTF8����

	if (lpUnicodeStr)//ת����ϣ������ͷŸ���Դ
	{
		free(lpUnicodeStr);
	}
	return nRetLen;

}

/*
*	���ܣ�utf8תGBK
*	������[in] UTF8 �����ַ���
*		  [out] GBK �����ַ���
*		  [in] GBK �����ַ������ȣ�ֻҪ��֤�����ܹ��н���������ݼ���
*	˵����
*/
int UTF8ToGBK(unsigned char * lpUTF8Str, unsigned char * lpGBKStr, int nGBKStrLen)

{

	wchar_t * lpUnicodeStr = NULL;
	int nRetLen = 0;

	if (!lpUTF8Str) //���UTF8�ַ���ΪNULL������˳�
	{
		return 0;
	}

	nRetLen = MultiByteToWideChar(CP_UTF8, 0, (char *)lpUTF8Str, -1, NULL, NULL); //��ȡת����Unicode���������Ҫ���ַ��ռ䳤��

	lpUnicodeStr = (wchar_t *)malloc(sizeof(wchar_t) * (nRetLen + 1));//new WCHAR[nRetLen + 1]; //ΪUnicode�ַ����ռ�

	nRetLen = MultiByteToWideChar(CP_UTF8, 0, (char *)lpUTF8Str, -1, lpUnicodeStr, nRetLen); //ת����Unicode����
	if (!nRetLen) //ת��ʧ��������˳�
	{
		return 0;
	}

	nRetLen = WideCharToMultiByte(CP_ACP, 0, lpUnicodeStr, -1, NULL, NULL, NULL, NULL); //��ȡת����GBK���������Ҫ���ַ��ռ䳤��

	if (!lpGBKStr) //���������Ϊ���򷵻�ת������Ҫ�Ŀռ��С
	{
		if (lpUnicodeStr)
		{
			free(lpUnicodeStr);
		}
		return nRetLen;
	}

	if (nGBKStrLen < nRetLen) //���������������Ȳ������˳�
	{
		if (lpUnicodeStr)
		{
			free(lpUnicodeStr);
		}
		return 0;
	}

	nRetLen = WideCharToMultiByte(CP_ACP, 0, lpUnicodeStr, -1, (char *)lpGBKStr, nRetLen, NULL, NULL); //ת����GBK����

	if (lpUnicodeStr)
	{
		free(lpUnicodeStr);
	}
		
	return nRetLen;
}


/*��ʱ���Ժ���-----------------------------------------------------------*/
int is_valid_utf8(const char *string, size_t string_len)
{
	int len = 0;
	const char *string_end = string + string_len;
	while (string < string_end) {
		if (!verify_utf8_sequence((const unsigned char*)string, &len)) {
			return 0;
		}
		string += len;
	}
	return 1;
}
#define IS_CONT(b) (((unsigned char)(b) & 0xC0) == 0x80) /* is utf-8 continuation byte */
static int verify_utf8_sequence(const unsigned char *string, int *len) {
	unsigned int cp = 0;
	*len = num_bytes_in_utf8_sequence(string[0]);

	if (*len == 1) {
		cp = string[0];
	}
	else if (*len == 2 && IS_CONT(string[1])) {
		cp = string[0] & 0x1F;
		cp = (cp << 6) | (string[1] & 0x3F);
	}
	else if (*len == 3 && IS_CONT(string[1]) && IS_CONT(string[2])) {
		cp = ((unsigned char)string[0]) & 0xF;
		cp = (cp << 6) | (string[1] & 0x3F);
		cp = (cp << 6) | (string[2] & 0x3F);
	}
	else if (*len == 4 && IS_CONT(string[1]) && IS_CONT(string[2]) && IS_CONT(string[3])) {
		cp = string[0] & 0x7;
		cp = (cp << 6) | (string[1] & 0x3F);
		cp = (cp << 6) | (string[2] & 0x3F);
		cp = (cp << 6) | (string[3] & 0x3F);
	}
	else {
		return 0;
	}

	/* overlong encodings */
	if ((cp < 0x80 && *len > 1) ||
		(cp < 0x800 && *len > 2) ||
		(cp < 0x10000 && *len > 3)) {
		return 0;
	}

	/* invalid unicode */
	if (cp > 0x10FFFF) {
		return 0;
	}

	/* surrogate halves */
	if (cp >= 0xD800 && cp <= 0xDFFF) {
		return 0;
	}

	return 1;
}

static int num_bytes_in_utf8_sequence(unsigned char c) {
	if (c == 0xC0 || c == 0xC1 || c > 0xF4 || IS_CONT(c)) {
		return 0;
	}
	else if ((c & 0x80) == 0) {    /* 0xxxxxxx */
		return 1;
	}
	else if ((c & 0xE0) == 0xC0) { /* 110xxxxx */
		return 2;
	}
	else if ((c & 0xF0) == 0xE0) { /* 1110xxxx */
		return 3;
	}
	else if ((c & 0xF8) == 0xF0) { /* 11110xxx */
		return 4;
	}
	return 0; /* won't happen */
}

/*��ʱ���Ժ�����-------------------------------------------------*/


/*
*	���º�����ϵͳ�����򻯺�����õĺ���---------------------------------------------------------------------------------------------------
*/


/*
*	���ܣ��γ�����������͵�Ӧ�����Ϣ
*	������	[in] ���յ�����Ϣ���ţ�������ʱ�����ʵ�֣�
			[in] �������ƣ������
			[in] ����ִ�еĽ��
			[in] Ҫ���ص����������飬��ֵӦ����һ���Ѿ����õ����飬ÿһ��Ԫ�ض�Ϊ���飬��[ [] , [] , [] , [] , [] , [] ]
			[out]����Ϣ���ظ����ú���������json����Ҫ�ɵ��ú�����ʾ�ͷ�
*	˵����
*/
char *  Procedure_Answer_Message(char * message_sn , char * cmd_name , int Res , JSON_Value *sub_value)
{
	JSON_Value *root_value = json_value_init_object();//��ʼ����һ��value��������Ϊobject��valueΪjson���ݵ�һ�����ͣ���ͬ��value���Ϳ���Ƕ��
	JSON_Object *root_object = json_value_get_object(root_value);//��ȡvalue�е�object�ĵ�ַ
	char * change_code[100] = { 0 };
	//JSON_Value *sub_value = json_value_init_array();
	//JSON_Array *sub_array = json_value_get_array(sub_value);

	time_t timep;
	time(&timep);
	json_object_set_string(root_object, "MSN", message_sn);
	json_object_set_string(root_object, "devid", counter->sn);
	json_object_set_string(root_object, "cmdid", cmd_name);
	Int_To_CharArray(Res, change_code);
	json_object_dotset_string(root_object, "Result.Res", change_code);
	json_object_dotset_string(root_object, "Result.Date", ctime(&timep));
	json_object_dotset_value(root_object, "Result.Data", sub_value);
	//�����ڴ������ͨ��smb.messageָ��ָ�򣬲����ڷ��ͳɹ����ͷ�
	char * json_string = (char *)malloc(json_serialization_size(root_value));//json_serialization_size��������ķ���ֵ�Ѿ������˽�ԭ�ַ������Ƚ�����չ����Ϊ��Ҫ����һЩ������ַ����绻�С���б�ܡ������ŵ�
	json_serialize_to_buffer(root_value, json_string, json_serialization_size(root_value));
	
	//������ݿ�
	
	SQL_INSERT_INTO_Up_Message(message_sn, json_string, timep);

	//�ͷ�json��Դ
	json_value_free(root_value);//ֻ���ͷŸ���Դ�����ڲ�������������Դ�ᱻ�ݹ��ͷ�
	return json_string;//��ָ������ź���������ʧЧ����ָ��ĸ��ڴ�������ⲿ��ʽ�ͷ�

}

/*
*	���ܣ����һ�ο�����
*	������[out]ִ�н��
*	˵����ִ���꿪�������ȴ�10���ӣ�Ȼ�󷵻ؿ��������ķ���ֵ
*/
char *  Procedure_Open_Lock(JSON_Object * json_object)
{

	int res = Locker_Open();
	return Procedure_Answer_Message(json_object_get_string(json_object, "MSN"), "Unlock", res, NULL);

}

/*
*	���ܣ�ִ��һ�ο�����
*	������[out]ֱ���Źػ���һ������ʱ����
*	˵������ִ�п���������Ȼ��ȴ�10���ӣ���ִ�л�ȡ��״̬�����ÿ�����ж�һ�η���ֵ�Ƿ�Ϊ�Źرգ�����������ظ���ȡ��״̬������ʱ�䲻����1���ӣ����ս���״̬����
*/
char *  Procedure_Open_Close(JSON_Object * json_object)
{

	int res = Locker_Open_Closed();
	return Procedure_Answer_Message(json_object_get_string(json_object, "MSN"), "Unlock_Close", res, NULL);

}

/*
*	���ܣ���ȡ��״̬
*	������[out]��ȡ����״̬
*	˵������������״̬
*/
char *  Procedure_Get_Locker_State(JSON_Object * json_object)
{

	int res = Locker_Get_Stat();
	return Procedure_Answer_Message(json_object_get_string(json_object, "MSN"), "Locker_State", res, NULL);

}

/*
*	���ܣ����ݳ��ذ��ţ�ִ��һ��ȥƤ
*	������
*	˵������ָ�����ذ�ִ������ȥƤ
*/
char * Procedure_Basic_Value_Set(JSON_Object * json_object)
{
	//��ȡ���ذ���
	char * board_id = json_object_get_string(json_object, "BN");
	int res = -1;

	if (board_id != NULL)
	{
		res = Board_Basic_Value_Set_By_id(board_id, 2);
	}

	return Procedure_Answer_Message(json_object_get_string(json_object, "MSN") , "Basic_Value", res, NULL);

}

/*
*	���ܣ�ȥƤУ׼���ұ���
*	������[in] ���ذ���
*		  [in] ȥƤ����
*		  [out] ִ�н��
*	˵���������ڵ���ʱ��board_info�ṹ���е�IsBasicVale�趨Ϊ0����ΪδȥƤ��Ȼ��ִ��ȥƤ�����ȴ�һ��ʱ����ж��Ƿ�ȥƤ�ɹ�
*/
int Board_Basic_Value_Set_By_id(char * board_id , int times)
{
	struct Board_Info * p = board_info;
	int boards_num = 0;
	printf("�ȴ�ȥƤ���");
	while (p != NULL)
	{

		if (strcmp(p->id, board_id) == 0)
		{
			p->IsBasicValue = 0;//�����Ƿ��Ѿ�ȥ��Ƥ���趨ΪδȥƤ״̬
			p->IsBasicValueSave = 0;//ȥƤδ����
			for (int i = 0; i < times; i++)
			{
				printf("...");
				Send_CMD(&hCom_C, p->id, BOARD_CMD_BASIC_VALUE, NULL, 0, 1, 0);
				Sleep(5000);
			}
			printf("...");
			Send_CMD(&hCom_C, p->id, BOARD_CMD_BASIC_VALUE_SAVE, NULL, 0, 1, 0);
			Sleep(1000);
			if (p->IsBasicValue == 0 || p->IsBasicValueSave == 0)
			{
				//ȥƤʧ��
				printf("\r\nȥƤʧ�ܣ���\r\n");
				return GEN_ERROR_COMMUNICATION_ERROR;
			}
			else
			{
				//ȥƤ�ɹ�
				printf("\r\n���ذ� �� %s  ȥƤ�ɹ�����\r\n" , p->id);
				return GEN_OK;
			}
		}

		p = p->next;
	}
	if (p == NULL)
	{
		printf("  ���ذ��Ŵ���\r\n");
		return SETTING_BASIC_VALUE_BOARD_ERROR;
	}
	return GEN_MEM_OVER;//����Ӧ����Զ����ﵽ�˴�,������˴�������ڴ����洦�ķ���

}


/*
*	���ܣ��趨ָ�����ذ�����ֵ
*	������[out]ִ�н��
*	˵������������״̬
*/
char *  Procedure_Set_Curavture_Value(JSON_Object * json_object)
{
	//��ȡ���ذ���
	char * board_id = json_object_get_string(json_object, "BN");
	char * str_weight = json_object_get_string(json_object, "Weight");//У׼����ֵ
	//char * ischecktem = json_object_get_string(json_object, "Check_Tem");
	char * save = json_object_get_string(json_object, "Save");

	//(int)CharNum_To_Double(ischecktem) /*0Ϊ������ 1Ϊ����*/,
	int res = Board_Curavture_Value_Set_Ex(board_id, (UINT16)CharNum_To_Double(str_weight), 2,  (int)CharNum_To_Double(save));
	JSON_Value *sub_value = json_value_init_array();
	JSON_Array *sub_array = json_value_get_array(sub_value);
	if (res == 0)
	{
		unsigned char * change_code[20];
		struct Board_Info * p = board_info;
		while (p != NULL)
		{

			if (strcmp(p->id, board_id) == 0)
			{
				Double_To_CharArray(p->Curvature.f, change_code);
				json_array_append_string(sub_array, change_code);
				break;
			}
			p = p->next;
		}
		return Procedure_Answer_Message(json_object_get_string(json_object, "MSN"), "Curavture_Value", res, sub_value);
	}
	return Procedure_Answer_Message(json_object_get_string(json_object, "MSN"), "Curavture_Value", res, NULL);

}

/*
*	���ܣ�����У׼
*	������
		  [in] ���ذ���
		  [in] У׼������ֵ
		  [in] У׼����
		  [in] �Ƿ񱣴�����ֵ 0 ������  1����
*	˵����Ŀǰδ�Է���ֵ�����κδ���
*		  
*		  fixed�����¶��ж��Ӵ˺������Ƴ�����Ϊ�˺���ΪУ׼һ���豸����������¶��ж���������У׼ĳһ�����ذ��ϵͳ�¶�У׼����������������ж������������ذ嶼����У׼��bug
*
*/
int Board_Curavture_Value_Set_Ex(char * board_id , UINT16 weight , int times , int save)
{

	struct Board_Info * p = board_info;

	//if (IsCheck_Tem)
	//{
	//	if (!sys_tem.IsCheck)
	//	{
	//		//�¶�û�б仯����У׼
	//		printf("���һ�β����¶�Ϊ %d ���϶� �� ϵͳ��ʼ�¶�Ϊ %d ���϶ȣ�����У׼\r\n", sys_tem.Tem_Cur, sys_tem.Tem);
	//		return 1;
	//	}
	//	else
	//	{
	//		//�¶ȱ仯��ʱ������У׼Ҫ���Ƚ��¶Ƚṹ�����ݽ��е���
	//		sys_tem.Tem = sys_tem.Tem_Cur;
	//		sys_tem.Time = 0;
	//		sys_tem.IsCheck = 0;
	//	}
	//}

	while (p != NULL)
	{

		if (strcmp(p->id, board_id) == 0)
		{

			if (weight < 1000)//���У׼�Ļ�������С��1000�ˣ���ִ��У׼���Ƿ���ʻ���Ҫ����
			{
				//printf("У׼�������-- %d  ������ԭ�����ʲ���\r\n", p->board_items_weight_all);
				printf("���ذ� %s У׼�������ᣬС��1000�� , �޷�У׼\r\n", p->id );
				return SETTING_CURAVTURE_VALUE_TOO_LIGHT;
			}

			p->ISCurvatureValue = 0;//���ó�δУ׼״̬
			p->ISCurvatureValueSave = 0;//���ó�δ����״̬
			for (int i = 0; i < times; i++)
			{
				//Send_CMD(&hCom_C, p->id, BOARD_CMD_CURVATURE, (unsigned char *)&p->board_items_weight_all, 2, 1, 1);
				Send_CMD(&hCom_C, p->id, BOARD_CMD_CURVATURE, (unsigned char *)&weight, 2, 1, 1);
				printf("...");
				Sleep(5000);
			}

			if (p->ISCurvatureValue == 0)
			{
				//У׼ʧ��
				printf("\r\nУ׼ʧ�ܣ���\r\n");
				return GEN_ERROR_COMMUNICATION_ERROR;
			}

			if (save == 1)
			{
				Send_CMD(&hCom_C, p->id, BOARD_CMD_CURVATURE_SAVE, (unsigned char *)&weight, 2, 1, 1);
				printf("...");
				Sleep(1000);
				if (p->ISCurvatureValueSave == 0)
				{
					//У׼ʧ��
					printf("\r\nУ׼����ʧ�ܣ���\r\n");
					return GEN_ERROR_COMMUNICATION_ERROR;
					//getchar(1);
					//sys_die("\r\nУ׼����ʧ�ܣ���\r\n");
				}
				else
				{
					printf("���ذ� %s ����У׼��ɣ�\r\n" , p->id);
					p->ISNeedCur = ISNEEDCUR_NO;
					return GEN_OK;
				}
			}
			else
			{
				printf("���ذ� %s ������У׼��ɣ�\r\n", p->id);
				p->ISNeedCur = ISNEEDCUR_NO;
				return GEN_OK;
			}

		}

		p = p->next;

	}
	if (p == NULL)
	{
		printf("  ���ذ��Ŵ���\r\n");
		return SETTING_CURAVTURE_VALUE_BOARD_ERROR;
	}
	return GEN_MEM_OVER;//����Ӧ����Զ����ﵽ�˴�,������˴�������ڴ����洦�ķ���

}

/*
*	���ܣ���ȡ���ذ�״̬��������ź��Ƿ���ҪУ׼
*	������
*	˵�����ú����������ڻ�ȡ�����ж��ٸ����ذ岢�ұ���Ƕ��٣�Ҳ�����ڻ�ȡ���ذ��Ƿ���ҪУ׼�Ļ���
*/
char * Procedure_Get_Board_State(void)
{
	//���������õ�json����
	JSON_Value *sub_value = json_value_init_array();
	JSON_Array *sub_array = json_value_get_array(sub_value);
	struct Board_Info * p = board_info;
	unsigned char * change_code[20];
	while (p != NULL)
	{
		json_array_append_string(sub_array, p->id);//��ӳ��ذ���
		Int_To_CharArray(p->ISNeedCur, change_code);
		json_array_append_string(sub_array, change_code);//��ӳ��ذ��Ƿ���ҪУ׼״̬
		p = p->next;
	}
	return Procedure_Answer_Message(json_object_get_string(json_object, "MSN") , "Board_State", 0, sub_value);//�������������ʧ�ܣ����res�̶�Ϊ0

}



/*
*	���ܣ��趨ָ�����ذ����
*	������[out]ִ�н��
*	˵����res����ֵ�����ó��غ�������ֵ�����ۼӵ�
*		  �ú���ֻ�ǳ��أ����շ��͹����ĳ��ذ��Ž��г��أ�������У׼
*		  
*/
char *  Procedure_Get_Weight_Value(JSON_Object * json_object)
{
	//ÿһ�����ذ嶼���������ݣ�һ���Ǳ�ţ�һ����������ذ��ϵĻ�������
	JSON_Array * sub_array_parse = json_object_get_array(json_object, "Data");
	//JSON_Array  * sub_sub_array_parse = NULL;

	//���������õ�json����
	JSON_Value *sub_value = json_value_init_array();
	JSON_Array *sub_array = json_value_get_array(sub_value);
	struct Board_Info * p = board_info;

	int res = 0;//���ڿ��ܲ���������ذ壬��˷���ֵ���߼���ֻҪ��ǰ�ĳ����д������ٽ��г��أ����ҷ��أ��Ѿ������ɹ�������Ҳ��һ������
	unsigned char * change_code[20];
	if (sub_array_parse != NULL)
	{
		for (int i = 0; i < json_array_get_count(sub_array_parse); i++)
		{

			//data����ÿһ��Ԫ�ض�Ϊ���飬��������������Ԫ�أ��ֱ�Ϊ���ذ��ţ��͸ó��ذ��ϻ�������
			res = Board_Get_Weight_Ex(json_array_get_string(sub_array_parse, i));
			if (res != 0)//�������û�гɹ�
			{
				return Procedure_Answer_Message(json_object_get_string(json_object, "MSN"), "Weight_Value", res, sub_value);
			}
			json_array_append_string(sub_array, json_array_get_string(sub_array_parse, i));//��ӳ��ذ���
			while (p != NULL)
			{

				if (strcmp(p->id, json_array_get_string(sub_array_parse, i)) == 0)
				{
					Int_To_CharArray(p->board_items_weight_all_after_close_door, change_code);
					json_array_append_string(sub_array, change_code);
					p = board_info;
					break;//����whileѭ�������ҽ�pָ������ָ��board_info����ͷ
				}
				p = p->next;
			}


		}
		return Procedure_Answer_Message(json_object_get_string(json_object, "MSN"), "Weight_Value", res, sub_value);
	}
	else//����ϴ���data��Ϊ�գ������û���ϴ��κγ��ذ��ź�У׼ֵ����˷��ش���
	{
		res = WEIGHT_VALUE_DATA_MISS;
		return Procedure_Answer_Message(json_object_get_string(json_object, "MSN"), "Weight_Value", res, sub_value);
	}

}

/*
*	���ܣ���ȡ���ذ嵱ǰ����
*	������
*	˵��������������У׼
*/
int Board_Get_Weight_Ex(char * board_id)
{
	struct Board_Info * p = board_info;
	int res = 0;
	while (p != NULL)
	{
		if (strcmp(p->id, board_id) == 0)
		{
			p->board_items_weight_all_after_close_door = 65534;//��ֵÿ��ʹ��ǰ����ֵ�趨Ϊһ�������ܲ�õ���ֵ
			Send_CMD(&hCom_C, p->id, BOARD_CMD_GET_WEIGHT, NULL, 0, 1, 0);
			Sleep(5000);
			if (p->board_items_weight_all_after_close_door != 65534)
			{
				printf("��ȡ������ɣ�\r\n");
				return GEN_OK;
			}
			else
			{
				printf("����ͨ���쳣��\r\n");
				return GEN_ERROR_COMMUNICATION_ERROR;
			}
		}
		p = p->next;
	}
	if (p == NULL)
	{
		printf("  ���ذ��Ŵ���\r\n");
		return WEIGHT_VALUE_BOARD_ERROR;
	}
	return GEN_MEM_OVER;//����Ӧ����Զ����ﵽ�˴�,������˴�������ڴ����洦�ķ���

}


/*
*	���ܣ�ִ��һ����������
*	������[out]ִ�н��
*	˵�������������͹��ӵ�ȫ�����ذ��źͳ��ذ��ϵĻ�������
*		  *���ڳ�ʱ���ŵĴ����������ȴ�����ʱ������Ծ�û�й��ϣ����򷵻���û�йرյ���Ϣ����ʱ�û��ֻ���Ӧ����ʽ���ֹ��Ű�ť���û����ź�Ӧ������������Ű�ť
*		  *�������յ��û����͵���Ϣ��ȥȷ�Ϲ��ӵ��ŵ�״̬�����ȷʵΪ�ر���ִ�г������̣�����Ծ������⣬�����û������쳣��Ϣ
*/
char *  Procedure_Sales_Ex(JSON_Object * json_object)
{
	//ÿһ�����ذ嶼���������ݣ�һ���Ǳ�ţ�һ����������ذ��ϵĻ�������
	JSON_Array * sub_array_parse = json_object_get_array(json_object, "Data");

	//���������õ�json����
	JSON_Value *sub_value = json_value_init_array();
	JSON_Array *sub_array = json_value_get_array(sub_value);
	struct Board_Info * p = board_info;
	unsigned char * change_code[20];
	int res = 0;
	if (sub_array_parse != NULL)
	{
		
		Sleep(200);
		//���ţ����ȴ�2����
		for (int i = 0; i < 2; i++)
		{
			//���ţ�����ʱ�䲻�رգ��˺���������1����
			res = Locker_Open_Closed();
			if (res == LOCKER_GET_STATE_OK)//�������ر���
			{
				break;
			}
		}
		if (res != LOCKER_GET_STATE_OK)
		{
			//�ų�ʱ��δ�ر�
			return Procedure_Answer_Message(json_object_get_string(json_object, "MSN"), "Shopping", res, NULL);//ֱ�ӷ���ִ�п����ź�ķ���ֵ
		}
		//��ʼ����
		Sleep(200);
		p = board_info;
		for (int i = 0; i < json_array_get_count(sub_array_parse); i++)
		{
			//������i����������Ϣ
			//sub_sub_array_parse = json_array_get_array(sub_array_parse, i);
			
			res = Board_Get_Weight_Ex(json_array_get_string(sub_array_parse, i));
			if (res != 0)//�������û�гɹ�
			{
				return Procedure_Answer_Message(json_object_get_string(json_object, "MSN"), "Shopping", res, sub_value);
			}
			json_array_append_string(sub_array, json_array_get_string(sub_array_parse, i));//��ӳ��ذ���
			while (p != NULL)
			{

				if (strcmp(p->id, json_array_get_string(sub_array_parse, i)) == 0)
				{
					Int_To_CharArray(p->board_items_weight_all_after_close_door, change_code);
					json_array_append_string(sub_array, change_code);
					p = board_info;
					break;//����whileѭ�������ҽ�pָ������ָ��board_info����ͷ
				}
				p = p->next;
			}
			if (p == NULL)//������ĳһ�����ذ��Ŵ��󣬼�δ�ҵ�ƥ��ı��
			{
				return Procedure_Answer_Message(json_object_get_string(json_object, "MSN"), "Shopping", WEIGHT_VALUE_BOARD_ERROR, NULL);//����Ӧ����Զ����ﵽ�˴�
			}

		}
		return Procedure_Answer_Message(json_object_get_string(json_object, "MSN"), "Shopping", res, sub_value);

	}
	else
	{
		//û���ṩ���ذ���Ϣ���޷������������
		return Procedure_Answer_Message(json_object_get_string(json_object, "MSN"), "Shopping", SHOPPING_WEIGHT_VALUE_DATA_MISS, NULL);
	}

}





/*
*	���ܣ���ȡ���ذ�������
*	������[out]ִ�н��
*	˵������curУ׼
*
*/
//int Board_Get_Weight_Ex_With_Cur(char * board_id, UINT16 weight/*�����ҪУ׼�������ΪУ׼ֵ*/, int times)
//{
//	struct Board_Info * p = board_info;
//	int res = 0;
//	res = Board_Curavture_Value_Set_Ex(board_id, weight, times, 1/*0Ϊ������ 1Ϊ����*/, 1/*����У׼���*/);//�ڳ����е�������У׼������У׼����Ϊ2�����ұ���У׼���
//																							  //��res������1����0����Ϊ����1Ϊ�¶�û�б仯����У׼��0ΪУ׼����
//	if (res != 1 && res != 0)
//	{
//		return res;
//	}
//	while (p != NULL)
//	{
//		if (strcmp(p->id, board_id) == 0)
//		{
//			p->board_items_weight_all_after_close_door = 65534;//��ֵÿ��ʹ��ǰ����ֵ�趨Ϊһ�������ܲ�õ���ֵ
//			Send_CMD(&hCom_C, p->id, BOARD_CMD_GET_WEIGHT, NULL, 0, 1, 0);
//			Sleep(5000);
//			if (p->board_items_weight_all_after_close_door != 65534)
//			{
//				printf("��ȡ������ɣ�\r\n");
//				return 0;
//			}
//			else
//			{
//				printf("����ͨ���쳣��\r\n");
//				return 6;
//			}
//		}
//		p = p->next;
//	}
//	if (p == NULL)
//	{
//		printf("  ���ذ��Ŵ���\r\n");
//		return 7;
//	}
//	return 8;//����Ӧ����Զ����ﵽ�˴�,������˴�������ڴ����洦�ķ���
//
//}