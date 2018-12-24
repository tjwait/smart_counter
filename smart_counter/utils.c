/*
 * ***** BEGIN LICENSE BLOCK *****
 * Version: MIT
 *
 * Portions created by Alan Antonuk are Copyright (c) 2012-2013
 * Alan Antonuk. All Rights Reserved.
 *
 * Portions created by VMware are Copyright (c) 2007-2012 VMware, Inc.
 * All Rights Reserved.
 *
 * Portions created by Tony Garnock-Jones are Copyright (c) 2009-2010
 * VMware, Inc. and Tony Garnock-Jones. All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use, copy,
 * modify, merge, publish, distribute, sublicense, and/or sell copies
 * of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 * ***** END LICENSE BLOCK *****
 */

#include <ctype.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <amqp.h>
#include <amqp_framing.h>
#include <stdint.h>

#include "utils.h"

//这个函数是一个可变参数函数，主要用于日志输出，在此分析一下
void die(const char *fmt, ...) {
  va_list ap;//这个是一个char * 
  va_start(ap, fmt);//此宏是将ap指向省略号"..."前面最后一个变量的地址，此例子为fmt
  vfprintf(stderr, fmt, ap);//此函数是将fmt输出按照ap参数要求发送到stderr流中，stderr流为系统定义的异常输出流，fmt可以理解为一个字符串，ap可以认为是
							//是fmt字符串中格式化输出的参数，如“aa%d”，则在ap中要有%d所对应的值的，因此我认为ap在调用va_start时指向了fmt，但其实际
							//使用时是从第二个元素开始使用的
  va_end(ap);//释放ap资源
  fprintf(stderr, "\n");//输出stderr流信息
  exit(1);
}

void die_on_error(int x, char const *context) {
  if (x < 0) {
    fprintf(stderr, "%s: %s\n", context, amqp_error_string2(x));
    exit(1);
  }
}

//新增函数，解决die_on_error出问题直接退出的逻辑
void die_on_error_ex(int x, char const *context) {
	if (x < 0) {
		fprintf(stderr, "%s: %s\n", context, amqp_error_string2(x));
	}
}

int die_on_amqp_error(amqp_rpc_reply_t x, char const *context) {
	char * s_buf[100] = { 0 };
  switch (x.reply_type) {
    case AMQP_RESPONSE_NORMAL:
		return AMQP_FUN_SUCCESS;

    case AMQP_RESPONSE_NONE:
		sprintf(s_buf, "%s: missing RPC reply type", context);
		LogWrite(ERR, "%s", s_buf);
      //fprintf(stderr, "%s: missing RPC reply type!\n", context);
      break;

    case AMQP_RESPONSE_LIBRARY_EXCEPTION:
		sprintf(s_buf, "%s: %s", context, amqp_error_string2(x.library_error));
		LogWrite(ERR, "%s", s_buf);
		//fprintf(stderr, "%s: %s\n", context, amqp_error_string2(x.library_error));
      break;

    case AMQP_RESPONSE_SERVER_EXCEPTION:
      switch (x.reply.id) {
        case AMQP_CONNECTION_CLOSE_METHOD: {
          amqp_connection_close_t *m =
              (amqp_connection_close_t *)x.reply.decoded;
		  sprintf(s_buf, "%s: server connection error %uh, message: %.*s", context, m->reply_code, (int)m->reply_text.len,(char *)m->reply_text.bytes);
		  LogWrite(ERR, "%s", s_buf);
		  //fprintf(stderr, "%s: server connection error %uh, message: %.*s\n",context, m->reply_code, (int)m->reply_text.len,(char *)m->reply_text.bytes);
          break;
        }
        case AMQP_CHANNEL_CLOSE_METHOD: {
          amqp_channel_close_t *m = (amqp_channel_close_t *)x.reply.decoded;

		  sprintf(s_buf, "%s: server channel error %uh, message: %.*s", context, m->reply_code, (int)m->reply_text.len,(char *)m->reply_text.bytes);
		  LogWrite(ERR, "%s", s_buf);
          //fprintf(stderr, "%s: server channel error %uh, message: %.*s\n",context, m->reply_code, (int)m->reply_text.len,(char *)m->reply_text.bytes);
          break;
        }
        default:
		  sprintf(s_buf, "%s: unknown server error, method id 0x%08X\n", context, x.reply.id);
		  LogWrite(ERR, "%s", s_buf);
          //fprintf(stderr, "%s: unknown server error, method id 0x%08X\n",context, x.reply.id);
          break;
      }
      break;
  }
  return AMQP_FUN_FAILURE;
  //exit(1);
}

static void dump_row(long count, int numinrow, int *chs) {
  int i;

  printf("%08lX:", count - numinrow);

  if (numinrow > 0) {
    for (i = 0; i < numinrow; i++) {
      if (i == 8) {
        printf(" :");
      }
      printf(" %02X", chs[i]);
    }
    for (i = numinrow; i < 16; i++) {
      if (i == 8) {
        printf(" :");
      }
      printf("   ");
    }
    printf("  ");
    for (i = 0; i < numinrow; i++) {
      if (isprint(chs[i])) {
        printf("%c", chs[i]);
      } else {
        printf(".");
      }
    }
  }
  printf("\n");
}

static int rows_eq(int *a, int *b) {
  int i;

  for (i = 0; i < 16; i++)
    if (a[i] != b[i]) {
      return 0;
    }

  return 1;
}

void amqp_dump(void const *buffer, size_t len) {
  unsigned char *buf = (unsigned char *)buffer;
  long count = 0;
  int numinrow = 0;
  int chs[16];
  int oldchs[16] = {0};
  int showed_dots = 0;
  size_t i;

  for (i = 0; i < len; i++) {
    int ch = buf[i];

    if (numinrow == 16) {
      int j;

      if (rows_eq(oldchs, chs)) {
        if (!showed_dots) {
          showed_dots = 1;
          printf(
              "          .. .. .. .. .. .. .. .. : .. .. .. .. .. .. .. ..\n");
        }
      } else {
        showed_dots = 0;
        dump_row(count, numinrow, chs);
      }

      for (j = 0; j < 16; j++) {
        oldchs[j] = chs[j];
      }

      numinrow = 0;
    }

    count++;
    chs[numinrow++] = ch;
  }

  dump_row(count, numinrow, chs);

  if (numinrow != 0) {
    printf("%08lX:\n", count);
  }
}
