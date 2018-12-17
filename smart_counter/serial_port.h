#pragma once

#include <Windows.h>
#include <stdio.h>
#include "sys.h"

#define SERIAL_SUCCESS 0
#define SERIAL_FAILURE -1

#define LOCKER_FREE 0
#define LOCKER_USED 1

#define SERIAL_REC_BUF_NODE_FREE 0
#define SERIAL_REC_BUF_NODE_USED 1

#define SERIAL_REC_BUF_NODE_NUM 50 //串口缓冲区节点数量
#define SERIAL_REC_BUF_LEN 50//串口缓冲区每个节点内数据缓冲区的长度
struct Serial_Rec_Buf
{
	char IsUsed;
	unsigned char data[SERIAL_REC_BUF_LEN];//数据缓冲区，长度不会超过这个值
	char len;//data的长度
};

extern int REC_BUF_LOCKER;
extern struct Serial_Rec_Buf srb[];

void init_serial_port(HANDLE * hCom, char * port, int baudrate);
int WriteChar(HANDLE hCom, BYTE* m_szWriteBuffer, DWORD m_nToSend);
void ReceiveChar(void * Com_handle);
void ReceiveCharT(void * Com_handle);


//DWORD serial_read_once(HANDLE hCom, char * data);
//void serial_read(HANDLE hCom);

//DWORD serial_write_once(HANDLE hCom, BYTE * data, DWORD len);
