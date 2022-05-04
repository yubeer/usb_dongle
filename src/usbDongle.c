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
#include		<stdint.h>
#include		<stdio.h>
#include		<stdbool.h>
#include		<stdlib.h>
#include		<sys/types.h>
#include		<unistd.h>
#include		<time.h>
#include		<string.h>
#include		<errno.h>
#include		"pthread.h"
#include		"config.h"
#include		"readConfig.h"
#include		"serial.h"
#include		"muxSerial.h"
#include		"muxUtils.h"
#include		"buffer.h"
#include		"readConfig.h"
#include		"usbDongleSearch.h"
#include		"dongleAtProcess.h"
#include		"log.h"
#include		"semaphore.h"

#define		DONGLE_UCI_ENABLE						APP_CONFIG_UCI_ENABLE
#define		MAX_FILE_FD								APP_CRONTRAB_MAX_FILE_FD
#define		DONGLE_AIR_MODE_TIME_SEC				APP_DONGLE_AIR_MODE_TIME_SEC
#define		DONGLE_USB_SEARCH_CYCLE					APP_DONGLE_SEARCH_CYCLE
#define		DONGLE_SOFT_RESET_TIME_SEC				APP_DONGLE_SOFT_RESET_TIME_SEC
#define		DONGLE_SEND_WAIT_TIME					APP_DONGLE_SEND_WAIT_TIME
#define		DONGLE_SERIAL_RECEIVE_BUF_SIZE			APP_DONGLE_SERIAL_RECEIVE_BUF_SIZE
#define		DONGLE_SERIAL_SEND_BUF_SIZE				APP_DONGLE_SERIAL_SEND_BUF_SIZE
#define		DONGLE_SERIAL_SELECT_TIME_SEC			APP_DONLEG_SERIAL_SELECT_TIME_SEC
#define		DONGLE_SERIAL_SELECT_TIME_USEC			APP_DONLEG_SERIAL_SELECT_TIME_USEC
#define		DONGLE_HARD_RESET()						
#define		SYSTEM_REBOOT()							do { FILE *stream = popen("/sbin/reboot","w");pclose(stream);} while(0)

#if	DONGLE_UCI_ENABLE
#include		"uciProcess.h"
static		char *uci_ptr = NULL;
#endif


int			errno;
static		char temp[64];
static		const char *manufacture[4] = {"HUAWEI","GOSUN","ZTE","OTHER"}; 

/**
 * @brief According to /etc/config/file->option - rebootTime , to reboot system when usb dongle access failure extend many times
 * 		rebootTime = 0		function disable reboot_enable =0
 * 		rebootTime != 0 	function enable reboot_enable = 1
 */
static      int reboot_enable = 0;


typedef	struct  
{
	FactoryType					factoryType;
	DongleTypeDef				*dtd;
	struct MuxSerialTypeDef		 *mstd;
	struct LogTypeDef 			*ltd;
}UsbDongleTypeDef;

sem_t sem_dongle_check;
sem_t sem_dongle_at_recv;
sem_t sem_dongle_rst_check;
volatile	sig_atomic_t _alarmFlag;
volatile	sig_atomic_t _runFlag;

void *task_dongle_check( void *args );
void *task_dongle_at_recv( void *args );
void *task_dongle_diag_process( void *args );
bool dongle_at_command_process( void *args, char *cmd );
void dongle_access_process(	void *args );
void dongle_query_process(	void *args );
void dongle_release_process	( void *args );
void dongle_error_process(	void *args );

/*when shell input kill pit_t for this fork,it stop*/
void sigterm_handler(int arg)
{
	_runFlag = -1;
}

/*when shell input kill pit_t for this fork,it stop*/
void sigalarm_handler(int arg)
{
	_alarmFlag = -1;
}

/*配置文件解析*/
char *conf_value_get_by_key( char *path,  char* value)
{
        memset(temp, 0, sizeof(char) * 64);
        rc_ctx_init();
        rc_ctx_config( path, value, temp);
        rc_ctx_destroy();
        return temp;
}


