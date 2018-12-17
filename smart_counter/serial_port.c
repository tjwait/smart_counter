
#include "serial_port.h"

/*以下内容大部来自 lumanman_ 的CSDN 博客 ，全文地址请点击：https://blog.csdn.net/lumanman_/article/details/76275513?utm_source=copy */

HANDLE hCom;

/*
*	功能：初始化串口
*	参数：
*	说明：若失败则程序退出
*/
void init_serial_port(char * port, int baudrate)
{
	//先简单的实现利用char * 打开串口
	if (strcmp(port, "com1") == 0)
	{
		hCom = CreateFile(L"\\\\.\\com1",//COM1口		
			GENERIC_READ, //允许读		
			0, //指定共享属性，由于串口不能共享，所以该参数必须为0		
			NULL,
			OPEN_EXISTING, //打开而不是创建 		
			0, //属性描述，该值为FILE_FLAG_OVERLAPPED，表示使用异步I/O，该参数为0，表示同步I/O操作		
			NULL);
	}
	else if (strcmp(port, "com2") == 0)
	{
		hCom = CreateFile(L"\\\\.\\com2",//COM1口		
			GENERIC_READ, //允许读		
			0, //指定共享属性，由于串口不能共享，所以该参数必须为0		
			NULL,
			OPEN_EXISTING, //打开而不是创建 		
			0, //属性描述，该值为FILE_FLAG_OVERLAPPED，表示使用异步I/O，该参数为0，表示同步I/O操作		
			NULL);
	}
	else if (strcmp(port, "com3") == 0)
	{
		hCom = CreateFile(L"\\\\.\\com3",//COM1口		
			GENERIC_READ, //允许读		
			0, //指定共享属性，由于串口不能共享，所以该参数必须为0		
			NULL,
			OPEN_EXISTING, //打开而不是创建 		
			0, //属性描述，该值为FILE_FLAG_OVERLAPPED，表示使用异步I/O，该参数为0，表示同步I/O操作		
			NULL);
	}
	else if (strcmp(port, "com4") == 0)
	{
		hCom = CreateFile(L"\\\\.\\com4",//COM1口		
			GENERIC_READ, //允许读		
			0, //指定共享属性，由于串口不能共享，所以该参数必须为0		
			NULL,
			OPEN_EXISTING, //打开而不是创建 		
			0, //属性描述，该值为FILE_FLAG_OVERLAPPED，表示使用异步I/O，该参数为0，表示同步I/O操作		
			NULL);
	}
	else if (strcmp(port, "com5") == 0)
	{
		hCom = CreateFile(L"\\\\.\\com5",//COM1口		
			GENERIC_READ, //允许读		
			0, //指定共享属性，由于串口不能共享，所以该参数必须为0		
			NULL,
			OPEN_EXISTING, //打开而不是创建 		
			0, //属性描述，该值为FILE_FLAG_OVERLAPPED，表示使用异步I/O，该参数为0，表示同步I/O操作		
			NULL);
	}
	else if (strcmp(port, "com6") == 0)
	{
		hCom = CreateFile(L"\\\\.\\com6",//COM1口		
			GENERIC_READ, //允许读		
			0, //指定共享属性，由于串口不能共享，所以该参数必须为0		
			NULL,
			OPEN_EXISTING, //打开而不是创建 		
			0, //属性描述，该值为FILE_FLAG_OVERLAPPED，表示使用异步I/O，该参数为0，表示同步I/O操作		
			NULL);
	}
	else if (strcmp(port, "com7") == 0)
	{
		hCom = CreateFile(L"\\\\.\\com7",//COM1口		
			GENERIC_READ, //允许读		
			0, //指定共享属性，由于串口不能共享，所以该参数必须为0		
			NULL,
			OPEN_EXISTING, //打开而不是创建 		
			0, //属性描述，该值为FILE_FLAG_OVERLAPPED，表示使用异步I/O，该参数为0，表示同步I/O操作		
			NULL);
	}
	else if (strcmp(port, "com8") == 0)
	{
		hCom = CreateFile(L"\\\\.\\com8",//COM1口		
			GENERIC_READ, //允许读		
			0, //指定共享属性，由于串口不能共享，所以该参数必须为0		
			NULL,
			OPEN_EXISTING, //打开而不是创建 		
			0, //属性描述，该值为FILE_FLAG_OVERLAPPED，表示使用异步I/O，该参数为0，表示同步I/O操作		
			NULL);
	}
	else if (strcmp(port, "com9") == 0)
	{
		hCom = CreateFile(L"\\\\.\\com9",//COM1口		
			GENERIC_READ, //允许读		
			0, //指定共享属性，由于串口不能共享，所以该参数必须为0		
			NULL,
			OPEN_EXISTING, //打开而不是创建 		
			0, //属性描述，该值为FILE_FLAG_OVERLAPPED，表示使用异步I/O，该参数为0，表示同步I/O操作		
			NULL);
	}
	else if (strcmp(port, "com10") == 0)
	{
		hCom = CreateFile(L"\\\\.\\com10",//COM1口		
			GENERIC_READ, //允许读		
			0, //指定共享属性，由于串口不能共享，所以该参数必须为0		
			NULL,
			OPEN_EXISTING, //打开而不是创建 		
			0, //属性描述，该值为FILE_FLAG_OVERLAPPED，表示使用异步I/O，该参数为0，表示同步I/O操作		
			NULL);
	}
	else if (strcmp(port, "com11") == 0)
	{
		hCom = CreateFile(L"\\\\.\\com11",//COM1口		
			GENERIC_READ, //允许读		
			0, //指定共享属性，由于串口不能共享，所以该参数必须为0		
			NULL,
			OPEN_EXISTING, //打开而不是创建 		
			0, //属性描述，该值为FILE_FLAG_OVERLAPPED，表示使用异步I/O，该参数为0，表示同步I/O操作		
			NULL);
	}
	else if (strcmp(port, "com12") == 0)
	{
		hCom = CreateFile(L"\\\\.\\com12",//COM1口		
			GENERIC_READ, //允许读		
			0, //指定共享属性，由于串口不能共享，所以该参数必须为0		
			NULL,
			OPEN_EXISTING, //打开而不是创建 		
			0, //属性描述，该值为FILE_FLAG_OVERLAPPED，表示使用异步I/O，该参数为0，表示同步I/O操作		
			NULL);
	}
	else if (strcmp(port, "com13") == 0)
	{
		hCom = CreateFile(L"\\\\.\\com13",//COM1口		
			GENERIC_READ, //允许读		
			0, //指定共享属性，由于串口不能共享，所以该参数必须为0		
			NULL,
			OPEN_EXISTING, //打开而不是创建 		
			0, //属性描述，该值为FILE_FLAG_OVERLAPPED，表示使用异步I/O，该参数为0，表示同步I/O操作		
			NULL);
	}
	else if (strcmp(port, "com14") == 0)
	{
		hCom = CreateFile(L"\\\\.\\com14",//COM1口		
			GENERIC_READ, //允许读		
			0, //指定共享属性，由于串口不能共享，所以该参数必须为0		
			NULL,
			OPEN_EXISTING, //打开而不是创建 		
			0, //属性描述，该值为FILE_FLAG_OVERLAPPED，表示使用异步I/O，该参数为0，表示同步I/O操作		
			NULL);
	}
	else if (strcmp(port, "com15") == 0)
	{
		hCom = CreateFile(L"\\\\.\\com15",//COM1口		
			GENERIC_READ, //允许读		
			0, //指定共享属性，由于串口不能共享，所以该参数必须为0		
			NULL,
			OPEN_EXISTING, //打开而不是创建 		
			0, //属性描述，该值为FILE_FLAG_OVERLAPPED，表示使用异步I/O，该参数为0，表示同步I/O操作		
			NULL);
	}
	else if (strcmp(port, "com16") == 0)
	{
		hCom = CreateFile(L"\\\\.\\com16",//COM1口		
			GENERIC_READ, //允许读		
			0, //指定共享属性，由于串口不能共享，所以该参数必须为0		
			NULL,
			OPEN_EXISTING, //打开而不是创建 		
			0, //属性描述，该值为FILE_FLAG_OVERLAPPED，表示使用异步I/O，该参数为0，表示同步I/O操作		
			NULL);
	}


	/*CreateFile函数的第一个参数原例子为TEXT("comX")其中"X"为任意编号,TEXT的功能就是在"com"前面加上一个L，即为L"com",加L代表该字符串为Unicode编码*/
	//hCom = CreateFile(port/*L"com8"*/,//COM1口		
	//			      GENERIC_READ, //允许读		
	//	              0, //指定共享属性，由于串口不能共享，所以该参数必须为0		
	//	              NULL,		
	//	              OPEN_EXISTING, //打开而不是创建 		
	//	              0, //属性描述，该值为FILE_FLAG_OVERLAPPED，表示使用异步I/O，该参数为0，表示同步I/O操作		
	//	              NULL);

	if (hCom == INVALID_HANDLE_VALUE)
	{
		sys_die("打开COM失败!\n");
	}
	else
	{
		printf("COM打开成功！\n");
	}
	SetupComm(hCom, 1024, 1024); //输入缓冲区和输出缓冲区的大小都是1024

	/*********************************超时设置**************************************/
	/* 在用readFile和WriteFile读写串口的时候需要考虑超时问题，超时的作用是在指定的时间内没有读入或发送指定数量的字符，
	ReadFile和WriteFile的操作仍然会结束。要查询当前的超时设置应该调用GetCommTimeouts函数，该函数会填充一个COMMTIMEOUTS
	结构。
	*  调用SetCommTimeouts可以用某一个COMMTIMEOUTS来设置超时。读写串口的超时总共有两种，间隔超时和总超时。
	   间隔超时是指读取两个字符之间的延时，总超时是指读写操作总共花费的时间。写操作只支持总超时，读操作两种超时都支持。
	   下列程序中对应设定超时的参数具体指超时时间是多久还不清楚，还需要再验证
	*/
	COMMTIMEOUTS TimeOuts;	
	//设定读超时,以下设定的读超时方案是非阻塞式的，即执行读取后马上将已经接收到的数据返回，不会等待，即使没有收到任何字符也不会等待。
	//另外还有其它设定方案如将以三个读超时都设定为MAXDWORD，则将会实现读取到需要的字节数才会停止（不知道这里指的需要的字节数在何处设定）
	//另外还可以将ReadIntervalTimeout设定为0，则间隔超时将不起作用，只使用总超时
	TimeOuts.ReadIntervalTimeout = MAXDWORD;//读间隔超时，相邻两个字符之间读取的超时时间，若为0则改参数不起作用
	//读时间系数和读时间常数这两个要相互配合才能使用，其公式为
	//总的读/写超时时间 = Read(Write)TotalTimeoutMultiplier x 要读/写的字节数 + Read(Write)TotalTimeoutConstant
	TimeOuts.ReadTotalTimeoutMultiplier = 0;//读时间系数	
	TimeOuts.ReadTotalTimeoutConstant = 0;//读时间常量	
	//设定写超时	
	TimeOuts.WriteTotalTimeoutMultiplier = 1;//写时间系数	
	TimeOuts.WriteTotalTimeoutConstant = 1;//写时间常量	
	SetCommTimeouts(hCom, &TimeOuts); //设置超时 	
	
	/*****************************************配置串口***************************/	
	DCB dcb;	
	GetCommState(hCom, &dcb);	
	dcb.BaudRate = baudrate; //波特率为9600	
	dcb.ByteSize = 8; //每个字节有8位	
	dcb.Parity = NOPARITY; //无奇偶校验位	
	dcb.StopBits = ONESTOPBIT; //一个停止位	
	SetCommState(hCom, &dcb);

	return SERIAL_SUCCESS;

}

