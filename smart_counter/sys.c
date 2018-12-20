
#include "sys.h"
#include "math.h"

#include "stdio.h"
#include "windows.h"

struct Sys_Tem sys_tem;

/*
*	功能：系统启动初始化
*	参数：
*	说明：系统启动初始化函数胡，流程为
			1、数据库连接初始化
			2、获取柜子基本信息，并填充至结构体中
			3、获取称重板基本信息，并填充至对应结构体中
			4、串口链接初始化
			5、测试称重板是否都链接正常
			6、测试柜子连接服务器链路是否正常

*/
void Init_System(void)
{
	init_db();
	Get_Counter_Info();
	Get_Board_Info();
	//Get_Item_Info();
	init_serial_port( &hCom_C, counter->com_port, 9600);
	init_serial_port(& hCom_Tem, counter->com_port_T, 9600);
	//Sleep(1000);
	//Board_Ready();//这个函数不能放在这里，因为这个函数执行完毕后处理数据的线程才启动
}



/*
*	功能：系统异常输出
*	参数：
*	说明：
*/
//这个函数是一个可变参数函数，主要用于日志输出，在此分析一下
void sys_die(const char *fmt, ...) {
	va_list ap;//这个是一个char * 
	va_start(ap, fmt);//此宏是将ap指向省略号"..."前面最后一个变量的地址，此例子为fmt
	vfprintf(stderr, fmt, ap);//此函数是将fmt输出按照ap参数要求发送到stderr流中，stderr流为系统定义的异常输出流，fmt可以理解为一个字符串，ap可以认为是
							  //是fmt字符串中格式化输出的参数，如“aa%d”，则在ap中要有%d所对应的值的，因此我认为ap在调用va_start时指向了fmt，但其实际
							  //使用时是从第二个元素开始使用的
	va_end(ap);//释放ap资源
	fprintf(stderr, "\n");//输出stderr流信息
	exit(1);//退出系统
}

/*
*	功能：检测柜子连接的称重板是否正常
*	参数：
*	说明：若异常则退出程序

*/
void Board_Ready(void)
{
	struct Board_Info * p = board_info;
	//在发送握手指令前先将板子状态都设置为unknow状态
	while (p != NULL)
	{
		p->Board_Stat = BOARD_STAT_UNKNOW;

		p = p->next;
	}
	p = board_info;
	char send_buf[6];
	send_buf[0] = 0x24;
	send_buf[1] = 0x00;//这个值是地址位高，一会再循环中要更改
	send_buf[2] = 0x00;//这个值是地址位低，一会再循环中要更改
	send_buf[3] = 0x06;
	send_buf[5] = 0x23;
	while (p != NULL)
	{
		send_buf[1] = ((ASCII_To_byte(p->id[0]) << 4) & 0xf0) + ASCII_To_byte(p->id[1]);
		send_buf[2] = ((ASCII_To_byte(p->id[2]) << 4) & 0xf0) + ASCII_To_byte(p->id[3]);
		send_buf[4] = Sum_Check(&send_buf[1], 3);
		WriteChar(hCom_C, send_buf, 6);
		Sleep(1000);

		p = p->next;
	}
	
	//Sleep(5000);//握手数据全部发送完毕后，等待，此时间是否合适还不确定
	//此时判断所有称重板的状态，不应该有未连接的情况
	p = board_info;
	while (p != NULL)
	{
		if (p->Board_Stat != BOARD_STAT_OK)
		{
			//称重板连接异常
			sys_die("Board Ready");
		}
		printf("称重板 %s 连接正常！\r\n" , p->id);
		p = p->next;
	}
	printf("称重板连接检测正常！\r\n");

}

/*
*	功能：去皮校准并且保存
*	参数：[in] 去皮次数
*	说明：函数在调用时将board_info结构体中的IsBasicVale设定为0，即为未去皮，然后执行去皮，并等待一段时间后判定是否去皮成功
*/
int Board_Basic_Value_Set(int times)
{
	struct Board_Info * p = board_info;
	int boards_num = 0;
	printf("等待去皮完成");
	while (p != NULL)
	{
		p->IsBasicValue = 0;//不论是否已经去过皮都设定为未去皮状态
		p->IsBasicValueSave = 0;//去皮未保存
		for (int i = 0; i < times; i++)
		{
			printf("...");
			Send_CMD(&hCom_C, p->id, BOARD_CMD_BASIC_VALUE, NULL, 0, 1 , 0);
			Sleep(5000);
		}
		printf("...");
		Send_CMD(&hCom_C, p->id, BOARD_CMD_BASIC_VALUE_SAVE, NULL, 0, 1 , 0);
		Sleep(1000);
		if (p->IsBasicValue == 0 || p->IsBasicValueSave == 0)
		{
			//去皮失败
			printf("\r\n去皮失败！！\r\n");
			return 1;
		}

		p = p->next;
	}
	printf("  去皮并且保存成功！\r\n");
	return 0;

}

/*
*	功能：去皮校准并且保存,并向服务器发送json数据
*	参数：[in] 去皮次数
*	说明：函数在调用时将board_info结构体中的IsBasicVale设定为0，即为未去皮，然后执行去皮，并等待一段时间后判定是否去皮成功
*			根据校准函数返回值来判断，若为0则代表成功，若为1则代表失败
*		  #注：由于上位机发送过来的校准命令，柜体对所有称重板执行校准命令，因此该函数的返回值应该是所有的称重板是否都已经去皮
*			正常，应该是若有任意一个称重板去皮失败，则命令失败，并且在data域中反馈每一个称重板去皮的结果
*/
char * Board_Basic_Value_Set_With_ACK()
{
	char change_code[100];
	int return_value = Board_Basic_Value_Set(2);//去皮的次数这个参数暂未对外开放
	JSON_Value *root_value = json_value_init_object();//初始化了一个value，其类型为object，value为json数据的一个类型，不同的value类型可以嵌套
	JSON_Object *root_object = json_value_get_object(root_value);//获取value中的object的地址

	JSON_Value *sub_value = json_value_init_array();
	JSON_Array *sub_array = json_value_get_array(sub_value);

	time_t timep;
	time(&timep);
	//printf("%s", ctime(&timep));
	json_object_set_string(root_object, "devid", counter->sn);
	json_object_set_string(root_object, "cmdid", "Basic_Value");
	Int_To_CharArray(return_value, change_code);
	json_object_dotset_string(root_object, "Result.Res", change_code);
	json_object_dotset_string(root_object, "Result.Date", ctime(&timep));
	//json_object_set_string(root_object, "Counter-SN", counter->sn);
	json_object_dotset_value(root_object, "Result.Data", "NULL");//没有返回数据
	char * json_string = (char *)malloc(json_serialization_size(root_value));//json_serialization_size这个函数的返回值已经考虑了将原字符串长度进行扩展，因为还要增加一些额外的字符，如换行、反斜杠、大括号等
	json_serialize_to_buffer(root_value, json_string, json_serialization_size(root_value));
	//释放json资源
	json_value_free(root_value);//只需释放根资源，其内部关联的所有资源会被递归释放

	return json_string;//改指针会随着函数跳出而失效，但指向的该内存必须在外部显式释放

}


/*
*	功能：曲率校准
*	参数：[in] 校准次数
		  [in] 是否保存曲率值 0 不保存  1保存
*	说明：目前未对返回值进行任何处理
*
*/
void Board_Curavture_Value_Set( int times , int save)
{
	
	struct Board_Info * p = board_info;
	
	if (!sys_tem.IsCheck)
	{
		//温度没有变化无需校准
		printf("最后一次测量温度为 %d 摄氏度 ， 系统初始温度为 %d 摄氏度，无需校准\r\n", sys_tem.Tem_Cur, sys_tem.Tem);
		return;
	}
	else
	{
		//温度变化及时间满足校准要求，先将温度结构体数据进行调整
		sys_tem.Tem = sys_tem.Tem_Cur;
		sys_tem.Time = 0;
		sys_tem.IsCheck = 0;
	}

	while (p != NULL)
	{
		if (p->board_items_weight_all < 1000)//如果校准的货物重量小于1000克，则不执行校准，是否合适还需要测试
		{
			//printf("校准货物过轻-- %d  ，采用原有曲率测量\r\n", p->board_items_weight_all);
			printf("称重板 %s 货物过轻  %d , 无法校准，采用原有曲率测量", p->id, p->board_items_weight_all);
			p = p->next;
			continue;
		}

		p->ISCurvatureValue = 0;//设置成未校准状态
		p->ISCurvatureValueSave = 0;//设置成未保存状态
		for (int i = 0; i < times; i++)
		{
			Send_CMD(&hCom_C, p->id, BOARD_CMD_CURVATURE, (unsigned char *)&p->board_items_weight_all, 2, 1, 1);
			printf("...");
			Sleep(5000);
			if (p->ISCurvatureValue == 0)
			{
				//校准失败
				printf("\r\n校准失败！！按任意键退出程序\r\n");
				getchar(1);
				sys_die("\r\n校准失败！！\r\n");
			}
		}
		if (save == 1)
		{
			Send_CMD(&hCom_C, p->id, BOARD_CMD_CURVATURE_SAVE, (unsigned char *)&p->board_items_weight_all, 2, 1, 1);
			printf("...");
			Sleep(1000);
			if (p->ISCurvatureValueSave == 0)
			{
				//校准失败
				printf("\r\n校准保存失败！！按任意键退出程序\r\n");
				getchar(1);
				sys_die("\r\n校准保存失败！！\r\n");
			}
		}
		printf("称重板 %s 校准完成！\r\n", p->id);
		p = p->next;

	}

	/*
	if ( p->board_items_weight_all < 1000)//如果校准的货物重量小于1000克，则不执行校准，是否合适还需要测试
	{
		printf("校准货物过轻-- %d  ，采用原有曲率测量\r\n", p->board_items_weight_all);
		return;
	}
	if (!sys_tem.IsCheck)
	{
		//温度没有变化无需校准
		printf("最后一次测量温度为 %d 摄氏度 ， 系统初始温度为 %d 摄氏度，无需校准\r\n" ,sys_tem.Tem_Cur , sys_tem.Tem);
		return;
	}
	else
	{
		//温度变化及时间满足校准要求，先将温度结构体数据进行调整
		sys_tem.Tem = sys_tem.Tem_Cur;
		sys_tem.Time = 0;
		sys_tem.IsCheck = 0;
	}
	printf("等待称重板  %s  校准完成" , p->id);

		p->ISCurvatureValue = 0;//设置成未校准状态
		p->ISCurvatureValueSave = 0;//设置成未保存状态
		for (int i = 0; i < times; i++)
		{
			Send_CMD(&hCom_C, p->id, BOARD_CMD_CURVATURE, (unsigned char *)&p->board_items_weight_all, 2, 1 , 1);
			printf("...");
			Sleep(5000);
			if (p->ISCurvatureValue == 0)
			{
				//校准失败
				printf("\r\n校准失败！！按任意键退出程序\r\n");
				getchar(1);
				sys_die("\r\n校准失败！！\r\n");
			}
		}
		if (save == 1)
		{
			Send_CMD(&hCom_C, p->id, BOARD_CMD_CURVATURE_SAVE, (unsigned char *)&p->board_items_weight_all, 2, 1 , 1);
			printf("...");
			Sleep(1000);
			if (p->ISCurvatureValueSave == 0)
			{
				//校准失败
				printf("\r\n校准保存失败！！按任意键退出程序\r\n");
				getchar(1);
				sys_die("\r\n校准保存失败！！\r\n");
			}
		}

	printf("称重板 %s 校准完成！\r\n" , p->id);
	*/
}

/*
*	功能：读取称重板当前克重
*	参数：
*	说明：目前未对返回值进行任何处理
*/
void Board_Get_Weight()
{
	struct Board_Info * p = board_info;
	while (p != NULL)
	{
		p->board_items_weight_all_after_close_door = 0;//该值每次使用前都清零
		Send_CMD(&hCom_C, p->id, BOARD_CMD_GET_WEIGHT, NULL, 0, 1, 0);
		Sleep(5000);
		p = p->next;
	}
	printf("读取重量完成！\r\n");
}

/*
*	功能：初始化温度结构体
*	参数：
*	说明：
*/
void Init_Tem()
{
	sys_tem.delay = 60000;
	sys_tem.IsCheck = 0;
	sys_tem.MaxTime = 5;
	sys_tem.Tem = 0;
	sys_tem.Tem_Dis = 5;//即温度偏差在5度内，不予校准
	sys_tem.Time = 0;
	sys_tem.Tem_Cur = 0;

	printf("获取初始化温度\r\n");
	char send_buf[6];
	send_buf[0] = '@';
	send_buf[1] = 0xFF;//这个值是地址位高，一会再循环中要更改
	send_buf[2] = 0xFE;//这个值是地址位低，一会再循环中要更改
	send_buf[3] = 0xFE;
	send_buf[4] = 0xFB;
	send_buf[5] = '#';
	WriteCharT(hCom_Tem, send_buf, 6);
	Sleep(2000);
	if (Tem[4] != -100)
	{
		printf("获取初始化温度正常  %d 摄氏度\r\n" , Tem[4]);
		sys_tem.Tem_Cur = sys_tem.Tem = Tem[4];
	}
	else
	{
		printf("获取初始化温度异常\r\n");
	}

}

