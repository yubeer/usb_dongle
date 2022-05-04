/**
 * Copyright (c) 2021-10 Designed By WanZiLian@Nanjing.Jiangsu

 *	All rights reseverd.
 *
 *  FielName:	dongle_at_process.h
 *
 *	Function:	at-command recevie process. 
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
#ifndef	__DONGLE_AT_PROCESS_H__
#define	__DONGLE_AT_PROCESS_H__


#define	CONFIG_DONGLE_USE_SERIAL											0	
#define	CONFIG_DONGLE_USE_USB												1

#define	USB_DONGLE_IMSI_SIZE												16U

/* DONGLE Base Info */
#if	CONFIG_DONGLE_USE_USB
	#define	USB_DONGLE_USB_SOCKET_SIZE										5U
	#define	USB_DONGLE_IMEI_SIZE											16U
	#define	USB_DONGLE_APN_SIZE												32U
	#define	USB_DONGLE_FACTORY_SIZE											64U
	#define	USB_DONGLE_MODEL_SIZE											64U	
	#define	USB_DONGLE_VERSION_SIZE											64U
#endif

/* C Lib Base Data Type*/
#include		<stdint.h>
#include		<stdbool.h>

/* DONGLE SERIAL Interface */
#if	CONFIG_DONGLE_USE_SERIAL 
	#include	<stm32f10x_SERIAL.h>
	#define		SB_DONGLE_SERIAL_SOCKET_SIZE									5U
	#define		DONGLE_SERIAL_ATTACH_IDENTIFY								"0x00000004"
#endif

typedef enum
{
	DONGLE_AT_QUERY = 0,
	DONGLE_AT_SET
}EnumAtMode;

typedef enum
{
	AT_STAGE_PREPARE = 0,
	AT_STAGE_INITIAL,			/* General At*/
	AT_STAGE_ACCESS,			/* Search net and Dial*/
	AT_STAGE_REGISTER,			/* Register*/
	AT_STAGE_QUERY,
	AT_STAGE_RELEASE,
	AT_STAGE_ERROR				/* error*/
}EnumAtStage;

typedef enum
{
	AT_OK = 0,
	AT_ERR,
	AT_NO_SUPPORT,
	AT_NONE
} EnumAtRetStatus;

typedef	enum 
{
	/*Base general at command*/
#if	CONFIG_DONGLE_USE_USB
	/*General At commadn*/
	_ATE = 0,							
	_ATI,
	_CIMI,						
	_CEREG,
	_CFUN,
	_CGREG,
	_CGDCONT,
	_CGDSCONT,
	_CGATT,
	_CGACT,
	_CGPADDR,
	_CMEE,							
	_COPS,
	_CPIN,
	_CSQ,
	_SYSINFO,
	

	/*Huawei - EM350&EM360 Voice control*/
	_CURC,
	
	/*Huawei*/
	_RESET,
	_SYSINFOEX,							
	_ABINFO,
	_LBINFO,
	_ANTPC,
	_CSGQRY,	
	_NDISDUP,
	_NDISSTATQRY,
	_SSWEN,
	_SPLMN,
	_SSIM,

	/* Gosun800*/
	_ZECMCALL,
	_ZCELLINFO,
	_ZGRST,
	_ZPAS,
	_ZSNT,
	_ZRST,
	_CGEV,
	_ZSINR,

#elif CONFIG_DONGLE_USE_SERIAL
	_SINR,
#else
	_SINR
#endif

	
#if CONFIG_DONGLE_USE_SERIAL
	/*Serial-Extension-Begin*/
	_DSMS,
	_ZIPCALL,
	_CCLK,
	_RATE,
	_OFF,
	_VER,
	_USLP,
	_ZCAPN,
	_ZIPOPEN,
	_ZIPSEND,
	_ZIPCLOSE,
	_ZIPSLCFG,
	_ZIPLISTEN,
	_ZIPRECV
#endif
	/*Serial-Extension-End*/
} EnumAtCmdCtx;

typedef struct
{
#if CONFIG_DONGLE_USE_USB
	char			factory[USB_DONGLE_FACTORY_SIZE];				
	char			ver[USB_DONGLE_VERSION_SIZE];				
	char			imei[USB_DONGLE_IMEI_SIZE];				
	char			model[USB_DONGLE_MODEL_SIZE];
#endif

#if CONFIG_DONGLE_USE_SERIAL 
	uint8_t			sver[5];
#endif
}DongleGeneralTypeDef;

