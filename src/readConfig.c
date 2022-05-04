/**
 * Copyright (c) 2021-10 Designed By WanZiLian@Nanjing.Jiangsu
 *	All rights reseverd.
 *
 *  FielName:	readConfig.c
 *
 *	Function:	Use Marcos-USE_LIBRARY_UCI to select how to get key words value loaded from config_file.
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
#include	<syslog.h>
#include	<stdio.h>
#include	<stdlib.h>
#include	<stdint.h>
#include	<string.h>
#include	<syslog.h>
#include	<sys/time.h>
#include	<time.h>
#include	"readConfig.h"

#if		CONFIG_UCI_ENABLE
	#include	<uci.h>
	#define		CONFIG_FILE						CONFIG_FILE_NAME
#endif

#if	CONFIG_UCI_ENABLE
static struct uci_context * ctx = NULL; 
struct uci_package *pkg = NULL;
struct uci_element *e;
struct uci_section *s;

void rc_ctx_init()
{
	ctx = uci_alloc_context(); 
}

void rc_ctx_destroy( )
{
	uci_unload(ctx, pkg); 
	uci_free_context(ctx);
	ctx = NULL;
}
	
ReadConfigEnum rc_ctx_config( char *config_file, char *option ,char *value )
{
	const char *ptr= NULL;

	if( !config_file )
	{
		#if APP_CONFIG_FILE_SYSLOG_ENABLE
		syslog(LOG_ERR,"rc config file not exist.");
		#endif
		#ifdef DEBUG
		printf("【%s】 config file not exist %s!\n");	
		#endif
		return READ_CONFIG_FILE_NOT_EXIST_ERR;
	}

	if( !ctx )
	{
		#if APP_CONFIG_FILE_SYSLOG_ENABLE
		syslog(LOG_ERR,"rc uci ctx is null.");
		#endif
		return READ_CONFIG_FILE_CTX_NULL_ERR;
	}
 
	if (UCI_OK != uci_load(ctx, config_file, &pkg))
	{	
		#if APP_CONFIG_FILE_SYSLOG_ENABLE
		syslog(LOG_ERR,"rc uci config file load error.");
		#endif
		return READ_CONFIG_FILE_LOADING_ERR;
	}
 
	uci_foreach_element(&pkg->sections, e)
	{
		s = uci_to_section(e);
		if(NULL != (ptr= uci_lookup_option_string(ctx, s, option)))
		{
			memcpy(value,ptr,strlen(ptr));
			#ifdef	DEBUG	
				printf("【%s】option=%s,value=%s\n",PROGRAM_NAME,option,value);
			#endif
			#if APP_CONFIG_FILE_SYSLOG_ENABLE
				syslog( LOG_INFO,"option=%s,value=%s\n",PROGRAM_NAME,option,value);
			#endif
			break;
		}
	}
	if( value == NULL )
		return READ_CONFIG_FILE_VALUE_NULL_ERR;
	else
		return READ_CONFIG_FILE_NONE_ERR;
}
#else

static char *ctx = NULL;

void rc_ctx_init()
{
	ctx = ( char * ) malloc( sizeof( char *) *  CONFIG_FILE_CONTEXT_MAX_SIZE);
	memset( ctx, 0, sizeof( char * ) *  CONFIG_FILE_CONTEXT_MAX_SIZE);
}

ReadConfigEnum rc_ctx_config( char *config_file , char *option, char *value )
{
	FILE *stream = NULL;
	char identify = '=';
	char *ptr = NULL;
	int length = 0;
	
	if( config_file == NULL )
	{
		#if APP_CONFIG_FILE_SYSLOG_ENABLE
		syslog(LOG_ERR,"rc config file not exist.");
		#endif
		return READ_CONFIG_FILE_NOT_EXIST_ERR;
	}

	if( !ctx )
	{
		#if APP_CONFIG_FILE_SYSLOG_ENABLE
		syslog(LOG_ERR,"rc uci ctx is null.");
		#endif
		return READ_CONFIG_FILE_CTX_NULL_ERR;
	}

	if( ( stream = fopen( config_file, "r" ) ) == NULL )
	{
		#if APP_CONFIG_FILE_SYSLOG_ENABLE
		syslog(LOG_ERR,"rc uci config file load error.");
		#endif
		return READ_CONFIG_FILE_LOADING_ERR;
	}

	while( fgets( ctx , CONFIG_FILE_CONTEXT_MAX_SIZE, stream ) )
	{
		if( (ctx[0] != '[')  &&  (ctx[0] != '#') && ( ctx[0] != '/')  && (ptr = strstr(ctx, (char *)option) ) != NULL )
		{
			ptr += strlen(option);
			if( (ptr = strchr(ptr,identify)) != NULL )
			{
				ptr = ptr+1;
				length = strlen(ptr);
				memcpy(value,ptr,length);
				if (  value[length-1] == '\n' )
					 value[length-1] = '\0';
			}
			break;
		}
	}

	if( value == NULL )
		return READ_CONFIG_FILE_VALUE_NULL_ERR;
	else
		return READ_CONFIG_FILE_NONE_ERR;
}

void rc_ctx_destroy( )
{
	free(ctx);
}
#endif

#if 0
int main( int argc, char *argv[] )
{
	char value[20];
	rc_ctx_init();
	if ( (rc_ctx_config("/etc/config/serialClient","log",value)) != READ_CONFIG_FILE_NONE_ERR )
		printf("...<0\n");
	else
	{
		printf("value=%s\n",value);
		rc_ctx_destroy();
	}
	return 0;
}
#endif
