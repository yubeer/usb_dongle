/**
 * Copyright (c) 2021-10 Designed By WanZiLian@Nanjing.Jiangsu

 *	All rights reseverd.
 *
 *  FielName:	log.h
 *
 *	Function:	According to app requirements using log-api generating debug log and print tips messages
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

#ifndef		__LOG_H__
#define		__LOG_H__

#define		LOG_ENABLE									APP_LOG_ENABLE
#define		LOG_CLASS_NAME_SIZE							APP_LOG_CLASS_NAME_SIZE
#define		LOG_FILE_TIME_STAMP_MAX_SIZE				APP_LOG_TIME_STAMP_MAX_SIZE
#define		LOG_FILE_NAME_MAX_SIZE						APP_LOG_FILE_NAME_MAX_SIZE
#define		LOG_FILE_PATH								APP_LOG_FILE_PATH


/**
*log local file open mode
*/
#define		LOG_FILE_OPEN_WRITE_OVERRIDE				"w+"
#define		LOG_FILE_OPEN_WRITE_APPEND					"a+"

#define		LOG_FILE_OPEN_WRITE_OVERRIDE_INDEX			0
#define		LOG_FILE_OPEN_WRITE_APPEND_INDEX			1

#include	<syslog.h>
#include	<stdio.h>
#include	<stdlib.h>
#include	<stdint.h>
#include	<string.h>
#include	<syslog.h>
#include	<sys/time.h>
#include	<time.h>
#include	"config.h"

struct LogTypeDef
{
	char *path;
	char *class;
	uint8_t enable:1;
	uint8_t	local_enable:1;
	uint8_t	syslog_enable:1;
	uint8_t local_mode:1;
	uint8_t log_priority;
	FILE *(*open)( struct LogTypeDef *p );
	void(*config)( struct LogTypeDef *p, uint8_t enable, uint8_t file_enable, uint8_t syslog_enable,uint8_t file_mode);
	int (*write)(  struct LogTypeDef *p, const char *format, ...);
	int (*close)( struct LogTypeDef *p, int file_enable, int syslog_enable  );
	int (*flush)( struct LogTypeDef *p);
	int (*callback)( struct LogTypeDef *p, void *pdata);
};

char *log_local_time();

void log_handler_init( struct LogTypeDef *p );

#endif	/* LOG_H */

