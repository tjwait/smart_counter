
#include "stdio.h"
#include <process.h>
#include "database_fun.h"//mysql为64位版本，因此需要在x64模式下调试
#include "amqp_fun.h"
#include "sys.h"
//连接数据库相关全局变量

//mq相关全局变量
int main()
{
	//柜子本地初始化，主要执行是连接数据库，初始化几个结构体数据，然后打开串口，启动串口接收线程，启动串口数据分析线程，测试称重板实际连接
	//是否同数据库内设定一致，锁是否为关闭状态，在初始化以上内容的时候，柜子并未连接网络，此时不会接收任何网络发送过来的命令
	Init_System();

	_beginthread(ReceiveChar, 0, &hCom_C);
	_beginthread(ReceiveCharT, 0, &hCom_Tem);
	_beginthread(Parse_Usart_Data_Run, 0, NULL);
	Board_Ready();
	Locker_Get_Stat();
	Sleep(200);
	Init_Tem();
	_beginthread(Counter_Get_Tem_Ex, 0, NULL);
	//初始化柜子网络内容
	init_amqp();
	//_beginthread(run_listen, 0, NULL);
	while (1)
	{
		run_listen("dummy");
		Destory_connection(conn , atoi(counter->channel));
		Sleep(5000);
		Sleep(5000);
	}
	getchar();
}








