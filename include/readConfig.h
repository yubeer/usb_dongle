/**
 * Copyright (c) 2021-10 Designed By WanZiLian@Nanjing.Jiangsu
 *	All rights reseverd.
 *
 *  FielName:	readConfig.h
 *
 *	Function:	Use Marcos-USE_LIBRARY_UCI to select how to get key words value loaded from config_file.
 *
 *	Company:WanZiLian Tech
 *
 *	Author:Mr.yubeer.Engineer <yubeer@163.com>
 *
 *	Version:1.0.0
 *
 *	Create Date:2021-10-14
 *
 */
#ifndef	__READ_CONFIG_H__
#define	__READ_CONFIG_H__

#define		CONFIG_FILE_PATH_PREFIX				APP_CONFIG_FILE_PATH_PREFIX
#define		CONFIG_FILE_NAME					APP_CONFIG_FILE_NAME
#define		CONFIG_FILE_CONTEXT_MAX_SIZE		APP_CONFIG_FILE_CONTEXT_MAX_SIZE
#define		CONFIG_UCI_ENABLE					APP_CONFIG_UCI_ENABLE
#define		CONFIG_FILE_SYSLOG_ENABLE			APP_CONFIG_FILE_SYSLOG_ENABLE

#include	"config.h"

#ifndef	PROGRAM_NAME
	#define		PROGRAM_NAME		"ReadConfig App"
#endif

typedef enum
{
	READ_CONFIG_FILE_NONE_ERR = 0,
	READ_CONFIG_FILE_CTX_NULL_ERR,
	READ_CONFIG_FILE_NOT_EXIST_ERR,
	READ_CONFIG_FILE_LOADING_ERR,
	READ_CONFIG_FILE_VALUE_NULL_ERR
}ReadConfigEnum;

void rc_ctx_init();

void rc_ctx_destroy();

ReadConfigEnum rc_ctx_config( char *config_file, char *option ,char *value );

#endif	/*READ_CONFIG_H*/
