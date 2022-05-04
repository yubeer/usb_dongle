/*	Copyright (c) 2020-12 Designed By WanZiLian@Nanjing.Jiangsu
 *	All rights reseverd.
 *
 *	Function:	serial.c
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
#include	<stdio.h>
#include	<stdlib.h>
#include	<stdint.h>
#include	<sys/select.h>
#include	<termios.h>
#include	<unistd.h>
#include	<sys/time.h>
#include	<string.h>
#include	<sys/types.h>
#include	<fcntl.h>
#include	<stddef.h>
#include	<malloc.h>
#include	<errno.h>
#include	<syslog.h>
#include	"serial.h"

static	struct termios serialTermios;

int serial_init_handler( struct SerialTypeDef *p );
static int serial_scan_unlock(struct SerialTypeDef *p);
static int serial_free( struct SerialTypeDef *p );
static void serial_flush( struct SerialTypeDef *p , int value );
static int serial_rateConfig( uint32_t *baudrate );
static void serial_parityConfig( char *parity );
static void serial_dataBitConfig( uint8_t *dataBit );
static void serial_stopBitConfig( uint8_t *stopBit );
static int serial_rateChange( struct SerialTypeDef *p , uint32_t baudrate );
static int serial_timeConfig( struct SerialTypeDef *p , struct timeval *tv );
static int serial_rawLocalConfig( struct SerialTypeDef *p );
static int serial_selfConfig( struct SerialTypeDef *p );
static int serial_config( struct SerialTypeDef *p );
static int serial_open( struct SerialTypeDef *p );
static int serial_close( struct SerialTypeDef *p );
static ssize_t serial_write( struct SerialTypeDef *p, uint8_t *pdata, int length );
static ssize_t serial_read( struct SerialTypeDef *p, uint8_t *pdata, int length );
static int serial_ctrl( struct SerialTypeDef *serial );

int serial_init_handler( struct SerialTypeDef *p )
{
	p->enable_syslog = 0;

	p->enable_local_log = 0;

	p->devName = ( char *)malloc( sizeof( char ) * SERIAL_DEVICE_NAME_MAX_SIZE );

	if( !p->devName )
		return -1;

	p->tv = ( struct timeval *)malloc( sizeof( struct timeval ) );

	if( !p->tv)
		return -1;

	memset( p->devName,0,sizeof(char) * SERIAL_DEVICE_NAME_MAX_SIZE );

	p->baudrate = SERIAL_BAUDRATE_DEFAULT_VALUE;

	p->parity = 'N';

	p->dataBit= 8;

	p->stopBit= 1;

	#if	SERIAL_NONBLOCK_ENABLE
	p->tv->tv_sec =  SERIAL_NONBLOCK_TIMEVAL_SEC_DEFAULT;
	p->tv->tv_usec = SERIAL_NONBLOCK_TIMEVAL_USEC_DEFAULT;
	#else
	p->tv->tv_sec =  0;
	p->tv->tv_usec = 0;	
	#endif

	p->fd = -1;

	p->status = SERIAL_STATUS_INIT;

	p->init = NULL;

	p->open = serial_open;

	p->close = serial_close;

	p->unlock  = serial_scan_unlock;

	p->read = serial_read;

	p->write = serial_write;

	p->ctrl = serial_ctrl;

	p->process = NULL;

	p->except = NULL;

	p->config = serial_config;

	p->rate_change = serial_rateChange; 

	p->timeval_config = serial_timeConfig; 

	p->flush = serial_flush; 

	p->free = serial_free; 

	return 0;
}

static int serial_scan_unlock(struct SerialTypeDef *p)
{
	char *path;

	if( p->devName != NULL )
	{
		path = (char *)malloc(sizeof(char)*(32 + strlen(p->devName)));
		memset( path, 0, sizeof(char)*(32 + strlen(p->devName)) );
		memcpy( path,SERIAL_LOCK_PATH_DEFAULT,strlen(SERIAL_LOCK_PATH_DEFAULT));
		memcpy( path+strlen(path), p->devName + strlen("/dev/"), strlen(p->devName) - strlen("/dev/")) ;

		#ifdef DEBUG
		printf("【%s】Serial unlock path=%s!\n",PROGRAM_NAME,path);
		#endif

		#if SERIAL_SYSLOG_ENABLE
			syslog( LOG_INFO,"【%s】Serial unlock path=%s!\n", PROGRAM_NAME,path);
		#endif
		
		if( access( path , F_OK ) < 0 )
		{
			free(path);
			return 0;
		}
		else
		{
			if(	remove( path ) < 0 )
			{
				free(path);
				#ifdef DEBUG
					printf("【%s】Serial unlock %s, need sudo,errno=%d\n", PROGRAM_NAME,path,errno);
				#endif
				#if SERIAL_SYSLOG_ENABLE
				syslog( LOG_INFO,"【%s】Serial unlock %s, need sudo,errno=%d\n", PROGRAM_NAME,path,errno);
				#endif
				return -1;
			}
			else
			{
				free(path);
				return 0;
			}
		}
	}
	return 0;
}

static int serial_free( struct SerialTypeDef *p )
{
	p->openFlag = 0;
	close(p->fd);
	return 0;
}

static void serial_flush( struct SerialTypeDef *p , int value )
{
    tcflush(p->fd, value);
}

static int serial_rateConfig( uint32_t *baudrate )
{
	int ret = -1;

	switch( *baudrate )
	{
		case 2000000:
			ret = cfsetispeed( &serialTermios, B2000000);
			ret = cfsetospeed( &serialTermios, B2000000);
			break;
		case 1500000:
			ret = cfsetispeed( &serialTermios, B1500000);
			ret = cfsetospeed( &serialTermios, B1500000);
			break;
		case 1000000:
			ret = cfsetispeed( &serialTermios, B1000000);
			ret = cfsetospeed( &serialTermios, B1000000);
			break;
		case 921600:
			ret = cfsetispeed( &serialTermios, B921600);
			ret = cfsetospeed( &serialTermios, B921600);
			break;
		case 576000:
			ret = cfsetispeed( &serialTermios, B576000);
			ret = cfsetospeed( &serialTermios, B576000);
			break;
		case 500000:
			ret = cfsetispeed( &serialTermios, B500000);
			ret = cfsetospeed( &serialTermios, B500000);
			break;
		case 460800:
			ret = cfsetispeed( &serialTermios, B460800);
			ret = cfsetospeed( &serialTermios, B460800);
			break;
		// case 256000:
		// 	ret = cfsetispeed( &serialTermios, B256000);
		// 	ret = cfsetospeed( &serialTermios, B256000);
		// 	break;
		case 230400:
			ret = cfsetispeed( &serialTermios, B230400);
			ret = cfsetospeed( &serialTermios, B230400);
			break;
		// case 128000:
		// 	ret = cfsetispeed( &serialTermios, B128000);
		// 	ret = cfsetospeed( &serialTermios, B128000);
		// 	break;
		case 115200:
			ret = cfsetispeed( &serialTermios, B115200);
			ret = cfsetospeed( &serialTermios, B115200);
			break;

		case 57600:
			ret = cfsetispeed( &serialTermios, B57600);
			ret = cfsetospeed( &serialTermios, B57600);
			break;

		case 38400:
			ret = cfsetispeed( &serialTermios, B38400);
			ret = cfsetospeed( &serialTermios, B38400);
			break;

		case 19200:
			ret = cfsetispeed( &serialTermios, B19200);
			ret = cfsetospeed( &serialTermios, B19200);
			break;

		case 9600:
			ret = cfsetispeed( &serialTermios, B9600);
			ret = cfsetospeed( &serialTermios, B9600);
			break;
		case 4800:
			ret = cfsetispeed( &serialTermios, B4800);
			ret = cfsetospeed( &serialTermios, B4800);
			break;
		case 2400:
			ret = cfsetispeed( &serialTermios, B2400);
			ret = cfsetospeed( &serialTermios, B2400);
			break;
		case 1200:
			ret = cfsetispeed( &serialTermios, B1200);
			ret = cfsetospeed( &serialTermios, B1200);
			break;
		default:
			ret = cfsetispeed( &serialTermios, B115200);
			ret = cfsetospeed( &serialTermios, B115200);
			break;
	}
	return ret;
}


static void serial_parityConfig( char *parity )
{
	switch( *parity)
	{
		case 'n':
		case 'N':
			serialTermios.c_cflag &= ~PARENB;
			serialTermios.c_cflag &= ~INPCK;
			break;

		case 'e':
		case 'E':
			serialTermios.c_cflag |= PARENB;
			serialTermios.c_cflag &= ~PARODD;
			serialTermios.c_iflag |= INPCK | ISTRIP;
			break;

		case 'o':
		case 'O':
			serialTermios.c_cflag |= PARENB;
			serialTermios.c_cflag |= PARODD;
			serialTermios.c_iflag |= INPCK | ISTRIP;
			break;

		default:
			serialTermios.c_cflag &= ~PARENB;
			break;
	}
}


static void serial_dataBitConfig( uint8_t *dataBit )
{
	switch( *dataBit )
	{
		case 5:
			serialTermios.c_cflag &= ~CSIZE;
			serialTermios.c_cflag |= CS5;
			break;

		case 6:
			serialTermios.c_cflag &= ~CSIZE;
			serialTermios.c_cflag |= CS6;
			break;

		case 7:
			serialTermios.c_cflag &= ~CSIZE;
			serialTermios.c_cflag |= CS7;
			break;

		case 8:
			serialTermios.c_cflag &= ~CSIZE;
			serialTermios.c_cflag |= CS8;
			break;
		default:
			serialTermios.c_cflag &= ~CSIZE;
			serialTermios.c_cflag |= CS8;
			break;
	
	}
}

static void serial_stopBitConfig( uint8_t *stopBit )
{
	switch( *stopBit )
	{
		case 1:
			serialTermios.c_cflag &= ~CSTOPB;
			break;

		case 2:
			serialTermios.c_cflag |= CSTOPB;
			break;

		default:
			serialTermios.c_cflag &= ~CSTOPB;
			break;
	
	}
}

static int serial_rateChange( struct SerialTypeDef *p , uint32_t baudrate )
{
	int ret = -1;
	if( p->fd < 0 )
		return ret;
	ret = serial_rateConfig( &baudrate );
	ret = tcflush( p->fd,TCIOFLUSH );
	ret = tcsetattr( p->fd ,TCSAFLUSH ,&serialTermios );
	return ret;
}

static int serial_timeConfig( struct SerialTypeDef *p , struct timeval *tv )
{
	p->tv->tv_sec = tv->tv_sec;
	p->tv->tv_usec = tv->tv_usec;
	return 0;
}

static int serial_rawLocalConfig( struct SerialTypeDef *p )
{
	int ret = -1;

	if ( ( ret = tcgetattr( p->fd, &serialTermios ) ) < 0 )
		return -1;

	bzero( &serialTermios,sizeof( serialTermios ) );	

	cfmakeraw( &serialTermios );

	if( ( ret = cfsetspeed( &serialTermios, p->baudrate ) ) < 0 )
		return -1;

	if ( ( ret = tcflush( p->fd,TCIOFLUSH) ) < 0 )
		return -1;

	if ( ( ret = tcsetattr( p->fd, TCSAFLUSH ,&serialTermios ) < 0 ) )
		return -1;

	return 0;
}

static int serial_selfConfig( struct SerialTypeDef *p )
{
	int ret = -1;

	if ( ( ret = tcgetattr( p->fd, &serialTermios ) ) < 0 )
	{
		return -1;
	}

	bzero( &serialTermios,sizeof( serialTermios ) );	

	if ( (ret = serial_rateConfig( &p->baudrate )) >= 0  )	
	{
	    serial_parityConfig( &p->parity );
	    serial_dataBitConfig( &p->dataBit );	
	    serial_stopBitConfig( &p->stopBit ) ;

		/*Flow*/

		serialTermios.c_cflag &= ~CRTSCTS;

		serialTermios.c_oflag &= ~OPOST;  //Output   

		serialTermios.c_cflag |= ( CLOCAL | CREAD );

		serialTermios.c_iflag &= ~( IGNBRK | BRKINT | PARMRK | INLCR | IGNCR | ICRNL | IXON | IXOFF );     

		serialTermios.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);  //Input    

		#ifndef	SERIAL_NONBLOCK_ENABLE
		/*延时100ms*/
		serialTermios.c_cc[VMIN] =  SERIAL_READ_VMIN;     
		serialTermios.c_cc[VTIME] = SERIAL_READ_VTIME;     
		#else
		serialTermios.c_cc[VMIN] = 0;     
		serialTermios.c_cc[VTIME] = 0;     
		#endif

		ret = tcsetattr( p->fd, TCSANOW, &serialTermios ) ;

		tcflush( p->fd ,TCIOFLUSH );
	}
	else
	{
		#ifdef	DEBUG
		printf("【%s】Serial %s config failure!\n", PROGRAM_NAME,p->devName);
		#endif
		#if SERIAL_SYSLOG_ENABLE
		syslog( LOG_INFO,"【%s】Serial %s config failuer!\n", PROGRAM_NAME,p->devName);
		#endif
		ret = -1;
	}

	#ifdef	DEBUG
	printf("【%s】Serial %s rate=%d,parity=%c,dataBit=%d,stopBit=%d config sucessfully!\n", PROGRAM_NAME,p->devName,p->baudrate,p->parity,p->dataBit,p->stopBit);
	#endif

	#if SERIAL_SYSLOG_ENABLE
	syslog( LOG_INFO,"【%s】Serial %s rate=%d,parity=%c,dataBit=%d,stopBit=%d config sucessfully!\n", PROGRAM_NAME,p->devName,p->baudrate,p->parity,p->dataBit,p->stopBit);
	#endif
	return 0;
}