static int dongle_conf_load( UsbDongleTypeDef *udtd, char *path )
{
	char *ptr = NULL;
	char *pvalue = (char *)malloc(sizeof(char)*128);

	if( path == NULL )
	{
		memcpy(pvalue,APP_CONFIG_FILE_PATH_PREFIX,strlen(APP_CONFIG_FILE_PATH_PREFIX));
		strcat(pvalue,APP_CONFIG_FILE_NAME);
	}

	#ifdef	DEBUG
		printf("【%s】dongle config path is %s\n",PROGRAM_NAME, pvalue);
	#endif

	udtd->ltd->enable = APP_LOG_ENABLE;

	udtd->ltd->local_enable = atoi(conf_value_get_by_key(pvalue,"log_local"));

	udtd->ltd->syslog_enable = atoi(conf_value_get_by_key(pvalue,"log_syslog"));

	#ifdef	DEBUG
		printf("【%s】config file read log_local = %d , log_syslog = %d \n",PROGRAM_NAME, udtd->ltd->local_enable , udtd->ltd->syslog_enable );
	#endif

	free(pvalue);

	return 0;
}

void *task_dongle_check( void *args )
{
	UsbDongleTypeDef *udtd = args;
	struct LogTypeDef *ltd = udtd->ltd;

	while(1)
	{
dongle_search_label:
		if( deviceSearch( &udtd->factoryType ) == 0 )
		{
			if ( atInterfaceSearchByFactoryProperty( udtd->mstd->serial->devName, &udtd->factoryType ) == 0 )
			{
				ltd->write(ltd,"【%s】dongle searched device name = %s,factory is %s!\n",PROGRAM_NAME,udtd->mstd->serial->devName,manufacture[udtd->factoryType]);
				/*uci-write factory info*/
				#if DONGLE_UCI_ENABLE
				uci_ptr = "uci -q get usb_dongle.global.dev";
				if ( uci_get_by_popen(uci_ptr,temp) == 0 )
				{
					if( strncmp(temp,udtd->mstd->serial->devName,strlen(udtd->mstd->serial->devName)) != 0 )	
					{
						uci_ptr = "uci -q set usb_dongle.global.dev=\"";
						uci_set_by_popen(uci_ptr,"uci -q commit usb_dongle",udtd->mstd->serial->devName);
					}
				}
				#endif
				
				if( udtd->factoryType == _GOSUN )
				{
					sleep( DONGLE_USB_SEARCH_CYCLE * 2 );
				}

				memset(temp,0,sizeof(char)*64);
				#ifdef   DEBUG
				printf("【%s】sem_dongle_check post!\n",PROGRAM_NAME);
				#endif
				sem_post( &sem_dongle_check );
				sleep( DONGLE_USB_SEARCH_CYCLE );
				if ( sem_wait( &sem_dongle_rst_check ) == 0 )
				{
					ltd->write(ltd,"【%s】dongle broken or pluged , rst check.!\n",PROGRAM_NAME);
					goto dongle_search_label;
				}
			}
			else
				sleep( DONGLE_USB_SEARCH_CYCLE );
		}
		else
			sleep( DONGLE_USB_SEARCH_CYCLE );
	}
}

void *task_dongle_at_recv( void *args )
{
	UsbDongleTypeDef *udtd = args;
   	struct MuxTypeDef *mux = udtd->mstd->mux;
	struct SerialTypeDef *serial = udtd->mstd->serial;
	struct sbuf *buf = udtd->mstd->serial_recv_buf;
	struct LogTypeDef *ltd = udtd->ltd;
	uint8_t ret = 0;
	uint8_t sret = 0;
	int count = 0;
	struct timeval tvs;
	for(;;)
	{
		if ( sem_wait(&sem_dongle_at_recv) == 0 )
		{
			ltd->write(ltd,"【%s】dongle task receive is running!\n",PROGRAM_NAME);
			ret = 1;
			mux_utils_init( mux );
 			mux_utils_del( mux, serial->fd, MUX_READ );
			while(ret)
			{
				tvs.tv_sec = serial->tv->tv_sec;
				tvs.tv_usec = serial->tv->tv_usec;
				mux_utils_add( mux, serial->fd, MUX_READ );
				if ( ( sret = mux_utils_wait( mux ,serial->fd + 1 , &tvs , MUX_READ ) )  > 0 )
				{
					if( mux_utils_isset( mux, serial->fd, MUX_READ ) )
					{
						if( (count = udtd->mstd->read(udtd->mstd)) <= 0 )
						{
							/**
							 * @brief receive bad fd or eio , restart device search
							 * 
							 */
							if( errno == EBADF || errno == EIO )
							{
								ltd->write(ltd,"【%s】dongle task receive errno=%d!\n",PROGRAM_NAME,errno);
								serial->openFlag = 0;
								ret = 0;
								dongle_error_process(udtd);
							}
							if( buf->cursize > 0 )
							{
								#ifdef DEBUG
								printf("【%s】dongle recv %s",PROGRAM_NAME,buf->buf);
								#endif
								if( buf->cursize >= buf->maxsize )
									buf->cursize = 0;
								ret = 0; 
								ltd->write(ltd,"【%s】dongle task receive data is =%s!\n",PROGRAM_NAME,buf->buf);
							}
						}
					}
					mux_utils_del( mux, serial->fd, MUX_READ );
				}
				else
				{
					if( sret == 0 )
					{
						ltd->write(ltd,"【%s】dongle task receive select expired!\n",PROGRAM_NAME);
						buffer_flush(buf);
					}
					if( errno == EBADF  || errno == EIO)
					{
						ltd->write(ltd,"【%s】dongle task receive errno=%d!\n",PROGRAM_NAME,errno);
						serial->openFlag = 0;
						ret = 0;
						dongle_error_process(udtd);
					}
				}
			}
		}
	}
}