/*
*	功能：获取柜子温度
*	参数：
*	说明：
*			代码的执行策略是，每指定时间获取一次温度值，若温度变化超过阀值，则会增加一次超出记录，若没有超出阀值，但超出记录不为0的时候，则超出次数减1
*			当超出此处到达最大值的时候，会触发校准阀值，这个值在执行称重的命令的时候，会被判断，如果被置位，则会在称重前执行曲率校准命令，并将校准置位
*			及超出阀值的次数进行恢复
*/
void Counter_Get_Tem()
{

	printf("开始周期检测温度，周期为 %d 毫秒\r\n" , sys_tem.delay);
	while(1)
	{ 
		Sleep(sys_tem.delay);
		printf("获取温度\r\n");
		char send_buf[6];
		send_buf[0] = '@';
		send_buf[1] = 0xFF;//这个值是地址位高
		send_buf[2] = 0xFE;//这个值是地址位低
		send_buf[3] = 0xFE;
		send_buf[4] = 0xFB;
		send_buf[5] = '#';
		WriteCharT(hCom_Tem, send_buf, 6);
		Sleep(2000);
		if (Tem[0] == '@' &&  Tem[4] != -100 && Tem[6] == '#')//-100是接收数据数组的默认值，如果为-100，则代表没有收到有效数据
		{
			sys_tem.Tem_Cur = (signed char)Tem[4];
			printf("获取温度正常 %d \r\n", sys_tem.Tem_Cur);

			if (sys_tem.Time == sys_tem.MaxTime)
			{
				sys_tem.IsCheck = 1;//改为最终会在校准函数中回复为0
			}
			else if(sys_tem.Time == 0)
			{
				sys_tem.IsCheck = 0;
			}

			if (abs(sys_tem.Tem - sys_tem.Tem_Cur) > sys_tem.Tem_Dis)//如果温度偏差超过容许最大值,并且还没有满足执行校准的时间要求的时候
			{
				if (sys_tem.Time < sys_tem.MaxTime)
				{
					sys_tem.Time++;
				}
				
			}
			else
			{
				if (sys_tem.Time > 0)
				{
					sys_tem.Time -= 1;
				}
			}
		}
		else
		{
			printf("温度传感器校验异常\r\n");
		}
		for (int i = 0; i < 7; i++)
		{
			Tem[i] = -100;//0x9C 这个值如果是放在有符号字符型就是-100 如果是无符号字符型就是156
		}
		//Send_CMD(&hCom_Tem, "FFFE", LOCKER_CMD_TEM, NULL, 0, 2, 0);//id 号为固定

	}
}



/*
*	功能：获取柜子温度
*	参数：
*	说明：
*			代码的执行策略更新后是将每一个称重板都设定一个是否需要校准的判断位，当触发校准条件的时候，将各个称重板校准位置位，然后更新当前温度值，
*			不再有回复的策略，即当温度恢复至原始温度也不会回复校准置位，只有校准才能将称重板校准位回复
*/
void Counter_Get_Tem_Ex()
{

	printf("\r\n开始周期检测温度，周期为 %d 毫秒\r\n", sys_tem.delay);
	while (1)
	{
		Sleep(sys_tem.delay);
		printf("\r\n获取温度\r\n");
		char send_buf[6];
		send_buf[0] = '@';
		send_buf[1] = 0xFF;//这个值是地址位高
		send_buf[2] = 0xFE;//这个值是地址位低
		send_buf[3] = 0xFE;
		send_buf[4] = 0xFB;
		send_buf[5] = '#';
		WriteCharT(hCom_Tem, send_buf, 6);
		Sleep(2000);
		if (Tem[0] == '@' &&  Tem[4] != -100 && Tem[6] == '#')//-100是接收数据数组的默认值，如果为-100，则代表没有收到有效数据
		{
			sys_tem.Tem_Cur = (signed char)Tem[4];
			printf("获取温度正常 %d \r\n", sys_tem.Tem_Cur);

			if (sys_tem.Time == sys_tem.MaxTime)
			{
				//sys_tem.IsCheck = 1;//改为最终会在校准函数中回复为0
				struct Board_Info * p = board_info;
				while (p != NULL)
				{
					p->ISNeedCur = ISNEEDCUR_YES;
					p = p->next;
				}
				sys_tem.Tem = sys_tem.Tem_Cur;
				sys_tem.Time = 0;
			}

			if (abs(sys_tem.Tem - sys_tem.Tem_Cur) > sys_tem.Tem_Dis)//如果温度偏差超过容许最大值,并且还没有满足执行校准的时间要求的时候
			{
				if (sys_tem.Time < sys_tem.MaxTime)
				{
					sys_tem.Time++;
				}

			}
		}
		else
		{
			printf("温度传感器校验异常\r\n");
		}
		for (int i = 0; i < 7; i++)
		{
			Tem[i] = -100;//0x9C 这个值如果是放在有符号字符型就是-100 如果是无符号字符型就是156
		}
		//Send_CMD(&hCom_Tem, "FFFE", LOCKER_CMD_TEM, NULL, 0, 2, 0);//id 号为固定

	}
}


/*
*	功能：开锁
*	参数：[out]	LOCKER_UNLOCK_STATE_OK 0x00//锁吸回正常，并且们已经被打开
				LOCKER_UNLOCK_STATE_OPEN_ALREADY 0x02//在发送开门指令的时候，们已经打开了
				LOCKER_UNLOCK_STATE_DONOT_OPEN 0x03//锁打开正常，但是门长时间没有被打开，又自动关上了
				LOCKER_UNLOCK_STATE_ERROR 0xFF//锁严重错误
				GEN_ERROR_COMMUNICATION_ERROR 0xFE //通信错误，该错误是一个通用错误，是指控制器同嵌入式设备通信异常

*	说明：这个函数会执行一次开门，并尝试等待一定时间，然后将是否开门的结果返回
*
*/
int Locker_Open()
{
	/*
	if (counter->locker_stat != 2 || counter->IsBusy != -1)//即门没有处于关闭状态,或者此时柜子正在执行其他命令
	{
		printf("无法执行开门\r\n");
		return;
	}
	*/
	//counter->IsBusy = 1;
	counter->locker_stat = -1;//赋值-1，用以判定该函数的返回值
	Send_CMD(&hCom_C, counter->locker_id, LOCKER_CMD_UNLOCK, NULL, 0, 2, 0);
	Sleep(5000);
	Sleep(5000);
	if (counter->locker_stat == -1)//考虑到此函数的嵌入式返回值存在一个bug，因此此处只判定是否有返回值
	{
		printf("未接收到锁的返回数据！\r\n");
		return GEN_ERROR_COMMUNICATION_ERROR;
	}
	return counter->locker_stat;
	//while (counter->locker_stat != 2)//在门没有关闭之前都会循环,此处要根据业务需要增加超时报警退出此循环的功能
	//{
	//	Locker_Get_Stat();
	//	Sleep(2000);
	//}
	//counter->IsBusy = -1;
	//counter->locker_stat = -1;
}

/*
*	功能：执行一次开关门
*	参数：[out]	LOCKER_GET_STATE_OPENING 0x01//门正在被打开
				LOCKER_GET_STATE_OK 0x02//门正常执行了一次开关动作
				LOCKER_GET_STATE_WAITING_OPEN 0x04//门正在等待被打开
				LOCKER_GET_STATE_ERROR 0xFF//锁严重错误
				GEN_ERROR_COMMUNICATION_ERROR 0xFE //通信错误，该错误是一个通用错误，是指控制器同嵌入式设备通信异常

*	说明：每次等待1分钟，然后判断门的状态，并返回，具体不同的状态逻辑如何处理交由调用代码

*/
int Locker_Open_Closed()
{
	/*
	if (counter->IsBusy != -1)//即门没有处于关闭状态,或者此时柜子正在执行其他命令
	{
		printf("无法执行开门\r\n");
		return;
	}
	*/
	int wait_time = 0;
	counter->locker_stat = -1;//赋值-1，用以判定该函数的返回值
	Send_CMD(&hCom_C, counter->locker_id, LOCKER_CMD_UNLOCK, NULL, 0, 2, 0);
	Sleep(5000);
	Sleep(5000);
	if (counter->locker_stat == -1)//考虑到此函数的嵌入式返回值存在一个bug，因此此处只判定是否有返回值
	{
		printf("未接收到锁的返回数据！\r\n");
		return GEN_ERROR_COMMUNICATION_ERROR;
	}

	do 
	{
		Locker_Get_Stat();
		Sleep(2000);
	} while (counter->locker_stat != 2 && ((wait_time++) < 15));

	//while (counter->locker_stat != 2 && ((wait_time ++) < 30))//在门没有关闭之前都会循环,此处要根据业务需要增加超时报警推出此循环的功能
	//{
	//	Locker_Get_Stat();
	//	Sleep(2000);
	//}
	printf("执行一次开关门的结果 ：%d\r\n" , counter->locker_stat);
	return counter->locker_stat;
	//counter->IsBusy = -1;
	//counter->locker_stat = -1;
}

/*
*	功能：获取锁状态
*	参数：[out]	LOCKER_GET_STATE_OPENING 0x01//门正在被打开
				LOCKER_GET_STATE_OK 0x02//门正常执行了一次开关动作
				LOCKER_GET_STATE_WAITING_OPEN 0x04//门正在等待被打开
				LOCKER_GET_STATE_ERROR 0xFF//锁严重错误
				GEN_ERROR_COMMUNICATION_ERROR 0xFE //通信错误，该错误是一个通用错误，是指控制器同嵌入式设备通信异常
*	说明：目前未对返回值进行任何处理

*/
int Locker_Get_Stat()
{
	//counter->IsBusy = 1;
	counter->locker_stat = -1;//赋值-1，用以判定该函数的返回值
	Send_CMD(&hCom_C, counter->locker_id, LOCKER_CMD_STATUS, NULL, 0, 2, 0);
	Sleep(2000);
	printf("获取锁状态：%d \r\n" , counter->locker_stat);
	return counter->locker_stat;
	//counter->IsBusy = -1;
	//printf("开锁完成！\r\n");
}

/*
*	功能：完成柜子整个销售流程，这个函数是面向销售的
*	参数：[out]返回函数生成上行清单的指针地址，如果没有生成则返回null
*	说明：从开锁至生成账单
*			生成账单形式是一个json，其组成见onenote笔记
*/
char *  Procedure_Sales()
{
	Get_Item_Info();//重新构建各称重板的items链表结构,这个函数也可以放在当货物有更改的时候再执行，暂不调整，待日后完善再调整
	Get_Boards_Items_Weight();//获取各层称重板上货物重量
	//开门前校准
	//struct Board_Info * board_info_p = board_info;
	//while (board_info_p != NULL)
	//{
		Board_Curavture_Value_Set( 2, 1);//校准和保存
		//board_info_p = board_info_p->next;
	//}
	//开门并等待关门
	Locker_Open();//目前是阻塞式的，直到关门
	Board_Get_Weight();//获取每个称重上现有货物的重量

	JSON_Value *root_value = json_value_init_object();//初始化了一个value，其类型为object，value为json数据的一个类型，不同的value类型可以嵌套
	JSON_Object *root_object = json_value_get_object(root_value);//获取value中的object的地址

	JSON_Value *sub_value = json_value_init_array();
	JSON_Array *sub_array = json_value_get_array(sub_value);

	JSON_Value *sub_sub_value[100];//此处定义的数量实际上是按照这个柜子能够放置的商品种类最大值，但该值被存在数据库，
								   //因此无法直接用于定义固定长度数组，因此此处设置一个较为大的值替代
	JSON_Array *sub_sub_array = NULL;
	int sub_sub_value_pos = 0;//使用的value的位置
	//JSON_Value *sub_sub_value = json_value_init_array();
	//JSON_Array *sub_sub_array = json_value_get_array(sub_sub_value);
	time_t timep;
	
	//printf("%s", ctime(&timep));
	json_object_set_string(root_object, "devid", counter->sn);
	json_object_set_string(root_object, "cmdid", "Shopping");
	//time(&timep);
	//json_object_set_string(root_object, "Date", ctime(&timep));
	//json_object_set_string(root_object, "Counter-SN", counter->sn);

	int list_items_num = 1;//购物单中的商品顺序号
	int Isale = 0;//如果在接下来的判定中，有商品被销售或者是取放了，此值变成1，退出while循环后执行消息发送函数
	//board_info_p = board_info;
	struct Board_Info * board_info_p = board_info;
	while (board_info_p != NULL)//轮询所有称重板的称得的重量
	{
		/*UINT16*/ double weight_buf =  board_info_p->board_items_weight_all - board_info_p->board_items_weight_all_after_close_door;
		if (abs(weight_buf) >= 0 && abs(weight_buf) <= ERROR_VALUE)//若重量变化小于误差值则认为没变化，注意无法是指正负误差，所以使用abs函数处理
		{
			//weight_buf = 0;//若小于10 则认为没有销售
			//代码到达此处则认为这个称重板没有任何货物变动
			board_info_p = board_info_p->next;
			continue;
		}
		else if (weight_buf < - ERROR_VALUE)//若有东西被放进称重板，则最终测量的结果会是负值，而且其值会小于负的最大误差值才对
		{
			//若代码到达此处，则代表有货物或者是异物被 放置到了这个称重板上，出现此种情况，要分析是销售环节还是取放货环节
			//若为销售环节则此处应该为异常，如果为取放货环节应该为正常，要启动分析程序，并且将放置的货物数量调整更新至数据库
		}
		else //正常销售
		{
			Isale = 1;
			//如果测量后发现最终某一个称重板拿走的货物重量同在拿货之前的总重量相差小于等于称重误差值，则认为拿走的就是柜子某一个称重板
			//上所有的货物，此处设定是为了避免出现货物拿不净的情况，即实际拿走了500克，但由于误差，那完货物后称重板称的重量为1克，这样
			//系统计算就相当于拿走了499克
			//此处处理适用于两种销售模式，因此先统一执行
			if (abs(weight_buf - board_info_p->board_items_weight_all) <= ERROR_VALUE)
			{
				//如果拿走货物的重量同开门的货物的数据库重量相差值为在误差以内，则认为拿走的就是全部的数据库的标定的数据
				//因此将拿走的货物重量设定为开门前数据库中的重量，将关门后的重量设定为0
				weight_buf = board_info_p->board_items_weight_all;
				//如果进入此if语句，对拿走的货物重量再重新计算后，关门后的称得的重量也要从新计算
				board_info_p->board_items_weight_all_after_close_door = 0;
			}


			if ( strcmp(board_info_p->type , "1") == 0)//按公斤销售
			{
				unsigned char change_code_buf[200] = { 0 };
				sub_sub_value[sub_sub_value_pos++] = json_value_init_array();//用一个实例化一个
				sub_sub_array = json_value_get_array(sub_sub_value[sub_sub_value_pos - 1]);
				GBKToUTF8(board_info_p->items->name, change_code_buf, 200);//商品名称汉子格式转换,此处主要如果商品名字不是汉子还不知道是否会报错
				json_array_append_string(sub_sub_array, change_code_buf);//商品名称
				json_array_append_string(sub_sub_array, board_info_p->type);//计价类型
				json_array_append_string(sub_sub_array, board_info_p->items->weight_price);//公斤价
				Int_To_CharArray(weight_buf, change_code_buf);
				json_array_append_string(sub_sub_array, change_code_buf);//购买的重量
				Double_To_CharArray(CharNum_To_Double(board_info_p->items->weight_price) * (weight_buf / 500.0)/*将克重换算成斤*/, change_code_buf);
				json_array_append_string(sub_sub_array, change_code_buf);//本条记录总价，总价不是指整个消费单的总价格
				json_array_append_value(sub_array, sub_sub_value[sub_sub_value_pos - 1]);
				/*
				unsigned char item_list_num_buf[20] = {0};//清单中的商品编号
				unsigned char item_list_buf[100] = { 0 };//清单中的商品信息
				unsigned char change_code_buf[200] = { 0 };
				Int_To_CharArray(list_items_num++, change_code_buf);
				strcat(item_list_num_buf, change_code_buf);

				GBKToUTF8(board_info_p->items->name, change_code_buf, 200);
				strcat(item_list_buf, change_code_buf);//商品名称
				strcat(item_list_buf, "#");
				strcat(item_list_buf, board_info_p->type);//计量方式
				strcat(item_list_buf, "#");
				strcat(item_list_buf, board_info_p->items->weight_price);//公斤价
				strcat(item_list_buf, "#");
				Int_To_CharArray(weight_buf, change_code_buf);
				strcat(item_list_buf, change_code_buf);//购买的重量 克
				strcat(item_list_buf, "#");
				*/
				//Double_To_CharArray(CharNum_To_Double(board_info_p->items->weight_price) * (weight_buf/500.0)/*将克重换算成斤*/, change_code_buf);
				/*
				strcat(item_list_buf, change_code_buf);//总价格，元
				strcat(item_list_buf, "#");
				json_object_set_string(root_object, item_list_num_buf, item_list_buf);
				*/
				//更新此条商品记录中的库存重量
				Int_To_CharArray(board_info_p->board_items_weight_all_after_close_door, change_code_buf);
				SQL_UPDATA1("smart_sales_counter.items", "weight_sum", change_code_buf \
							, "item_id", board_info_p->items->item_id); 
			}
			else if (strcmp(board_info_p->type, "2") == 0)//按单品数量销售
			{

			}
		}

		board_info_p = board_info_p->next;
	}
	if (Isale == 1)
	{
		time(&timep);
		json_object_dotset_string(root_object, "Result.Res", "0");//SUCCESS 更改为 0
		json_object_dotset_string(root_object, "Result.Date", ctime(&timep));
		//json_object_set_string(root_object, "Counter-SN", counter->sn);
		json_object_dotset_value(root_object, "Result.Data", sub_value);
		//将json字符序列化处理，并深拷贝至一个字符串中，函数返回该字符串，
		char * json_string = (char *)malloc(json_serialization_size(root_value));//json_serialization_size这个函数的返回值已经考虑了将原字符串长度进行扩展，因为还要增加一些额外的字符，如换行、反斜杠、大括号等
		json_serialize_to_buffer(root_value, json_string, json_serialization_size(root_value));
		//释放json资源
		json_value_free(root_value);//只需释放根资源，其内部关联的所有资源会被递归释放

		return json_string;//改指针会随着函数跳出而失效，但指向的该内存必须在外部显式释放
	}
	else
	{
		//没有货物被销售，此处还有可能要处理一些异常的应答消息，暂时只写当没有货物被销售的时候的应答
		time(&timep);
		json_object_dotset_string(root_object, "Result.Res", "SUCCESS");
		json_object_dotset_string(root_object, "Result.Date", ctime(&timep));
		//json_object_set_string(root_object, "Counter-SN", counter->sn);
		json_object_dotset_value(root_object, "Result.Data", "NULL");//代表没有货物被销售
																	 //将json字符序列化处理，并深拷贝至一个字符串中，函数返回该字符串，
		char * json_string = (char *)malloc(json_serialization_size(root_value));//json_serialization_size这个函数的返回值已经考虑了将原字符串长度进行扩展，因为还要增加一些额外的字符，如换行、反斜杠、大括号等
		memset(json_string, 0, json_serialization_size(root_value));
		json_serialize_to_buffer(root_value, json_string, json_serialization_size(root_value));
		//释放json资源
		json_value_free(root_value);//只需释放根资源，其内部关联的所有资源会被递归释放

		return json_string;//改指针会随着函数跳出而失效，但指向的该内存必须在外部显式释放
	}
	return NULL;

}

