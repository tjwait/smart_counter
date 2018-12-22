
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
	//�������ļ��ж�ȡһ���ַ��������Ұ��ա�path=%s\n����ʽ����ֵ��value
	fscanf(fpath, "path=%s\n", value);
	//strcat(value, "/");
	strcat(value, "\\");//�˴����ӷ�б�ܣ�ϵͳĬ��Ϊ��־�ļ�һ���ᱣ����ĳһ�����·�����ļ����У������Ƿ���ĳһ�����Ŀ¼������Ŀǰ���Է������Ŀ¼����Ҳ���ᱨ��
	//�γ�Ҫ�������־���ļ���
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
*��־������Ϣ
* */
static LOGSET *getlogset() {
	char path[512] = { 0x0 };
	//����ǰ�����ľ���Ŀ¼���Ƶ�path��
	getcwd(path, sizeof(path));
	//strcat(path, "/log.conf");
	strcat(path, "\\log.conf");//����б�ܸ���Ϊwindows��ʽ
	if (access(path, F_OK) == 0) {
		if (ReadConfig(path) != 0) {//����������ǵ���0�ģ���˲�������if�����
			logsetting.loglevel = INFO;
			logsetting.maxfilelen = 4096;
		}
	}
	else {
		logsetting.loglevel = INFO;
		logsetting.maxfilelen = 4096;
	}
	return &logsetting;//����һ��ȫ�ֱ���
}

/*
*��ȡ����
* */
static char * getdate(char *date) {
	time_t timer = time(NULL);
	strftime(date, 11, "%Y-%m-%d", localtime(&timer));
	return date;
}

/*
*��ȡʱ��
* */
static void settime() {
	time_t timer = time(NULL);
	strftime(loging.logtime, 20, "%Y-%m-%d %H:%M:%S", localtime(&timer));
}

/*
*�����δ�ӡ
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
	//logsetΪһ���ṹ��
	LOGSET *logsetting;
	//��ȡ��־������Ϣ�����logsetting�ṹ��
	if ((logsetting = getlogset()) == NULL) {
		perror("Get Log Set Fail!");
		return -1;
	}
	//��������ļ�����־����ȼ�ͬҪ��ӡ����־�ȼ���ͬ��ֱ�ӷ��ز���ӡ����־��Ϣ
	if ((loglevel&(logsetting->loglevel)) != loglevel)
		return -1;

	//���¿�ʼ����ĳһ����־д���ļ��Ĺ��̣�loging��ָĳһ����־��¼
	memset(&loging, 0, sizeof(LOG));
	//��ȡ��־ʱ��
	settime();//��������־��Ϣ�γɵ�ʱ�丳ֵ��loging�ṹ�壬�ں����ڲ�ʵ�ֵ�
	if (strlen(logsetting->filepath) == 0) {
		//�����־�����ļ��е���־��Ŀ¼Ϊ�գ������˲��ִ���
		//���´���Ϊ�Զ�������־�ļ������·��
		char *path = getenv("HOME");//��ȡ��������home��Ŀ¼λ�ã�Ŀǰ�ҵĵ�����Ӧ��û���趨�û������������Ӧ�÷��ص���null
		//���¼��д���Ӧ��û���ǻ�������homeû���趨��pathΪ�յ����������Ҫ������
		memcpy(logsetting->filepath, path, strlen(path));

		getdate(strdate);
		strcat(strdate, ".log");
		strcat(logsetting->filepath, "/");
		strcat(logsetting->filepath, strdate);
	}
	//�����������ļ������ɹ������־�ļ����Ƹ�ֵ��loging�ṹ���filepath��
	memcpy(loging.filepath, logsetting->filepath, MAXFILEPATH);
	//����־�ļ�
	if (loging.logfile == NULL)
		loging.logfile = fopen(loging.filepath, "a+");//"a+" Ϊ�򿪻����½�һ���ļ������Զ�����ֻ����δ��ĩβ׷д
		//loging.logfile = fopen("d:\\aa.log", "a+");
	if (loging.logfile == NULL) {
		perror("Open Log File Fail!");
		return -1;
	}
	//д����־������־ʱ��
	fprintf(loging.logfile, "[%s] [%s] ", LogLevelText[loglevel - 1], loging.logtime);
	return 0;
}

/*
*��־д��
*˵����	�ú���ÿһ�ζ����¶�ȡ��־�����ļ���Ϣ��������־����Ŀ¼���ܹ��������־�ȼ���������־�ļ����ɹ���
*		ÿһ�춼������һ������ 2018-12-21.log ��ʽ���ļ���һ���ڶ��д����־�����ļ������ظ��½�
*
*˵����	��־����ȼ���ָ���ڱ���־���ϵͳ����Ϊ�涨��	NONE = 0,INFO = 1,DEBUG = 2,WARN = 3,ERR = 4,ALL = 255
*		������������ȼ����ڱ���У�������Ҫ�ɽ�ĳһ������趨ΪINFO���������DEBUG���������־��������ļ���
*		�ɹ涨�ܹ��������־���ͣ���������ĳһ����־�������ͬ�����ļ��в������ʱ�������־�Ͳ����������
*		������Ҫ��Ϊ��������ϵͳ��ͬ�Ŀ���ʱ���ж���־�������Ҫ���������־��Ϣ���з��࣬���������־���������
* */
int LogWrite(unsigned char loglevel, char *fromat, ...)
{
	int  rtv = -1;
	va_list args;

	//[Ϊ֧�ֶ��߳���Ҫ����] pthread_mutex_lock(&mutex_log); //lock. 

	if (mutex_lock(log_locker) == -1)
	{
		return rtv;
	}

	do {
		//��ʼ����־
		if (initlog(loglevel) != 0)//�˺����������ִ�У�������־�ļ��б�����־�ĵȼ��ʹ�ӡ����־��date����
		{
			rtv = -1;
			break;
		}
		//��ӡ��־��Ϣ
		va_start(args, fromat);
		PrintfLog(fromat, args);
		va_end(args);
		//�ļ�ˢ��
		fflush(loging.logfile);
		//��־�ر�
		if (loging.logfile != NULL)
			fclose(loging.logfile);
		loging.logfile = NULL;
		rtv = 0;
	} while (0);

	//[Ϊ֧�ֶ��߳���Ҫ����] pthread_mutex_unlock(&mutex_log); //unlock. 
	mutex_unlock(log_locker);
	return rtv;
}

/*
*	���̻߳���������
*	[in] �� 0Ϊδʹ�ã�1Ϊʹ��
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
*	���̻߳���������
*	[in] �� 0Ϊδʹ�ã�1Ϊʹ��
*/
int mutex_unlock(int locker)
{
	locker = 0;
	return 0;

}