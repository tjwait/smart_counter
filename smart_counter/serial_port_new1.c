
#include "serial_port.h"

/*���ڽ������ݻ�����*/
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
*	���ܣ���ʼ������
*	������
*	˵������ʧ��������˳�
*/
void init_serial_port(HANDLE * hCom , char * port, int baudrate)
{
	//�ȼ򵥵�ʵ������char * �򿪴���
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
		//sys_die("��COMʧ��!\n");
		sprintf(s_buf, "%s Open Failed!", port);
		LogWrite(ERR, "%s", s_buf);
		exit(-1);
	}
		
	else
	{
		//printf("COM�򿪳ɹ���\n");
		sprintf(s_buf, "%s Open SUCCESS!", port);
		LogWrite(INFO, "%s", s_buf);
		memset(s_buf, 0, sizeof(s_buf));
		if (setupdcb(hCom ,baudrate) == 0)
		{
			sprintf(s_buf, "%s Setup DCB SUCCESS", port);
			LogWrite(INFO, "%s", s_buf);
			//printf("setup dcb �ɹ���\n");
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
			//printf("setup timeout �ɹ���\n");
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
		PurgeComm(* hCom , PURGE_RXCLEAR | PURGE_TXCLEAR | PURGE_RXABORT | PURGE_TXABORT); // �ڶ�д����֮ǰ����Ҫ��PurgeComm()������ջ�����
																						 //PURGE_TXABORT      �ж�����д�������������أ���ʹд������û����ɡ�
																						 //PURGE_RXABORT      �ж����ж��������������أ���ʹ��������û����ɡ�
																						 //PURGE_TXCLEAR      ������������
																						 //PURGE_RXCLEAR      ������뻺����
	}

	return SERIAL_SUCCESS;

}