static int serial_config( struct SerialTypeDef *p )
{
	int ret = -1;
	#if	SERIAL_RAW_MODE
		ret = serial_rawLocalConfig( p );
	#else
		ret = serial_selfConfig( p );
	#endif

	return ret;
}

static int serial_open( struct SerialTypeDef *p )
{
 	#if	SERIAL_NONBLOCK_ENABLE
 	 p->fd = open( p->devName, O_RDWR | O_NOCTTY | O_NONBLOCK );
 	#else
 	 p->fd = open( p->devName, O_RDWR | O_NOCTTY  );
 	#endif
	 if( p->fd < 0)
	 {
		 ;
		#ifdef	DEBUG
			printf("【%s】Serial %s open failure!\n", PROGRAM_NAME,p->devName);
		#endif
		#if SERIAL_SYSLOG_ENABLE
		syslog( LOG_INFO,"【%s】Serial %s open failure!\n", PROGRAM_NAME,p->devName);
		#endif
	 }
	 return p->fd;
}


static int serial_close( struct SerialTypeDef *p )
{
	p->openFlag = 0;
	return ( close( p->fd ) );
}

static ssize_t serial_write( struct SerialTypeDef *p, uint8_t *pdata, int length )
{
	return  write( p->fd, pdata, length );
}

