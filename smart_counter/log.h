#pragma once
#pragma once

#include "stdio.h"
#include "string.h"
#include "stdlib.h"
#include "time.h"
#include "stdarg.h"
#include "windows.h"

//断言函数使用
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
	ERR = 4,//原值为ERROR，该名字被系统占用了
	ALL = 255
}LOGLEVEL;

typedef struct log {
	char logtime[20];//日志信息的形成时间
	char filepath[MAXFILEPATH];//日志信息的保存路径
	FILE *logfile;//根据日志信息保存的文件路径获取的文件指针
}LOG;

typedef struct logseting {
	char filepath[MAXFILEPATH];//日志的保存的目录
	unsigned int maxfilelen;//日志目录字符串的长度
	unsigned char loglevel;//日志显示等级
}LOGSET;

int LogWrite(unsigned char loglevel, char *fromat, ...);

int mutex_lock(int locker);

int mutex_unlock(int locker);