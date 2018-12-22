
#ifndef _global_h
#define _global_h

#ifdef	__cplusplus
extern "C" {
#endif

#include "database_fun.h"
#include "log.h"

#define CF(a , b)  (a##b)
//#define CFF(a)     (#a)
//#define CFFF(a)    CFF(a)

#define BOARD_CMD_BASIC_VALUE 0x01//ȥƤ
#define BOARD_CMD_BASIC_VALUE_SAVE 0x11//ȥƤ����
#define BOARD_CMD_CURVATURE		0x02//����У׼
#define BOARD_CMD_CURVATURE_SAVE	0x12//����У׼����
#define BOARD_CMD_GET_WEIGHT	0x03//��ȡ��������λΪ��
#define BOARD_CMD_SHAKE	0x06//��������
//���������������йس��ذ��ַ�Ļ�ȡ���趨�������û�а���������Ϊ�й��趨��Ӧ�÷������ϵͳ֮��

#define LOCKER_CMD_UNLOCK 1
#define LOCKER_CMD_STATUS_ACTIVE_SEND 2
#define LOCKER_CMD_STATUS 3
	#define LOCKER_STATUS_LOCK_OPEN_SUCCESS 0
	#define LOCKER_STATUS_LOCK_ALREADY_OPEN 1//ע��0ֵ��1ֵ�����������ڴ˳�������Ϊ����ͬ��״̬��0ֵ��lockerִ�п����������ܷ���ֵ
											 //����λ�����������״̬Ӧ�ü�ⲻ��0ֵ
	#define LOCKER_STATUS_LOCK_CLOSED       2
	#define LOCKER_STATUS_LOCK_OPEN_NO_OPEN 3
	#define LOCKER_STATUS_LOCK_WAITING_OPEN 4//���ֵ��������о���أ����·����ִ����������
											 //���ܳ��ִ������ָ�����ȴ����򿪻��ߵȴ���о�������ϣ����Ҫ������Ŀǰ��״̬�ж���ʵ��״̬
	#define LOCKER_STATUS_LOCK_ERROR        0xFF//�����������ص����⣬��Ҫ��Ա���ֳ�����

#define LOCKER_CMD_TEM 0xFE //����������������ȡ�����¶�ָ��
/*���ӵ�һЩ�趨����*/
#define ERROR_VALUE 5

extern struct counter_info * counter;
extern struct Board_Info * board_info;//���ذ���Ϣͷָ��
extern struct scheme scheme_globle[10];
extern HANDLE hCom_C;
extern HANDLE hCom_Tem;

extern unsigned char  Tem[];

#ifdef	__cplusplus
}
#endif

#endif /* _global_h */

