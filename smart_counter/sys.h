#pragma once

#include "database_fun.h"
#include "serial_port.h"
#include "global.h"
#include "parson.h"
#include "time.h"

struct Sys_Tem
{
	int Tem;//最后一次校准时的温度
	signed char Tem_Cur;//测量数值
	int Tem_Dis;//不用校准的温度偏差，即当实际测得温度在在 Tem±Tem_Re这个范围内就不需要启动校准次数统计
	int Time;//温度超出不用校准范围的次数
	int MaxTime;//若Time 等于 MaxTime时将IsCheck置位
	int IsCheck;//是否需要校准 0为不需要 1为需要
	int delay;//温度监测的间隔
};


void Init_System(void);
void Board_Ready(void);
int Board_Basic_Value_Set(int times);
char * Board_Basic_Value_Set_With_ACK();
void Board_Curavture_Value_Set(int times, int save);
//void Board_Curavture_Value_Set(UINT16 weight, int times, int save);
void Board_Get_Weight();
void Init_Tem();
void Counter_Get_Tem();
int Locker_Open(); //新系统中使用
int Locker_Open_Closed();//新系统中使用
int Locker_Get_Stat();//新系统中使用

char *  Procedure_Sales();
char *  Procedure_On_Shelf(JSON_Object * json_object);
char *  Procedure_Off_Shelf(JSON_Object * json_object);
char *  Procedure_Pick_And_Place(int Type);

void Parse_Usart_Data_Run();
void Send_CMD(HANDLE  * hCom, unsigned char * id, unsigned char CMD, unsigned char * cmd_data, int cmd_data_len, char dev_kind, int little);
void sys_die(const char *fmt, ...);
unsigned char Sum_Check(unsigned char * data, int len);
byte ASCII_To_byte(unsigned char buf);
double CharNum_To_Double(const unsigned char * data);
void Double_To_CharArray(double num, unsigned char * data);
void Int_To_CharArray(int num, unsigned char * data);
void HexStringFormatForPrintf(unsigned char * in, int in_len, unsigned char * out);
int GBKToUTF8(unsigned char * lpGBKStr, unsigned char * lpUTF8Str, int nUTF8StrLen);
int UTF8ToGBK(unsigned char * lpUTF8Str, unsigned char * lpGBKStr, int nGBKStrLen);
int is_valid_utf8(const char *string, size_t string_len);

//新系统使用的业务函数返回值定义
#define LOCKER_UNLOCK_STATE_OK 0x00//锁吸回正常，并且们已经被打开
#define LOCKER_UNLOCK_STATE_OPEN_ALREADY 0x02//在发送开门指令的时候，们已经打开了
#define LOCKER_UNLOCK_STATE_DONOT_OPEN 0x03//锁打开正常，但是门长时间没有被打开，又自动关上了
#define LOCKER_UNLOCK_STATE_ERROR 0xFF//锁严重错误


#define LOCKER_GET_STATE_OPENING 0x01//门正在被打开
#define LOCKER_GET_STATE_OK 0x02//门正常执行了一次开关动作
#define LOCKER_GET_STATE_WAITING_OPEN 0x04//门正在等待被打开
#define LOCKER_GET_STATE_ERROR 0xFF//锁严重错误

#define SETTING_BASIC_VALUE_BOARD_ERROR 0x05//去皮称重板编号错误
#define SETTING_CURAVTURE_VALUE_TOO_LIGHT 0x06//校准重量太清
#define SETTING_CURAVTURE_VALUE_BOARD_ERROR 0x07//设定曲率值称重板编号错误

#define WEIGHT_VALUE_BOARD_ERROR 0x08//获取重量称重板编号错误
#define WEIGHT_VALUE_DATA_MISS 0x09 //下行数据缺少称重板信息

#define SHOPPING_SETTING_CURAVTURE_VALUE_ERROR 0x10//在销售流程中，校准环节出现问题
#define SHOPPING_WEIGHT_VALUE_DATA_MISS 0x11//下行数据缺少称重板信息

#define GEN_OK 0x00 //通用正常返回值
#define GEN_MEM_OVER 0xFD //内存有溢出风险
#define GEN_ERROR_COMMUNICATION_ERROR 0xFE //通信错误，该错误是一个通用错误，是指控制器同嵌入式设备通信异常



//新系统的所使用的函数，以上部分函数也在新系统中使用
char *  Procedure_Answer_Message(char * message_sn, char * cmd_name, int Res, JSON_Value *sub_value);
char *  Procedure_Open_Lock(JSON_Object * json_object);
char *  Procedure_Open_Close(JSON_Object * json_object);
char *  Procedure_Get_Locker_State(JSON_Object * json_object);
char *  Procedure_Basic_Value_Set(JSON_Object * json_object);
char *  Procedure_Set_Curavture_Value(JSON_Object * json_object);
char *  Procedure_Get_Board_State(void);
char *  Procedure_Get_Weight_Value(JSON_Object * json_object);
char *  Procedure_Sales_Ex(JSON_Object * json_object);

int Board_Basic_Value_Set_By_id(char * board_id, int times);
int Board_Curavture_Value_Set_Ex(char * board_id, UINT16 weight, int times , int save);
int Board_Get_Weight_Ex(char * board_id);
void Counter_Get_Tem_Ex();