static ssize_t serial_read( struct SerialTypeDef *p, uint8_t *pdata, int length )
{
	ssize_t ret =  read( p->fd, pdata, length);
	#ifdef DEBUG
	printf("【%s】Serial %s read data is %s\n", PROGRAM_NAME,p->devName,pdata);
	#endif
	return ret;
}

static int serial_ctrl( struct SerialTypeDef *serial )
{
	int ret = -1;

	switch( serial->status )
	{
		case SERIAL_STATUS_INIT:
			if ( serial->unlock( serial ) >= 0 )
				serial->status = SERIAL_STATUS_CREATE;
			else
				serial->status = SERIAL_STATUS_ERROR;
			break;

		case SERIAL_STATUS_CREATE:
				/*打开标记*/
			if( serial->open( serial ) >= 0 )
			{
				serial->openFlag = 1;
				serial->status = SERIAL_STATUS_CONFIG;
			}
			else
			{
				serial->openFlag = 0;
				serial->status = SERIAL_STATUS_ERROR;
			}
			break;

		case SERIAL_STATUS_CONFIG:
			if( serial->config( serial ) >= 0 )
			{
				serial->status = SERIAL_STATUS_PROCESS;
			}
			else
			{
				serial->openFlag = 0;
				serial->status = SERIAL_STATUS_ERROR;
			}
			break;

		case SERIAL_STATUS_PROCESS:
			if( serial->process( serial ) < 0 )
				serial->status = SERIAL_STATUS_ERROR;
			else
			{
				serial->status = SERIAL_STATUS_INIT;
				ret = 0;
			}
			break;

		case SERIAL_STATUS_ERROR:
				ret = serial->except( serial );
			break;

		default:
			serial->status = SERIAL_STATUS_INIT;	
			break;
	}

	return ret;
}



