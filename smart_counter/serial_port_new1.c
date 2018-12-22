
#include "serial_port.h"

/*串口接收数据缓冲区*/
struct Serial_Rec_Buf srb[50];
int REC_BUF_LOCKER = LOCKER_FREE;

HANDLE hCom_C;
HANDLE hCom_Tem;
OVERLAPPED m_ov;
COMSTAT comstat;

OVERLAPPED m_ov_T;
COMSTAT comstat_T;

DWORD    m_dwCommEvents;

/*
*	功能：初始化串口
*	参数：
*	说明：若失败则程序退出
*/
void init_serial_port(HANDLE * hCom , char * port, int baudrate)
{
	//先简单的实现利用char * 打开串口
	char s_buf[100] = { 0 };
	char szStr[100] = "\\\\.\\";
	strcat(szStr, port);
	WCHAR wszClassName[256];
	memset(wszClassName, 0, sizeof(wszClassName));
	MultiByteToWideChar(CP_ACP, 0, szStr, strlen(szStr) + 1, wszClassName,
		sizeof(wszClassName) / sizeof(wszClassName[0]));


	*hCom = CreateFile(wszClassName,
		GENERIC_READ | GENERIC_WRITE,
		0,
		0,
		OPEN_EXISTING,
		FILE_FLAG_OVERLAPPED,
		0);
	if (*hCom == INVALID_HANDLE_VALUE)
	{ 
		//sys_die("打开COM失败!\n");
		sprintf(s_buf, "%s Open Failed!", port);
		LogWrite(ERR, "%s", s_buf);
		exit(-1);
	}
		
	else
	{
		//printf("COM打开成功！\n");
		sprintf(s_buf, "%s Open SUCCESS!", port);
		LogWrite(INFO, "%s", s_buf);
		memset(s_buf, 0, sizeof(s_buf));
		if (setupdcb(hCom ,baudrate) == 0)
		{
			sprintf(s_buf, "%s Setup DCB SUCCESS", port);
			LogWrite(INFO, "%s", s_buf);
			//printf("setup dcb 成功！\n");
		}
		else
		{
			//sys_die("setup dcb!\n");
			sprintf(s_buf, "%s Setup DCB Failed!", port);
			LogWrite(ERR, "%s", s_buf);
			exit(-1);
		}
		memset(s_buf, 0, sizeof(s_buf));
		if (setuptimeout(hCom , 0, 0, 0, 0, 0) == 0)
		{
			//printf("setup timeout 成功！\n");
			sprintf(s_buf, "%s Setup timeout SUCCESS", port);
			LogWrite(INFO, "%s", s_buf);
		}
		else
		{
			//sys_die("setup timeout!\n");
			sprintf(s_buf, "%s Setup timeout Failed!", port);
			LogWrite(ERR, "%s", s_buf);
			exit(-1);
		}
		PurgeComm(* hCom , PURGE_RXCLEAR | PURGE_TXCLEAR | PURGE_RXABORT | PURGE_TXABORT); // 在读写串口之前，还要用PurgeComm()函数清空缓冲区
																						 //PURGE_TXABORT      中断所有写操作并立即返回，即使写操作还没有完成。
																						 //PURGE_RXABORT      中断所有读操作并立即返回，即使读操作还没有完成。
																						 //PURGE_TXCLEAR      清除输出缓冲区
																						 //PURGE_RXCLEAR      清除输入缓冲区
	}

	return SERIAL_SUCCESS;

}

