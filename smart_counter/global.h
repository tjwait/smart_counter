
#ifndef _global_h
#define _global_h

#ifdef	__cplusplus
extern "C" {
#endif

#include "database_fun.h"

#define CF(a , b)  (a##b)
//#define CFF(a)     (#a)
//#define CFFF(a)    CFF(a)

#define BOARD_CMD_BASIC_VALUE 0x01//去皮
#define BOARD_CMD_BASIC_VALUE_SAVE 0x11//去皮保存
#define BOARD_CMD_CURVATURE		0x02//曲率校准
#define BOARD_CMD_CURVATURE_SAVE	0x12//曲率校准保存
#define BOARD_CMD_GET_WEIGHT	0x03//获取重量，单位为克
#define BOARD_CMD_SHAKE	0x06//板子握手
//对于以上命令中有关称重板地址的获取和设定相关命令没有包含，我认为有关设定不应该放在这个系统之中

#define LOCKER_CMD_UNLOCK 1
#define LOCKER_CMD_STATUS_ACTIVE_SEND 2
#define LOCKER_CMD_STATUS 3
	#define LOCKER_STATUS_LOCK_OPEN_SUCCESS 0
	#define LOCKER_STATUS_LOCK_ALREADY_OPEN 1//注意0值和1值这两个数据在此程序中认为是相同的状态，0值是locker执行开锁命令后可能返回值
											 //若上位机主动检测门状态应该检测不到0值
	#define LOCKER_STATUS_LOCK_CLOSED       2
	#define LOCKER_STATUS_LOCK_OPEN_NO_OPEN 3
	#define LOCKER_STATUS_LOCK_WAITING_OPEN 4//这个值是在门锁芯吸回，其下方出现磁铁的情况，
											 //可能出现此情况是指门正等待被打开或者等待锁芯落下锁上，因此要根据锁目前的状态判定其实际状态
	#define LOCKER_STATUS_LOCK_ERROR        0xFF//锁出现了严重的问题，需要人员到现场处理

#define LOCKER_CMD_TEM 0xFE //利用锁的命令编码获取柜子温度指令
/*柜子的一些设定参数*/
#define ERROR_VALUE 5

extern struct counter_info * counter;
extern struct Board_Info * board_info;//称重板信息头指针
extern struct scheme scheme_globle[10];
extern HANDLE hCom_C;
extern HANDLE hCom_Tem;

extern unsigned char  Tem[];

#ifdef	__cplusplus
}
#endif

#endif /* _global_h */