/*
*	功能：货物上架，只向一个称重板提供商品上架
*	参数：[in] 接收到的json数据
*		  [out] 返回的结果
*	说明：数据格式为 Devid :  ？？   Cmdid:??    Data:[  [ ]   [ ]   [ ] … [ ] ]，其中data各个子数组的顺序应该同数据库中items顺序相同
*         上架的方式：
*					 如果销售模式为1 则要判定对应称重板中是否已经有商品如果有则上架失败（此处判定是通过counter结构体中items链表是否为空来判断）
*					 要用户先下架方可完成上架，如果没有则判断发送过来的data域中的子集数量是否为1，
*					 如果不唯一则上架失败（应该不能发生，服务端应该控制该种方式的出现）提醒用户只能上架一个商品
*					 如果上架多个商品并且销售模式不为1，那所有的商品所要上架的目标称重板编号应该相同，否则上架失败。
*					 模式2的上架方式暂未确定，此处暂不表达
*/
char *  Procedure_On_Shelf(JSON_Object * json_object)
{
	int error_code = 0;//0代表一切正常
	//先判定接收到的json字符串中的data域中的数据是否合法
	JSON_Array * sub_array_parse = json_object_get_array(json_object, "Data");
	JSON_Array  * sub_sub_array_parse = NULL;
	if (sub_array_parse != NULL)
	{
		if (json_array_get_count(sub_array_parse) > 1)//如果数据域中的子数组的数量大于1，则代表上传了多个商品信息，因此销售模式只能为2，否则数据错误
		{
			for (int i = 0; i < json_array_get_count(sub_array_parse); i++)//每一个传送过来的商品信息中的销售模式都必须为不等于1才行
			{
				sub_sub_array_parse = json_array_get_array(sub_array_parse, i);
				//因为检测到有子数组至少有两个，如果销售模式等于1，那就为错误
				if (CharNum_To_Double(json_array_get_string(sub_sub_array_parse, 5)) == 1)
				{
					//商品销售模式设定错误
					error_code = 1;
					break;
				}
			}
		}
		if (json_array_get_count(sub_array_parse) > 1 && (!error_code))
		{
			//代码至此代表上传的商品信息的销售模式没有错误，以下判定如果上传的商品信息有多条，那所有的商品信息所对应的称重板编号都必须相同
			sub_sub_array_parse = json_array_get_array(sub_array_parse, 0);//获取第一个子数组的称重板编号，后面比对都是跟第一个比
			char * board_id = json_array_get_string(sub_sub_array_parse, 2);
			for (int i = 0; i < json_array_get_count(sub_array_parse); i++)//每一个传送过来的商品信息中的称重板编号必须相同
			{
				sub_sub_array_parse = json_array_get_array(sub_array_parse, i);
				if (strcmp(board_id, json_array_get_string(sub_sub_array_parse, 2)))//如果相同则不会进入if
				{
					//称重板绑定错误
					error_code = 2;
					break;//有一个错误就直接退出此循环
				}
			}
		}

	}
	else if(error_code == 0)
	{
		//数据域错误
		error_code = 3;
		
	}

	int isfind = 0;//判断是否找到了目标称重板
	struct Board_Info * board_info_p = board_info;
	//以下填充数据的逻辑为，首先先从json商品清单数组中拿出第一个子数组（即为一个商品信息记录），然后查找这个记录是属于哪个称重板的，
	//不会出现轮询所有称重板后没找到匹配的，因为如果有这种情况，在之前的判断中应该已经会记录错误代码了，并且代码不会进入该while之中
	//如果销售模式为2，则可能会向同一个称重板上传多个商品信息，这个时候不会出现多个商品信息不属于一个称重板，因为之前已经做判定了
	//因此只要第一个商品信息所属的称重板，则如果有多个商品，则应该都属于该称重板的
	while (board_info_p != NULL && (!error_code))
	{
		//寻找到该条记录是要更新哪个称重板的
		sub_sub_array_parse = json_array_get_array(sub_array_parse, 0);
		if (!strcmp(board_info_p->id, json_array_get_string(sub_sub_array_parse, 2)))//比对商品信息中的称重板编号
		{
			//代码到达此处代表已经找到称重板
			isfind = 1;
			if ((CharNum_To_Double(json_array_get_string(sub_sub_array_parse, 5)) == 1) && \
				(!strcmp(board_info_p->type, json_array_get_string(sub_sub_array_parse, 5)))  )//如果传输过来的记录销售模式为1，并且该称重板的销售模式也为1
			{
				//销售模式为1
				if (board_info_p->items != NULL)
				{
					error_code = 4;//需要先进行商品下架
					break;
				}
				//更新数据库信息，并且需要重新初始化board中的items链表
				if (SQL_INSERT_INTO_ItemsTable(\
					json_array_get_string(sub_sub_array_parse, 0), json_array_get_string(sub_sub_array_parse, 1), \
					json_array_get_string(sub_sub_array_parse, 2), json_array_get_string(sub_sub_array_parse, 3), \
					json_array_get_string(sub_sub_array_parse, 4), json_array_get_string(sub_sub_array_parse, 5), \
					json_array_get_string(sub_sub_array_parse, 6), json_array_get_string(sub_sub_array_parse, 7), \
					json_array_get_string(sub_sub_array_parse, 8), json_array_get_string(sub_sub_array_parse, 9), \
					json_array_get_string(sub_sub_array_parse, 10)) != 1)
				{
					error_code = 5;//商品数据库插入错误
				}
				else
				{
					//数据库执行成功
					Get_Item_Info();//重新构建各称重板的items链表结构,因为数据库已经改变
				}
				break;
			}
			else if ((CharNum_To_Double(json_array_get_string(sub_sub_array_parse, 5)) == 2) && \
				(!strcmp(board_info_p->type, json_array_get_string(sub_sub_array_parse, 5))) )
			{
				//销售模式为2
				//首先查询要更新的全部商品信息是否同该称重板已经上架的商品相同的，如果有则异常
				for (int i = 0; i < json_array_get_count(sub_array_parse); i++)
				{
					sub_sub_array_parse = json_array_get_array(sub_array_parse, i);
					//每一个子数组的商品编号必须在这个称重板上没有出现过
					if (SQL_SELECT("smart_sales_counter.items", "item_id", json_array_get_string(sub_sub_array_parse, 0)) != 0 )
					{
						//商品已经上架
						error_code = 8;
						break;
					}
				}
				//开始执行insert 语句
				if (!error_code)
				{
					for (int i = 0; i < json_array_get_count(sub_array_parse); i++)
					{
						sub_sub_array_parse = json_array_get_array(sub_array_parse, i);
						if (SQL_INSERT_INTO_ItemsTable(\
							json_array_get_string(sub_sub_array_parse, 0), json_array_get_string(sub_sub_array_parse, 1), \
							json_array_get_string(sub_sub_array_parse, 2), json_array_get_string(sub_sub_array_parse, 3), \
							json_array_get_string(sub_sub_array_parse, 4), json_array_get_string(sub_sub_array_parse, 5), \
							json_array_get_string(sub_sub_array_parse, 6), json_array_get_string(sub_sub_array_parse, 7), \
							json_array_get_string(sub_sub_array_parse, 8), json_array_get_string(sub_sub_array_parse, 9), \
							json_array_get_string(sub_sub_array_parse, 10)) != 1)
						{
							error_code = 9;//商品数据库插入错误
							break;
						}
					}
				}
				if (!error_code)
				{
					//数据库执行成功
					Get_Item_Info();//重新构建各称重板的items链表结构,因为数据库已经改变
				}
				break;

			}
			else
			{
				error_code = 6;//商品销售模式同目标称重板不符
				break;
			}

		}
		board_info_p = board_info_p->next;
		
	}
	if (isfind == 0 && error_code == 0)
	{
		//没有找到目标称重板
		error_code = 7;
	}
	//if (!error_code)//如果至此都没有发生错误
	//{
		//返回成功数据
		time_t timep;
		char * res_num[20];
		JSON_Value *root_value = json_value_init_object();//初始化了一个value，其类型为object，value为json数据的一个类型，不同的value类型可以嵌套
		JSON_Object *root_object = json_value_get_object(root_value);//获取value中的object的地址
		json_object_set_string(root_object, "devid", counter->sn);
		json_object_set_string(root_object, "cmdid", "OnShelf");
		time(&timep);
		Int_To_CharArray(error_code, res_num);
		json_object_dotset_string(root_object, "Result.Res", res_num);
		json_object_dotset_string(root_object, "Result.Date", ctime(&timep));
		//json_object_set_string(root_object, "Counter-SN", counter->sn);
		json_object_dotset_value(root_object, "Result.Data", "NULL");//没有数据
		char * json_string = (char *)malloc(json_serialization_size(root_value));//json_serialization_size这个函数的返回值已经考虑了将原字符串长度进行扩展，因为还要增加一些额外的字符，如换行、反斜杠、大括号等
		memset(json_string, 0, json_serialization_size(root_value));
		json_serialize_to_buffer(root_value, json_string, json_serialization_size(root_value));
		//释放json资源
		json_value_free(root_value);//只需释放根资源，其内部关联的所有资源会被递归释放

		return json_string;//改指针会随着函数跳出而失效，但指向的该内存必须在外部显式释放

	//}
	//else
	//{
	//	//返回成功数据
	//	time_t timep;
	//	char * res_num[20];
	//	JSON_Value *root_value = json_value_init_object();//初始化了一个value，其类型为object，value为json数据的一个类型，不同的value类型可以嵌套
	//	JSON_Object *root_object = json_value_get_object(root_value);//获取value中的object的地址
	//	json_object_set_string(root_object, "devid", counter->sn);
	//	json_object_set_string(root_object, "cmdid", "OnShelf");
	//	time(&timep);
	//	Int_To_CharArray(error_code, res_num);
	//	json_object_dotset_string(root_object, "Result.Res", res_num);
	//	json_object_dotset_string(root_object, "Result.Date", ctime(&timep));
	//	//json_object_set_string(root_object, "Counter-SN", counter->sn);
	//	json_object_dotset_value(root_object, "Result.Data", "NULL");
	//																 //将json字符序列化处理，并深拷贝至一个字符串中，函数返回该字符串，
	//	char * json_string = (char *)malloc(json_serialization_size(root_value));//json_serialization_size这个函数的返回值已经考虑了将原字符串长度进行扩展，因为还要增加一些额外的字符，如换行、反斜杠、大括号等
	//	memset(json_string, 0, json_serialization_size(root_value));
	//	json_serialize_to_buffer(root_value, json_string, json_serialization_size(root_value));
	//	//释放json资源
	//	json_value_free(root_value);//只需释放根资源，其内部关联的所有资源会被递归释放

	//	return json_string;//改指针会随着函数跳出而失效，但指向的该内存必须在外部显式释放
	//}

	//针对错误不同的编码应该使用switch case去逐个判断然后生产对应的错误信息并且发送，但考虑到还有一种销售模式没有完成，因此暂不编写此段代码

}

