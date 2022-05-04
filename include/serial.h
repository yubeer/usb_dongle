/*	Copyright (c) 2020-12 Designed By WanZiLian@Nanjing.Jiangsu
 *	All rights reseverd.
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
 *	Modify Date:xxxx-xx-xx
 *
 */
#ifndef		__SERIAL_H__
#define		__SERIAL_H__

/*串口异常时，操作系统会将接口锁定，锁定文件默认路径*/
#define		SERIAL_LOCK_PATH_DEFAULT						"/var/lock/LCK.."
#define		SERIAL_BAUDRATE_DEFAULT_VALUE					115200
#define		SERIAL_DEVICE_NAME_MAX_SIZE						16
#define		SERIAL_NONBLOCK_ENABLE							0
#define		SERIAL_SYSLOG_ENABLE							0
#define		SERIAL_RAW_MODE									0

#if	SERIAL_NONBLOCK_ENABLE
	/*多路复用，超时时间*/
	#define	SERIAL_NONBLOCK_TIMEVAL_SEC_DEFAULT				5
	#define	SERIAL_NONBLOCK_TIMEVAL_USEC_DEFAULT	        0
#else
		/*非阻塞,串口读取数据，超时时间*/
	#define	SERIAL_READ_VTIME								5
	#define	SERIAL_READ_VMIN								0
#endif

#include	<stdio.h>
#include	<stdlib.h>     
#include	<stdbool.h>
#include	<unistd.h>     
#include	<sys/types.h>  
#include	<sys/stat.h>   
#include	<string.h>
#include	<fcntl.h>      
#include	<termios.h>   
#include	<errno.h>   
#include	<sys/select.h>
#include	<sys/time.h>
#include	<sys/wait.h>
#include	<stddef.h>
#include	<stdint.h>
#include	<syslog.h>

#ifndef	PROGRAM_NAME
	#define		PROGRAM_NAME		"Serial App"
#endif

typedef	enum
{
	SERIAL_STATUS_INIT = 0,
	SERIAL_STATUS_CREATE,
	SERIAL_STATUS_CONFIG,
	SERIAL_STATUS_PROCESS,
	SERIAL_STATUS_ERROR
}SerialStatus;

/*Serial Structure*/
struct SerialTypeDef{
	uint8_t		enable_syslog:1;
	uint8_t		enable_local_log:1;
	char		*devName;
	int			fd;
	uint32_t	baudrate;
	char		parity;
	uint8_t		dataBit;
	uint8_t		stopBit;
	uint8_t		*txdata;
	uint8_t		*rxdata;
	struct		timeval	*tv;
	uint8_t     openFlag;
    int         hw_fifo_size;

	SerialStatus status;

	int (*init)( struct SerialTypeDef *p );
	int	(*open)( struct SerialTypeDef *p );
	int	(*close)( struct SerialTypeDef *p );
	int	(*unlock)( struct SerialTypeDef *p );
	ssize_t	(*read)( struct SerialTypeDef *p , uint8_t *pdata, int length );
	ssize_t	(*write)( struct SerialTypeDef *p ,uint8_t *pdata, int length );
	int (*except)( struct SerialTypeDef *p );
	int (*ctrl)( struct SerialTypeDef *p );
	int (*process)( struct SerialTypeDef *p );
	int (*config)( struct SerialTypeDef *p );
	int (*rate_change)( struct SerialTypeDef *p, uint32_t rate );
	int (*timeval_config)( struct SerialTypeDef *p, struct timeval *tv );
	void (*flush)( struct SerialTypeDef *p, int value );
	int (*free)( struct SerialTypeDef *p );
};

int serial_init_handler( struct SerialTypeDef *p );

#endif/* SERIAL_H */

