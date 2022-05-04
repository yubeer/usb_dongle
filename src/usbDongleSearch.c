/**
 * Copyright (c) 2021-10 Designed By WanZiLian@Nanjing.Jiangsu

 *	All rights reseverd.
 *
 *  FielName:	usb_dongle_device_check.c
 * 		1. Search key words from file-given. 
 *		2. Check Usb-hotplug event
 *	Function:	Application config file
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
#define			_GNU_SOURCE
#include		<string.h>  
#include		<fcntl.h>  
#include		<sys/socket.h>  
#include		<sys/types.h>  
#include		<linux/netlink.h>  
#include		<sys/stat.h>
#include		<errno.h>
#include		<dirent.h>
#include		<unistd.h>  
#include		<ctype.h>
#include		"usbDongleSearch.h"


#ifdef DEBUG
	#define		DPRINTF(fmt, ...)			fprintf(stdout, "%s(%d): " fmt, __func__, __LINE__, ##__VA_ARGS__)
#else
	#define		DPRINTF(fmt,...)			do{} while(0)

#endif


static	char temp[CMP_KEY_WORdS_MAX_SIZE];
static  char rxCmd[SHELL_POPEN_MAX_SIZE];
/*	Funciton:
 *		Using of lsusb to select usbdevice(pid)
 *	Paramter:		
 *		ModemGeneralTypeDef *mgtd->modem_at_process.h
 *		idProduct
 *		idVendor
 *		mFactory
 *	Return:
 *		false->not existed!
 *		true->existed!
 *
 *	Author:Xinhy
 *
 *	Create Date:2020-12-21
 *
 *	Modify Date:
 *
 */
uint8_t deviceSearch( FactoryType *factoryType )
{
	uint8_t ret = 1;

	FILE *fstream = NULL;

	char *ptr = rxCmd;

	memset( rxCmd, 0, sizeof(rxCmd));

	/* Like lsusb | grep 12d1 */
	if( ( fstream = popen("lsusb","r") ) != NULL )
	{
		while( fgets( rxCmd,SHELL_POPEN_MAX_SIZE,fstream ) != NULL)
		{
			//pclose(fstream);
			/*Huawei*/ 
			if( ( ptr = strcasestr( rxCmd, APP_USB_DONGLE_HUAWEI_VID ) ) != NULL )
			{
				*factoryType = _HUAWEI;
				ret = 0;
				break;
			}	
			else if( ( ptr = strcasestr( rxCmd, APP_USB_DONGLE_GOSUN_VID ) ) != NULL )
			{
				*factoryType = _GOSUN;
				ret = 0;
				break;
			}
			else if( ( ptr = strcasestr( rxCmd, APP_USB_DONGLE_ZTE_VID ) ) != NULL )
			{
				*factoryType = _ZTE;
				ret = 0;
				break;
			}
			else
				ret = 1;
		}
	}
	pclose(fstream);
	return ret;
}