void *task_dongle_diag_process( void *args )
{
	UsbDongleTypeDef *udtd = args;
	struct SerialTypeDef *serial = udtd->mstd->serial;
	DongleTypeDef *dtd = udtd->dtd;
	struct LogTypeDef *ltd = udtd->ltd;
	dtd->_stage = AT_STAGE_PREPARE;

	while(1)
	{
		switch( dtd->_stage)
		{
			/* initial */
			case AT_STAGE_PREPARE:
				if ( sem_wait(&sem_dongle_check) == 0 )
				{
					dtd->_stage = AT_STAGE_INITIAL;
					#ifdef DEBUG
						printf("【%s】usb dongle diag stage initial!\n",PROGRAM_NAME);
					#endif
					ltd->write(ltd,"【%s】dongle diag stage initial!\n",PROGRAM_NAME);
				}
				break;

			case AT_STAGE_INITIAL:
			 	if( serial->ctrl(serial) >= 0 /*&& serial->openFlag == 0*/ )
				{
					serial->tv->tv_sec = DONGLE_SERIAL_SELECT_TIME_SEC;
					serial->tv->tv_usec = DONGLE_SERIAL_SELECT_TIME_USEC;
					dtd->_stage = AT_STAGE_ACCESS;
					dtd->_cmd = _ATE;
					#ifdef DEBUG
						printf("【%s】usb dongle diag config okay!\n",PROGRAM_NAME);
					#endif
					ltd->write(ltd,"【%s】dongle diag stage config okay!\n",PROGRAM_NAME);
					if( udtd->factoryType == _GOSUN )
					{
						sleep( 2 * DONGLE_SEND_WAIT_TIME );
					}
				}
				break;

			case AT_STAGE_ACCESS:
				ltd->write(ltd,"【%s】dongle diag stage access ...!\n",PROGRAM_NAME);
				dongle_access_process( args );
				break;
		
			case AT_STAGE_QUERY:
				ltd->write(ltd,"【%s】dongle diag stage query ...!\n",PROGRAM_NAME);
				dongle_query_process( args );
				break;
			/*erro code*/

			case AT_STAGE_RELEASE:
				ltd->write(ltd,"【%s】dongle diag stage release ...!\n",PROGRAM_NAME);
				dongle_release_process( args );
				break;

			case AT_STAGE_ERROR:
				ltd->write(ltd,"【%s】dongle diag stage error process ...!\n",PROGRAM_NAME);
				dongle_error_process( args );
				break;

			default:
				dtd->_stage = AT_STAGE_INITIAL;
				break;
		}
	}
}



