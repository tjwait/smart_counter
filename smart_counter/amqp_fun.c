
#include "amqp_fun.h"
#pragma comment(lib, "rabbitmq.4.lib")

amqp_socket_t *amqp_socket = NULL;
amqp_connection_state_t conn;

amqp_bytes_t queuename;//queue 的名字
                       //exchange 的名字同柜子的SN编号
                       //要连接的服务器信息在counter结构体中

int init_amqp()
{
	//需要初始化的内容添加至此
	queuename.bytes = counter->queue_name;//初始化queue的名字，该名字来自于数据库设定，目前其值同柜子的SN号相同
									//用柜体信息的SN编号命名queue,此处可能会有所不妥，应该是由服务器建立好queue，并将名字存储在数据库中
								  //此处直接进行绑定，而无需在声明，注意此处声明的queue是用于服务器向柜子发送数据用的，该queue可以同
								  //柜子上行用的queue复用
	queuename.len = strlen(counter->queue_name);
	return AMQP_FUN_SUCCESS;
}

//监听数据函数
int run_listen(void * dummy)
{
	dummy = NULL;
	//amqp_socket_t *socket = NULL;
	//amqp_connection_state_t conn;
	int status;
	conn = amqp_new_connection();//此处是建立一个amqp连接的结构体并初始化该结构，该结构体需要使用amqp_destroy_connection()进行资源释放
								 //函数返回一个amqp_connection_state_t 指针则代表成功，若返回null或者是0则代表失败

	amqp_socket = amqp_tcp_socket_new(conn);//此函数是建立tcp收发的关键，函数接收amqp_new_connection()函数返回的结构体指针
											//此函数在内部建立了一个amqp_tcp_socket_t结构体，该结构保存了连接状态，sock句柄编号以及收发等功能型函数，
											//其中有关收发等重要功能函数是通过内部一个klass指针，指向amqp_socket_class_t结构体变量amqp_tcp_socket_class的地址来实现的
											//amqp_tcp_socket_class该结构体变量内元素均为函数指针，共有6个函数，分别为接收、发送、打开、关闭、获取句柄和删除
											//最后amqp_tcp_socket_new函数通过amqp_set_socket函数将amqp_tcp_socket_t结构体变量赋给amqp_new_connection()返回的结构体
											//中的socket参数，并且将amqp_tcp_socket_t结构体变量返回
											//即这个函数的返回值其实已经被conn变量内的conn->socket这个指针所指向了
	if (!amqp_socket)//
	{
		die("creating TCP socket");
	}

	status = amqp_socket_open(amqp_socket, counter->server_ip , atoi(counter->server_port));//打开tcp连接函数，其内部实际上调用了klass指针指向的结构体重的open函数来实现的
	if (status) 
	{
		die("opening TCP socket");
	}

	//登录，amqp_login参数为  
	//1、连接对象，即amqp_new_connection函数的返回值，此处为conn
	//2、连接的虚拟机，一般都为"/"，broker虚拟机的概念我还不是特别清楚，还需要进一步了解，在broker中可以设定不同的登录用户所对应的虚拟机名称，guest刚好默认是"/"
	//3、最大的channel数量，该值在连接对象初始化的时候被设定为0，即为无限制，最大是65535，channel-0为保留，服务器端也可设定此值，在实际使用中使用较小的一个数值作为限定
	//4、帧最大长度，4096位最小，2的31次方-1为最大，131072为128K，这个数值作为默认值最好
	//5、心跳功能间隔，设定一个秒数来作为heartbeat的间隔时间，如果设定为0则为不使用
	//6、登录服务器的认知机制，SASL为c/s的一种认证机制，并非amqp所特有的，在此处提供两种认证机制
	//		*AMQP_SASL_METHOD_PLAIN 利用此种方式，在该参数后面要提供用户名和密码两个参数，例如如下面使用的方式
	//		*AMQP_SASL_METHOD_EXTERNAL 利用此种方式，在改参数后要提供一个认证字符串，这个如何使用还需要再了解,不知道是否为不需要密码的用户米（新建用的时候，是否需要密码是可选项）
	die_on_amqp_error(amqp_login(conn, "/", 0, 131072, 360 , AMQP_SASL_METHOD_PLAIN, counter->mq_name, counter->mq_pw), "Logging in");

	amqp_channel_open(conn, atoi(counter->channel));
	/*
	*	amqp_get_rpc_reply函数能够对应于大多数AMQP同步的方法，其返回一个指向调用方法结果的指针，当有异常的时候，该函数返回null，此时需要通过一些方法了解到是什么原因造成的异常
	*	该函数返回一个API操作的最新返回值的实例
	*	该函数只使用在被调用函数自身无执行结果返回的时候（不知道此处翻译是否正确）
	*	这个函数实际上是返回连接对象conn中的most_recent_api_result的值，此值为一个结构体amqp_rpc_reply_t，该结构体内有一个主体值和两个辅助数值
	*		*reply_type为主体值，若为1即AMQP_RESPONSE_NORMAL，则代表刚刚执行的api正常，若为AMQP_RESPONSE_SERVER_EXCEPTION或者AMQP_RESPONSE_LIBRARY_EXCEPTION则代表刚刚执行的api不正常
	*		*若不正常则需要分析辅助值reply（一个结构体）和library_error（一个int型）来分析原因，此处可见die_on_amqp_error函数就是做此分析之用的
	*/
	die_on_amqp_error(amqp_get_rpc_reply(conn), "Opening channel");

    //声明queue，以下参数详见onenote笔记
	/*  @param [in] state connection state
	* @param [in] channel the channel to do the RPC on //发送给哪个channel
	* @param [in] queue queue //这个参数若为空则为让系统自动命名
	* @param [in] passive passive //不知道是什么作用
	* @param [in] durable durable //持久化处理
	* @param [in] exclusive exclusive //排他，即只能有一个连接
	* @param [in] auto_delete auto_delete //在所有订阅该queue的连接断开后queue删除
	* @param [in] arguments arguments
	* 此函函数执行内容较为清晰，将参数填入函数内部的amqp_queue_declare_t结构体变量，并且调用amqp_simple_rpc_decoded（）函数发送声明queue方法并等待回复
	* 目前需要了解的是最后一个参数，即声明queue可选的一些属性如何添加
	*/
	amqp_queue_declare_ok_t *r = amqp_queue_declare(conn, atoi(counter->channel), queuename , 0, 0, 0, 1, amqp_empty_table);//此函数的一些参数的行为还需要再深入测试
	die_on_amqp_error(amqp_get_rpc_reply(conn), "Declaring queue");
	//将queue同exchange绑定
	/**
	* amqp_queue_bind
	*
	* @param [in] state connection state
	* @param [in] channel the channel to do the RPC on
	* @param [in] queue queue
	* @param [in] exchange exchange
	* @param [in] routing_key routing_key
	* @param [in] arguments arguments
	* @returns amqp_queue_bind_ok_t
	*/
	amqp_queue_bind(conn, atoi(counter->channel), queuename, amqp_cstring_bytes(counter->exchange_name),amqp_cstring_bytes(counter->routingkey), amqp_empty_table);//此处的routingkey也是同SN编号相同
	die_on_amqp_error(amqp_get_rpc_reply(conn), "Binding queue");
	/*
	* @param [in] state connection state
	* @param [in] channel the channel to do the RPC on
	* @param [in] queue queue
	* @param [in] consumer_tag consumer_tag
	* @param [in] no_local no_local
	* @param [in] no_ack no_ack
	* @param [in] exclusive exclusive
	* @param [in] arguments arguments
	*/
	amqp_basic_consume(conn, atoi(counter->channel), queuename, amqp_empty_bytes, 0, 1, 0,amqp_empty_table);
	die_on_amqp_error(amqp_get_rpc_reply(conn), "Consuming");

	{
		printf("\r\n");
		printf("amqp listen start!! \r\n");
		printf("server ip is : %s \r\n" , counter->server_ip);
		printf("server port is : %s \r\n" , counter->server_port);
		printf("exchange name is : %s \r\n" , counter->exchange_name);
		printf("queue name is : %s \r\n" , queuename.bytes);
		printf("routingkey is : %s \r\n", counter->routingkey);
		printf("\r\n");
		printf("\r\n");
		for (;;) 
		{
			amqp_rpc_reply_t res;
			amqp_envelope_t envelope;

			amqp_maybe_release_buffers(conn);

			//這個函數的第三個參數為超時時間，如果為null則為阻塞式，最後一個參數暫不使用，直接為0即可
			printf("\r\n");
			printf("\r\n");
			printf("Wait for messaage ...\r\n");
			printf("\r\n");
			printf("\r\n");
			res = amqp_consume_message(conn, &envelope, NULL, 0);//此处设定的是阻塞式监听

			if (AMQP_RESPONSE_NORMAL != res.reply_type) //以下几行代码的正确性还需要测试，若触发if条件后代表有异常，在持续等待信息不知道是否正确
				                                        //也许重复执行该函数也是正确的处理方式
			{
				//接收到了异常信息？？
				//amqp_destroy_envelope(&envelope);//不知道在此处执行此句是否会有问题，需要测试###
				//continue;
				break;
			}


			//解析接收到的数据，注意目前测试的情况是消费者必须要执行完所有处理内容才会再次接受服务器传输过来的数据，因此不会出现当该线程
			//正在处理一个命令的时候又接收到了另外的一个命令的情况，因此counter结构体中的isbusy值在此处并未使用
			json_parse_fun(envelope.message.body.bytes);
			amqp_destroy_envelope(&envelope);
		}
	}

	die_on_amqp_error(amqp_channel_close(conn, 1, AMQP_REPLY_SUCCESS),
		"Closing channel");
	die_on_amqp_error(amqp_connection_close(conn, AMQP_REPLY_SUCCESS),
		"Closing connection");
	die_on_error(amqp_destroy_connection(conn), "Ending connection");

	return AMQP_FUN_SUCCESS;
}

