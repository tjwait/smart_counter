
#include "log.h"
#define MAXLEVELNUM (3)

LOGSET logsetting;
LOG loging;

int log_locker = 0;

const static char LogLevelText[4][10] = { "INFO","DEBUG","WARN","ERR" };

static char * getdate(char *date);

static unsigned char getcode(char *path) {
	unsigned char code = 255;
	if (strcmp("INFO", path) == 0)
		code = 1;
	else if (strcmp("WARN", path) == 0)
		code = 3;
	else if (strcmp("ERR", path) == 0)
		code = 4;
	else if (strcmp("NONE", path) == 0)
		code = 0;
	else if (strcmp("DEBUG", path) == 0)
		code = 2;
	return code;
}

static unsigned char ReadConfig(char *path) {
	char value[512] = { 0x0 };
	char data[50] = { 0x0 };

	FILE *fpath = fopen(path, "r");
	if (fpath == NULL)
		return -1;
	//从配置文件中读取一个字符串，并且按照“path=%s\n”格式流赋值给value
	fscanf(fpath, "path=%s\n", value);
	//strcat(value, "/");
	strcat(value, "\\");//此处增加反斜杠，系统默认为日志文件一定会保存在某一个相对路径的文件夹中，而不是放在某一个相对目录根部，目前测试放在相对目录根部也不会报错
	//形成要保存得日志的文件名
	getdate(data);
	strcat(data, ".log");
	strcat(value, data);
	if (strcmp(value, logsetting.filepath) != 0)
		memcpy(logsetting.filepath, value, strlen(value));
	memset(value, 0, sizeof(value));

	fscanf(fpath, "level=%s\n", value);
	logsetting.loglevel = getcode(value);
	fclose(fpath);
	return 0;
}
/*
*日志设置信息
* */
static LOGSET *getlogset() {
	char path[512] = { 0x0 };
	//将当前工作的绝对目录复制到path中
	getcwd(path, sizeof(path));
	//strcat(path, "/log.conf");
	strcat(path, "\\log.conf");//将反斜杠更改为windows样式
	if (access(path, F_OK) == 0) {
		if (ReadConfig(path) != 0) {//正常情况下是等于0的，因此不会进入该if语句内
			logsetting.loglevel = INFO;
			logsetting.maxfilelen = 4096;
		}
	}
	else {
		logsetting.loglevel = INFO;
		logsetting.maxfilelen = 4096;
	}
	return &logsetting;//这是一个全局变量
}

/*
*获取日期
* */
static char * getdate(char *date) {
	time_t timer = time(NULL);
	strftime(date, 11, "%Y-%m-%d", localtime(&timer));
	return date;
}

/*
*获取时间
* */
static void settime() {
	time_t timer = time(NULL);
	strftime(loging.logtime, 20, "%Y-%m-%d %H:%M:%S", localtime(&timer));
}

/*
*不定参打印
* */
static void PrintfLog(char * fromat, va_list args) {
	int d;
	char c, *s;
	while (*fromat)
	{
		switch (*fromat) {
		case 's': {
			s = va_arg(args, char *);
			fprintf(loging.logfile, "%s", s);
			break; }
		case 'd': {
			d = va_arg(args, int);
			fprintf(loging.logfile, "%d", d);
			break; }
		case 'c': {
			c = (char)va_arg(args, int);
			fprintf(loging.logfile, "%c", c);
			break; }
		default: {
			if (*fromat != '%'&&*fromat != '\n')
				fprintf(loging.logfile, "%c", *fromat);
			break; }
		}
		fromat++;
	}
	fprintf(loging.logfile, "%s", "\n");
}

