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
#ifndef		__USB_DONGLE_SEARCH_H__
#define		__USB_DONGLE_SEARCH_H__

#include	<stdio.h>
#include	<stdlib.h>
#include	<stdint.h>
#include	"config.h"

#define		USB_DONGLE_MODEM_HUAWEI_VID					APP_USB_DONGLE_HUAWEI_VID
#define		USB_DONGLE_MODEM_GOSUN_VID					APP_USB_DONGLE_GOSUN_VID
#define		USB_DONGLE_MODEM_ZTE_VID					APP_USB_DONGLE_ZTE_VID
#define     USB_DONGLE_MODEM_OTHER_VID					APP_USB_DONGLE_OTHER_VID

#define		SHELL_POPEN_MAX_SIZE						APP_POPEN_MAX_SIZE
#define		USB_HOTPLUG_BUF_MAX_SIZE					APP_USB_BUF_MAX_SIZE
#define		CMP_KEY_WORdS_MAX_SIZE						32

#ifndef	PROGRAM_NAME
	#define		PROGRAM_NAME		"UsbDongleSearch App"
#endif

typedef	enum	
{
	_HUAWEI = 0,
	_GOSUN,
	_ZTE,
	_OTHER
}FactoryType;


uint8_t deviceSearch( FactoryType *factoryType );

uint8_t atInterfaceSearchByFactoryProperty( char *devName, FactoryType *factoryType );

uint8_t udhcpcIpByFactoryType( FactoryType factoryType );

char *usbHotPlugCheck( );

uint8_t searchKeyWordsFromFile( char *filePath , uint8_t argc , char *argv[]);

#endif