/*
*	功能：初始化串口参数
*	参数：[in]  rate_arg 比特率
*	说明：若失败则程序退出
*/
int setupdcb(HANDLE * hCom , int rate_arg)
{
	DCB  dcb;
	int rate = rate_arg;
	memset(&dcb, 0, sizeof(dcb)); //在一段内存块中填充某个给定的值，是对较大的结构//体或数组进行清零操作的一种最快方法
	if (!GetCommState( * hCom, &dcb))//获取当前DCB配置
	{
		return -1;
	}
	/* -------------------------------------------------------------------- */
	// set DCB to configure the serial port
	dcb.DCBlength = sizeof(dcb);
	/* ---------- Serial Port Config ------- */
	dcb.BaudRate = rate;
	dcb.Parity = NOPARITY;
	dcb.fParity = 0;
	dcb.StopBits = ONESTOPBIT;
	dcb.ByteSize = 8;
	dcb.fOutxCtsFlow = 0;
	dcb.fOutxDsrFlow = 0;
	dcb.fDtrControl = DTR_CONTROL_DISABLE;
	dcb.fDsrSensitivity = 0;
	dcb.fRtsControl = RTS_CONTROL_DISABLE;
	dcb.fOutX = 0;
	dcb.fInX = 0;
	/* ----------------- misc parameters ----- */
	dcb.fErrorChar = 0;
	dcb.fBinary = 1;
	dcb.fNull = 0;
	dcb.fAbortOnError = 0;
	dcb.wReserved = 0;
	dcb.XonLim = 2;
	dcb.XoffLim = 4;
	dcb.XonChar = 0x13;
	dcb.XoffChar = 0x19;
	dcb.EvtChar = 0;
	/* -------------------------------------------------------------------- */
	// set DCB
	if (!SetCommState( * hCom, &dcb))
	{
		return SERIAL_FAILURE;
	}
	else
		return SERIAL_SUCCESS;
}


/*
*	功能：设置串口超时参数
*	参数：[in]  hcom 串口句柄
*	说明：在用readfile和writefile读写串行口时，需要考虑超时问题, 读写串口的超时有两
*		  种：间隔超时和总超时, 写操作只支持总超时，而读操作两种超时均支持, 如果所有
*		  写超时参数均为0，那么就不使用写超时。
*/

int setuptimeout(HANDLE * hCom , DWORD ReadInterval, DWORD ReadTotalMultiplier, DWORD ReadTotalconstant, DWORD WriteTotalMultiplier, DWORD WriteTotalconstant)
{
	COMMTIMEOUTS timeouts;
	timeouts.ReadIntervalTimeout = ReadInterval; //读间隔超时
	timeouts.ReadTotalTimeoutConstant = ReadTotalconstant; //读时间系数
	timeouts.ReadTotalTimeoutMultiplier = ReadTotalMultiplier; //读时间常量
	timeouts.WriteTotalTimeoutConstant = WriteTotalconstant; // 写时间系数
	timeouts.WriteTotalTimeoutMultiplier = WriteTotalMultiplier; //写时间常//量, 总超时的计算公式是：总超时＝时间系数×要求读/写的字符数＋时间常量
	if (!SetCommTimeouts( * hCom, &timeouts))
	{
		return SERIAL_FAILURE;
	}
	else
		return SERIAL_SUCCESS;
}

/*
*	功能：串口写数据
*	参数：
*		[in]  待写入数据缓冲区地址
*		[in]  写入数据长度
*		[out] 返回值为读取到的数据长度
*	说明：
*/
int WriteChar(HANDLE hCom , BYTE* m_szWriteBuffer, DWORD m_nToSend)
{
	BOOL bWrite = TRUE;
	BOOL bResult = TRUE;
	DWORD BytesSent = 0;
	HANDLE    m_hWriteEvent = 0;
	ResetEvent(m_hWriteEvent);
	if (bWrite) {
		m_ov.Offset = 0;
		m_ov.OffsetHigh = 0;
		// Clear buffer
		bResult = WriteFile(hCom,    // Handle to COMM Port, 串口的句柄
			m_szWriteBuffer,        // Pointer to message buffer in calling finction
									// 即以该指针的值为首地址的nNumberOfBytesToWrite
									// 个字节的数据将要写入串口的发送数据缓冲区
			m_nToSend,                // Length of message to send, 要写入的数据的字节数
			&BytesSent,                // Where to store the number of bytes sent
									   // 指向指向一个DWORD数值，该数值返回实际写入的字节数
			&m_ov);                // Overlapped structure
								   // 重叠操作时，该参数指向一个OVERLAPPED结构，
								   // 同步操作时，该参数为NULL
		if (!bResult) {                // 当ReadFile和WriteFile返回FALSE时，不一定就是操作失
									   //败，线程应该调用GetLastError函数分析返回的结果
			DWORD dwError = GetLastError();
			switch (dwError) {
			case ERROR_IO_PENDING: //GetLastError函数返回//ERROR_IO_PENDING。这说明重叠操作还未完成
								   // continue to GetOverlappedResults()
				BytesSent = 0;
				bWrite = FALSE;
				break;
			default:break;
			}
		}
	} // end if(bWrite)
	if (!bWrite) {
		bWrite = TRUE;
		bResult = GetOverlappedResult(hCom,    // Handle to COMM port
			&m_ov,        // Overlapped structure
			&BytesSent,        // Stores number of bytes sent
			TRUE);             // Wait flag

							   // deal with the error code
		if (!bResult) {
			printf("GetOverlappedResults() in WriteFile()");
		}
	} // end if (!bWrite)

	  // Verify that the data size send equals what we tried to send
	if (BytesSent != m_nToSend) {
		printf("WARNING: WriteFile() error.. Bytes Sent: %d; Message Length: %d\n", BytesSent, strlen((char*)m_szWriteBuffer));
	}
	return 1;
}