/*
*	功能：货物下架
*	参数：[in] 接收到的json数据
*		  [out] 返回的结果
*	说明：数据格式为 Devid :  ？？   Cmdid:??    Data:[  [ ]   [ ]   [ ] … [ ] ]，其中data各个子数组的顺序应该同数据库中items顺序相同
*         下架的方式：
*					#货物下架是指针对某一个称重板上的货物进行下架处理，下架是删除该称重板关联的items列表中的商品，
*						并且删除柜子里面数据库中的记录
*					#执行该命令之后一般流程为对该柜子的该称重板执行上架，然后执行一次不检测开关门，对内部货物进行调整后方可正常运行柜子
*						因为下架商品信息后，items链表会重新构建，此时items链表信息会同称重板上实际货物信息不符，但若执行上架是可以的，因为
*						上架流程也不会对商品进行称重测量，最终要执行一次不检测开关门，调整内部货物同上下架后设定的结果一致，也可以对同一个
*						柜子不同称重板执行多次上下架命令后然后再统一进行不检测开关门，对货物进行统一调整
*					#总之不论执行了多少次上下架，上下架也不用交替进行，但最终一定要通过一次不检测开关门来结束柜子的上下架状态，否则柜子不能
*						销售
*					#商品下架上传的商品信息其实不需要完整的信息，只需有商品编号和目标称重板信息就行，但为同上架格式相符，减少代码更改，此处
*						仍旧建议上传一条完整的商品信息
*/
char *  Procedure_Off_Shelf(JSON_Object * json_object)
{
	int error_code = 0;//0代表一切正常
	//先判定接收到的json字符串中的data域中的数据是否合法
	JSON_Array * sub_array_parse = json_object_get_array(json_object, "Data");
	JSON_Array  * sub_sub_array_parse = NULL;
	if (sub_array_parse != NULL)
	{
		if (json_array_get_count(sub_array_parse) > 1)//如果数据域中的子数组的数量大于1，则代表上传了多个商品信息，因此销售模式只能为2，否则数据错误
		{
			for (int i = 0; i < json_array_get_count(sub_array_parse); i++)//每一个传送过来的商品信息中的销售模式都必须为不等于1才行
			{
				sub_sub_array_parse = json_array_get_array(sub_array_parse, i);
				//因为检测到有子数组至少有两个，如果销售模式等于1，那就为错误
				if (CharNum_To_Double(json_array_get_string(sub_sub_array_parse, 5)) == 1)
				{
					//商品销售模式设定错误
					error_code = 1;
					break;
				}
			}
		}
		if (json_array_get_count(sub_array_parse) > 1 && (!error_code))
		{
			//代码至此代表上传的商品信息的销售模式没有错误，以下判定如果上传的商品信息有多条，那所有的商品信息所对应的称重板编号都必须相同
			sub_sub_array_parse = json_array_get_array(sub_array_parse, 0);//获取第一个子数组的称重板编号，后面比对都是跟第一个比
			char * board_id = json_array_get_string(sub_sub_array_parse, 2);
			for (int i = 0; i < json_array_get_count(sub_array_parse); i++)//每一个传送过来的商品信息中的称重板编号必须相同
			{
				sub_sub_array_parse = json_array_get_array(sub_array_parse, i);
				if (strcmp(board_id, json_array_get_string(sub_sub_array_parse, 2)))//如果相同则不会进入if
				{
					//称重板绑定错误
					error_code = 2;
					break;//有一个错误就直接退出此循环
				}
			}
		}

	}
	else if (error_code == 0)
	{
		//数据域错误
		error_code = 3;

	}
	
	int isfind = 0;//判断是否找到了目标称重板
	struct Board_Info * board_info_p = board_info;
	while (board_info_p != NULL && (!error_code))
	{
		//寻找到要下架那个称重板上的货物，不论是模式1还是模式2，不论有几个要下架的商品信息，其首个元素的目标称重板编号和后面的所有元素的
		//目标称重板编号均是相同的，如果不相同在之前的数据合法性检测中将被发现，代码不会运行至此
		sub_sub_array_parse = json_array_get_array(sub_array_parse, 0);
		if (!strcmp(board_info_p->id, json_array_get_string(sub_sub_array_parse, 2)))//比对商品信息中的称重板编号
		{
			//代码到达此处代表已经找到称重板
			isfind = 1;//已经找到对应的称重板
			//如果传输过来的记录销售模式为1，并且该称重板的销售模式也为1则进入以下if
			if ((CharNum_To_Double(json_array_get_string(sub_sub_array_parse, 5)) == 1) && \
				(!strcmp(board_info_p->type, json_array_get_string(sub_sub_array_parse, 5))))
			{
				//销售模式为1
				if (board_info_p->items == NULL)
				{
					error_code = 4;//该称重板上并没有商品无需下架
					break;
				}
				//要下架的商品信息同该称重板上的货物信息是否相同,因为销售模式为1，因此只获取items链表的首个节点
				if (strcmp(board_info_p->items->item_id, json_array_get_string(sub_sub_array_parse, 0)))
				{
					//代码运行至此代表要下架的商品编号同该称重板上商品编号不同
					error_code = 5;
					break;
				}
				//更新数据库信息，并且需要重新初始化board中的items链表
				if (SQL_DELETE("smart_sales_counter.items", "item_id", json_array_get_string(sub_sub_array_parse, 0)) != 1)
				{
					error_code = 6;//商品数据库插入错误
					break;
				}
				else
				{
					//数据库执行成功
					Get_Item_Info();//重新构建各称重板的items链表结构,因为数据库已经改变
				}
				break;
			}
			else if ((CharNum_To_Double(json_array_get_string(sub_sub_array_parse, 5)) == 2) && \
				(!strcmp(board_info_p->type, json_array_get_string(sub_sub_array_parse, 5))))
			{
				//销售模式为2

			}
			else
			{
				error_code = 7;//商品销售模式同目标称重板不符
				break;
			}

		}
		board_info_p = board_info_p->next;
	}
	if (isfind == 0 && error_code == 0)
	{
		//没有找到目标称重板
		error_code = 8;
	}
	if (!error_code)//如果至此都没有发生错误
	{
		//返回成功数据
		time_t timep;
		char * res_num[20];
		JSON_Value *root_value = json_value_init_object();//初始化了一个value，其类型为object，value为json数据的一个类型，不同的value类型可以嵌套
		JSON_Object *root_object = json_value_get_object(root_value);//获取value中的object的地址
		json_object_set_string(root_object, "devid", counter->sn);
		json_object_set_string(root_object, "cmdid", "OffShelf");
		time(&timep);
		Int_To_CharArray(error_code, res_num);
		json_object_dotset_string(root_object, "Result.Res", res_num);
		json_object_dotset_string(root_object, "Result.Date", ctime(&timep));
		//json_object_set_string(root_object, "Counter-SN", counter->sn);
		json_object_dotset_value(root_object, "Result.Data", "NULL");//没有数据
		char * json_string = (char *)malloc(json_serialization_size(root_value));//json_serialization_size这个函数的返回值已经考虑了将原字符串长度进行扩展，因为还要增加一些额外的字符，如换行、反斜杠、大括号等
		memset(json_string, 0, json_serialization_size(root_value));
		json_serialize_to_buffer(root_value, json_string, json_serialization_size(root_value));
		//释放json资源
		json_value_free(root_value);//只需释放根资源，其内部关联的所有资源会被递归释放

		return json_string;//改指针会随着函数跳出而失效，但指向的该内存必须在外部显式释放

	}
	else
	{
		//返回成功数据
		time_t timep;
		char * res_num[20];
		JSON_Value *root_value = json_value_init_object();//初始化了一个value，其类型为object，value为json数据的一个类型，不同的value类型可以嵌套
		JSON_Object *root_object = json_value_get_object(root_value);//获取value中的object的地址
		json_object_set_string(root_object, "devid", counter->sn);
		json_object_set_string(root_object, "cmdid", "OffShelf");
		time(&timep);
		Int_To_CharArray(error_code, res_num);
		json_object_dotset_string(root_object, "Result.Res", res_num);
		json_object_dotset_string(root_object, "Result.Date", ctime(&timep));
		//json_object_set_string(root_object, "Counter-SN", counter->sn);
		json_object_dotset_value(root_object, "Result.Data", "NULL");
																	 //将json字符序列化处理，并深拷贝至一个字符串中，函数返回该字符串，
		char * json_string = (char *)malloc(json_serialization_size(root_value));//json_serialization_size这个函数的返回值已经考虑了将原字符串长度进行扩展，因为还要增加一些额外的字符，如换行、反斜杠、大括号等
		memset(json_string, 0, json_serialization_size(root_value));
		json_serialize_to_buffer(root_value, json_string, json_serialization_size(root_value));
		//释放json资源
		json_value_free(root_value);//只需释放根资源，其内部关联的所有资源会被递归释放

		return json_string;//改指针会随着函数跳出而失效，但指向的该内存必须在外部显式释放
	}



}


