
#include "stdio.h"
#include <process.h>
#include "database_fun.h"//mysqlΪ64λ�汾�������Ҫ��x64ģʽ�µ���
#include "amqp_fun.h"
#include "sys.h"

//�������ݿ����ȫ�ֱ���

//mq���ȫ�ֱ���
int main()
{
	LogWrite(INFO, "%s", "---------------------------------------------------------------");
	LogWrite(INFO, "%s", "---------------------------------------------------------------");
	LogWrite(INFO, "%s", "Smart Counter System Start!");
	//���ӱ��س�ʼ������Ҫִ�����������ݿ⣬��ʼ�������ṹ�����ݣ�Ȼ��򿪴��ڣ��������ڽ����̣߳������������ݷ����̣߳����Գ��ذ�ʵ������
	//�Ƿ�ͬ���ݿ����趨һ�£����Ƿ�Ϊ�ر�״̬���ڳ�ʼ���������ݵ�ʱ�򣬹��Ӳ�δ�������磬��ʱ��������κ����緢�͹���������
	LogWrite(INFO, "%s", "Init System Start!");
	Init_System();

	LogWrite(INFO, "%s", "ReceiveChar thread Start!");
	_beginthread(ReceiveChar, 0, &hCom_C);
	LogWrite(INFO, "%s", "ReceiveCharT thread Start!");
	_beginthread(ReceiveCharT, 0, &hCom_Tem);
	LogWrite(INFO, "%s", "Board Com Data Parse thread Start!");
	_beginthread(Parse_Usart_Data_Run, 0, NULL);
	LogWrite(INFO, "%s", "Check Board State Start!");
	Board_Ready();

	//��ȡ����״̬��Ŀ���ǣ��ڻ�ȡ֮��Ὣ����״̬�����ڹ��ӵ���Ϣ�Ľṹ���У���λ����������ͨ���ӿڵ��û�ȡ���״̬
	//�ݴ˿����ж������Ƿ����쳣���⣬�Ƿ����ִ��ĳһ��ҵ�������ǿ��ŵģ��ǾͲ�����ִ����������ҵ��
	LogWrite(INFO, "%s", "Get Locker State Start!");
	Locker_Get_Stat();

	Sleep(200);
	Init_Tem();
	_beginthread(Counter_Get_Tem_Ex, 0, NULL);
	//��ʼ��������������
	init_amqp();
	//_beginthread(run_listen, 0, NULL);
	while (1)
	{
		run_listen("dummy");
		Destory_connection(conn , atoi(counter->channel));
		Sleep(5000);
		Sleep(5000);
	}
	getchar();
}