/*
*	功能：读取一次串口数据
*	参数：[in]  hcom 串口句柄
		  [in]	* data 读取出的数据，应保证数组足够长
		  [out] 返回值为读取到的数据长度
*	说明：
*/
DWORD serial_read_once(HANDLE hCom , char * data)
{
	if (hCom == NULL)
	{
		return SERIAL_FAILURE;
	}
	DWORD wCount;//实际读取的字节数
	int bReadStat;

	//char str[100] = { 0 };

	bReadStat = ReadFile(hCom, data, sizeof(data), &wCount, NULL);
	if (!bReadStat)
	{
		printf("读串口失败!");
		CloseHandle(hCom);
		return FALSE;
	}
	else
	{
		return wCount;
	}
}

void serial_read(HANDLE hCom)
{
	if (hCom == NULL)
	{
		return;
	}
	DWORD wCount;//实际读取的字节数
	int bReadStat;

	char str[100] = { 0 };

	while (1)
	{
		//PurgeComm(hCom, PURGE_TXCLEAR | PURGE_RXCLEAR); //清空缓冲区
		/* 若成功则返回非0，不成功则返回0
		BOOL ReadFile(
						HANDLE       hFile,
						LPVOID       lpBuffer,
						DWORD        nNumberOfBytesToRead,
						LPDWORD      lpNumberOfBytesRead,
						LPOVERLAPPED lpOverlapped
					);
		*/
		bReadStat = ReadFile(hCom, str, sizeof(str), &wCount, NULL);
		if (!bReadStat)
		{
			printf("读串口失败!");
			CloseHandle(hCom);
			return FALSE;
		}
		else
		{
			//当执行ReadFile函数后，若成功完成了读取这个动作，但没有读取到任何数据，则代码也会到达此处，因此为做更好的读取数据判断，
			//此处加入判断读取到了多少个字节数，即使用wCount变量
			printf("info : %s\r\n", str);//str若正常读取到数据，会在数据的最后一位添加结束符，因此此处不会将str的100个元素全部打印出来
			memset(str, 0, strlen(str));
		}
		Sleep(1000);
	}
}

/*
*	功能：写取一次串口数据
*	参数：[in]  hcom 串口句柄
		  [in]	* data 带写入的数据
          [in]  int 待写入数据长度
		  [out] 执行结果
*	说明：
*/
DWORD serial_write_once(HANDLE hCom, BYTE * data , DWORD len)
{
	if (hCom == NULL)
	{
		return SERIAL_FAILURE;
	}
	DWORD wRitten = 0;

	//char str[100] = { 0 };

	//if (WriteFile(hCom, data, len, &wRitten, 0))
	if (WriteFile(hCom, data, len, &wRitten, 0))
	{

		return wRitten;

	}
	else
	{
		DWORD dwError = GetLastError();
		return SERIAL_FAILURE;
	}
}





