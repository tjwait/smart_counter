
#include "amqp_fun.h"
#pragma comment(lib, "rabbitmq.4.lib")

amqp_socket_t *amqp_socket = NULL;
amqp_connection_state_t conn;

struct Send_Message_Buf smb;

amqp_bytes_t queuename;//queue ������
                       //exchange ������ͬ���ӵ�SN���
                       //Ҫ���ӵķ�������Ϣ��counter�ṹ����

int init_amqp()
{
	//��Ҫ��ʼ���������������
	queuename.bytes = counter->queue_name;//��ʼ��queue�����֣����������������ݿ��趨��Ŀǰ��ֵͬ���ӵ�SN����ͬ
									//�ù�����Ϣ��SN�������queue,�˴����ܻ��������ף�Ӧ�����ɷ�����������queue���������ִ洢�����ݿ���
								  //�˴�ֱ�ӽ��а󶨣���������������ע��˴�������queue�����ڷ���������ӷ��������õģ���queue����ͬ
								  //���������õ�queue����
	queuename.len = strlen(counter->queue_name);
	return AMQP_FUN_SUCCESS;
}

//�������ݺ���
int run_listen(void * dummy)
{
	dummy = NULL;
	//amqp_socket_t *socket = NULL;
	//amqp_connection_state_t conn;
	int status;
	amqp_frame_t frame;//�@��׃��������ÿ�ν��յ����e�`�Ĕ�����
	conn = amqp_new_connection();//�˴��ǽ���һ��amqp���ӵĽṹ�岢��ʼ���ýṹ���ýṹ����Ҫʹ��amqp_destroy_connection()������Դ�ͷ�
								 //��������һ��amqp_connection_state_t ָ�������ɹ���������null������0�����ʧ��

	amqp_socket = amqp_tcp_socket_new(conn);//�˺����ǽ���tcp�շ��Ĺؼ�����������amqp_new_connection()�������صĽṹ��ָ��
											//�˺������ڲ�������һ��amqp_tcp_socket_t�ṹ�壬�ýṹ����������״̬��sock�������Լ��շ��ȹ����ͺ�����
											//�����й��շ�����Ҫ���ܺ�����ͨ���ڲ�һ��klassָ�룬ָ��amqp_socket_class_t�ṹ�����amqp_tcp_socket_class�ĵ�ַ��ʵ�ֵ�
											//amqp_tcp_socket_class�ýṹ�������Ԫ�ؾ�Ϊ����ָ�룬����6���������ֱ�Ϊ���ա����͡��򿪡��رա���ȡ�����ɾ��
											//���amqp_tcp_socket_new����ͨ��amqp_set_socket������amqp_tcp_socket_t�ṹ���������amqp_new_connection()���صĽṹ��
											//�е�socket���������ҽ�amqp_tcp_socket_t�ṹ���������
											//����������ķ���ֵ��ʵ�Ѿ���conn�����ڵ�conn->socket���ָ����ָ����
	if (!amqp_socket)//
	{
		//��Ϊ������˴�amqp_socketָ��Ϊnull����˲����ͷ��κ���Դ������Ҫ�˳����߳�
		//die("creating TCP socket");
		//printf("creating MQ Server TCP socket error , Retry in senonds\r\n");
		LogWrite(INFO, "%s", "creating MQ Server TCP socket error , Retry in senonds");
		return AMQP_FUN_FAILURE;
	}

	status = amqp_socket_open(amqp_socket, counter->server_ip , atoi(counter->server_port));//��tcp���Ӻ��������ڲ�ʵ���ϵ�����klassָ��ָ��Ľṹ���ص�open������ʵ�ֵ�
	if (status) 
	{
		//�ͷ�socket��Դ
		//printf("\r\nConnection MQ Server error  , Retry in senonds\r\n");
		LogWrite(INFO, "%s", "Connection MQ Server error  , Retry in senonds");
		return AMQP_FUN_FAILURE;
	}

	//��¼��amqp_login����Ϊ  
	//1�����Ӷ��󣬼�amqp_new_connection�����ķ���ֵ���˴�Ϊconn
	//2�����ӵ��������һ�㶼Ϊ"/"��broker������ĸ����һ������ر����������Ҫ��һ���˽⣬��broker�п����趨��ͬ�ĵ�¼�û�����Ӧ����������ƣ�guest�պ�Ĭ����"/"
	//3������channel��������ֵ�����Ӷ����ʼ����ʱ���趨Ϊ0����Ϊ�����ƣ������65535��channel-0Ϊ��������������Ҳ���趨��ֵ����ʵ��ʹ����ʹ�ý�С��һ����ֵ��Ϊ�޶�
	//4��֡��󳤶ȣ�4096λ��С��2��31�η�-1Ϊ���131072Ϊ128K�������ֵ��ΪĬ��ֵ���
	//5���������ܼ�����趨һ����������Ϊheartbeat�ļ��ʱ�䣬����趨Ϊ0��Ϊ��ʹ��
	//6����¼����������֪���ƣ�SASLΪc/s��һ����֤���ƣ�����amqp�����еģ��ڴ˴��ṩ������֤����
	//		*AMQP_SASL_METHOD_PLAIN ���ô��ַ�ʽ���ڸò�������Ҫ�ṩ�û�����������������������������ʹ�õķ�ʽ
	//		*AMQP_SASL_METHOD_EXTERNAL ���ô��ַ�ʽ���ڸĲ�����Ҫ�ṩһ����֤�ַ�����������ʹ�û���Ҫ���˽�,��֪���Ƿ�Ϊ����Ҫ������û��ף��½��õ�ʱ���Ƿ���Ҫ�����ǿ�ѡ�
	//die_on_amqp_error(amqp_login(conn, "/", 0, 131072, 60 , AMQP_SASL_METHOD_PLAIN, counter->mq_name, counter->mq_pw), "Logging in");
	if (die_on_amqp_error(amqp_login(conn, "/", 0, 131072, 60, AMQP_SASL_METHOD_PLAIN, counter->mq_name, counter->mq_pw), "Logging in") == AMQP_FUN_FAILURE)
	{
		LogWrite(INFO, "%s", "Retry in senonds");
		return AMQP_FUN_FAILURE;
	}

	amqp_channel_open(conn, atoi(counter->channel));
	/*
	*	amqp_get_rpc_reply�����ܹ���Ӧ�ڴ����AMQPͬ���ķ������䷵��һ��ָ����÷��������ָ�룬�����쳣��ʱ�򣬸ú�������null����ʱ��Ҫͨ��һЩ�����˽⵽��ʲôԭ����ɵ��쳣
	*	�ú�������һ��API���������·���ֵ��ʵ��
	*	�ú���ֻʹ���ڱ����ú���������ִ�н�����ص�ʱ�򣨲�֪���˴������Ƿ���ȷ��
	*	�������ʵ�����Ƿ������Ӷ���conn�е�most_recent_api_result��ֵ����ֵΪһ���ṹ��amqp_rpc_reply_t���ýṹ������һ������ֵ������������ֵ
	*		*reply_typeΪ����ֵ����Ϊ1��AMQP_RESPONSE_NORMAL�������ո�ִ�е�api��������ΪAMQP_RESPONSE_SERVER_EXCEPTION����AMQP_RESPONSE_LIBRARY_EXCEPTION�����ո�ִ�е�api������
	*		*������������Ҫ��������ֵreply��һ���ṹ�壩��library_error��һ��int�ͣ�������ԭ�򣬴˴��ɼ�die_on_amqp_error�����������˷���֮�õ�
	*/
	if(die_on_amqp_error(amqp_get_rpc_reply(conn), "Opening channel") == AMQP_FUN_FAILURE)
	{
		LogWrite(INFO, "%s", "Retry in senonds");
		return AMQP_FUN_FAILURE;
	}

    //����queue�����²������onenote�ʼ�
	/*  @param [in] state connection state
	* @param [in] channel the channel to do the RPC on //���͸��ĸ�channel
	* @param [in] queue queue //���������Ϊ����Ϊ��ϵͳ�Զ�����
	* @param [in] passive passive //��֪����ʲô����
	* @param [in] durable durable //�־û�����
	* @param [in] exclusive exclusive //��������ֻ����һ������
	* @param [in] auto_delete auto_delete //�����ж��ĸ�queue�����ӶϿ���queueɾ��
	* @param [in] arguments arguments
	* �˺�����ִ�����ݽ�Ϊ���������������뺯���ڲ���amqp_queue_declare_t�ṹ����������ҵ���amqp_simple_rpc_decoded����������������queue�������ȴ��ظ�
	* Ŀǰ��Ҫ�˽�������һ��������������queue��ѡ��һЩ����������
	*/
	amqp_queue_declare_ok_t *r = amqp_queue_declare(conn, atoi(counter->channel), queuename , 0, 0, 0, 1, amqp_empty_table);//�˺�����һЩ��������Ϊ����Ҫ���������
	if(die_on_amqp_error(amqp_get_rpc_reply(conn), "Declaring queue") == AMQP_FUN_FAILURE)
	{
		LogWrite(INFO, "%s", "Retry in senonds");
		return AMQP_FUN_FAILURE;
	}
	//��queueͬexchange��
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
	amqp_queue_bind(conn, atoi(counter->channel), queuename, amqp_cstring_bytes(counter->exchange_name),amqp_cstring_bytes(counter->routingkey), amqp_empty_table);//�˴���routingkeyҲ��ͬSN�����ͬ
	//die_on_amqp_error(amqp_get_rpc_reply(conn), "Binding queue");
	if (die_on_amqp_error(amqp_get_rpc_reply(conn), "Binding queue") == AMQP_FUN_FAILURE)
	{
		LogWrite(INFO, "%s", "Retry in senonds");
		return AMQP_FUN_FAILURE;
	}
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
	//die_on_amqp_error(amqp_get_rpc_reply(conn), "Consuming");
	if (die_on_amqp_error(amqp_get_rpc_reply(conn), "Consuming") == AMQP_FUN_FAILURE)
	{
		LogWrite(INFO, "%s", "Retry in senonds");
		return AMQP_FUN_FAILURE;
	}

	{

		char * s_buf[100];
		LogWrite(INFO, "%s", "amqp listen start");
		sprintf(s_buf, "server ip is : %s", counter->server_ip);
		LogWrite(INFO, "%s", s_buf);
		sprintf(s_buf, "server port is : %s", counter->server_port);
		LogWrite(INFO, "%s", s_buf);
		sprintf(s_buf, "exchange name is : %s", counter->exchange_name);
		LogWrite(INFO, "%s", s_buf);
		sprintf(s_buf, "queue name is : %s", queuename.bytes);
		LogWrite(INFO, "%s", s_buf);
		sprintf(s_buf, "routingkey is : %s", counter->routingkey);
		LogWrite(INFO, "%s", s_buf);
		//printf("\r\n");
		//printf("amqp listen start!! \r\n");
		//printf("server ip is : %s \r\n" , counter->server_ip);
		//printf("server port is : %s \r\n" , counter->server_port);
		//printf("exchange name is : %s \r\n" , counter->exchange_name);
		//printf("queue name is : %s \r\n" , queuename.bytes);
		//printf("routingkey is : %s \r\n", counter->routingkey);
		//printf("\r\n");
		//printf("\r\n");
		for (;;) 
		{
			amqp_rpc_reply_t res;
			amqp_envelope_t envelope;

			amqp_maybe_release_buffers(conn);
			//������Ƿ���û�����гɹ�������
			if (smb.Isused)
			{
				LogWrite(INFO, "%s", "A Message Need to Resend");
				//printf("\r\n��δ���ͳɹ�������Ҫ���·���\r\n");
				if (Amqp_public_message(conn, "amq.direct", "server", smb.message) == AMQP_FUN_SUCCESS)
				{
					free(smb.message);
					smb.Isused = 0;
					LogWrite(INFO, "%s", "A Resend Message Send SUCCESS");
					//printf("\r\n���ݷ��ͳɹ�\r\n");
				}
				else
				{
					LogWrite(ERR, "%s", "A Resend Message Send FAILURE");
					//printf("���ݷ���ʧ��\r\n");
				}
				
			}

			//�@�������ĵ����������鳬�r�r�g�������null�t������ʽ������һ����������ʹ�ã�ֱ�Ӟ�0����
			LogWrite(INFO, "%s", "Wait for messaage ...");
			//printf("\r\n");
			//printf("\r\n");
			//printf("Wait for messaage ...\r\n");
			//printf("\r\n");
			//printf("\r\n");
			res = amqp_consume_message(conn, &envelope, NULL, 0);//�˴��趨��������ʽ����

			if (AMQP_RESPONSE_NORMAL != res.reply_type) //���¼��д������ȷ�Ի���Ҫ���ԣ�������if������������쳣���ڳ����ȴ���Ϣ��֪���Ƿ���ȷ
				                                        //Ҳ���ظ�ִ�иú���Ҳ����ȷ�Ĵ���ʽ
			{
				//Ŀǰ�����������֮������������ִ�е��˴�
				//res.reply_type��ֵλ��amqp_response_type_enum_ö����
				//res.library_error�ķ���ֵ��λ��amqp_status_enum_���ö����
				if (AMQP_RESPONSE_LIBRARY_EXCEPTION == res.reply_type && AMQP_STATUS_UNEXPECTED_STATE == res.library_error)
				{
					if (AMQP_STATUS_OK != amqp_simple_wait_frame(conn, &frame)) 
					{	
						//�@�����Z��Ĉ���Ҋamqp_consume_message����Ҫ��
						return AMQP_FUN_FAILURE;
					}
					/*
					* �����Z���������յ��Į�����Ϣ��ʲ�N��������ͬ�ă����M��̎��
					*/
					if (AMQP_FRAME_METHOD == frame.frame_type) {
						switch (frame.payload.method.id) {
						case AMQP_BASIC_ACK_METHOD://����ACK���صĵط�
							/* if we've turned publisher confirms on, and we've published a
							* message here is a message being confirmed.
							*/
							//printf("AMQP_BASIC_ACK_METHOD");
							LogWrite(INFO, "%s", "AMQP_BASIC_ACK_METHOD");
							break;
						case AMQP_BASIC_RETURN_METHOD://�����Ϣ����·��ʧ��
							/* if a published message couldn't be routed and the mandatory
							* flag was set this is what would be returned. The message then
							* needs to be read.
							*/
						{
							amqp_message_t message;
							res = amqp_read_message(conn, frame.channel, &message, 0);
							if (AMQP_RESPONSE_NORMAL != res.reply_type) {
								LogWrite(ERR, "%s", "AMQP_BASIC_RETURN_METHOD");
								return AMQP_FUN_FAILURE;
							}
							//printf("AMQP_BASIC_RETURN_METHOD");
							//�������յ���һ��ACK
							LogWrite(INFO, "%s", "AMQP_BASIC_RETURN_METHOD");
							amqp_destroy_message(&message);
						}

						break;

						case AMQP_CHANNEL_CLOSE_METHOD://�򲻴��ڵ�exchange�������ݵ�ʱ��ᴥ���˴���
							/* a channel.close method happens when a channel exception occurs,
							* this can happen by publishing to an exchange that doesn't exist
							* for example.
							*
							* In this case you would need to open another channel redeclare
							* any queues that were declared auto-delete, and restart any
							* consumers that were attached to the previous channel.
							*/
							//printf("AMQP_CHANNEL_CLOSE_METHOD");
							LogWrite(ERR, "%s", "AMQP_CHANNEL_CLOSE_METHOD");
							return AMQP_FUN_FAILURE;

						case AMQP_CONNECTION_CLOSE_METHOD:
							/* a connection.close method happens when a connection exception
							* occurs, this can happen by trying to use a channel that isn't
							* open for example.
							*
							* In this case the whole connection must be restarted.
							*/
							//printf("AMQP_CONNECTION_CLOSE_METHOD");
							LogWrite(ERR, "%s", "AMQP_CONNECTION_CLOSE_METHOD");
							return AMQP_FUN_FAILURE;

						default:
							//fprintf(stderr, "An unexpected method was received %u\n",frame.payload.method.id);
							sprintf(s_buf, "An unexpected method was received %u", frame.payload.method.id);
							LogWrite(ERR, "%s", s_buf);
							return AMQP_FUN_FAILURE;
						}
					}
				}
				else if (1)//Ŀǰ�Ĳ����������һ��if�������󣬾�ֱ�ӽ����if�����
				{
					//�������������amqp_consume_message��������������ʽ������Ȼ������˴�����
					LogWrite(ERR, "%s", "AMQP Consumer Has Some Error , Thread Need To Restart");
					return AMQP_FUN_FAILURE;
				}

			}
			
			else//������ܵ�����������
			{
				//�������յ������ݣ�ע��Ŀǰ���Ե�����������߱���Ҫִ�������д������ݲŻ��ٴν��ܷ�����������������ݣ���˲�����ֵ����߳�
				//���ڴ���һ�������ʱ���ֽ��յ��������һ���������������counter�ṹ���е�isbusyֵ�ڴ˴���δʹ��
				json_parse_fun(envelope.message.body.bytes);
				amqp_destroy_envelope(&envelope);
			}
			
		}
	}
	//����Ӧ�ò���ﵽ�˴�
	return AMQP_FUN_SUCCESS;
}

