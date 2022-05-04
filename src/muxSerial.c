/**
 * Copyright (c) 2021-10 Designed By WanZiLian@Nanjing.Jiangsu
 *	All rights reseverd.
 *
 *  FielName:	muxSerial.c
 *
 *	Function:	
 *
 *	Company:	WanZiLian Technical Corporation
 *
 *	Author:		yubeer Electrical Engineer <yubeer@163.com>
 *
 *	Version:1.0.0
 *
 *	Create Date:2021-10-14
 *
 */
#include	"muxSerial.h"
#include	"serial.h"
#include	"buffer.h"


static int mux_serial_init( struct MuxSerialTypeDef *mstd );
static int mux_serial_read( struct MuxSerialTypeDef *mstd );
static int mux_serial_write( struct MuxSerialTypeDef *mstd );

static int serial_process_handler( struct SerialTypeDef *serial);

static int serial_except_handler( struct SerialTypeDef *serial);

#if		SERIAL_NONBLOCK_ENABLE
/*非阻塞时使用*/
static struct sigaction serial_act;

static int serial_send_flag = 0;

struct itimerval serial_itv;

void serial_recv_timer_set( int time )
{
	serial_itv.it_interval.tv_sec = 0;

	serial_itv.it_interval.tv_usec = 0;

	serial_itv.it_value.tv_sec = 0;

	serial_itv.it_value.tv_usec = time*1000;

	setitimer(ITIMER_REAL,&serial_itv,NULL);
}

void serial_expired_handler( int signo ,siginfo_t *p, void *args )
{
	serial_send_flag = 1;

	serial_recv_timer_set(0);
}

void serial_sigaction_init()
{
	serial_act.sa_sigaction = serial_expired_handler;

	sigemptyset(&serial_act.sa_mask);

	//serial_act.sa_flags = SA_SIGINFO|SA_RESTART;
	/*在打断系统调用后恢复原位置*/
	serial_act.sa_flags = SA_RESTART;

	serial_itv.it_interval.tv_sec = 0;

	serial_itv.it_interval.tv_usec = 0;

	serial_itv.it_value.tv_sec = 0;

	serial_itv.it_value.tv_usec = MUX_SERIAL_RECV_EXPIRED_SIZE;

	sigaction( SIGALRM, &serial_act, NULL );

	serial_send_flag = 0;
}
#endif

int mux_serial_init_handler( struct MuxSerialTypeDef *mstd )
{
	mstd->init = mux_serial_init;
	mstd->read = mux_serial_read;
	mstd->write = mux_serial_write;
	return 0;
}

int mux_serial_init( struct MuxSerialTypeDef *mstd )
{
	mstd->serial->except = serial_except_handler;

	mstd->serial->process = serial_process_handler;

	return 0;
}

/*串口数据处理*/
static int serial_process_handler( struct SerialTypeDef *serial )
{
	return 0;
}

/*串口多路复用读取阻塞超时*/
int mux_serial_read( struct MuxSerialTypeDef *mstd )
{
	struct SerialTypeDef *serial = mstd->serial;
	struct sbuf  *serial_buf = mstd->serial_recv_buf;
	int count = -1;
	/*存储数据*/
	for(;;)
	{
		if( ( count = read( serial->fd, serial_buf->buf+serial_buf->cursize, serial_buf->maxsize - serial_buf->cursize )) > 0 )
		{
			serial_buf->cursize += count;
		}
		else
		{
			#ifdef _DEBUG
			printf("【%s】serial read errno=%d\n",PROGRAM_NAME,errno);
			#endif
			break;
		}
	}
	return count;
}

int mux_serial_write( struct MuxSerialTypeDef *mstd )
{
	struct SerialTypeDef *serial = mstd->serial;
	struct sbuf *buf = mstd->serial_send_buf;
	
	if ( write( serial->fd, buf->buf, buf->cursize) < 0 )
	{
		if( errno != EAGAIN || errno != EINTR )
		{
			serial->openFlag = 0;
			return -1;
		}
	}
	buf->cursize = 0;
	buffer_flush( buf );
	return 0;
}

/*串口异常处理*/
static int serial_except_handler( struct SerialTypeDef *serial )
{
	if( serial->openFlag == 0 )
	{
		serial->free( serial );
		serial->status = SERIAL_STATUS_INIT;
	}
	return 0;
}