/*
*	功能：串口写数据
*	参数：
*		[in]  待写入数据缓冲区地址
*		[in]  写入数据长度
*		[out] 返回值为读取到的数据长度
*	说明：
*/
int WriteCharT(HANDLE hCom, BYTE* m_szWriteBuffer, DWORD m_nToSend)
{
	BOOL bWrite = TRUE;
	BOOL bResult = TRUE;
	DWORD BytesSent = 0;
	HANDLE    m_hWriteEvent = 0;
	ResetEvent(m_hWriteEvent);
	if (bWrite) {
		m_ov_T.Offset = 0;
		m_ov_T.OffsetHigh = 0;
		// Clear buffer
		bResult = WriteFile(hCom,    // Handle to COMM Port, 串口的句柄
			m_szWriteBuffer,        // Pointer to message buffer in calling finction
									// 即以该指针的值为首地址的nNumberOfBytesToWrite
									// 个字节的数据将要写入串口的发送数据缓冲区
			m_nToSend,                // Length of message to send, 要写入的数据的字节数
			&BytesSent,                // Where to store the number of bytes sent
									   // 指向指向一个DWORD数值，该数值返回实际写入的字节数
			&m_ov_T);                // Overlapped structure
								   // 重叠操作时，该参数指向一个OVERLAPPED结构，
								   // 同步操作时，该参数为NULL
		if (!bResult) {                // 当ReadFile和WriteFile返回FALSE时，不一定就是操作失
									   //败，线程应该调用GetLastError函数分析返回的结果
			DWORD dwError = GetLastError();
			switch (dwError) {
			case ERROR_IO_PENDING: //GetLastError函数返回//ERROR_IO_PENDING。这说明重叠操作还未完成
								   // continue to GetOverlappedResults()
				BytesSent = 0;
				bWrite = FALSE;
				break;
			default:break;
			}
		}
	} // end if(bWrite)
	if (!bWrite) {
		bWrite = TRUE;
		bResult = GetOverlappedResult(hCom,    // Handle to COMM port
			&m_ov_T,        // Overlapped structure
			&BytesSent,        // Stores number of bytes sent
			TRUE);             // Wait flag

							   // deal with the error code
		if (!bResult) {
			printf("GetOverlappedResults() in WriteFile()");
		}
	} // end if (!bWrite)

	  // Verify that the data size send equals what we tried to send
	if (BytesSent != m_nToSend) {
		printf("WARNING: WriteFile() error.. Bytes Sent: %d; Message Length: %d\n", BytesSent, strlen((char*)m_szWriteBuffer));
	}
	return 1;
}