int atCmdProcess( void *args, char *cmd )
{
	int ret = 1;
	UsbDongleTypeDef *udtd = args;
	struct SerialTypeDef *serial = udtd->mstd->serial;
	struct sbuf *rbuf = udtd->mstd->serial_recv_buf;
	struct LogTypeDef *ltd = udtd->ltd;
	DongleTypeDef *dtd = udtd->dtd;
	uint8_t counter = 0;
	uint32_t repeatCount = 0;

	if ( serial->write( serial, (uint8_t *)cmd, strlen( cmd )) > 0 )
	{
		ltd->write(ltd,"【%s】dongle serial send %s !\n",PROGRAM_NAME, cmd);
		#ifdef DEBUG
			printf("【%s】dongle send %s",PROGRAM_NAME,cmd);
		#endif
		sem_post(&sem_dongle_at_recv);
		sleep( DONGLE_SEND_WAIT_TIME );
		goto LABEL_JUDGE;
LABEL_JUDGE:	
		if( rbuf->cursize > 0 )
		{
			if( atCmdReceiveProcess( dtd, (char *)rbuf->buf ) == AT_OK )
				ret = 0;
				
			if( ( strncmp( cmd, "AT+ZECMCALL=1\r\n",strlen("AT+ZECMCALL=1\r\n")) == 0) )
			{
				if( strstr( (char *)rbuf->buf,"CONNECT") != NULL )
					ret = 0;
			}
		}
		else
		{
			if( ( strncmp( cmd,"AT+ZECMCALL=1\r\n",strlen("AT+ZECMCALL=1\r\n")) == 0) )
			{
				if( counter < 9 )	//45 sec
				{
					counter++;
					sleep( 5 * DONGLE_SEND_WAIT_TIME );
					goto LABEL_JUDGE;
				}
				else
					counter = 0;
			}
			else
			{
				sleep( DONGLE_SEND_WAIT_TIME *3 );
				if( dtd->dongleAtFailureStruct.softFailureCount < DONGLE_SOFT_RESET_TIME_SEC)
					dtd->dongleAtFailureStruct.softFailureCount++;
				else
				{
					dtd->dongleAtFailureStruct.softFailureCount = 0;
					DONGLE_HARD_RESET();//hard reset
					sleep(25);
				}
			}
		}
	}
	else
	{
		ltd->write(ltd,"【%s】dongle serial send errno=%d !\n",PROGRAM_NAME, errno);
		switch( errno )
		{
			case EBADF:
				serial->openFlag = 0;
				dtd->_stage = AT_STAGE_ERROR;
				dtd->_cmd = _ATE;
				break;
			case EFAULT:
				dtd->_stage = AT_STAGE_ERROR;
				dtd->_cmd = _ATE;
				break;

			case EIO:
				serial->openFlag = 0;
				dtd->_stage = AT_STAGE_ERROR;
				dtd->_cmd = _ATE;
				DONGLE_HARD_RESET();//hard reset
				sleep(25);
				break;

			case EAGAIN:
				if( dtd->dongleAtFailureStruct.softFailureCount < DONGLE_SOFT_RESET_TIME_SEC)
				{
					dtd->dongleAtFailureStruct.softFailureCount++;
					sleep(2);
				}
				else
				{
					dtd->dongleAtFailureStruct.softFailureCount = 0;
					dtd->_stage = AT_STAGE_ERROR;
					dtd->_cmd = _ATE;
					DONGLE_HARD_RESET();//hard reset
					sleep(25);
				}
			break;
		}
	}
	buffer_flush(rbuf);
	return ret;
}

