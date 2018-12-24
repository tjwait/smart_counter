
#include "stdio.h"
#include <process.h>
#include "database_fun.h"//mysql为64位版本，因此需要在x64模式下调试
#include "amqp_fun.h"
#include "sys.h"

//连接数据库相关全局变量

//mq相关全局变量
int main()
{
	LogWrite(INFO, "%s", "---------------------------------------------------------------");
	LogWrite(INFO, "%s", "---------------------------------------------------------------");
	LogWrite(INFO, "%s", "Smart Counter System Start!");
	//柜子本地初始化，主要执行是连接数据库，初始化几个结构体数据，然后打开串口，启动串口接收线程，启动串口数据分析线程，测试称重板实际连接
	//是否同数据库内设定一致，锁是否为关闭状态，在初始化以上内容的时候，柜子并未连接网络，此时不会接收任何网络发送过来的命令
	LogWrite(INFO, "%s", "Init System Start!");
	Init_System();

	LogWrite(INFO, "%s", "ReceiveChar thread Start!");
	_beginthread(ReceiveChar, 0, &hCom_C);
	LogWrite(INFO, "%s", "ReceiveCharT thread Start!");
	_beginthread(ReceiveCharT, 0, &hCom_Tem);
	LogWrite(INFO, "%s", "Board Com Data Parse thread Start!");
	_beginthread(Parse_Usart_Data_Run, 0, NULL);
	LogWrite(INFO, "%s", "Check Board State Start!");
	Board_Ready();

	//获取锁的状态的目的是，在获取之后会将锁的状态保存在柜子的信息的结构体中，上位服务器可以通过接口调用获取这个状态
	//据此可以判定柜子是否有异常问题，是否可以执行某一个业务，如门是开着的，那就不能再执行销售流程业务
	LogWrite(INFO, "%s", "Get Locker State Start!");
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