//传入envelope.message.body.bytes参数，这个地方未来可以更改为多线程
//更改时要注意将amqp的数据消息深拷贝，以便在次主线程释放该资源的时候，从线程不会报错
/*
* 对上下行命令的规划（是上下行是柜子同服务器之间），暂在此处编辑，成熟后要形成文档
* devid : xxxxx 设备编号
* cmdid : xxxxx 命令编号
* cmddata: xxxxx 命令所需数据，可以为空
* 后续为命令所需要的数据，若没有数据则为空
*/
static int json_parse_fun(char * amqp_message)
{
	JSON_Value * root_value = json_parse_string(amqp_message);
	//JSON_Value * val = NULL;
	JSON_Object * json_object_buf = NULL;

	if (root_value != NULL)//如果为空则代表错误
	{
		//开始解析内容，根值应该为一个object，否则不处理，是否合适还需测试，对于解析错误的地方如id号不对或者其他情况应该考虑向服务器回复错误信息
		if (json_value_get_type(root_value) == JSONObject)
		{
			json_object_buf = json_value_get_object(root_value);
			//获取devid字符串的值
			char * devid_buf = json_object_get_string(json_object_buf, "devid");
			if (devid_buf != NULL && (strcmp(counter->sn, devid_buf) == 0) || (strcmp(" broadcast", devid_buf) == 0))
			{
				Check_Cmd(json_object_buf);
			}
			//val = json_object_get_value(json_object_buf, "devid");
			//if (val != NULL && json_value_get_type(val) == JSONString)
			//{
				/*
				*	如果发送过来的数据中，devid等于本柜子的id或者等于 broadcast 则此柜子会继续处理该条指令
				*/
				//if ((strcmp(counter->sn, json_value_get_string(val)) == 0)  || (strcmp(" broadcast", json_value_get_string(val)) == 0))
				//{
					//Check_Cmd(json_object_buf);
				//}
			//}
		}
		else
		{
			//接收数据类型错误
			printf("Received message  not a JSON Object\r\n");
		}
	}
	else
	{
		//解析字符串有错误
		printf("Received message was not a JSON\r\n");
	}

	json_value_free(root_value);

	return AMQP_FUN_SUCCESS;

}