uint8_t atInterfaceSearchByFactoryProperty( char *devName, FactoryType *factoryType )
{
	uint8_t ret = 1;
	FILE *fstream = NULL;
	char *ptr = NULL;
	//char rxCmd[USB_HOTPLUG_BUF_MAX_SIZE];

	memset(rxCmd,0,sizeof(rxCmd));
	memset(devName,0,strlen(devName));

	switch( *factoryType )
	{
		case _HUAWEI:
			if( ( fstream = popen("find /sys/devices/ -name bInterfaceProtocol | xargs grep -is '12'\
							bInterfaceProtocol","r") ) != NULL )
			{
				if( fgets( rxCmd,SHELL_POPEN_MAX_SIZE,fstream ) != NULL)
				{
					if( strstr(rxCmd,"bInterfaceProtocol:12") != NULL )
					{
						rxCmd[strlen(rxCmd)-strlen("bInterfaceProtocol:12")-1] = '\0';
						chdir(rxCmd);
						if((fstream= popen("ls -d tty*","r")) != NULL )
						{
							if( fgets( devName,SHELL_POPEN_MAX_SIZE,fstream ) != NULL)
							{
								if( strncmp(devName,"tty",3) == 0 )
								{
									devName[strlen(devName)-1] = '\0';
									memmove( devName+5, devName, strlen(devName));
									memcpy( devName, "/dev/", 5);
									#ifdef	DEBUG
										printf("【%s】devName=%s\n",PROGRAM_NAME,devName);
									#endif
									ret = 0;									
								}
								else
									ret = 1;
							}
						}
					}
				}
			}
			pclose(fstream);
			break;

		case _GOSUN:
			//if( ( fstream = popen("find /sys/devices/ -name bInterfaceNumber | xargs grep -is '02'
			if( ( fstream = popen("find /sys/devices/ -name interface | xargs grep -is 'AT'","r") ) != NULL )
			{
				if( fgets( rxCmd,SHELL_POPEN_MAX_SIZE,fstream ) != NULL)
				{
					//if( strstr(rxCmd,"interface:USB-AT") != NULL )
					//if( strstr(rxCmd,"bInterfaceNumber:02") != NULL )
					//if( strstr(rxCmd,"interface:") != NULL )
					if( (ptr=strstr(rxCmd,"interface:")) != NULL )
					{
						//rxCmd[strlen(rxCmd)-strlen("interface:USB-AT")-1] = '\0';
						//rxCmd[strlen(rxCmd)-strlen("bInterfaceNumber:02")-1] = '\0';
						rxCmd[strlen(rxCmd)-strlen(ptr)] = '\0';
						chdir(rxCmd);
						if((fstream= popen("ls -d tty*","r")) != NULL )
						{
							if( fgets( devName,SHELL_POPEN_MAX_SIZE,fstream ) != NULL)
							{
								if( strncmp(devName,"tty",3) == 0)
								{
									devName[strlen(devName)-1] = '\0';
									memmove( devName+5, devName, strlen(devName));
									memcpy( devName, "/dev/", 5);
									#ifdef	DEBUG
									printf("【%s】devName=%s\n",PROGRAM_NAME,devName);
									#endif
									ret = 0;	
								}
								else
									ret = 1;
							}
						}
					}
				}
			}
			pclose(fstream);
			break;

		case _ZTE:
			if( ( fstream = popen("find /sys/devices/ -name interface | xargs grep -is 'USB-AT'\
							interface","r") ) != NULL )
			{
				if( fgets( rxCmd,SHELL_POPEN_MAX_SIZE,fstream ) != NULL)
				{
					pclose(fstream);
					if( strstr(rxCmd,"interface:USB-AT") != NULL )
					{
						rxCmd[strlen(rxCmd)-strlen("interface:USB-AT")-1] = '\0';
						chdir(rxCmd);
						if((fstream= popen("ls -d tty*","r")) != NULL )
						{
							if( fgets( devName,SHELL_POPEN_MAX_SIZE,fstream ) != NULL)
							{
								if( strncmp(devName,"tty",3) == 0)
								{
									devName[strlen(devName)-1] = '\0';

									memmove( devName+5, devName, strlen(devName));
									memcpy( devName, "/dev/", 5);

									#ifdef	DEBUG
									printf("【%s】devName=%s\n",PROGRAM_NAME,devName);
									#endif
									ret = 0;	
								}
								else
									ret = 1;
							}
						}
					}
				
				}
			}
			pclose(fstream);
			break;

		default: ret = 1;
			break;
	}
	
	return ret;
}


uint8_t udhcpcIpByFactoryType( FactoryType factoryType )
{
	uint8_t ret = 1;
	FILE *fstream = NULL;
	//char rxCmd[SHELL_POPEN_MAX_SIZE];
	memset(rxCmd,0,sizeof(rxCmd));

	switch( factoryType )
	{
		case _HUAWEI:
			if ( (fstream = popen("udhcpc -q -i wwan0","r")) != NULL )
			{
				while( fgets(rxCmd,SHELL_POPEN_MAX_SIZE,fstream) != NULL )
				{
					if( strcasestr( rxCmd,"ifconfig wwan0") != NULL )
					{
						#ifdef	DEBUG
							printf("【%s】ifconfig wwan0 %s\n",PROGRAM_NAME,rxCmd);
						#endif
						ret = 0;
						break;
					}
				}
			
			}
			break;

		case _GOSUN:
		case _ZTE:
			if ( (fstream = popen("udhcpc -q -i usb0","r")) != NULL )
			{
				while( fgets(rxCmd,SHELL_POPEN_MAX_SIZE,fstream) != NULL )
				{
					if( strcasestr( rxCmd,"ifconfig usb0") != NULL )
					{
						#ifdef	DEBUG
							printf("【%s】ifconfig wwan0 %s\n",PROGRAM_NAME,rxCmd);
						#endif
						ret = 0;
						break;
					}
				}
			
			}
			break;
	}
	return ret;
}