/*
*	功能：接收数据函数
*	参数：无
*	说明：
*/
void ReceiveChar(void * Com_handle)
{
	HANDLE * hCom = (HANDLE *) Com_handle;
	BOOL  bRead = TRUE;
	BOOL  bResult = TRUE;
	DWORD dwError = 0;
	DWORD BytesRead = 0;
	BOOL mark = FALSE;//是否接收到了反斜杠
	unsigned char RXBuff;
	unsigned char RXBuff_Array[50];
	unsigned char RXBuff_Array_Len = 0;
	while (1) 
	{
		bResult = ClearCommError( * hCom, &dwError, &comstat);
		// 在使用ReadFile 函数进行读操作前，应先使用ClearCommError函数清除错误
		if (comstat.cbInQue == 0)// COMSTAT结构返回串口状态信息 ，本文只用到了cbInQue成员变量，该成员变量的值代表输入缓冲区的字节数
		{
			continue;
		}
		if (bRead) 
		{
			bResult = ReadFile(	* hCom, // Handle to COMM port串口的句柄
								&RXBuff,// RX Buffer Pointer// 读入的数据存储的地址，即读入的数据将存//储在以该指针的值为首地址的一片内存区
								1,// Read one byte要读入的数据的字节数,
								&BytesRead, // Stores number of bytes read, 指向一个DWORD//数值，该数值返回读操作实际读入的字节数
								&m_ov);           // pointer to the m_ov structure// 重叠操作时，该参数指向一个OVERLAPPED结构，同步操作时，该参数为NULL
			//printf("%c", RXBuff);
			if (RXBuff == '$' || RXBuff == '@')
			{
				if (!mark)
				{
					//如果之前没有收到反斜杠，则代表此处开始重新接受一个新的数据
					RXBuff_Array_Len = 0;
					RXBuff_Array[RXBuff_Array_Len++] = RXBuff;
				}
				else
				{
					mark = FALSE;
					RXBuff_Array[RXBuff_Array_Len++] = RXBuff;
				}
			}
			else if(RXBuff == 0x5C)//如果接收到了反斜杠,此处是处理对于数据本身就是反斜杠的处理方式，即反斜杠前面也有反斜杠
			{
				if (!mark)
				{
					//如果之前没有收到反斜杠
					mark = TRUE;
				}
				else
				{
					mark = FALSE;
					RXBuff_Array[RXBuff_Array_Len++] = RXBuff;
				}	
			}
			//if (RXBuff == '$' || RXBuff == '@')
			//{
				//printf("REC new start...\r\n");
				//RXBuff_Array_Len = 0;
			//}
			else if (RXBuff == '#')//
			{
				if (!mark)
				{
					//如果之前没有收到反斜杠，则代表此处开始重新接受一个新的数据
					RXBuff_Array[RXBuff_Array_Len++] = RXBuff;
					while (REC_BUF_LOCKER != LOCKER_FREE) { Sleep(20); }//等待锁释放
					REC_BUF_LOCKER = LOCKER_USED;
					for (int i = 0; i < SERIAL_REC_BUF_NODE_NUM; i++)
					{
						if (srb[i].IsUsed == SERIAL_REC_BUF_NODE_FREE)
						{
							srb[i].IsUsed = SERIAL_REC_BUF_NODE_USED;
							srb[i].len = RXBuff_Array_Len;
							//strcpy(srb[i].data, RXBuff_Array);//拷贝不含'\0'
							for (int j = 0; j < RXBuff_Array_Len; j++)
							{
								srb[i].data[j] = RXBuff_Array[j];
							}
							REC_BUF_LOCKER = LOCKER_FREE;
							LogWrite(INFO, "%s", "Board Com Rec A New Message");
							break;
						}
					}

				}
				else
				{
					mark = FALSE;
					RXBuff_Array[RXBuff_Array_Len++] = RXBuff;
				}
			}
			else
			{
				RXBuff_Array[RXBuff_Array_Len++] = RXBuff;
			}
			
			if (!bResult) 
			{// 当ReadFile和WriteFile返回FALSE时，不一定就是操作失//败，线程应该调用GetLastError函数分析返回的结果
				switch (dwError = GetLastError()) 
				{
					case ERROR_IO_PENDING:
						bRead = FALSE;
						break;
					default:break;
				}
			}
			else 
			{
				bRead = TRUE;
			}
		}  // close if (bRead)
		if (!bRead) 
		{
			bRead = TRUE;
			bResult = GetOverlappedResult(	* hCom,    // Handle to COMM port
											&m_ov,    // Overlapped structure
											&BytesRead,        // Stores number of bytes read
											TRUE);             // Wait flag
		}
		Sleep(50);
	}
}