/*
*	���ܣ���ʼ�����ڲ���
*	������[in]  rate_arg ������
*	˵������ʧ��������˳�
*/
int setupdcb(HANDLE * hCom , int rate_arg)
{
	DCB  dcb;
	int rate = rate_arg;
	memset(&dcb, 0, sizeof(dcb)); //��һ���ڴ�������ĳ��������ֵ���ǶԽϴ�Ľṹ//�������������������һ����췽��
	if (!GetCommState( * hCom, &dcb))//��ȡ��ǰDCB����
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
*	���ܣ����ô��ڳ�ʱ����
*	������[in]  hcom ���ھ��
*	˵��������readfile��writefile��д���п�ʱ����Ҫ���ǳ�ʱ����, ��д���ڵĳ�ʱ����
*		  �֣������ʱ���ܳ�ʱ, д����ֻ֧���ܳ�ʱ�������������ֳ�ʱ��֧��, �������
*		  д��ʱ������Ϊ0����ô�Ͳ�ʹ��д��ʱ��
*/

int setuptimeout(HANDLE * hCom , DWORD ReadInterval, DWORD ReadTotalMultiplier, DWORD ReadTotalconstant, DWORD WriteTotalMultiplier, DWORD WriteTotalconstant)
{
	COMMTIMEOUTS timeouts;
	timeouts.ReadIntervalTimeout = ReadInterval; //�������ʱ
	timeouts.ReadTotalTimeoutConstant = ReadTotalconstant; //��ʱ��ϵ��
	timeouts.ReadTotalTimeoutMultiplier = ReadTotalMultiplier; //��ʱ�䳣��
	timeouts.WriteTotalTimeoutConstant = WriteTotalconstant; // дʱ��ϵ��
	timeouts.WriteTotalTimeoutMultiplier = WriteTotalMultiplier; //дʱ�䳣//��, �ܳ�ʱ�ļ��㹫ʽ�ǣ��ܳ�ʱ��ʱ��ϵ����Ҫ���/д���ַ�����ʱ�䳣��
	if (!SetCommTimeouts( * hCom, &timeouts))
	{
		return SERIAL_FAILURE;
	}
	else
		return SERIAL_SUCCESS;
}

/*
*	���ܣ�����д����
*	������
*		[in]  ��д�����ݻ�������ַ
*		[in]  д�����ݳ���
*		[out] ����ֵΪ��ȡ�������ݳ���
*	˵����
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
		bResult = WriteFile(hCom,    // Handle to COMM Port, ���ڵľ��
			m_szWriteBuffer,        // Pointer to message buffer in calling finction
									// ���Ը�ָ���ֵΪ�׵�ַ��nNumberOfBytesToWrite
									// ���ֽڵ����ݽ�Ҫд�봮�ڵķ������ݻ�����
			m_nToSend,                // Length of message to send, Ҫд������ݵ��ֽ���
			&BytesSent,                // Where to store the number of bytes sent
									   // ָ��ָ��һ��DWORD��ֵ������ֵ����ʵ��д����ֽ���
			&m_ov);                // Overlapped structure
								   // �ص�����ʱ���ò���ָ��һ��OVERLAPPED�ṹ��
								   // ͬ������ʱ���ò���ΪNULL
		if (!bResult) {                // ��ReadFile��WriteFile����FALSEʱ����һ�����ǲ���ʧ
									   //�ܣ��߳�Ӧ�õ���GetLastError�����������صĽ��
			DWORD dwError = GetLastError();
			switch (dwError) {
			case ERROR_IO_PENDING: //GetLastError��������//ERROR_IO_PENDING����˵���ص�������δ���
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
*	���ܣ�����д����
*	������
*		[in]  ��д�����ݻ�������ַ
*		[in]  д�����ݳ���
*		[out] ����ֵΪ��ȡ�������ݳ���
*	˵����
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
		bResult = WriteFile(hCom,    // Handle to COMM Port, ���ڵľ��
			m_szWriteBuffer,        // Pointer to message buffer in calling finction
									// ���Ը�ָ���ֵΪ�׵�ַ��nNumberOfBytesToWrite
									// ���ֽڵ����ݽ�Ҫд�봮�ڵķ������ݻ�����
			m_nToSend,                // Length of message to send, Ҫд������ݵ��ֽ���
			&BytesSent,                // Where to store the number of bytes sent
									   // ָ��ָ��һ��DWORD��ֵ������ֵ����ʵ��д����ֽ���
			&m_ov_T);                // Overlapped structure
								   // �ص�����ʱ���ò���ָ��һ��OVERLAPPED�ṹ��
								   // ͬ������ʱ���ò���ΪNULL
		if (!bResult) {                // ��ReadFile��WriteFile����FALSEʱ����һ�����ǲ���ʧ
									   //�ܣ��߳�Ӧ�õ���GetLastError�����������صĽ��
			DWORD dwError = GetLastError();
			switch (dwError) {
			case ERROR_IO_PENDING: //GetLastError��������//ERROR_IO_PENDING����˵���ص�������δ���
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
*	���ܣ��������ݺ���
*	��������
*	˵����
*/
void ReceiveChar(void * Com_handle)
{
	HANDLE * hCom = (HANDLE *) Com_handle;
	BOOL  bRead = TRUE;
	BOOL  bResult = TRUE;
	DWORD dwError = 0;
	DWORD BytesRead = 0;
	BOOL mark = FALSE;//�Ƿ���յ��˷�б��
	unsigned char RXBuff;
	unsigned char RXBuff_Array[50];
	unsigned char RXBuff_Array_Len = 0;
	while (1) 
	{
		bResult = ClearCommError( * hCom, &dwError, &comstat);
		// ��ʹ��ReadFile �������ж�����ǰ��Ӧ��ʹ��ClearCommError�����������
		if (comstat.cbInQue == 0)// COMSTAT�ṹ���ش���״̬��Ϣ ������ֻ�õ���cbInQue��Ա�������ó�Ա������ֵ�������뻺�������ֽ���
		{
			continue;
		}
		if (bRead) 
		{
			bResult = ReadFile(	* hCom, // Handle to COMM port���ڵľ��
								&RXBuff,// RX Buffer Pointer// ��������ݴ洢�ĵ�ַ������������ݽ���//�����Ը�ָ���ֵΪ�׵�ַ��һƬ�ڴ���
								1,// Read one byteҪ��������ݵ��ֽ���,
								&BytesRead, // Stores number of bytes read, ָ��һ��DWORD//��ֵ������ֵ���ض�����ʵ�ʶ�����ֽ���
								&m_ov);           // pointer to the m_ov structure// �ص�����ʱ���ò���ָ��һ��OVERLAPPED�ṹ��ͬ������ʱ���ò���ΪNULL
			//printf("%c", RXBuff);
			if (RXBuff == '$' || RXBuff == '@')
			{
				if (!mark)
				{
					//���֮ǰû���յ���б�ܣ������˴���ʼ���½���һ���µ�����
					RXBuff_Array_Len = 0;
					RXBuff_Array[RXBuff_Array_Len++] = RXBuff;
				}
				else
				{
					mark = FALSE;
					RXBuff_Array[RXBuff_Array_Len++] = RXBuff;
				}
			}
			else if(RXBuff == 0x5C)//������յ��˷�б��,�˴��Ǵ���������ݱ�����Ƿ�б�ܵĴ���ʽ������б��ǰ��Ҳ�з�б��
			{
				if (!mark)
				{
					//���֮ǰû���յ���б��
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
					//���֮ǰû���յ���б�ܣ������˴���ʼ���½���һ���µ�����
					RXBuff_Array[RXBuff_Array_Len++] = RXBuff;
					while (REC_BUF_LOCKER != LOCKER_FREE) { Sleep(20); }//�ȴ����ͷ�
					REC_BUF_LOCKER = LOCKER_USED;
					for (int i = 0; i < SERIAL_REC_BUF_NODE_NUM; i++)
					{
						if (srb[i].IsUsed == SERIAL_REC_BUF_NODE_FREE)
						{
							srb[i].IsUsed = SERIAL_REC_BUF_NODE_USED;
							srb[i].len = RXBuff_Array_Len;
							//strcpy(srb[i].data, RXBuff_Array);//��������'\0'
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
			{// ��ReadFile��WriteFile����FALSEʱ����һ�����ǲ���ʧ//�ܣ��߳�Ӧ�õ���GetLastError�����������صĽ��
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
*	���ܣ��������ݺ���
*	��������
*	˵����T�Ǳ����¶ȴ�����ʹ�ô���
*/
unsigned char Tem[6] = { -100 };//ȫ���¶ȴ���ͨ�����ݣ����¶ȳ�ʼ���������¶Ȼ�ȡ������ʹ��
void ReceiveCharT(void * Com_handle)
{
	HANDLE * hCom = (HANDLE *)Com_handle;
	BOOL  bRead = TRUE;
	BOOL  bResult = TRUE;
	DWORD dwError = 0;
	DWORD BytesRead = 0;
	BOOL mark = FALSE;//�Ƿ���յ��˷�б��
	unsigned char RXBuff;
	unsigned char RXBuff_Array[50];
	unsigned char RXBuff_Array_Len = 0;
	while (1)
	{
		bResult = ClearCommError(*hCom, &dwError, &comstat_T);
		// ��ʹ��ReadFile �������ж�����ǰ��Ӧ��ʹ��ClearCommError�����������
		if (comstat_T.cbInQue == 0)// COMSTAT�ṹ���ش���״̬��Ϣ ������ֻ�õ���cbInQue��Ա�������ó�Ա������ֵ�������뻺�������ֽ���
		{
			continue;
		}
		if (bRead)
		{
			bResult = ReadFile(*hCom, // Handle to COMM port���ڵľ��
				&RXBuff,// RX Buffer Pointer// ��������ݴ洢�ĵ�ַ������������ݽ���//�����Ը�ָ���ֵΪ�׵�ַ��һƬ�ڴ���
				1,// Read one byteҪ��������ݵ��ֽ���,
				&BytesRead, // Stores number of bytes read, ָ��һ��DWORD//��ֵ������ֵ���ض�����ʵ�ʶ�����ֽ���
				&m_ov_T);           // pointer to the m_ov structure// �ص�����ʱ���ò���ָ��һ��OVERLAPPED�ṹ��ͬ������ʱ���ò���ΪNULL
								  //printf("%c", RXBuff);
			if (RXBuff == '$' || RXBuff == '@')
			{
				if (!mark)
				{
					//���֮ǰû���յ���б�ܣ������˴���ʼ���½���һ���µ�����
					RXBuff_Array_Len = 0;
					RXBuff_Array[RXBuff_Array_Len++] = RXBuff;
				}
				else
				{
					mark = FALSE;
					RXBuff_Array[RXBuff_Array_Len++] = RXBuff;
				}
			}
			else if (RXBuff == 0x5C)//������յ��˷�б��,�˴��Ǵ���������ݱ�����Ƿ�б�ܵĴ���ʽ������б��ǰ��Ҳ�з�б��
			{
				if (!mark)
				{
					//���֮ǰû���յ���б��
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
					//���֮ǰû���յ���б�ܣ������˴���ʼ���½���һ���µ�����
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
			{// ��ReadFile��WriteFile����FALSEʱ����һ�����ǲ���ʧ//�ܣ��߳�Ӧ�õ���GetLastError�����������صĽ��
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
*	���ܣ���ȡһ�δ�������
*	������[in]  hcom ���ھ��
[in]	* data ��ȡ�������ݣ�Ӧ��֤�����㹻��
[out] ����ֵΪ��ȡ�������ݳ���
*	˵����
*/
/*
DWORD serial_read_once(HANDLE hCom, char * data)
{
	if (hCom == NULL)
	{
		return SERIAL_FAILURE;
	}
	DWORD wCount;//ʵ�ʶ�ȡ���ֽ���
	int bReadStat;

	//char str[100] = { 0 };

	bReadStat = ReadFile(hCom, data, sizeof(data), &wCount, NULL);
	if (!bReadStat)
	{
		printf("������ʧ��!");
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
	DWORD wCount;//ʵ�ʶ�ȡ���ֽ���
	int bReadStat;

	char str[100] = { 0 };

	while (1)
	{
		//PurgeComm(hCom, PURGE_TXCLEAR | PURGE_RXCLEAR); //��ջ�����
		/* ���ɹ��򷵻ط�0�����ɹ��򷵻�0
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
			printf("������ʧ��!");
			CloseHandle(hCom);
			return FALSE;
		}
		else
		{
			//��ִ��ReadFile���������ɹ�����˶�ȡ�����������û�ж�ȡ���κ����ݣ������Ҳ�ᵽ��˴������Ϊ�����õĶ�ȡ�����жϣ�
			//�˴������ж϶�ȡ���˶��ٸ��ֽ�������ʹ��wCount����
			printf("info : %s\r\n", str);//str��������ȡ�����ݣ��������ݵ����һλ��ӽ���������˴˴����Ὣstr��100��Ԫ��ȫ����ӡ����
			memset(str, 0, strlen(str));
		}
		Sleep(1000);
	}
}

/*
*	���ܣ�дȡһ�δ�������
*	������[in]  hcom ���ھ��
[in]	* data ��д�������
[in]  int ��д�����ݳ���
[out] ִ�н��
*	˵����
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