/*
*	功能：完成一次取放货
*	参数：[in] 取放货类型，0为正常取放货，1为不检测，直接开关门，1位用以在商品上下架之后执行的货物取放流程
*		  [out]返回函数生成上行清单的指针地址，如果没有生成则返回null
*	说明：从开锁至生成取放货清单，生成清单形式是一个json，其组成见onenote笔记
*		  取放货有两种形式，一种是在商品完成上下架之后执行的取放货，此时只执行开关门，不称重，只回传门关闭的情况，其原因为：
*			#商品完成上架后，数据库已经写入，其中包括上架商品重量，但此时称重板上还没有货物，因此接下来要执行的是开门将货物
*				放上去，但由于放之前称重板没有货物，因此无法执行校准工作，也就无法读出正确的商品重量
*			#商品下架完成，此时数据库已经删除，而且称重板items链表也已经没有该商品记录，因此在开门前如果执行校准，则曲率值会异常
*				因为商品数据库信息已经删除，但是货物没有取走，如果按照数据库中重量在开门前校准，其称重板上实际货物重量同数据库中
*				记录不一致，因此无法读取重量
*			#注：考虑到商品上下架后在实际货物改动完成之前柜子会出现货物和数据库信息不一致的情况，因此此时柜子应该是除接受上下架之另外
*				其它指令一概不接受，并且上位机软件应该控制此流程，保证在上下架完成后，实际货物改动之前不能接受用户的其它服务
*/
char *  Procedure_Pick_And_Place(int Type)
{
	JSON_Value *root_value = json_value_init_object();//初始化了一个value，其类型为object，value为json数据的一个类型，不同的value类型可以嵌套
	JSON_Object *root_object = json_value_get_object(root_value);//获取value中的object的地址

	JSON_Value *sub_value = json_value_init_array();
	JSON_Array *sub_array = json_value_get_array(sub_value);

	time_t timep;
	time(&timep);
	json_object_set_string(root_object, "devid", counter->sn);
	if (Type)//执行一次不测量的开关门
	{
		Locker_Open();//目前是阻塞式的，直到关门
		//发送成功执行一次开关门
		//printf("%s", ctime(&timep));
		json_object_set_string(root_object, "cmdid", "Unlock");
		json_object_dotset_string(root_object, "Result.Res", "0");
		json_object_dotset_string(root_object, "Result.Date", ctime(&timep));
		json_object_dotset_value(root_object, "Result.Data", "NULL");
		//将json字符序列化处理，并深拷贝至一个字符串中，函数返回该字符串，
		char * json_string = (char *)malloc(json_serialization_size(root_value));//json_serialization_size这个函数的返回值已经考虑了将原字符串长度进行扩展，因为还要增加一些额外的字符，如换行、反斜杠、大括号等
		json_serialize_to_buffer(root_value, json_string, json_serialization_size(root_value));
		//释放json资源
		json_value_free(root_value);//只需释放根资源，其内部关联的所有资源会被递归释放
		return json_string;//改指针会随着函数跳出而失效，但指向的该内存必须在外部显式释放

	}
	else//执行一次正常的取放货
	{
		Get_Item_Info();//重新构建各称重板的items链表结构,这个函数也可以放在当货物有更改的时候再执行，暂不调整，待日后完善再调整
		Get_Boards_Items_Weight();//获取各层称重板上货物重量
		struct Board_Info * board_info_p = board_info;
		//开门前校准
		//while (board_info_p != NULL)
		//{
			Board_Curavture_Value_Set( 2 , 1);//校准和保存
			//board_info_p = board_info_p->next;
		//}
		//开门并等待关门
		Locker_Open();//目前是阻塞式的，直到关门
		Board_Get_Weight();//获取每个称重上现有货物的重量
		JSON_Value *sub_sub_value[100];//此处定义的数量实际上是按照这个柜子能够放置的商品种类最大值，但该值被存在数据库，
									   //因此无法直接用于定义固定长度数组，因此此处设置一个较为大的值替代
		JSON_Array *sub_sub_array = NULL;
		int sub_sub_value_pos = 0;//使用的value的位置
		json_object_set_string(root_object, "cmdid", "P&P");

		int Isale = 0;//如果在接下来的判定中，有商品被取放了，此值变成1，退出while循环后执行消息发送函数
		board_info_p = board_info;
		double weight_buf = 0;
		double weight_buf_fix = 0;//计价模式2使用的，最终获取的是准确的重量，是称得重量的修正值
		double price = 0;//计价模式2使用，最终获取的是要付的费用
		while (board_info_p != NULL)//轮询所有称重板的称得的重量
		{
			//weight_buf为本次称得的重量变化值，带有正负号
			weight_buf = board_info_p->board_items_weight_all - board_info_p->board_items_weight_all_after_close_door;
			//若重量变化小于误差值则认为没变化，注意无法是指正负误差，所以使用abs函数处理
			if (abs(weight_buf) >= 0 && abs(weight_buf) <= ERROR_VALUE)
			{
				//weight_buf = 0;//若小于10 则认为没有销售
				//代码到达此处则认为这个称重板没有任何货物变动
				board_info_p = board_info_p->next;
				continue;
			}
			Isale = 1;//代码到达此处代表该称重板重量有变化
			unsigned char change_code_buf[200] = { 0 };
			if (strcmp(board_info_p->type, "1") == 0)//按公斤计算上下架货物
			{
				//对于销售模式为1的货物，其每次校准和测量都会出现微小的偏差，但由于无法判定其销售或者是取走的货物的实际重量，因此占整个误差值
				//会随着销售或者是放入而持续累加。对于取走货物，若是开门前柜子上的货物重量同数据库中的重量相差很小，则取货误差可以忽略，但这个
				//误差值会随着销售次数的增多而持续积累
				//如果测量后发现最终某一个称重板拿走的货物重量同在拿货之前的总重量相差小于等于称重误差值，则认为拿走的就是柜子某一个称重板
				//上所有的货物，此处设定是为了避免出现货物拿不净的情况，即实际拿走了500克，但由于误差，那完货物后称重板称的重量为1克，这样
				//系统计算就相当于拿走了499克
				//这个问题只会出现在销售模式1中，因为2中是根据测量结果寻找拿走的货物组合，最终数据库减少的是组合标识的重量而不是实际重量
				if (abs(weight_buf - board_info_p->board_items_weight_all) <= ERROR_VALUE)
				{
					//如果拿走货物的重量同开门的货物的数据库重量相差值为在误差以内，则认为拿走的就是全部的数据库的标定的数据
					//因此将拿走的货物重量设定为开门前数据库中的重量，将关门后的重量设定为0
					weight_buf = board_info_p->board_items_weight_all;
					//如果进入此if语句，对拿走的货物重量再重新计算后，关门后的称得的重量也要从新计算
					board_info_p->board_items_weight_all_after_close_door = 0;
				}
				//重量取整

					sub_sub_value[sub_sub_value_pos++] = json_value_init_array();//
					sub_sub_array = json_value_get_array(sub_sub_value[sub_sub_value_pos - 1]);
					//此处格式转换的说明，为了便于windows打印中午不出现乱码，因此数据库的连接对象我设置的为GBK码，board_info->itmes->name这个字段
					//是从数据库查询结果中直接赋值过来的，而且包含中文，因此其编码为GBK码，此时要填充至json结构中，json要求必须是utf8编码，因此要进行转换
					GBKToUTF8(board_info_p->items->name, change_code_buf, 200);//商品名称汉子格式转换,此处主要如果商品名字不是汉子还不知道是否会报错--应该不会报错
					json_array_append_string(sub_sub_array, change_code_buf);//商品名称
					json_array_append_string(sub_sub_array, board_info_p->type);//计价类型
					json_array_append_string(sub_sub_array, board_info_p->items->weight_price);//公斤价
					//符号的调整，面向拿取货物同销售环节不同，销售环节在正常情况下不会出现货物重量开门后比开门前变大的情况
					//因此销售环节拿取货物的重量应该不会被负值，根据算法，负值其实是代表有货物被放入货架上，但考虑到我们的
					//阅读习惯，一个值为正值的时候是增加，当为负的时候其实减少，同算法刚好相反，因此此处对符号进行调整，即
					//如果是被拿走了将weight_buf更改为负值，如果为放入了将weight_buf更改为正值
					weight_buf = 0 - weight_buf;
					Int_To_CharArray(weight_buf, change_code_buf);
					json_array_append_string(sub_sub_array, change_code_buf);//取放的重量，此处应该是带正负号的
					//计算取走货物的价格，此处要将正负号去掉，统一为正数
					Double_To_CharArray(CharNum_To_Double(board_info_p->items->weight_price) * (abs(weight_buf) / 500.0)/*将克重换算成斤*/, change_code_buf);
					json_array_append_string(sub_sub_array, change_code_buf);//本条记录总价，总价不是指整个消费单的总价格
					json_array_append_value(sub_array, sub_sub_value[sub_sub_value_pos - 1]);
					//更新此条商品记录中的库存重量
					//因为销售模式为1，因此称重板上只能有一种货物，因此此时关门后的总重量就是称重板上对对应的商品重量
					//由于是取放货，这个重量可能比开门前小（拿走），也可能比开门前大（放入）
					Int_To_CharArray(board_info_p->board_items_weight_all_after_close_door, change_code_buf);
					SQL_UPDATA1("smart_sales_counter.items", "weight_sum", change_code_buf \
						, "item_id", board_info_p->items->item_id);
			}
			else if (strcmp(board_info_p->type, "2") == 0)//按单品数量销售
			{
				//如果计价模式为2
				//注意最后一个参数的引用方式，柜子的销售可以是混合模式，sub_sub_value是data域内各个子数组，因此不论是模式1还是模式2都应该是持续累加sub_sub_value_pos值
				weight_buf = 0 - weight_buf;//改变weight_buf符号，正好为放入，负号为取出
				int sub_sub_value_pos_start = sub_sub_value_pos;
				sub_sub_value_pos += Scheme_Parse(board_info_p->scheme_id, weight_buf, &weight_buf_fix, &price, &sub_sub_value[sub_sub_value_pos]);
				for (int i = sub_sub_value_pos_start; i < sub_sub_value_pos ; i++)
				{
					json_array_append_value(sub_array, sub_sub_value[i]);
				}
				
			}

			board_info_p = board_info_p->next;
		}
		if (Isale == 1)
		{
			time(&timep);
			json_object_dotset_string(root_object, "Result.Res", "0");
			json_object_dotset_string(root_object, "Result.Date", ctime(&timep));
			//json_object_set_string(root_object, "Counter-SN", counter->sn);
			json_object_dotset_value(root_object, "Result.Data", sub_value);
			//将json字符序列化处理，并深拷贝至一个字符串中，函数返回该字符串，
			char * json_string = (char *)malloc(json_serialization_size(root_value));//json_serialization_size这个函数的返回值已经考虑了将原字符串长度进行扩展，因为还要增加一些额外的字符，如换行、反斜杠、大括号等
			json_serialize_to_buffer(root_value, json_string, json_serialization_size(root_value));
			//释放json资源
			json_value_free(root_value);//只需释放根资源，其内部关联的所有资源会被递归释放

			return json_string;//改指针会随着函数跳出而失效，但指向的该内存必须在外部显式释放
		}
		else
		{
			//没有货物被销售，此处还有可能要处理一些异常的应答消息，暂时只写当没有货物被销售的时候的应答
			time(&timep);
			json_object_dotset_string(root_object, "Result.Res", "0");
			json_object_dotset_string(root_object, "Result.Date", ctime(&timep));
			//json_object_set_string(root_object, "Counter-SN", counter->sn);
			json_object_dotset_value(root_object, "Result.Data", "NULL");//代表没有货物被销售
																		 //将json字符序列化处理，并深拷贝至一个字符串中，函数返回该字符串，
			char * json_string = (char *)malloc(json_serialization_size(root_value));//json_serialization_size这个函数的返回值已经考虑了将原字符串长度进行扩展，因为还要增加一些额外的字符，如换行、反斜杠、大括号等
			memset(json_string, 0, json_serialization_size(root_value));
			json_serialize_to_buffer(root_value, json_string, json_serialization_size(root_value));
			//释放json资源
			json_value_free(root_value);//只需释放根资源，其内部关联的所有资源会被递归释放

			return json_string;//改指针会随着函数跳出而失效，但指向的该内存必须在外部显式释放
		}


	}
	return NULL;
}