void dongle_access_process(	void *args )
{
	/* 2021-10-10增加模块脱网后，操作系统复位功能*/
	uint32_t	hardRebootCount = 0;
	UsbDongleTypeDef *udtd = args;
	struct SerialTypeDef *serial = udtd->mstd->serial;
	struct sbuf *rbuf = udtd->mstd->serial_recv_buf;
	DongleTypeDef *dtd = udtd->dtd;
	memset(temp,0,sizeof(char)*64);
	
	while(dtd->_stage == AT_STAGE_ACCESS)
	{
		switch( dtd->_cmd )
		{
			case _ATE:
				printf("send ATE0\n");
				if( atCmdProcess( udtd, "ATE0\r\n" ) == 0 )
					dtd->_cmd = _CURC;
				break;

			case _CURC:
				atCmdProcess( udtd, "AT^CURC=0\r\n" );
				dtd->_cmd = _ATI;
				break;

			case _ATI:
				if ( atCmdProcess( udtd, "ATI\r\n" ) == 0 )
				{
					#if DONGLE_UCI_ENABLE
					uci_ptr = "uci -q get usb_dongle.global.model";
					if ( uci_get_by_popen(uci_ptr,temp) == 0 )
					{
						if( strncmp(temp,dtd->dongleGeneralStruct.model,strlen(dtd->dongleGeneralStruct.model)) != 0 )	
						{
							uci_ptr = "uci -q set usb_dongle.global.model=\"";
							uci_set_by_popen(uci_ptr,"uci -q commit usb_dongle",dtd->dongleGeneralStruct.model);
							uci_ptr = "uci -q set usb_dongle.global.manufacture=\"";
							uci_set_by_popen(uci_ptr,"uci -q commit usb_dongle",dtd->dongleGeneralStruct.factory);
							uci_ptr = "uci -q set usb_dongle.global.revision=\"";
							uci_set_by_popen(uci_ptr,"uci -q commit usb_dongle",dtd->dongleGeneralStruct.ver);

							uci_ptr = "uci -q set usb_dongle.global.imei=\"";
							uci_set_by_popen(uci_ptr,"uci -q commit usb_dongle",dtd->dongleGeneralStruct.imei);
							memset(temp,0,sizeof(char)*64);
						}
					}
					/*2021-10-10 增加脱网重启时间设置功能，0->不重启*/
					uci_ptr = "uci -q get usb_dongle.global.rebootTime";
					if ( uci_get_by_popen(uci_ptr,temp) == 0 )
					{
						dtd->dongleAtFailureStruct.hardFailureCount = atoi(temp);
						if( atoi(temp) == 0 )
							reboot_enable = 0;
						else
							reboot_enable = 1;
						hardRebootCount = atoi(temp)/APP_DONGLE_SEND_WAIT_TIME;
						memset(temp,0,sizeof(char)*64);
					}
					#endif
					dtd->_cmd = _CIMI;
				}
				break;

			case _CIMI:
				if( atCmdProcess( udtd, "AT+CIMI\r\n" ) == 0 )
				{
					#if DONGLE_UCI_ENABLE
					uci_ptr = "uci -q get usb_dongle.global.imsi";
					if ( uci_get_by_popen(uci_ptr,temp) == 0 )
					{
						if( strncmp(temp,(char *)dtd->dongleUsimStruct.imsi,strlen((char *)dtd->dongleUsimStruct.imsi)) != 0 )	
						{
							uci_ptr = "uci -q set usb_dongle.global.imsi=\"";
							uci_set_by_popen(uci_ptr,"uci -q commit usb_dongle",(char *)dtd->dongleUsimStruct.imsi);
							memset(temp,0,sizeof(char)*64);
						}
					}
					#endif
					sleep( DONGLE_SEND_WAIT_TIME * 2);
					if( udtd->factoryType == _GOSUN)
						dtd->_cmd = _SYSINFO;
					else
						dtd->_cmd = _CGREG;
				}
				break;

			case _CGREG:
				if( atCmdProcess( udtd, "AT+CGREG?\r\n" ) == 0 )
				{
					switch( dtd->dongleRegisterStruct.registerStatus )
					{
						case 0:
						case 3:
						case 4:
							/*2021-10-10 增加系统硬件重启功能 (30s-AirSoftCycle+10)*/
							if(	dtd->dongleAtFailureStruct.hardFailureCount != 0 );
							{
								if( reboot_enable ==  1 )
								{
									if( hardRebootCount == 0 )
										SYSTEM_REBOOT();
									else
									{
										hardRebootCount--;
									}
								}
							}

							if( dtd->dongleAtFailureStruct.airFailureCount < DONGLE_AIR_MODE_TIME_SEC )
							{
								dtd->dongleAtFailureStruct.airFailureCount++;

								sleep( DONGLE_SEND_WAIT_TIME * 2);
							}
							else
							{
								dtd->dongleAtFailureStruct.airFailureCount = 0;
								atCmdProcess( udtd, "AT+CFUN=0\r\n" );
								sleep( DONGLE_SEND_WAIT_TIME * 10);
								atCmdProcess( udtd, "AT+CFUN=1\r\n" );
								sleep( DONGLE_SEND_WAIT_TIME * 5);
							}
							break;
						case 1:
						case 5:
							#if DONGLE_UCI_ENABLE
							uci_ptr = "uci -q get usb_dongle.global.cell";
							if ( uci_get_by_popen(uci_ptr,temp) == 0 )
							{
								if( atoi(temp) - dtd->dongleRegisterStruct.cellId != 0 )	
								{
									uci_ptr = "uci -q set usb_dongle.global.cell=\"";
									memset(temp,0,sizeof(char)*64);
									sprintf(temp,"%08x",dtd->dongleRegisterStruct.cellId);
									uci_set_by_popen(uci_ptr,"uci -q commit usb_dongle",temp);
									memset(temp,0,sizeof(char)*64);

									uci_ptr = "uci -q set usb_dongle.global.lac=\"";
									memset(temp,0,sizeof(char)*64);
									sprintf(temp,"%04x",dtd->dongleRegisterStruct.lac);
									uci_set_by_popen(uci_ptr,"uci -q commit usb_dongle",temp);
									memset(temp,0,sizeof(char)*64);

									uci_ptr = "uci -q set usb_dongle.global.band=\"";
									memset(temp,0,sizeof(char)*64);
									sprintf(temp,"%d",dtd->dongleFreqStruct.bandId);
									uci_set_by_popen(uci_ptr,"uci -q commit usb_dongle",temp);
									memset(temp,0,sizeof(char)*64);
								}
							}
							#endif
							 dtd->_cmd = _SYSINFO; 
							break;

						case 2: dtd->_cmd = _SYSINFO; 
							break;

						default:
								dtd->_cmd = _CGREG;
							break;
					}
				}
				break;

			case _NDISDUP:
				 atCmdProcess( udtd, "AT^NDISDUP=1,0\r\n" );
				 sleep( DONGLE_SEND_WAIT_TIME * 2 );

				 atCmdProcess( udtd, "AT^NDISDUP=1,1\r\n" );
				 sleep( DONGLE_SEND_WAIT_TIME * 15 );	//2021-10-10-网络拨号的最长时间为15s
				 if ( udhcpcIpByFactoryType( udtd->factoryType ) == 0 )
				 {
					#if DONGLE_UCI_ENABLE
					uci_ptr = "uci -q set usb_dongle.global.register=\"";
					uci_set_by_popen(uci_ptr,"uci -q commit usb_dongle","attach");
					#endif
					dtd->_stage = AT_STAGE_QUERY;
					dtd->_cmd = _CSQ;
				 }
				 else
				 {
					#if DONGLE_UCI_ENABLE
					uci_ptr = "uci -q set usb_dongle.global.register=\"";
					uci_set_by_popen(uci_ptr,"uci -q commit usb_dongle","unattach");
					#endif
					atCmdProcess( udtd, "AT^NDISDUP=1,0\r\n" );
					dtd->_cmd = _CGREG;
				 }
				break;

			case _SYSINFO:
				if( atCmdProcess( udtd, "AT^SYSINFO\r\n" ) == 0 )
				{
					switch( dtd->dongleSearchStruct.srvStatus)
					{
						case 0:
						case 1:
						case 3:
							if( udtd->factoryType == _GOSUN)
								dtd->_cmd = _SYSINFO;
							else
								dtd->_cmd = _CGREG;
							break;
						case 2:
							switch( udtd->factoryType )
							{
								case _HUAWEI:
									dtd->_cmd = _NDISDUP; 
									break;
								case _GOSUN:
									dtd->_cmd = _ZECMCALL;
									break;
								case _ZTE: 
									dtd->_cmd = _CGACT; 
									break;
								default:
									break;

							}
							break;

						case 4:
							if( dtd->dongleAtFailureStruct.airFailureCount < DONGLE_AIR_MODE_TIME_SEC )
								dtd->dongleAtFailureStruct.airFailureCount++;
							else
							{
								atCmdProcess( udtd, "AT+CFUN=0\r\n" );
								sleep( DONGLE_SEND_WAIT_TIME * 10);
								atCmdProcess( udtd, "AT+CFUN=1\r\n" );
								sleep( DONGLE_SEND_WAIT_TIME * 5);
								if( udtd->factoryType == _GOSUN)
									dtd->_cmd = _SYSINFO;
								else
									dtd->_cmd = _CGREG;
							}
						   break;	
					}
				}
				break;

			case _ZECMCALL:
				if( atCmdProcess( udtd, "AT+ZECMCALL=1\r\n" ) == 0 )
				{
					#if DONGLE_UCI_ENABLE
					uci_ptr = "uci -q set usb_dongle.global.register=\"";
					uci_set_by_popen(uci_ptr,"uci -q commit usb_dongle","attach");
					#endif
					dtd->_stage = AT_STAGE_QUERY;
					dtd->_cmd = _CSQ;
				}
				else
				{
					atCmdProcess( udtd, "AT+CFUN=0\r\n" );
					sleep( DONGLE_SEND_WAIT_TIME * 10);
					atCmdProcess( udtd, "AT+CFUN=1\r\n" );
					sleep( DONGLE_SEND_WAIT_TIME * 5);
					dtd->_cmd = _SYSINFO;
				}
				break;

			case _CGACT:
				;
				break;
		}
	}
}