static int initlog(unsigned char loglevel) {
	char strdate[30] = { 0x0 };
	//logset为一个结构体
	LOGSET *logsetting;
	//获取日志配置信息，填充logsetting结构体
	if ((logsetting = getlogset()) == NULL) {
		perror("Get Log Set Fail!");
		return -1;
	}
	//如果配置文件的日志输出等级同要打印的日志等级不同即直接返回不打印该日志信息
	if ((loglevel&(logsetting->loglevel)) != loglevel)
		return -1;

	//以下开始处理某一条日志写入文件的过程，loging是指某一条日志记录
	memset(&loging, 0, sizeof(LOG));
	//获取日志时间
	settime();//将该条日志信息形成的时间赋值给loging结构体，在函数内部实现的
	if (strlen(logsetting->filepath) == 0) {
		//如果日志配置文件中的日志存目录为空，则进入此部分代码
		//以下代码为自动配置日志文件保存的路径
		char *path = getenv("HOME");//获取环境变量home的目录位置，目前我的电脑中应该没有设定该环境变量，因此应该返回的是null
		//以下几行代码应该没考虑环境变量home没有设定，path为空的情况，还需要测试下
		memcpy(logsetting->filepath, path, strlen(path));

		getdate(strdate);
		strcat(strdate, ".log");
		strcat(logsetting->filepath, "/");
		strcat(logsetting->filepath, strdate);
	}
	//将根据配置文件及生成规则的日志文件名称赋值给loging结构体的filepath中
	memcpy(loging.filepath, logsetting->filepath, MAXFILEPATH);
	//打开日志文件
	if (loging.logfile == NULL)
		loging.logfile = fopen(loging.filepath, "a+");//"a+" 为打开或者新建一个文件，可以读，但只能在未见末尾追写
		//loging.logfile = fopen("d:\\aa.log", "a+");
	if (loging.logfile == NULL) {
		perror("Open Log File Fail!");
		return -1;
	}
	//写入日志级别，日志时间
	fprintf(loging.logfile, "[%s] [%s] ", LogLevelText[loglevel - 1], loging.logtime);
	return 0;
}

/*
*日志写入
*说明：	该函数每一次都重新读取日志配置文件信息，包括日志保存目录和能够输出的日志等级，根据日志文件生成规则，
*		每一天都会生成一个形如 2018-12-21.log 格式的文件，一天内多次写入日志，该文件不会重复新建
*
*说明：	日志输入等级是指，在本日志输出系统中人为规定了	NONE = 0,INFO = 1,DEBUG = 2,WARN = 3,ERR = 4,ALL = 255
*		五种六项输出等级，在编程中，根据需要可将某一个输出设定为INFO输出或者是DEBUG输出，在日志输出配置文件中
*		可规定能够输出的日志类型，当程序中某一个日志输出类型同配置文件中不相符的时候，则该日志就不会输出，此
*		功能主要是为了满足在系统不同的开发时期中对日志输出的需要，将输出日志信息进行分类，避免输出日志过多的问题
* */
int LogWrite(unsigned char loglevel, char *fromat, ...)
{
	int  rtv = -1;
	va_list args;

	//[为支持多线程需要加锁] pthread_mutex_lock(&mutex_log); //lock. 

	if (mutex_lock(log_locker) == -1)
	{
		return rtv;
	}

	do {
		//初始化日志
		if (initlog(loglevel) != 0)//此函数如果正常执行，会在日志文件中本条日志的等级和打印该日志的date数据
		{
			rtv = -1;
			break;
		}
		//打印日志信息
		va_start(args, fromat);
		PrintfLog(fromat, args);
		va_end(args);
		//文件刷出
		fflush(loging.logfile);
		//日志关闭
		if (loging.logfile != NULL)
			fclose(loging.logfile);
		loging.logfile = NULL;
		rtv = 0;
	} while (0);

	//[为支持多线程需要加锁] pthread_mutex_unlock(&mutex_log); //unlock. 
	mutex_unlock(log_locker);
	return rtv;
}

/*
*	多线程互斥锁加锁
*	[in] 锁 0为未使用，1为使用
*/
int mutex_lock(int locker)
{
	int time = 0;
	while (locker == 1)
	{
		if (time == 20)
		{
			break;
		}
		Sleep(200);
		time++;
	}

	if (time == 20)
	{
		return -1;
	}
	locker = 1;
	return 0;

}


/*
*	多线程互斥锁解锁
*	[in] 锁 0为未使用，1为使用
*/
int mutex_unlock(int locker)
{
	locker = 0;
	return 0;

}