/*
*	功能：处理接收到的串口数据
*	参数：
*	说明：
*/
void Parse_Usart_Data_Run()
{
	while (1)
	{
		while (REC_BUF_LOCKER != LOCKER_FREE) { Sleep(20); }
		REC_BUF_LOCKER = LOCKER_USED;

		for (int i = 0; i < SERIAL_REC_BUF_NODE_NUM; i++)
		{
			if (srb[i].IsUsed == SERIAL_REC_BUF_NODE_USED)//如果节点被使用
			{
				if (srb[i].len < 6)//不管何种命令其长度都不可能小于6
				{
					char msg[200];
					printf("rec len error!\r\n");
					printf("error msg : %s ", msg);
					HexStringFormatForPrintf(srb[i].data, srb[i].len, msg);
					printf("\r\n");
					for (int j = 0; j < srb[i].len; j++)
					{
						srb[i].data[j] = 0x00;
					}
					srb[i].len = 0;
					srb[i].IsUsed = SERIAL_REC_BUF_NODE_FREE;
					continue;
				}
				if ( (srb[i].data[0] != '$' && srb[i].data[0] != '@') || srb[i].data[srb[i].len - 1] != '#' )
				{
					char msg[200];
					printf("data format error!\r\n");
					printf("error msg : %s ", msg);
					HexStringFormatForPrintf(srb[i].data, srb[i].len, msg);
					printf("\r\n");
					for (int j = 0; j < srb[i].len; j++)
					{
						srb[i].data[j] = 0x00;
					}
					srb[i].len = 0;
					srb[i].IsUsed = SERIAL_REC_BUF_NODE_FREE;
					continue;
				}
				if (Sum_Check(&srb[i].data[1], srb[i].len - 3) != srb[i].data[srb[i].len - 2])
				{
					char msg[200];
					printf("crc error！\r\n");
					printf("error msg : %s ", msg);
					HexStringFormatForPrintf(srb[i].data, srb[i].len, msg);
					printf("\r\n");
					for (int j = 0; j < srb[i].len; j++)
					{
						srb[i].data[j] = 0x00;
					}
					srb[i].len = 0;
					srb[i].IsUsed = SERIAL_REC_BUF_NODE_FREE;
					continue;
				}
				//代码到达此处代表，代表校验类判定类工作都正常，开始处理命令
				if (srb[i].data[0] == '$')//称重板返回命令
				{
					if (srb[i].data[3] == BOARD_CMD_SHAKE)//称重板握手指令
					{
						struct Board_Info * p = board_info;
						while (p != NULL)//轮询所有称重板，并比对握手信息是给那个称重板的，将对应称重板状态改为握手成功
						{
							if ((srb[i].data[1] == ((ASCII_To_byte(p->id[0]) << 4) & 0xf0) + ASCII_To_byte(p->id[1])) &&
								(srb[i].data[2] == ((ASCII_To_byte(p->id[2]) << 4) & 0xf0) + ASCII_To_byte(p->id[3])))
							{
								p->Board_Stat = BOARD_STAT_OK;
								//break;
							}

							p = p->next;
						}
						for (int j = 0; j < srb[i].len; j++)
						{
							srb[i].data[j] = 0x00;
						}
						srb[i].len = 0;
						srb[i].IsUsed = SERIAL_REC_BUF_NODE_FREE;

					}
					else if (srb[i].data[3] == BOARD_CMD_BASIC_VALUE)//称重板去皮指令
					{
						struct Board_Info * p = board_info;
						while (p != NULL)//轮询所有称重板，并比对握手信息是给那个称重板的，将对应称重板状态改为握手成功
						{
							if ((srb[i].data[1] == ((ASCII_To_byte(p->id[0]) << 4) & 0xf0) + ASCII_To_byte(p->id[1])) &&
								(srb[i].data[2] == ((ASCII_To_byte(p->id[2]) << 4) & 0xf0) + ASCII_To_byte(p->id[3])))
							{
								p->IsBasicValue = 1;//去皮完成
								//break;
							}

							p = p->next;
						}
						for (int j = 0; j < srb[i].len; j++)
						{
							srb[i].data[j] = 0x00;
						}
						srb[i].len = 0;
						srb[i].IsUsed = SERIAL_REC_BUF_NODE_FREE;

					}
					else if (srb[i].data[3] == BOARD_CMD_BASIC_VALUE_SAVE)//称重板去皮保存指令
					{
						struct Board_Info * p = board_info;
						while (p != NULL)//轮询所有称重板，并比对握手信息是给那个称重板的，将对应称重板状态改为握手成功
						{
							if ((srb[i].data[1] == ((ASCII_To_byte(p->id[0]) << 4) & 0xf0) + ASCII_To_byte(p->id[1])) &&
								(srb[i].data[2] == ((ASCII_To_byte(p->id[2]) << 4) & 0xf0) + ASCII_To_byte(p->id[3])))
							{
								p->IsBasicValueSave = 1;//去皮完成
													//break;
							}

							p = p->next;
						}
						for (int j = 0; j < srb[i].len; j++)
						{
							srb[i].data[j] = 0x00;
						}
						srb[i].len = 0;
						srb[i].IsUsed = SERIAL_REC_BUF_NODE_FREE;

					}
					else if (srb[i].data[3] == BOARD_CMD_CURVATURE)//称重板校准命令
					{
						struct Board_Info * p = board_info;
						while (p != NULL)//轮询所有称重板，并比对握手信息是给那个称重板的，将对应称重板状态改为握手成功
						{
							if ((srb[i].data[1] == ((ASCII_To_byte(p->id[0]) << 4) & 0xf0) + ASCII_To_byte(p->id[1])) &&
								(srb[i].data[2] == ((ASCII_To_byte(p->id[2]) << 4) & 0xf0) + ASCII_To_byte(p->id[3])))
							{
								p->ISCurvatureValue = 1;//校准成功
								p->Curvature.x[0] = srb[i].data[4];
								p->Curvature.x[1] = srb[i].data[5];
								p->Curvature.x[2] = srb[i].data[6];
								p->Curvature.x[3] = srb[i].data[7];
							}

							p = p->next;
						}
						for (int j = 0; j < srb[i].len; j++)
						{
							srb[i].data[j] = 0x00;
						}
						srb[i].len = 0;
						srb[i].IsUsed = SERIAL_REC_BUF_NODE_FREE;

					}
					else if (srb[i].data[3] == BOARD_CMD_CURVATURE_SAVE)//称重板校准保存命令
					{
						struct Board_Info * p = board_info;
						while (p != NULL)//轮询所有称重板，并比对握手信息是给那个称重板的，将对应称重板状态改为握手成功
						{
							if ((srb[i].data[1] == ((ASCII_To_byte(p->id[0]) << 4) & 0xf0) + ASCII_To_byte(p->id[1])) &&
								(srb[i].data[2] == ((ASCII_To_byte(p->id[2]) << 4) & 0xf0) + ASCII_To_byte(p->id[3])))
							{
								p->ISCurvatureValueSave = 1;//校准成功
								p->Curvature.x[0] = srb[i].data[4];
								p->Curvature.x[1] = srb[i].data[5];
								p->Curvature.x[2] = srb[i].data[6];
								p->Curvature.x[3] = srb[i].data[7];
							}

							p = p->next;
						}
						for (int j = 0; j < srb[i].len; j++)
						{
							srb[i].data[j] = 0x00;
						}
						srb[i].len = 0;
						srb[i].IsUsed = SERIAL_REC_BUF_NODE_FREE;

					}
					else if (srb[i].data[3] == BOARD_CMD_GET_WEIGHT)//测量称重板货物重量返回数据
					{
						struct Board_Info * p = board_info;
						while (p != NULL)//轮询所有称重板，并比对握手信息是给那个称重板的，将对应称重板状态改为握手成功
						{
							if ((srb[i].data[1] == ((ASCII_To_byte(p->id[0]) << 4) & 0xf0) + ASCII_To_byte(p->id[1])) &&
								(srb[i].data[2] == ((ASCII_To_byte(p->id[2]) << 4) & 0xf0) + ASCII_To_byte(p->id[3])))
							{
								p->board_items_weight_all_after_close_door = ((srb[i].data[4] << 8) & 0xff00) + (srb[i].data[5] & 0x00ff);
							}

							p = p->next;
						}
						for (int j = 0; j < srb[i].len; j++)
						{
							srb[i].data[j] = 0x00;
						}
						srb[i].len = 0;
						srb[i].IsUsed = SERIAL_REC_BUF_NODE_FREE;

					}
					else
					{
						//处理其他各类未识别的命令
						char msg[200];
						HexStringFormatForPrintf(srb[i].data, srb[i].len, msg);
						printf("board other msg : %s\r\n", msg);
						for (int j = 0; j < srb[i].len; j++)
						{
							srb[i].data[j] = 0x00;
						}
						srb[i].len = 0;
						srb[i].IsUsed = SERIAL_REC_BUF_NODE_FREE;
					}
				}
				else if (srb[i].data[0] == '@')
				{
					if (srb[i].data[3] == LOCKER_CMD_STATUS)//获取锁状态
					{
						struct counter_info * p = counter;
						if (p != NULL)//轮询所有称重板，并比对握手信息是给那个称重板的，将对应称重板状态改为握手成功
						{
							if ((srb[i].data[1] == ((ASCII_To_byte(p->locker_id[0]) << 4) & 0xf0) + ASCII_To_byte(p->locker_id[1])) &&
								(srb[i].data[2] == ((ASCII_To_byte(p->locker_id[2]) << 4) & 0xf0) + ASCII_To_byte(p->locker_id[3])))
							{
								
								if (srb[i].data[4] == 0xFF) { printf("locker error\r\n"); }
								else if (srb[i].data[4] == 0x02) { printf("locker closed\r\n"); }
								else if (srb[i].data[4] == 0x01) { printf("locker opening\r\n"); }
								else if (srb[i].data[4] == 0x04) { printf("locker waiting for open or close\r\n"); }
								
								counter->locker_stat = srb[i].data[4];
							}
						}
						for (int j = 0; j < srb[i].len; j++)
						{
							srb[i].data[j] = 0x00;
						}
						srb[i].len = 0;
						srb[i].IsUsed = SERIAL_REC_BUF_NODE_FREE;
					}
					else if (srb[i].data[3] == LOCKER_CMD_UNLOCK)//获取执行开门函数后的返回数据
					{
						struct counter_info * p = counter;
						if (p != NULL)//轮询所有称重板，并比对握手信息是给那个称重板的，将对应称重板状态改为握手成功
						{
							if ((srb[i].data[1] == ((ASCII_To_byte(p->locker_id[0]) << 4) & 0xf0) + ASCII_To_byte(p->locker_id[1])) &&
								(srb[i].data[2] == ((ASCII_To_byte(p->locker_id[2]) << 4) & 0xf0) + ASCII_To_byte(p->locker_id[3])))
							{
								counter->locker_stat = srb[i].data[4];
							}
						}
						for (int j = 0; j < srb[i].len; j++)
						{
							srb[i].data[j] = 0x00;
						}
						srb[i].len = 0;
						srb[i].IsUsed = SERIAL_REC_BUF_NODE_FREE;
					}
					else
					{
						//处理其他各类未识别的命令
						char msg[200];
						HexStringFormatForPrintf(srb[i].data, srb[i].len, msg);
						printf("locker other msg : %s\r\n", msg);
						for (int j = 0; j < srb[i].len; j++)
						{
							srb[i].data[j] = 0x00;
						}
						srb[i].len = 0;
						srb[i].IsUsed = SERIAL_REC_BUF_NODE_FREE;
					}
					
				}


			}//以上代码为if接受缓冲区的某一个节点被使用的时候才进入
		}


		REC_BUF_LOCKER = LOCKER_FREE;
		Sleep(500);
	}


}



/*
*	功能：向称重板或者锁发送命令
*	参数：
			[in] id 设备编号
			[in] cmd 命令编号
			[in] cmd_data 命令数据
			[in] cmd_data_len 命令数据长度
			[in] dev_kind 目标设备类型  1为称重板 2为锁
			[in] little 是否对cmd_data进行大小端转换  0为不转换  1为转换  注：此处是指cmd_data的发送顺序是由小至大 还是由大至小
*	说明：目前函数没有返回值，待日后再完善
*/

void Send_CMD(HANDLE  * hCom , unsigned char * id , unsigned char CMD , unsigned char * cmd_data , int cmd_data_len , char dev_kind , int little)
{
	unsigned char send_buf[100];
	int send_buf_len = 0;
	if (dev_kind == 1)//向称重板发送命令
	{
		send_buf[send_buf_len++] = '$';
	}
	else if (dev_kind == 2)//向锁发送吗命令
	{
		send_buf[send_buf_len++] = '@';
	}
	else
	{
		//设备类型编码错误
	}
	send_buf[send_buf_len++] = ((ASCII_To_byte(id[0]) << 4) & 0xf0) + ASCII_To_byte(id[1]);
	send_buf[send_buf_len++] = ((ASCII_To_byte(id[2]) << 4) & 0xf0) + ASCII_To_byte(id[3]);
	send_buf[send_buf_len++] = CMD;
	if(little == 0)
	{
		for (int i = 0; i < cmd_data_len; i++)
		{
			send_buf[send_buf_len++] = cmd_data[i];
		}
	}
	else if (little == 1)
	{
		for (int i = cmd_data_len - 1; i >= 0; i--)
		{
			send_buf[send_buf_len++] = cmd_data[i];
		}
	}
	
	send_buf[send_buf_len] = Sum_Check(&send_buf[1], send_buf_len - 1);
	send_buf_len++;
	send_buf[send_buf_len++] = '#';
	WriteChar(* hCom ,send_buf, send_buf_len);

}

/*
*	功能：和校验
*	参数：
*	说明：
*/

unsigned char Sum_Check(unsigned char * data, int len)
{
	char buf = 0x00;
	for (int i = 0; i < len; i++)
	{
		buf += data[i];
	}
	return buf;
}

/*
*	功能：char 字符（内部编码为ASCII形式的十六进制字符如 AB则为  0x41  0x42） 转换位  byte形式的 如0xAB
*	参数：
*	说明：char指向的必须为数字的ascii，否则为错误,
*/

byte ASCII_To_byte(unsigned char buf)
{
	if (buf >= '0' && buf <= '9')
	{
		return (buf - '0');
	}
	else if (buf >= 'A' && buf <= 'F')
	{
		return (buf - 'A' + 10);
	}
	else if (buf >= 'a' && buf <= 'f')
	{
		return (buf - 'a' + 10);
	}
}

/*
*	功能：char 字符 数字字符串 转 double型
*	参数：
*	说明：
*/

double CharNum_To_Double( const unsigned char * data)
{
	char * p = data;
	double value = 0;
	double value_dec = 0;
	int sw = 0;//整数小数切换
	int i = 0;
	while (*p != '\0')
	{
		if (*p >= '0' && *p <= '9' && sw == 0)
		{
			value = value * 10 + (*p - '0');
		}
		else if (*p == '.')
		{
			if(sw == 0)
			{
				sw = 1;
			}
			else
			{
				//不能出现两个小数点
				return 0;
			}
			
		}
		else if (*p >= '0' && *p <= '9' && sw == 1)
		{
			value_dec = value_dec * 10 + (*p - '0');
			i++;
		}
		else
		{
			//代码到达此处代表字符串里面有非double型数据
			return 0;
		}
		p++;
	}

	value = (value + value_dec / pow(10,i));
	return value;
}

/*
*	功能：double 数字转 char 数组
*	参数：
*	说明：小数点保留两位,第三位四舍五入,正负数均试用，要保证接收数据的数组长度足够用，函数会自动在末尾添加结束标识符'\0'
*/

void Double_To_CharArray(double num, unsigned char * data)
{
	unsigned char buf[20];
	unsigned char temp;
	int num_buf = abs(num * 1000);//不论正负，先统一改成正数
	if ((num_buf % 10) >= 5) { num_buf += 10; }
	num_buf /= 10;
	int num_len = 1;
	if (num_buf == 0)
	{
		//如果输入的数字是0,则直接赋值并返回
		data[0] = '0';
		data[1] = '.';
		data[2] = '0';
		data[3] = '0';
		data[4] = '\0';
		return;
	}
	while (num_buf / (pow(10, num_len)) >= 1)
	{
		num_len++;
	}
	
	for (int i = 0 ; i < num_len; i++)
	{
		buf[i] = num_buf % 10;
		buf[i] += '0';
		num_buf /= 10;
	}
	//前后顺序颠倒
	for (int i = 0, j = num_len - 1; i < j; i++, j--)
	{
		temp = buf[i];
		buf[i] = buf[j];
		buf[j] = temp;
	}
	//到此处，根据小数点保留两位的方式，num_len此时必须为3，下面再增加1位小数点为，最终num_len最小应该为4位
	if (num_len < 3)
	{
		for (int i = 2; i >= 0; i--)
		{
			if (num_len == 0)
			{
				buf[i] = '0';
			}
			else
			{
				buf[i] = buf[--num_len];
			}
			
		}
		num_len = 3;
	}
	/*
	if (num >= 0.1 && num < 1)
	{
		//如果输入的double数字小于1，则此处该指数会变成字符串“??”,两个问好为小数点后保留的两位数字，因此此处要在该字符串开头加一个0字符
		//否则转化出来的数据小数点左侧没有数据
		for (int i = num_len; i > 0; i--)
		{
			buf[i] = buf[i - 1];
		}
		buf[0] = '0';//增加开头的0字符
	}
	*/
	buf[num_len] = buf[num_len - 1];
	buf[num_len-1] = buf[num_len - 2];
	buf[num_len - 2] = '.';//在原数组的倒数第二个位置上添加小数点，整个字符串后移一位，长度加1
	num_len++;//整个数组长度加1

	if (num < 0)
	{
		data[0] = '-';//若原数小于零，则增加负号
		data[num_len + 1] = '\0';
	}
	else
	{
		data[num_len] = '\0';
	}
		
	for (int i = 0; i < num_len; i++)//注意此处是小等于num_len，因为要加一个小数点因此要多一位
	{
		if (num < 0)
		{
			data[i+1] = buf[i];
		}
		else
		{
			data[i] = buf[i];
		}
	}

}

/*
*	功能：整数数字转 char 数组
*	参数：
*	说明：若输入的为浮点型数字，则直接去除小数,正负数均试用，要保证接收数据的数组长度足够用，函数会自动在末尾添加结束标识符'\0'
*/

void Int_To_CharArray(int num, unsigned char * data)
{
	unsigned char buf[20];
	unsigned char temp;
	int num_buf = abs(num);//不论正负，先同意改成正数
	int num_len = 1;
	while (num_buf / (pow(10, num_len)) >= 1)
	{
		num_len++;
	}

	for (int i = 0; i < num_len; i++)
	{
		buf[i] = num_buf % 10;
		buf[i] += '0';
		num_buf /= 10;
	}
	for (int i = 0, j = num_len - 1; i < j; i++, j--)
	{
		temp = buf[i];
		buf[i] = buf[j];
		buf[j] = temp;
	}
	
	if (num < 0)
	{
		data[0] = '-';//若原数小于零，则增加负号
		data[num_len + 1] = '\0';
	}
	else
	{
		data[num_len] = '\0';
	}

	for (int i = 0; i < num_len; i++)
	{
		if (num < 0)
		{
			data[i + 1] = buf[i];
		}
		else
		{
			data[i] = buf[i];
		}
	}

}

/*
*	功能：hex字符串按照两个字节为一个hex数进行格式化并返回byte数组
*	参数：
*	说明：

*/

