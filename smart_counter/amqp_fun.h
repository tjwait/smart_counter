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

int init_amqp();
int run_listen(void * dummy);