/*
*	功能：接收数据函数
*	参数：无
*	说明：T是表明温度传感器使用串口
*/
unsigned char Tem[6] = { -100 };//全局温度串口通信数据，在温度初始化函数和温度获取函数中使用
void ReceiveCharT(void * Com_handle)
{
	HANDLE * hCom = (HANDLE *)Com_handle;
	BOOL  bRead = TRUE;
	BOOL  bResult = TRUE;
	DWORD dwError = 0;
	DWORD BytesRead = 0;
	BOOL mark = FALSE;//是否接收到了反斜杠
	unsigned char RXBuff;
	unsigned char RXBuff_Array[50];
	unsigned char RXBuff_Array_Len = 0;
	while (1)
	{
		bResult = ClearCommError(*hCom, &dwError, &comstat_T);
		// 在使用ReadFile 函数进行读操作前，应先使用ClearCommError函数清除错误
		if (comstat_T.cbInQue == 0)// COMSTAT结构返回串口状态信息 ，本文只用到了cbInQue成员变量，该成员变量的值代表输入缓冲区的字节数
		{
			continue;
		}
		if (bRead)
		{
			bResult = ReadFile(*hCom, // Handle to COMM port串口的句柄
				&RXBuff,// RX Buffer Pointer// 读入的数据存储的地址，即读入的数据将存//储在以该指针的值为首地址的一片内存区
				1,// Read one byte要读入的数据的字节数,
				&BytesRead, // Stores number of bytes read, 指向一个DWORD//数值，该数值返回读操作实际读入的字节数
				&m_ov_T);           // pointer to the m_ov structure// 重叠操作时，该参数指向一个OVERLAPPED结构，同步操作时，该参数为NULL
								  //printf("%c", RXBuff);
			if (RXBuff == '$' || RXBuff == '@')
			{
				if (!mark)
				{
					//如果之前没有收到反斜杠，则代表此处开始重新接受一个新的数据
					RXBuff_Array_Len = 0;
					RXBuff_Array[RXBuff_Array_Len++] = RXBuff;
				}
				else
				{
					mark = FALSE;
					RXBuff_Array[RXBuff_Array_Len++] = RXBuff;
				}
			}
			else if (RXBuff == 0x5C)//如果接收到了反斜杠,此处是处理对于数据本身就是反斜杠的处理方式，即反斜杠前面也有反斜杠
			{
				if (!mark)
				{
					//如果之前没有收到反斜杠
					mark = TRUE;
				}
				else
				{
					mark = FALSE;
					RXBuff_Array[RXBuff_Array_Len++] = RXBuff;
				}
			}
			//if (RXBuff == '$' || RXBuff == '@')
			//{
			//printf("REC new start...\r\n");
			//RXBuff_Array_Len = 0;
			//}
			else if (RXBuff == '#')//
			{
				if (!mark)
				{
					//如果之前没有收到反斜杠，则代表此处开始重新接受一个新的数据
					LogWrite(INFO, "%s", "Temperature Com Rec A New Message");
					RXBuff_Array[RXBuff_Array_Len++] = RXBuff;
					for (int j = 0; j < RXBuff_Array_Len; j++)
					{
						Tem[j] = RXBuff_Array[j];
					}
				}
				else
				{
					mark = FALSE;
					RXBuff_Array[RXBuff_Array_Len++] = RXBuff;
				}
			}
			else
			{
				RXBuff_Array[RXBuff_Array_Len++] = RXBuff;
			}
			
			if (!bResult)
			{// 当ReadFile和WriteFile返回FALSE时，不一定就是操作失//败，线程应该调用GetLastError函数分析返回的结果
				switch (dwError = GetLastError())
				{
				case ERROR_IO_PENDING:
					bRead = FALSE;
					break;
				default:break;
				}
			}
			else
			{
				bRead = TRUE;
			}
		}  // close if (bRead)
		if (!bRead)
		{
			bRead = TRUE;
			bResult = GetOverlappedResult(*hCom,    // Handle to COMM port
				&m_ov_T,    // Overlapped structure
				&BytesRead,        // Stores number of bytes read
				TRUE);             // Wait flag
		}
		Sleep(50);
	}
}


/*
*	功能：读取一次串口数据
*	参数：[in]  hcom 串口句柄
[in]	* data 读取出的数据，应保证数组足够长
[out] 返回值为读取到的数据长度
*	说明：
*/
/*
DWORD serial_read_once(HANDLE hCom, char * data)
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
*/

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
DWORD serial_write_once(HANDLE hCom, BYTE * data, DWORD len)
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