void HexStringFormatForPrintf(unsigned char * in , int in_len , unsigned char * out)
{
	unsigned char buf[200];
	unsigned char * p = in;
	int k = 0;
	int out_len = 0;

	for (int i = 0; i < in_len; i++)
	{
		if (*p != ' ')
		{
			buf[out_len++] = *p;
		}
		p++;
	}

	buf[out_len] = '\0';

	for (int j = 0; j < out_len; j++)
	{
		/*
		if (buf[j] >= 0x00 && buf[j] <= 0x09)
		{
			out[k++] = buf[j] + '0';
		}
		*/
		/*
		else if (buf[j] >= 0x0A && buf[j] <= 0x0F)
		{
			out[k++] = buf[j] - 10 + 'A';
		}
		*/
		if (buf[j] == '$' || buf[j] == '@' || buf[j] == '#')
		{
			out[k++] = buf[j];
		}
		else
		{
			out[k++] = (buf[j] >> 4 & 0x0f) <= 9 ? (buf[j] >> 4 & 0x0f) + '0' : (buf[j] >> 4 & 0x0f) - 10 + 'A';
			out[k++] = (buf[j] & 0x0f) <= 9 ? (buf[j] & 0x0f) + '0' : (buf[j] & 0x0f) - 10 + 'A';
		}
	}
	out[k] = '\0';
}


/*
*	功能：GBK转utf8
*	参数：[in] GBK 编码字符串
*		  [out] UTF8 编码字符串
*		  [in] UTF8 编码字符串长度，只要保证长度能够承接输出的数据即可
*	说明：
*/
int GBKToUTF8(unsigned char * lpGBKStr, unsigned char * lpUTF8Str, int nUTF8StrLen)
{

	wchar_t * lpUnicodeStr = NULL;

	int nRetLen = 0;

	if (!lpGBKStr) //如果GBK字符串为NULL则出错退出
	{
		return 0;
	}

	nRetLen = MultiByteToWideChar(CP_ACP, 0, (char *)lpGBKStr, -1, NULL, NULL); //获取转换到Unicode编码后所需要的字符空间长度

	lpUnicodeStr = (wchar_t *)malloc(sizeof(wchar_t) * (nRetLen + 1));	//new WCHAR[nRetLen + 1]; //为Unicode字符串空间

	nRetLen = MultiByteToWideChar(CP_ACP, 0, (char *)lpGBKStr, -1, lpUnicodeStr, nRetLen); //转换到Unicode编码

	if (!nRetLen) //转换失败则出错退出
	{
		return 0;
	}
	nRetLen = WideCharToMultiByte(CP_UTF8, 0, lpUnicodeStr, -1, NULL, 0, NULL, NULL); //获取转换到UTF8编码后所需要的字符空间长度

	if (!lpUTF8Str) //输出缓冲区为空则返回转换后需要的空间大小
	{
		if (lpUnicodeStr)
		{
			free(lpUnicodeStr);
		}
		return nRetLen;

	}

	if (nUTF8StrLen < nRetLen) //如果输出缓冲区长度不够则退出

	{
		if (lpUnicodeStr)
		{
			free(lpUnicodeStr);
		}
		return 0;
	}

	nRetLen = WideCharToMultiByte(CP_UTF8, 0, lpUnicodeStr, -1, (char *)lpUTF8Str, nUTF8StrLen, NULL, NULL); //转换到UTF8编码

	if (lpUnicodeStr)//转换完毕，最终释放该资源
	{
		free(lpUnicodeStr);
	}
	return nRetLen;

}

/*
*	功能：utf8转GBK
*	参数：[in] UTF8 编码字符串
*		  [out] GBK 编码字符串
*		  [in] GBK 编码字符串长度，只要保证长度能够承接输出的数据即可
*	说明：
*/
int UTF8ToGBK(unsigned char * lpUTF8Str, unsigned char * lpGBKStr, int nGBKStrLen)

{

	wchar_t * lpUnicodeStr = NULL;
	int nRetLen = 0;

	if (!lpUTF8Str) //如果UTF8字符串为NULL则出错退出
	{
		return 0;
	}

	nRetLen = MultiByteToWideChar(CP_UTF8, 0, (char *)lpUTF8Str, -1, NULL, NULL); //获取转换到Unicode编码后所需要的字符空间长度

	lpUnicodeStr = (wchar_t *)malloc(sizeof(wchar_t) * (nRetLen + 1));//new WCHAR[nRetLen + 1]; //为Unicode字符串空间

	nRetLen = MultiByteToWideChar(CP_UTF8, 0, (char *)lpUTF8Str, -1, lpUnicodeStr, nRetLen); //转换到Unicode编码
	if (!nRetLen) //转换失败则出错退出
	{
		return 0;
	}

	nRetLen = WideCharToMultiByte(CP_ACP, 0, lpUnicodeStr, -1, NULL, NULL, NULL, NULL); //获取转换到GBK编码后所需要的字符空间长度

	if (!lpGBKStr) //输出缓冲区为空则返回转换后需要的空间大小
	{
		if (lpUnicodeStr)
		{
			free(lpUnicodeStr);
		}
		return nRetLen;
	}

	if (nGBKStrLen < nRetLen) //如果输出缓冲区长度不够则退出
	{
		if (lpUnicodeStr)
		{
			free(lpUnicodeStr);
		}
		return 0;
	}

	nRetLen = WideCharToMultiByte(CP_ACP, 0, lpUnicodeStr, -1, (char *)lpGBKStr, nRetLen, NULL, NULL); //转换到GBK编码

	if (lpUnicodeStr)
	{
		free(lpUnicodeStr);
	}
		
	return nRetLen;
}


/*临时测试函数-----------------------------------------------------------*/
int is_valid_utf8(const char *string, size_t string_len)
{
	int len = 0;
	const char *string_end = string + string_len;
	while (string < string_end) {
		if (!verify_utf8_sequence((const unsigned char*)string, &len)) {
			return 0;
		}
		string += len;
	}
	return 1;
}
#define IS_CONT(b) (((unsigned char)(b) & 0xC0) == 0x80) /* is utf-8 continuation byte */
static int verify_utf8_sequence(const unsigned char *string, int *len) {
	unsigned int cp = 0;
	*len = num_bytes_in_utf8_sequence(string[0]);

	if (*len == 1) {
		cp = string[0];
	}
	else if (*len == 2 && IS_CONT(string[1])) {
		cp = string[0] & 0x1F;
		cp = (cp << 6) | (string[1] & 0x3F);
	}
	else if (*len == 3 && IS_CONT(string[1]) && IS_CONT(string[2])) {
		cp = ((unsigned char)string[0]) & 0xF;
		cp = (cp << 6) | (string[1] & 0x3F);
		cp = (cp << 6) | (string[2] & 0x3F);
	}
	else if (*len == 4 && IS_CONT(string[1]) && IS_CONT(string[2]) && IS_CONT(string[3])) {
		cp = string[0] & 0x7;
		cp = (cp << 6) | (string[1] & 0x3F);
		cp = (cp << 6) | (string[2] & 0x3F);
		cp = (cp << 6) | (string[3] & 0x3F);
	}
	else {
		return 0;
	}

	/* overlong encodings */
	if ((cp < 0x80 && *len > 1) ||
		(cp < 0x800 && *len > 2) ||
		(cp < 0x10000 && *len > 3)) {
		return 0;
	}

	/* invalid unicode */
	if (cp > 0x10FFFF) {
		return 0;
	}

	/* surrogate halves */
	if (cp >= 0xD800 && cp <= 0xDFFF) {
		return 0;
	}

	return 1;
}

static int num_bytes_in_utf8_sequence(unsigned char c) {
	if (c == 0xC0 || c == 0xC1 || c > 0xF4 || IS_CONT(c)) {
		return 0;
	}
	else if ((c & 0x80) == 0) {    /* 0xxxxxxx */
		return 1;
	}
	else if ((c & 0xE0) == 0xC0) { /* 110xxxxx */
		return 2;
	}
	else if ((c & 0xF0) == 0xE0) { /* 1110xxxx */
		return 3;
	}
	else if ((c & 0xF8) == 0xF0) { /* 11110xxx */
		return 4;
	}
	return 0; /* won't happen */
}

/*临时测试函数完-------------------------------------------------*/


/*
*	以下函数是系统经过简化后的所用的函数---------------------------------------------------------------------------------------------------
*/


/*
*	功能：形成向服务器发送的应答的消息
*	参数：	[in] 接收到的消息串号（可以用时间戳等实现）
			[in] 命令名称，即编号
			[in] 命令执行的结果
			[in] 要返回的数据子数组，该值应该是一个已经填充好的数组，每一个元素都为数组，即[ [] , [] , [] , [] , [] , [] ]
			[out]将消息返回给调用函数，其中json数据要由调用函数显示释放
*	说明：
*/
char *  Procedure_Answer_Message(char * message_sn , char * cmd_name , int Res , JSON_Value *sub_value)
{
	JSON_Value *root_value = json_value_init_object();//初始化了一个value，其类型为object，value为json数据的一个类型，不同的value类型可以嵌套
	JSON_Object *root_object = json_value_get_object(root_value);//获取value中的object的地址
	char * change_code[100] = { 0 };
	//JSON_Value *sub_value = json_value_init_array();
	//JSON_Array *sub_array = json_value_get_array(sub_value);

	time_t timep;
	time(&timep);
	json_object_set_string(root_object, "MSN", message_sn);
	json_object_set_string(root_object, "devid", counter->sn);
	json_object_set_string(root_object, "cmdid", cmd_name);
	Int_To_CharArray(Res, change_code);
	json_object_dotset_string(root_object, "Result.Res", change_code);
	json_object_dotset_string(root_object, "Result.Date", ctime(&timep));
	json_object_dotset_value(root_object, "Result.Data", sub_value);
	//以下内存会最终通过smb.message指针指向，并且在发送成功后被释放
	char * json_string = (char *)malloc(json_serialization_size(root_value));//json_serialization_size这个函数的返回值已经考虑了将原字符串长度进行扩展，因为还要增加一些额外的字符，如换行、反斜杠、大括号等
	json_serialize_to_buffer(root_value, json_string, json_serialization_size(root_value));
	//释放json资源
	json_value_free(root_value);//只需释放根资源，其内部关联的所有资源会被递归释放
	return json_string;//改指针会随着函数跳出而失效，但指向的该内存必须在外部显式释放

}

/*
*	功能：完成一次开关门
*	参数：[out]执行结果
*	说明：执行完开锁命令后等待10秒钟，然后返回开锁函数的返回值
*/
char *  Procedure_Open_Lock(JSON_Object * json_object)
{

	int res = Locker_Open();
	return Procedure_Answer_Message(json_object_get_string(json_object, "MSN"), "Unlock", res, NULL);

}

/*
*	功能：执行一次开关门
*	参数：[out]直至门关或者一分钟延时到达
*	说明：先执行开锁函数，然后等待10秒钟，再执行获取锁状态的命令，每两秒判断一次返回值是否为门关闭，如果不是则重复获取锁状态，但总时间不超过1分钟，最终将锁状态返回
*/
char *  Procedure_Open_Close(JSON_Object * json_object)
{

	int res = Locker_Open_Closed();
	return Procedure_Answer_Message(json_object_get_string(json_object, "MSN"), "Unlock_Close", res, NULL);

}

/*
*	功能：获取锁状态
*	参数：[out]获取锁的状态
*	说明：返回锁的状态
*/
char *  Procedure_Get_Locker_State(JSON_Object * json_object)
{

	int res = Locker_Get_Stat();
	return Procedure_Answer_Message(json_object_get_string(json_object, "MSN"), "Locker_State", res, NULL);

}

/*
*	功能：根据称重板编号，执行一次去皮
*	参数：
*	说明：对指定称重板执行两次去皮
*/
char * Procedure_Basic_Value_Set(JSON_Object * json_object)
{
	//获取称重板编号
	char * board_id = json_object_get_string(json_object, "BN");
	int res = -1;

	if (board_id != NULL)
	{
		res = Board_Basic_Value_Set_By_id(board_id, 2);
	}

	return Procedure_Answer_Message(json_object_get_string(json_object, "MSN") , "Basic_Value", res, NULL);

}

/*
*	功能：去皮校准并且保存
*	参数：[in] 称重板编号
*		  [in] 去皮次数
*		  [out] 执行结果
*	说明：函数在调用时将board_info结构体中的IsBasicVale设定为0，即为未去皮，然后执行去皮，并等待一段时间后判定是否去皮成功
*/
int Board_Basic_Value_Set_By_id(char * board_id , int times)
{
	struct Board_Info * p = board_info;
	int boards_num = 0;
	printf("等待去皮完成");
	while (p != NULL)
	{

		if (strcmp(p->id, board_id) == 0)
		{
			p->IsBasicValue = 0;//不论是否已经去过皮都设定为未去皮状态
			p->IsBasicValueSave = 0;//去皮未保存
			for (int i = 0; i < times; i++)
			{
				printf("...");
				Send_CMD(&hCom_C, p->id, BOARD_CMD_BASIC_VALUE, NULL, 0, 1, 0);
				Sleep(5000);
			}
			printf("...");
			Send_CMD(&hCom_C, p->id, BOARD_CMD_BASIC_VALUE_SAVE, NULL, 0, 1, 0);
			Sleep(1000);
			if (p->IsBasicValue == 0 || p->IsBasicValueSave == 0)
			{
				//去皮失败
				printf("\r\n去皮失败！！\r\n");
				return GEN_ERROR_COMMUNICATION_ERROR;
			}
			else
			{
				//去皮成功
				printf("\r\n称重板 ： %s  去皮成功！！\r\n" , p->id);
				return GEN_OK;
			}
		}

		p = p->next;
	}
	if (p == NULL)
	{
		printf("  称重板编号错误！\r\n");
		return SETTING_BASIC_VALUE_BOARD_ERROR;
	}
	return GEN_MEM_OVER;//代码应该永远不会达到此处,若到达此处则代表内存有益处的风险

}