void dongle_query_process(	void *args )
{
	UsbDongleTypeDef *udtd = args;
	struct SerialTypeDef *serial = udtd->mstd->serial;
	struct sbuf *rbuf = udtd->mstd->serial_recv_buf;
	DongleTypeDef *dtd = udtd->dtd;
	memset(temp,0,sizeof(char)*64);

	while( dtd->_stage == AT_STAGE_QUERY )
	{
		switch( dtd->_cmd )
		{
			case _CSQ:
				if( atCmdProcess( udtd, "AT+CSQ\r\n" ) == 0 )
				{
					memset(temp,0,sizeof(char)*16);
					#if DONGLE_UCI_ENABLE
					uci_ptr = "uci -q set usb_dongle.global.rssi=";
					if ( uci_get_by_popen(uci_ptr,temp) == 0 )
					{
						if( atoi(temp) - dtd->dongleRegisterStruct.rssi != 0 )	
						{
							uci_ptr = "uci -q set usb_dongle.global.rssi=\"";
							memset(temp,0,sizeof(char)*16);
							sprintf(temp,"%i",dtd->dongleRegisterStruct.rssi);
							uci_set_by_popen(uci_ptr,"uci -q commit usb_dongle",temp);
						}
					}
					#endif
					if( udtd->factoryType == _GOSUN)
						dtd->_cmd = _ZPAS;
					else
						dtd->_cmd = _CGREG;
					sleep( DONGLE_SEND_WAIT_TIME * 2 );
				}
				break;

			case _CGREG:
				if( atCmdProcess( udtd, "AT+CGREG?\r\n" ) == 0 )
				{
					switch( dtd->dongleRegisterStruct.registerStatus )
					{
						case 0:
						case 2:
						case 3:
						case 4:
							#if DONGLE_UCI_ENABLE
							uci_ptr = "uci -q set usb_dongle.global.register=\"";
							uci_set_by_popen(uci_ptr,"uci -q commit usb_dongle","unattach");
							#endif
							dtd->_stage = AT_STAGE_RELEASE;
							dtd->_cmd = _ATE;
							break;
						case 1:
						case 5:
							dtd->_cmd = _CSQ;
							break;
						default:
							dtd->_cmd = _CGREG;
							break;
					
					}
				}

				 break;

			case _ZCELLINFO:
				if( atCmdProcess( udtd, "AT+ZCELLINFO?\r\n" ) == 0 )
				{
					#if DONGLE_UCI_ENABLE
					uci_ptr = "uci -q get usb_dongle.global.cell";
					if ( uci_get_by_popen(uci_ptr,temp) == 0 )
					{
						if( atoi(temp) - dtd->dongleRegisterStruct.cellId != 0 )	
						{
							uci_ptr = "uci -q set usb_dongle.global.cell=\"";
							memset(temp,0,sizeof(char)*64);
							sprintf(temp,"%08x",dtd->dongleRegisterStruct.cellId);
							uci_set_by_popen(uci_ptr,"uci -q commit usb_dongle",temp);
							memset(temp,0,sizeof(char)*64);

							uci_ptr = "uci -q set usb_dongle.global.lac=\"";
							memset(temp,0,sizeof(char)*64);
							sprintf(temp,"%04x",dtd->dongleRegisterStruct.lac);
							uci_set_by_popen(uci_ptr,"uci -q commit usb_dongle",temp);
							memset(temp,0,sizeof(char)*64);
						}
					}
					#endif
				}
					dtd->_cmd = _CSQ;
				 break;

			case _ZPAS:
				if( atCmdProcess( udtd, "AT+ZPAS?\r\n" ) == 0 )
				{
					dtd->_cmd = _ZCELLINFO;
				}
				else
					dtd->_cmd = _CSQ;
				 break;

			default:
				  dtd->_cmd = _CSQ;
				  break;
		}
	
	}
}