/*命令检查函数，该函数的参数json_object变量会在退出函数后由上层函数释放资源
* 接收到命令后，需要根据柜子的状态选择是否处理，如果柜子正处于拒接接收的状态
* 则不应该处理任何服务器端发送过来的数据
*/

static int Check_Cmd(JSON_Object * json_object)
{

	//JSON_Value * val = json_object_get_value(json_object, "cmdid");
	char * cmd = json_object_get_string(json_object, "cmdid");
	if (cmd != NULL)
	{
		//char * cmd = json_value_get_string(val);
		if (strcmp(cmd, "Shopping") == 0)//执行一次消费
		{

			printf("执行一次消费\r\n");
			//char * result_p = Procedure_Sales();

			char * result_p = Procedure_Sales_Ex(json_object);
			if (result_p != NULL)
			{
				Amqp_public_message(conn, "amq.direct", "server", result_p);
				free(result_p);
			}
			
		}
		else if (strcmp(cmd, "Unlock") == 0)
		{
			printf("执行一次开门\r\n");
			char * result_p = Procedure_Open_Lock();
			Amqp_public_message(conn, "amq.direct", "server", result_p);
			free(result_p);
		}
		else if (strcmp(cmd, "Unlock_Close") == 0)
		{
			printf("执行一次开关门\r\n");
			char * result_p = Procedure_Open_Close();
			Amqp_public_message(conn, "amq.direct", "server", result_p);
			free(result_p);
		}
		else if (strcmp(cmd, "Locker_State") == 0)
		{
			printf("获取柜子门的状态\r\n");
			char * result_p = Procedure_Get_Locker_State();
			Amqp_public_message(conn, "amq.direct", "server", result_p);
			free(result_p);
		}
		else if (strcmp(cmd, "Basic_Value") == 0)//指定称重板编号执行去皮
		{
			printf("执行一次去皮\r\n");
			char * result_p = Procedure_Basic_Value_Set(json_object);
			Amqp_public_message(conn, "amq.direct", "server", result_p);
			free(result_p);
		}
		else if (strcmp(cmd, "Curavture_Value") == 0)//指定称重板编号曲率校准
		{
			printf("执行一次曲率校准\r\n");
			char * result_p = Procedure_Set_Curavture_Value(json_object);
			Amqp_public_message(conn, "amq.direct", "server", result_p);
			free(result_p);
		}
		else if (strcmp(cmd, "Weight_Value") == 0)//指定称重板编号的重量获取,可以指定多个
		{
			printf("执行一次称重\r\n");
			char * result_p = Procedure_Get_Weight_Value(json_object);
			Amqp_public_message(conn, "amq.direct", "server", result_p);
			free(result_p);
		}
		else if (strcmp(cmd, "Board_State") == 0)//获取称重板状态
		{
			printf("执行一次获取称重板状态\r\n");
			char * result_p = Procedure_Get_Board_State();
			Amqp_public_message(conn, "amq.direct", "server", result_p);
			free(result_p);
		}
		//else if (strcmp(cmd, "Basic_Value") == 0)//执行去皮
		//{
		//	printf("执行一次去皮\r\n");
		//	char * result_p = Board_Basic_Value_Set_With_ACK();//正常的取放货
		//	Amqp_public_message(conn, "amq.direct", "server", result_p);
		//	free(result_p);
		//}
		else if (strcmp(cmd, "SQL_Select") == 0)//执行sql select语句，经测试就算查询没有查询到结果也没问题
		{
			//执行传输过来的sql语句，并返回结果,结果分为两类，select类和增删改的结果
			//select结果是查询的数据，增删改的结果是影响的数据数量
			printf("执行一条sql select语句\r\n");
			char * result_p =  Procedure_SQL_Select(json_object);
			Amqp_public_message(conn, "amq.direct", "server", result_p);
			free(result_p);
			//printf("cmdid is : sql\r\n");
		}
		else if (strcmp(cmd, "SQL_Updata") == 0)//执行sql 增删改语句，此处Updata不是指sql语句updata，而是更新的意思，更新是指增删改
		{

			printf("执行一条sql 更新语句\r\n");
			char * result_p = Procedure_SQL_Updata(json_object);
			Amqp_public_message(conn, "amq.direct", "server", result_p);
			free(result_p);
			//Procedure_SQL_Select(json_object);
			//printf("cmdid is : sql\r\n");
		}
		else if (strcmp(cmd, "status") == 0)//获取柜子状态
		{
			//此处可返回
		}
		else
		{
			printf("Cmdid unknown!\n\n");
		}
	}

	return AMQP_FUN_SUCCESS;

}

//发送数据函数，简化版
/*
*	消息发送函数简化版
*	应该把channel这个参数也拿出来,这个参数目前直接采用数据库里面设定编号
*/
static int Amqp_public_message(amqp_connection_state_t state, char * exchange, char * routingkey, char * message)
{
	amqp_basic_properties_t props;
	props._flags = AMQP_BASIC_CONTENT_TYPE_FLAG | AMQP_BASIC_DELIVERY_MODE_FLAG;
	props.content_type = amqp_cstring_bytes("text/plain");
	props.delivery_mode = 2; /* persistent delivery mode */
	die_on_error(amqp_basic_publish(conn, atoi(counter->channel), amqp_cstring_bytes(exchange),\
		         amqp_cstring_bytes(routingkey), 0, 0,&props, amqp_cstring_bytes(message)), "Publishing");

	return AMQP_FUN_SUCCESS;
}