//�ر�mq����
void Destory_connection(amqp_connection_state_t conn, amqp_channel_t channel)
{
	//amqp_rpc_reply_t res;
	//die_on_amqp_error(amqp_channel_close(conn, channel, AMQP_REPLY_SUCCESS),"Closing channel");
	//die_on_amqp_error(amqp_connection_close(conn, AMQP_REPLY_SUCCESS),"Closing connection");
	//die_on_error(amqp_destroy_connection(conn), "Ending connection");
	LogWrite(INFO, "%s", "Destory MQ Connection");
	amqp_channel_close(conn, channel, AMQP_REPLY_SUCCESS);
	amqp_connection_close(conn, AMQP_REPLY_SUCCESS);
	amqp_destroy_connection(conn);
	//printf("destroy connection res is : %d", r);
}

//����envelope.message.body.bytes����������ط�δ�����Ը���Ϊ���߳�
//����ʱҪע�⽫amqp��������Ϣ������Ա��ڴ����߳��ͷŸ���Դ��ʱ�򣬴��̲߳��ᱨ��
/*
* ������������Ĺ滮�����������ǹ���ͬ������֮�䣩�����ڴ˴��༭�������Ҫ�γ��ĵ�
* devid : xxxxx �豸���
* cmdid : xxxxx ������
* cmddata: xxxxx �����������ݣ�����Ϊ��
* ����Ϊ��������Ҫ�����ݣ���û��������Ϊ��
*/
static int json_parse_fun(char * amqp_message)
{
	JSON_Value * root_value = json_parse_string(amqp_message);
	//JSON_Value * val = NULL;
	JSON_Object * json_object_buf = NULL;

	if (root_value != NULL)//���Ϊ����������
	{
		//��ʼ�������ݣ���ֵӦ��Ϊһ��object�����򲻴����Ƿ���ʻ�����ԣ����ڽ�������ĵط���id�Ų��Ի����������Ӧ�ÿ�����������ظ�������Ϣ
		if (json_value_get_type(root_value) == JSONObject)
		{
			json_object_buf = json_value_get_object(root_value);
			//��ȡdevid�ַ�����ֵ
			char * devid_buf = json_object_get_string(json_object_buf, "devid");
			if (devid_buf != NULL && (strcmp(counter->sn, devid_buf) == 0) || (strcmp(" broadcast", devid_buf) == 0))
			{
				//���ж�������յ������ݵ�sn�����Ƿ��Ѿ��Ǵ�����ģ�����Ǿͽ���������������·���һ��
				char * message_buf = Get_up_message_message("up_message", "msn", json_object_get_string(json_object_buf, "MSN"));
				if (message_buf == NULL)
				{
					//printf("\r\n�յ���һ���µ����ʼ����\r\n");
					LogWrite(INFO, "%s", "Rec A New Message , Start Parse CMD");
					Check_Cmd(json_object_buf);
				}
				else
				{
					//printf("\r\n�յ���һ��MSN�ظ��������������������������ϴ�\r\n");
					LogWrite(INFO, "%s", "Rec Message MSN Duplication , Resend To Server");
					Amqp_public_message(conn, "amq.direct", "server", message_buf);
					free(message_buf);//��ʽ�ͷ�
				}
			}

		}
		else
		{
			//�����������ʹ���
			//printf("Received message  not a JSON Object\r\n");
			LogWrite(ERR, "%s", "Rec Message Type Error , not a JSON Object");
		}
	}
	else
	{
		//�����ַ����д���
		//printf("Received message was not a JSON\r\n");
		LogWrite(ERR, "%s", "Rec Message Error , not a JSON");
	}

	json_value_free(root_value);

	return AMQP_FUN_SUCCESS;

}