typedef	struct
{
	uint8_t			imsi[USB_DONGLE_IMSI_SIZE];
}DongleUsimTypeDef;

/*
 *	Structure	DONGLEFreqTypeDef
 *
 *	Target:	Define DONGLE USIM informaton.
 *
 *	mBandId: local band	63
 *	mAirBandId:			73
 *	mFreqLowID:			58660
 *	mFreqHighID:		59260
 *	mEarfcnLow:			5660
 *	mEarfcnHigh:		6259
 *
 */
#if	CONFIG_DONGLE_USE_USB
typedef struct
{
	uint8_t			bandId;
	uint8_t			airBandId;
	uint16_t		freqLow;		
	uint16_t		freqHigh;
	uint16_t		earfcnLow;
	uint16_t		earfcnHigh;
}DongleFreqTypeDef;


typedef struct
{
	uint8_t			srvStatus;				
	uint8_t			srvDomain;
	uint8_t			roamStatus;
	uint8_t			sysMode;
	uint8_t			simStatus;				/* SystemSubMode		25->TDD */
	uint8_t			sysSubmode;
	uint8_t			cfun;
}DongleSearchTypeDef;


typedef struct
{
	uint8_t			registerStatus;
	uint8_t			bearId;
	uint32_t		cellId;
	uint16_t		cellType;
	uint16_t		freq;
	int16_t			rssi;
	int16_t			rsrp;
	int16_t			rsrq;
	int16_t			sinr;
	uint8_t			apn[USB_DONGLE_APN_SIZE];
	uint32_t		ipAddr;
	uint32_t		submask;
	uint32_t		dns;
	uint32_t		gateway;
	uint16_t		tac;
	uint16_t		lac;
	uint32_t		pCI;
	uint16_t		gtchBler;
	uint16_t		gpcchBler;
}DongleRegisterTypeDef;

#endif

typedef struct
{
	uint8_t				resetFlag;					//0->soft,1->hard
	uint32_t			airFailureCount;
	uint32_t			softFailureCount;
	uint32_t			hardFailureCount;
}DongleAtFailureTypeDef;


/* socket-id -type*/
#if CONFIG_DONGLE_USE_SERIAL
typedef	struct
{
	uint8_t		registerStatus;
	uint8_t		wakeupFlag;
	uint8_t		socketId;
	uint8_t		sendFlag;
	uint8_t		*recvDataPtr;
	uint8_t		modeType;
	uint32_t	serverIp;
	uint16_t	serverPort;
	uint16_t	clientPort;
}DongleSerialSrvTypeDef;
#endif

/*Define DONGLE-ALL Property*/
typedef struct
{
	EnumAtMode					_mode;
	EnumAtStage					_stage;
	EnumAtCmdCtx				_cmd;
	DongleGeneralTypeDef		dongleGeneralStruct;
	DongleUsimTypeDef			dongleUsimStruct;
#if	CONFIG_DONGLE_USE_USB
	DongleFreqTypeDef			dongleFreqStruct;
	DongleSearchTypeDef			dongleSearchStruct;
	DongleRegisterTypeDef		dongleRegisterStruct;
#endif

#if	CONFIG_DONGLE_USE_SERIAL
	DongleSerialSrvTypeDef		dongleSrvStruct;
#endif

	DongleAtFailureTypeDef		dongleAtFailureStruct;
#if CONFIG_DONGLE_USE_SERIAL
	uint8_t *(*recvProcess)( UsartTypeDef SERIALx,  uint8_t *pdata, uint16_t length );
	uint8_t *(*sendProcess)( UsartTypeDef SERIALx,  uint8_t *pdata, uint16_t length );
#endif


#if CONFIG_DONGLE_USE_USB
	uint8_t *(*recvProcess)(  uint8_t *pdata, uint16_t length );
	uint8_t *(*sendProcess)(  uint8_t *pdata, uint16_t length );
#endif
}DongleTypeDef;

void dongle_init_handler(DongleTypeDef *dtd);
/* AT-Command Process */
EnumAtRetStatus atCmdReceiveProcess(DongleTypeDef *mpt,char *pdata);

#endif