void dongle_release_process( void *args )
{
	UsbDongleTypeDef *udtd = args;
	DongleTypeDef *dtd = udtd->dtd;
	while( dtd->_stage == AT_STAGE_RELEASE)
	{
		switch( udtd->factoryType )
		{
			case _HUAWEI:
				atCmdProcess( udtd, "AT^NDISDUP=1,0\r\n" ) ;
				sleep( DONGLE_SEND_WAIT_TIME * 2 );
				dtd->_cmd = _ATE;
				break;

			case _ZTE:
				break;

			case _GOSUN:
				if( atCmdProcess( udtd, "AT+ECMCALL=0\r\n" ) == 0 )
				{
					sleep( DONGLE_SEND_WAIT_TIME * 4 );
					dtd->_cmd = _ATE;
				}
				break;

			default:
				break;
		}
		dtd->_stage = AT_STAGE_ACCESS;
	}
}

void dongle_error_process(void *args )
{
	UsbDongleTypeDef *udtd = args;
	DongleTypeDef *dtd = udtd->dtd;
	struct sbuf *rbuf = udtd->mstd->serial_recv_buf;
	struct sbuf *tbuf = udtd->mstd->serial_send_buf;
	struct SerialTypeDef *serial = udtd->mstd->serial;
	struct LogTypeDef *ltd = udtd->ltd;

	if( dtd->_stage == AT_STAGE_ERROR)
	{
		serial->close( serial );
		sleep( DONGLE_USB_SEARCH_CYCLE * 2 );
		dtd->_cmd = _ATE;
		dtd->_stage = AT_STAGE_PREPARE;
		buffer_flush(rbuf);
		buffer_flush(tbuf);
		sleep( DONGLE_USB_SEARCH_CYCLE * 10 );
		#ifdef DEBUG
		printf("【%s】diag process modem\n",PROGRAM_NAME);
		#endif
		/**
		 * @brief start dongle search task
		 * 
		 */
		sem_post(&sem_dongle_rst_check);

		ltd->write(ltd,"【%s】dongle diag error process sem_rst_check send okay !\n",PROGRAM_NAME);
	}
}