/*	Funciton:
 *		According to key words-given to search file and content.
 *
 *	Paramter:		
 *		char *filepath:	searching path-given
 *		uint8_t argc:	parameter count
 *		char *argv[]:
 *						argc=0,  argv->NULL("")-> judge file(dir) path existed or not.
 *						argc=1,  argv->fileName-> judge file path existed or not.
 *						argc>=2, argv->fileName,keywords0,1...-> judge file path existed or not.
	
 *	Return:
 *		false->not existed!
 *		true->existed!
 *
 *	Author:Xinhy
 *
 *	Create Date:2020-12-15
 *
 *	Modify Date:
 *
 */
uint8_t searchKeyWordsFromFile( char *filePath , uint8_t argc , char *argv[])
{
	uint8_t ret = 1;

	FILE *fstream;

	//char temp[CMP_KEY_WORdS_MAX_SIZE];

	if( argc == 0 )
	{
		if( !access( filePath , F_OK ) )
		{
			ret = 0;
		}
	}
	else if ( argc == 1 )
	{
		if( !access( filePath , F_OK ) )
		{
			ret = 0;
		}
	}
	else if( argc == 2 )
	{
		if( !access( filePath , F_OK ) && 
			!chdir( filePath ) && 
			!access( argv[0], F_OK) )		//argv[0]->fileName
		{
			ret = 0;
		}
	}
	else
	{
		if( !access( filePath , F_OK ) && 
			!chdir( filePath ) && 
			!access( argv[0], F_OK) )
		{
			if ( (fstream = fopen( argv[0], "r" )) != NULL )
			{
				for( uint8_t i = 1; i < argc ; i++ )
				{
					fseek(fstream,0,SEEK_SET);
					if( fgets( temp , strlen(argv[i]) + 1, fstream) != NULL )
					{
						if( ( strncmp( temp, argv[i], strlen(argv[i])) == 0 ) &&
							( i == argc - 1 ) )
						{
							ret = 0;
						}
					}
					else
						break;
				}
			}
		}
	}
	return ret;
}

/*	Funciton:
 *		Using Socket Netlink to check usb-device hotplug event and record core log.
 *
 *	Paramter:		
 *		struct timeval tv:	select voting cycle;
 *		char *buf:	log-info buffer
	
 *	Return:
 *		false->check-failure
 *		true ->check-success
 *
 *	Author:Xinhy
 *
 *	Create Date:2020-12-15
 *
 *	Modify Date:
 *
 */

char *usbHotPlugCheck( )
{
	fd_set fds;  
	char *ptr;
	struct sockaddr_nl client;  
	int32_t CppLive, rcvlen ,ret;  
	int32_t buffersize = 1024;  
	struct timeval tv;
	CppLive = socket(AF_NETLINK, SOCK_RAW, NETLINK_KOBJECT_UEVENT);  
	memset(&client, 0, sizeof(client));  
	client.nl_family = AF_NETLINK;  
	client.nl_pid = getpid();  
	client.nl_groups = 1; 
	setsockopt(CppLive, SOL_SOCKET, SO_RCVBUF, &buffersize, sizeof(buffersize));  
	bind(CppLive, (struct sockaddr*)&client, sizeof(client));  
	while (1) 
	{  
	   char buf[USB_HOTPLUG_BUF_MAX_SIZE] = "";  
	   ptr = buf;
       FD_ZERO(&fds);  
       FD_SET(CppLive, &fds);  
       tv.tv_sec = 0;  
       tv.tv_usec = 100 * 1000;  

	   ret = select(CppLive + 1, &fds, NULL, NULL, &tv);  
	   if(ret < 0)  
	        continue;  
	   if(!(ret > 0 && FD_ISSET(CppLive, &fds)))  
	        continue;  
	    /* receive data */  
	   rcvlen = recv(CppLive, &buf, sizeof(buf), 0);  
	   if (rcvlen > 0) 
		{  
			if( strcasestr(buf,"add@") != NULL )
				ptr+=4;
			else if( strcasestr(buf,"remove@") != NULL )
				ptr+=7;
		   break;
		     /*You can do something here to make the program more perfect!!!*/  
	    } 
	 }  
	close(CppLive);  
	 return ptr;
}

