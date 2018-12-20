#pragma once //这个预编译功能同ifndef define 这两句相类似

#include "global.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "parson.h"

#include <amqp.h>
#include <amqp_tcp_socket.h>

#include <assert.h>

#include "utils.h"

#define AMQP_FUN_SUCCESS 0
#define AMQP_FUN_FAILURE -1

struct Send_Message_Buf
{
	char * message;
	int Isused;//message中是否有未发送数据，0为没有，1为有，message是在执行某一个业务完毕后发送数据的时候没有发送成功，这样该数据就不能被释放
			   //按照现有流程，若出现不能发送数据的情况，则该线程会退出，进而重新连接服务器，因此在服务器每次连接成功后，都先判断下是否有没有
			   //发送的数据，如果有则先发送后再释放
};

extern amqp_connection_state_t conn;

int init_amqp();
int run_listen(void * dummy);
void Destory_connection(amqp_connection_state_t conn, amqp_channel_t channel);