int main(int argc,char *argv[])
{
	pid_t				pc;		

	uint16_t			i=0;
	/* check usb device*/
	pthread_t thread_dongle_check;

	/*at-diag*/
	pthread_t thread_dongle_diag_process;

	/*at-receive*/
	pthread_t thread_dongle_at_recv;

	UsbDongleTypeDef udtd;

	/*openwrt use fork emerg!--!
	pc = fork();

	if ( pc < 0 )
	{
		exit(1);
	}
	else if ( pc > 0 )
	{
		exit(0);
	}
	
	setsid();

	chdir("/");

	umask(0);

	for ( i = 0;i<MAX_FILE_FD;i++)
	{
	  close(i);
	}
        */

	memset(&udtd,0,sizeof(UsbDongleTypeDef));

	memset(temp,0,sizeof(char)*64);

	udtd.dtd =  ( DongleTypeDef  * )malloc( sizeof(  DongleTypeDef  ) );

	udtd.ltd = (struct LogTypeDef *)malloc(sizeof(struct LogTypeDef ));
	
	log_handler_init(udtd.ltd);

	udtd.ltd->log_priority = LOG_INFO;

	udtd.mstd = ( struct MuxSerialTypeDef  * )malloc( sizeof( struct MuxSerialTypeDef  ) );

	udtd.mstd->serial = ( struct SerialTypeDef * )malloc( sizeof( struct SerialTypeDef ) );

	udtd.mstd->mux = ( struct MuxTypeDef * )malloc( sizeof( struct MuxTypeDef ) );

	udtd.mstd->serial_recv_buf = ( struct sbuf * )malloc( sizeof( struct sbuf ) );

	udtd.mstd->serial_send_buf = ( struct sbuf * )malloc( sizeof( struct sbuf ) );

	buffer_init( udtd.mstd->serial_recv_buf,NULL,DONGLE_SERIAL_RECEIVE_BUF_SIZE);

	buffer_init( udtd.mstd->serial_send_buf,NULL,DONGLE_SERIAL_SEND_BUF_SIZE);
	
	dongle_init_handler(udtd.dtd);

	serial_init_handler( udtd.mstd->serial );

    mux_serial_init_handler( udtd.mstd );

	udtd.mstd->init(udtd.mstd);

    mux_utils_init( udtd.mstd->mux);

	if( sem_init( &sem_dongle_check,0,0) != 0 )
		return -1;
	if( sem_init( &sem_dongle_at_recv,0,0) != 0 )
		return -1;
	if( sem_init( &sem_dongle_rst_check,0,0) != 0 )
		return -1;

	#if DONGLE_UCI_ENABLE
	uci_ptr = "uci -q set usb_dongle.global.register=\"";
	uci_set_by_popen(uci_ptr,"uci -q commit usb_dongle","unattach");
	#endif
    
	dongle_conf_load( &udtd, NULL );

	if ( pthread_create( &thread_dongle_check, NULL, &task_dongle_check, &udtd ) == 0 )
	{
		pthread_detach( thread_dongle_check );

		if ( pthread_create( &thread_dongle_at_recv, NULL, &task_dongle_at_recv, &udtd ) == 0 )
			pthread_detach( thread_dongle_at_recv);

		if ( pthread_create( &thread_dongle_diag_process, NULL, &task_dongle_diag_process, &udtd ) == 0 )
			pthread_detach( thread_dongle_diag_process);
	}

	pthread_exit(0);

	free(udtd.dtd);

	free(udtd.mstd);

	return 0;
}