/*�����麯�����ú����Ĳ���json_object���������˳����������ϲ㺯���ͷ���Դ
* ���յ��������Ҫ���ݹ��ӵ�״̬ѡ���Ƿ���������������ھܽӽ��յ�״̬
* ��Ӧ�ô����κη������˷��͹���������
*/

static int Check_Cmd(JSON_Object * json_object)
{

	//JSON_Value * val = json_object_get_value(json_object, "cmdid");
	char * cmd = json_object_get_string(json_object, "cmdid");
	if (cmd != NULL)
	{
		//char * cmd = json_value_get_string(val);
		if (strcmp(cmd, "Shopping") == 0)//ִ��һ������
		{

			printf("ִ��һ������\r\n");
			//char * result_p = Procedure_Sales();

			//char * result_p = Procedure_Sales_Ex(json_object);
			smb.message = Procedure_Sales_Ex(json_object);
			if (smb.message != NULL)
			{
				smb.Isused = 1;
				//�˴���ҵ���߼�����Ϊ���������Ϣ��ʱ����ʧ�ܣ��ڽ������������ߵȴ����ݵ�ʱ��Ҳ�������ʱ���߳̾ͻ��˳������Ƿ�����л���Ҫ��֤
				if (Amqp_public_message(conn, "amq.direct", "server", smb.message) == AMQP_FUN_SUCCESS)
				{
					free(smb.message);
					smb.Isused = 0;
				}
				
			}
			
		}
		else if (strcmp(cmd, "Unlock") == 0)
		{
			printf("ִ��һ�ο���\r\n");
			smb.message = Procedure_Open_Lock(json_object);
			if (smb.message != NULL)
			{
				smb.Isused = 1;
				//�˴���ҵ���߼�����Ϊ���������Ϣ��ʱ����ʧ�ܣ��ڽ������������ߵȴ����ݵ�ʱ��Ҳ�������ʱ���߳̾ͻ��˳������Ƿ�����л���Ҫ��֤
				if (Amqp_public_message(conn, "amq.direct", "server", smb.message) == AMQP_FUN_SUCCESS)
				{
					free(smb.message);
					smb.Isused = 0;
				}

			}
		}
		else if (strcmp(cmd, "Unlock_Close") == 0)
		{
			printf("ִ��һ�ο�����\r\n");
			smb.message = Procedure_Open_Close(json_object);
			if (smb.message != NULL)
			{
				smb.Isused = 1;
				//�˴���ҵ���߼�����Ϊ���������Ϣ��ʱ����ʧ�ܣ��ڽ������������ߵȴ����ݵ�ʱ��Ҳ�������ʱ���߳̾ͻ��˳������Ƿ�����л���Ҫ��֤
				if (Amqp_public_message(conn, "amq.direct", "server", smb.message) == AMQP_FUN_SUCCESS)
				{
					free(smb.message);
					smb.Isused = 0;
				}

			}
		}
		else if (strcmp(cmd, "Locker_State") == 0)
		{
			printf("��ȡ�����ŵ�״̬\r\n");
			smb.message = Procedure_Get_Locker_State(json_object);
			if (smb.message != NULL)
			{
				smb.Isused = 1;
				//�˴���ҵ���߼�����Ϊ���������Ϣ��ʱ����ʧ�ܣ��ڽ������������ߵȴ����ݵ�ʱ��Ҳ�������ʱ���߳̾ͻ��˳������Ƿ�����л���Ҫ��֤
				if (Amqp_public_message(conn, "amq.direct", "server", smb.message) == AMQP_FUN_SUCCESS)
				{
					free(smb.message);
					smb.Isused = 0;
				}

			}
		}
		else if (strcmp(cmd, "Basic_Value") == 0)//ָ�����ذ���ִ��ȥƤ
		{
			printf("ִ��һ��ȥƤ\r\n");
			smb.message = Procedure_Basic_Value_Set(json_object);
			if (smb.message != NULL)
			{
				smb.Isused = 1;
				//�˴���ҵ���߼�����Ϊ���������Ϣ��ʱ����ʧ�ܣ��ڽ������������ߵȴ����ݵ�ʱ��Ҳ�������ʱ���߳̾ͻ��˳������Ƿ�����л���Ҫ��֤
				if (Amqp_public_message(conn, "amq.direct", "server", smb.message) == AMQP_FUN_SUCCESS)
				{
					free(smb.message);
					smb.Isused = 0;
				}

			}
		}
		else if (strcmp(cmd, "Curavture_Value") == 0)//ָ�����ذ�������У׼
		{
			printf("ִ��һ������У׼\r\n");
			smb.message = Procedure_Set_Curavture_Value(json_object);
			if (smb.message != NULL)
			{
				smb.Isused = 1;
				//�˴���ҵ���߼�����Ϊ���������Ϣ��ʱ����ʧ�ܣ��ڽ������������ߵȴ����ݵ�ʱ��Ҳ�������ʱ���߳̾ͻ��˳������Ƿ�����л���Ҫ��֤
				if (Amqp_public_message(conn, "amq.direct", "server", smb.message) == AMQP_FUN_SUCCESS)
				{
					free(smb.message);
					smb.Isused = 0;
				}

			}
		}
		else if (strcmp(cmd, "Weight_Value") == 0)//ָ�����ذ��ŵ�������ȡ,����ָ�����
		{
			printf("ִ��һ�γ���\r\n");
			smb.message = Procedure_Get_Weight_Value(json_object);
			if (smb.message != NULL)
			{
				smb.Isused = 1;
				//�˴���ҵ���߼�����Ϊ���������Ϣ��ʱ����ʧ�ܣ��ڽ������������ߵȴ����ݵ�ʱ��Ҳ�������ʱ���߳̾ͻ��˳������Ƿ�����л���Ҫ��֤
				if (Amqp_public_message(conn, "amq.direct", "server", smb.message) == AMQP_FUN_SUCCESS)
				{
					free(smb.message);
					smb.Isused = 0;
				}

			}
		}
		else if (strcmp(cmd, "Board_State") == 0)//��ȡ���ذ�״̬
		{
			printf("ִ��һ�λ�ȡ���ذ�״̬\r\n");
			smb.message = Procedure_Get_Board_State();
			if (smb.message != NULL)
			{
				smb.Isused = 1;
				//�˴���ҵ���߼�����Ϊ���������Ϣ��ʱ����ʧ�ܣ��ڽ������������ߵȴ����ݵ�ʱ��Ҳ�������ʱ���߳̾ͻ��˳������Ƿ�����л���Ҫ��֤
				if (Amqp_public_message(conn, "amq.direct", "server", smb.message) == AMQP_FUN_SUCCESS)
				{
					free(smb.message);
					smb.Isused = 0;
				}

			}
		}
		else if (strcmp(cmd, "SQL_Select") == 0)//ִ��sql select��䣬�����Ծ����ѯû�в�ѯ�����Ҳû����
		{
			//ִ�д��������sql��䣬�����ؽ��,�����Ϊ���࣬select�����ɾ�ĵĽ��
			//select����ǲ�ѯ�����ݣ���ɾ�ĵĽ����Ӱ�����������
			printf("ִ��һ��sql select���\r\n");
			smb.message =  Procedure_SQL_Select(json_object);
			if (smb.message != NULL)
			{
				smb.Isused = 1;
				//�˴���ҵ���߼�����Ϊ���������Ϣ��ʱ����ʧ�ܣ��ڽ������������ߵȴ����ݵ�ʱ��Ҳ�������ʱ���߳̾ͻ��˳������Ƿ�����л���Ҫ��֤
				if (Amqp_public_message(conn, "amq.direct", "server", smb.message) == AMQP_FUN_SUCCESS)
				{
					free(smb.message);
					smb.Isused = 0;
				}

			}

		}
		else if (strcmp(cmd, "SQL_Updata") == 0)//ִ��sql ��ɾ����䣬�˴�Updata����ָsql���updata�����Ǹ��µ���˼��������ָ��ɾ��
		{

			printf("ִ��һ��sql �������\r\n");
			smb.message = Procedure_SQL_Updata(json_object);
			if (smb.message != NULL)
			{
				smb.Isused = 1;
				//�˴���ҵ���߼�����Ϊ���������Ϣ��ʱ����ʧ�ܣ��ڽ������������ߵȴ����ݵ�ʱ��Ҳ�������ʱ���߳̾ͻ��˳������Ƿ�����л���Ҫ��֤
				if (Amqp_public_message(conn, "amq.direct", "server", smb.message) == AMQP_FUN_SUCCESS)
				{
					free(smb.message);
					smb.Isused = 0;
				}

			}
			
		}
		else if (strcmp(cmd, "status") == 0)//��ȡ����״̬
		{
			//�˴��ɷ���
		}
		else
		{
			printf("Cmdid unknown!\n\n");
		}
	}

	return AMQP_FUN_SUCCESS;

}