/*
*	功能：设定指定称重板曲率值
*	参数：[out]执行结果
*	说明：返回锁的状态
*/
char *  Procedure_Set_Curavture_Value(JSON_Object * json_object)
{
	//获取称重板编号
	char * board_id = json_object_get_string(json_object, "BN");
	char * str_weight = json_object_get_string(json_object, "Weight");//校准的数值
	//char * ischecktem = json_object_get_string(json_object, "Check_Tem");
	char * save = json_object_get_string(json_object, "Save");

	//(int)CharNum_To_Double(ischecktem) /*0为不考虑 1为考虑*/,
	int res = Board_Curavture_Value_Set_Ex(board_id, (UINT16)CharNum_To_Double(str_weight), 2,  (int)CharNum_To_Double(save));
	JSON_Value *sub_value = json_value_init_array();
	JSON_Array *sub_array = json_value_get_array(sub_value);
	if (res == 0)
	{
		unsigned char * change_code[20];
		struct Board_Info * p = board_info;
		while (p != NULL)
		{

			if (strcmp(p->id, board_id) == 0)
			{
				Double_To_CharArray(p->Curvature.f, change_code);
				json_array_append_string(sub_array, change_code);
				break;
			}
			p = p->next;
		}
		return Procedure_Answer_Message(json_object_get_string(json_object, "MSN"), "Curavture_Value", res, sub_value);
	}
	return Procedure_Answer_Message(json_object_get_string(json_object, "MSN"), "Curavture_Value", res, NULL);

}

/*
*	功能：曲率校准
*	参数：
		  [in] 称重板编号
		  [in] 校准的重量值
		  [in] 校准次数
		  [in] 是否保存曲率值 0 不保存  1保存
*	说明：目前未对返回值进行任何处理
*		  
*		  fixed：将温度判定从此函数中移除，因为此函数为校准一个设备，如果增加温度判定，则会出现校准某一个承重板后系统温度校准条件被清除，根据判定条件后续称重板都无需校准的bug
*
*/
int Board_Curavture_Value_Set_Ex(char * board_id , UINT16 weight , int times , int save)
{

	struct Board_Info * p = board_info;

	//if (IsCheck_Tem)
	//{
	//	if (!sys_tem.IsCheck)
	//	{
	//		//温度没有变化无需校准
	//		printf("最后一次测量温度为 %d 摄氏度 ， 系统初始温度为 %d 摄氏度，无需校准\r\n", sys_tem.Tem_Cur, sys_tem.Tem);
	//		return 1;
	//	}
	//	else
	//	{
	//		//温度变化及时间满足校准要求，先将温度结构体数据进行调整
	//		sys_tem.Tem = sys_tem.Tem_Cur;
	//		sys_tem.Time = 0;
	//		sys_tem.IsCheck = 0;
	//	}
	//}

	while (p != NULL)
	{

		if (strcmp(p->id, board_id) == 0)
		{

			if (weight < 1000)//如果校准的货物重量小于1000克，则不执行校准，是否合适还需要测试
			{
				//printf("校准货物过轻-- %d  ，采用原有曲率测量\r\n", p->board_items_weight_all);
				printf("称重板 %s 校准重量过轻，小于1000克 , 无法校准\r\n", p->id );
				return SETTING_CURAVTURE_VALUE_TOO_LIGHT;
			}

			p->ISCurvatureValue = 0;//设置成未校准状态
			p->ISCurvatureValueSave = 0;//设置成未保存状态
			for (int i = 0; i < times; i++)
			{
				//Send_CMD(&hCom_C, p->id, BOARD_CMD_CURVATURE, (unsigned char *)&p->board_items_weight_all, 2, 1, 1);
				Send_CMD(&hCom_C, p->id, BOARD_CMD_CURVATURE, (unsigned char *)&weight, 2, 1, 1);
				printf("...");
				Sleep(5000);
			}

			if (p->ISCurvatureValue == 0)
			{
				//校准失败
				printf("\r\n校准失败！！\r\n");
				return GEN_ERROR_COMMUNICATION_ERROR;
			}

			if (save == 1)
			{
				Send_CMD(&hCom_C, p->id, BOARD_CMD_CURVATURE_SAVE, (unsigned char *)&weight, 2, 1, 1);
				printf("...");
				Sleep(1000);
				if (p->ISCurvatureValueSave == 0)
				{
					//校准失败
					printf("\r\n校准保存失败！！\r\n");
					return GEN_ERROR_COMMUNICATION_ERROR;
					//getchar(1);
					//sys_die("\r\n校准保存失败！！\r\n");
				}
				else
				{
					printf("称重板 %s 保存校准完成！\r\n" , p->id);
					p->ISNeedCur = ISNEEDCUR_NO;
					return GEN_OK;
				}
			}
			else
			{
				printf("称重板 %s 不保存校准完成！\r\n", p->id);
				p->ISNeedCur = ISNEEDCUR_NO;
				return GEN_OK;
			}

		}

		p = p->next;

	}
	if (p == NULL)
	{
		printf("  称重板编号错误！\r\n");
		return SETTING_CURAVTURE_VALUE_BOARD_ERROR;
	}
	return GEN_MEM_OVER;//代码应该永远不会达到此处,若到达此处则代表内存有益处的风险

}

/*
*	功能：获取称重板状态，包括编号和是否需要校准
*	参数：
*	说明：该函数可以用在获取柜子有多少个称重板并且编号是多少，也可用在获取称重板是否需要校准的环节
*/
char * Procedure_Get_Board_State(void)
{
	//发送数据用的json变量
	JSON_Value *sub_value = json_value_init_array();
	JSON_Array *sub_array = json_value_get_array(sub_value);
	struct Board_Info * p = board_info;
	unsigned char * change_code[20];
	while (p != NULL)
	{
		json_array_append_string(sub_array, p->id);//添加称重板编号
		Int_To_CharArray(p->ISNeedCur, change_code);
		json_array_append_string(sub_array, change_code);//添加称重板是否需要校准状态
		p = p->next;
	}
	return Procedure_Answer_Message(json_object_get_string(json_object, "MSN") , "Board_State", 0, sub_value);//这个函数不可能失败，因此res固定为0

}



/*
*	功能：设定指定称重板称重
*	参数：[out]执行结果
*	说明：res返回值是沿用称重函数返回值继续累加的
*		  该函数只是称重，按照发送过来的称重板编号进行称重，不进行校准
*		  
*/
char *  Procedure_Get_Weight_Value(JSON_Object * json_object)
{
	//每一个称重板都有两个数据，一个是编号，一个是这个称重板上的货物重量
	JSON_Array * sub_array_parse = json_object_get_array(json_object, "Data");
	//JSON_Array  * sub_sub_array_parse = NULL;

	//发送数据用的json变量
	JSON_Value *sub_value = json_value_init_array();
	JSON_Array *sub_array = json_value_get_array(sub_value);
	struct Board_Info * p = board_info;

	int res = 0;//由于可能测量多个称重板，因此返回值的逻辑是只要当前的称重有错误则不再进行称重，并且返回，已经测量成功的数据也会一并返回
	unsigned char * change_code[20];
	if (sub_array_parse != NULL)
	{
		for (int i = 0; i < json_array_get_count(sub_array_parse); i++)
		{

			//data域中每一个元素都为数组，子数组中有两个元素，分别为称重板编号，和该称重板上货物重量
			res = Board_Get_Weight_Ex(json_array_get_string(sub_array_parse, i));
			if (res != 0)//如果测量没有成功
			{
				return Procedure_Answer_Message(json_object_get_string(json_object, "MSN"), "Weight_Value", res, sub_value);
			}
			json_array_append_string(sub_array, json_array_get_string(sub_array_parse, i));//添加称重板编号
			while (p != NULL)
			{

				if (strcmp(p->id, json_array_get_string(sub_array_parse, i)) == 0)
				{
					Int_To_CharArray(p->board_items_weight_all_after_close_door, change_code);
					json_array_append_string(sub_array, change_code);
					p = board_info;
					break;//跳出while循环，并且将p指针重新指向board_info链表头
				}
				p = p->next;
			}


		}
		return Procedure_Answer_Message(json_object_get_string(json_object, "MSN"), "Weight_Value", res, sub_value);
	}
	else//如果上传的data域为空，则代表没有上传任何称重板编号和校准值，因此返回错误
	{
		res = WEIGHT_VALUE_DATA_MISS;
		return Procedure_Answer_Message(json_object_get_string(json_object, "MSN"), "Weight_Value", res, sub_value);
	}

}

/*
*	功能：读取称重板当前克重
*	参数：
*	说明：函数不进行校准
*/
int Board_Get_Weight_Ex(char * board_id)
{
	struct Board_Info * p = board_info;
	int res = 0;
	while (p != NULL)
	{
		if (strcmp(p->id, board_id) == 0)
		{
			p->board_items_weight_all_after_close_door = 65534;//该值每次使用前降该值设定为一个不可能测得的数值
			Send_CMD(&hCom_C, p->id, BOARD_CMD_GET_WEIGHT, NULL, 0, 1, 0);
			Sleep(5000);
			if (p->board_items_weight_all_after_close_door != 65534)
			{
				printf("读取重量完成！\r\n");
				return GEN_OK;
			}
			else
			{
				printf("重量通信异常！\r\n");
				return GEN_ERROR_COMMUNICATION_ERROR;
			}
		}
		p = p->next;
	}
	if (p == NULL)
	{
		printf("  称重板编号错误！\r\n");
		return WEIGHT_VALUE_BOARD_ERROR;
	}
	return GEN_MEM_OVER;//代码应该永远不会达到此处,若到达此处则代表内存有益处的风险

}


/*
*	功能：执行一次销售流程
*	参数：[out]执行结果
*	说明：服务器发送柜子的全部称重板编号和称重板上的货物重量
*		  *对于超时关门的处理方案，若等待若干时间后门仍旧没有关上，程序返回门没有关闭的信息，此时用户手机上应该显式出现关门按钮，用户关门后应该主动点击关门按钮
*		  *服务器收到用户发送的信息后去确认柜子的门的状态，如果确实为关闭则执行称重流程，如果仍旧有问题，则向用户发送异常信息
*/
char *  Procedure_Sales_Ex(JSON_Object * json_object)
{
	//每一个称重板都有两个数据，一个是编号，一个是这个称重板上的货物重量
	JSON_Array * sub_array_parse = json_object_get_array(json_object, "Data");

	//发送数据用的json变量
	JSON_Value *sub_value = json_value_init_array();
	JSON_Array *sub_array = json_value_get_array(sub_value);
	struct Board_Info * p = board_info;
	unsigned char * change_code[20];
	int res = 0;
	if (sub_array_parse != NULL)
	{
		
		Sleep(200);
		//开门，最多等待2分钟
		for (int i = 0; i < 2; i++)
		{
			//开门，若长时间不关闭，此函数会阻塞1分钟
			res = Locker_Open_Closed();
			if (res == LOCKER_GET_STATE_OK)//门正常关闭了
			{
				break;
			}
		}
		if (res != LOCKER_GET_STATE_OK)
		{
			//门长时间未关闭
			return Procedure_Answer_Message(json_object_get_string(json_object, "MSN"), "Shopping", res, NULL);//直接返回执行开关门后的返回值
		}
		//开始称重
		Sleep(200);
		p = board_info;
		for (int i = 0; i < json_array_get_count(sub_array_parse); i++)
		{
			//解析第i个子数组信息
			//sub_sub_array_parse = json_array_get_array(sub_array_parse, i);
			
			res = Board_Get_Weight_Ex(json_array_get_string(sub_array_parse, i));
			if (res != 0)//如果测量没有成功
			{
				return Procedure_Answer_Message(json_object_get_string(json_object, "MSN"), "Shopping", res, sub_value);
			}
			json_array_append_string(sub_array, json_array_get_string(sub_array_parse, i));//添加称重板编号
			while (p != NULL)
			{

				if (strcmp(p->id, json_array_get_string(sub_array_parse, i)) == 0)
				{
					Int_To_CharArray(p->board_items_weight_all_after_close_door, change_code);
					json_array_append_string(sub_array, change_code);
					p = board_info;
					break;//跳出while循环，并且将p指针重新指向board_info链表头
				}
				p = p->next;
			}
			if (p == NULL)//即代表某一个称重板编号错误，即未找到匹配的编号
			{
				return Procedure_Answer_Message(json_object_get_string(json_object, "MSN"), "Shopping", WEIGHT_VALUE_BOARD_ERROR, NULL);//代码应该永远不会达到此处
			}

		}
		return Procedure_Answer_Message(json_object_get_string(json_object, "MSN"), "Shopping", res, sub_value);

	}
	else
	{
		//没有提供称重板信息，无法完成销售流程
		return Procedure_Answer_Message(json_object_get_string(json_object, "MSN"), "Shopping", SHOPPING_WEIGHT_VALUE_DATA_MISS, NULL);
	}

}


/*
*	功能：读取称重板货物克重
*	参数：[out]执行结果
*	说明：含cur校准
*
*/
//int Board_Get_Weight_Ex_With_Cur(char * board_id, UINT16 weight/*如果需要校准则此重量为校准值*/, int times)
//{
//	struct Board_Info * p = board_info;
//	int res = 0;
//	res = Board_Curavture_Value_Set_Ex(board_id, weight, times, 1/*0为不考虑 1为考虑*/, 1/*保留校准结果*/);//在称重中调用曲率校准函数，校准次数为2，并且保留校准结果
//																							  //若res不等于1或者0，则为出错，1为温度没有变化无需校准，0为校准正常
//	if (res != 1 && res != 0)
//	{
//		return res;
//	}
//	while (p != NULL)
//	{
//		if (strcmp(p->id, board_id) == 0)
//		{
//			p->board_items_weight_all_after_close_door = 65534;//该值每次使用前降该值设定为一个不可能测得的数值
//			Send_CMD(&hCom_C, p->id, BOARD_CMD_GET_WEIGHT, NULL, 0, 1, 0);
//			Sleep(5000);
//			if (p->board_items_weight_all_after_close_door != 65534)
//			{
//				printf("读取重量完成！\r\n");
//				return 0;
//			}
//			else
//			{
//				printf("重量通信异常！\r\n");
//				return 6;
//			}
//		}
//		p = p->next;
//	}
//	if (p == NULL)
//	{
//		printf("  称重板编号错误！\r\n");
//		return 7;
//	}
//	return 8;//代码应该永远不会达到此处,若到达此处则代表内存有益处的风险
//
//}