#pragma once
#pragma once

#include "stdio.h"
#include "string.h"
#include "stdlib.h"
#include "time.h"
#include "stdarg.h"
#include "windows.h"

//���Ժ���ʹ��
#define F_OK 0
#define R_OK 2
#define W_OK 4
#define X_OK 6

#define MAXLEN (2048)
#define MAXFILEPATH (512)
#define MAXFILENAME (50)
typedef enum {
	ERROR_1 = -1,
	ERROR_2 = -2,
	ERROR_3 = -3
}ERROR0;


typedef enum {
	NONE = 0,
	INFO = 1,
	DEBUG = 2,
	WARN = 3,
	ERR = 4,//ԭֵΪERROR�������ֱ�ϵͳռ����
	ALL = 255
}LOGLEVEL;

typedef struct log {
	char logtime[20];//��־��Ϣ���γ�ʱ��
	char filepath[MAXFILEPATH];//��־��Ϣ�ı���·��
	FILE *logfile;//������־��Ϣ������ļ�·����ȡ���ļ�ָ��
}LOG;

typedef struct logseting {
	char filepath[MAXFILEPATH];//��־�ı����Ŀ¼
	unsigned int maxfilelen;//��־Ŀ¼�ַ����ĳ���
	unsigned char loglevel;//��־��ʾ�ȼ�
}LOGSET;

int LogWrite(unsigned char loglevel, char *fromat, ...);

int mutex_lock(int locker);

int mutex_unlock(int locker);