//�������ݺ������򻯰�
/*
*	��Ϣ���ͺ����򻯰�
*	Ӧ�ð�channel�������Ҳ�ó���,�������Ŀǰֱ�Ӳ������ݿ������趨���
*
*	ע����������������ڷ��͹������������������δ��������ʱ������ʱ�򣨿�����Ϊ����ʱ��ͬҪ��������ʱ������Ͻ�������ʱִ�з������
*		���ᱨ����������û�з��͵ġ�����������������������ˮ�˺���������Ψһ��ŵ��ظ��ж����ƣ������ִ�����������λ���ڵȴ�һ��ʱ��
*		δ���յ��ظ���Ϣ�����ظ��������������Ų��䣩��ϵͳ���Ѿ��ָ���������Ὣ֮ǰû�з��ͳ�ȥ�������ظ�����
*	ע�����ڷ���ǰ�������������ݷ��͹�����ռ�ø��̹߳���������ַ���ʧ�ܵ������listen�̻߳��˳��������Զ�����������������������������
*		��Ὣ�����ٴη�������������������շ��ͻ��档���ٴη���ʧ�ܴ��������Ծ�û�лظ����˴��������ݲ��ᶪʧ����������κ��쳣�����
*		���ݶ�ʧ�󣬷�������ͨ���ظ�����ָ�MSN���䣩��������������ݿ��н�δ�ɹ����͵���������
*/
static int Amqp_public_message(amqp_connection_state_t state, char * exchange, char * routingkey, char * message)
{
	amqp_basic_properties_t props;
	props._flags = AMQP_BASIC_CONTENT_TYPE_FLAG | AMQP_BASIC_DELIVERY_MODE_FLAG;
	props.content_type = amqp_cstring_bytes("text/plain");
	props.delivery_mode = 2; /* persistent delivery mode */

	/*
	*	amqp_basic_publish�����ķ���ֵΪ���¼�����¼���ֵ���Ǹ�ֵ
	*         - AMQP_STATUS_TIMER_FAILURE: system timer facility returned an error
	*           the message was not sent.
	*         - AMQP_STATUS_HEARTBEAT_TIMEOUT: connection timed out waiting for a
	*           heartbeat from the broker. The message was not sent.
	*         - AMQP_STATUS_NO_MEMORY: memory allocation failed. The message was
	*           not sent.
	*         - AMQP_STATUS_TABLE_TOO_BIG: a table in the properties was too large
	*           to fit in a single frame. Message was not sent.
	*         - AMQP_STATUS_CONNECTION_CLOSED: the connection was closed.
	*         - AMQP_STATUS_SSL_ERROR: a SSL error occurred.
	*         - AMQP_STATUS_TCP_ERROR: a TCP error occurred. errno or
	*           WSAGetLastError() may provide more information
	*/
	int res = amqp_basic_publish(conn, atoi(counter->channel), amqp_cstring_bytes(exchange), \
		amqp_cstring_bytes(routingkey), 1, 0, &props, amqp_cstring_bytes(message));
	//die_on_amqp_error(amqp_get_rpc_reply(conn), "Publishing");
	die_on_error_ex( res , "Publishing");
	if (res == AMQP_STATUS_OK)
	{
		return AMQP_FUN_SUCCESS;
	}
	else
	{
		return res;
	}
	
}
