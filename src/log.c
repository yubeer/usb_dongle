/**
 * Copyright (c) 2021-10 Designed By WanZiLian@Nanjing.Jiangsu

 *	All rights reseverd.
 *
 *  FielName:	log.c
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

#include        <stdio.h>
#include        <stdlib.h>
#include        <sys/time.h>
#include        <stdarg.h>
#include        <linux/fs.h>
#include        "log.h"

static char local_time[LOG_FILE_TIME_STAMP_MAX_SIZE];
static time_t   now_time;
static struct tm *local_tm_info;
static FILE *fstream;

static const char *file_mode[2] ={LOG_FILE_OPEN_WRITE_OVERRIDE,LOG_FILE_OPEN_WRITE_APPEND};
/**
 * @brief functions
 * 
 * @param p 
 */
void log_handler_init( struct LogTypeDef *p);
static FILE *log_open( struct LogTypeDef *p );
static void log_config( struct LogTypeDef *p, uint8_t enable, uint8_t file_enable, uint8_t syslog_enable,uint8_t file_mode);
static int log_close( struct LogTypeDef *p, int local, int sys );
static int log_write( struct LogTypeDef *p , const char *format, ... );
static int log_flush( struct LogTypeDef *p );

/**
 * @brief log_local_time to get local time 
 * 
 * @return char* 
 */
char *log_local_time()
{
        time(&now_time);
        local_tm_info = localtime(&now_time);
        strftime(local_time,LOG_FILE_TIME_STAMP_MAX_SIZE,"%Y-%m-%d:%H:%M:%S",local_tm_info);
        return local_time;
}

void log_handler_init( struct LogTypeDef *p )
{
        p->path = (char *)malloc(sizeof(char) * 64);
        memcpy(p->path,LOG_FILE_PATH,strlen(LOG_FILE_PATH));
        p->class = (char *)malloc(sizeof(char *) * LOG_CLASS_NAME_SIZE);
        p->enable = LOG_ENABLE;
        p->local_enable = 0;
        p->syslog_enable = 0;
        p->local_mode = LOG_FILE_OPEN_WRITE_OVERRIDE_INDEX;
        p->log_priority = LOG_DEBUG;
        p->open = log_open;
        p->config = log_config;
        p->write = log_write;
        p->close = log_close;
        p->flush = log_flush;
}


static FILE *log_open( struct LogTypeDef *p )
{
        if( !p->enable )
                return NULL;
        if( p->syslog_enable == 1 )
        {
                if( p->class == NULL || strlen(p->class) == 0 )                       
                        openlog( "log debug " ,LOG_CONS | LOG_NOWAIT | LOG_PID, LOG_USER);
                else
                        openlog( p->class,LOG_CONS | LOG_NOWAIT | LOG_PID, LOG_USER);
        }
        if( p->local_enable == 1 )
        {
                return ( fstream = fopen( p->path, file_mode[p->local_mode] ) );           
        }
        return NULL;
}

static void log_config( struct LogTypeDef *p, uint8_t enable, uint8_t file_enable, uint8_t syslog_enable,uint8_t file_mode)
{
        p->enable = enable;
        p->local_enable = file_enable;
        p->syslog_enable = syslog_enable;
        p->local_mode = file_mode;
}

static int log_close( struct LogTypeDef *p, int file_enable, int syslog_enable )
{
        if( file_enable == 1 && p->local_enable == 1 )
        {
                p->local_enable = 0;
                fclose( fstream );
        }
        if( syslog_enable == 1 )
                p->syslog_enable = 0;
        return 0;
}

static int log_write( struct LogTypeDef *p, const char *format , ...)
{
        if( !p->enable )
                return -1;
        va_list ap;
        if(  p->local_enable == 1 )
        {
                if( fstream == NULL )
                {
                        #ifdef DEBUG
                        printf("log fstream is null ,reopen %s\n",fstream,p->path);
                        #endif
                        if ( (fstream = log_open(p)) == NULL )
                                return -1;
                }
                        
                va_start(ap,format);

                if ( fprintf(  fstream,"【%s】\t",log_local_time()) < 0 )
                        return -1;

                if ( vfprintf( fstream, format, ap) < 0 )
                        return -1;
        }
        if( p->syslog_enable == 1 )
        {
                vsyslog( p->log_priority, format, ap);
        }
        va_end(ap);
        return 0;
}


static int log_flush( struct LogTypeDef *p )
{
        uint8_t local_mode_origin = 0;
        if( p->local_enable == 1 && fstream != NULL )
        {
                local_mode_origin = p->local_mode;
                p->local_mode = LOG_FILE_OPEN_WRITE_OVERRIDE_INDEX;
                if( log_open(p) == NULL )
                {       p->local_mode = local_mode_origin;
                        return -1;
                }
                else
                {
                        p->local_mode = local_mode_origin;
                        if ( fclose(fstream) == EOF )
                                return -1;
                }
                        
        }
        return 0;
}

#if 0
int main( int argc, char *argv[] )
{
        struct LogTypeDef *ppp = (struct LogTypeDef *)malloc( sizeof( struct LogTypeDef *));
        log_handler_init(ppp);
        ppp->log_priority = LOG_INFO;
        ppp->enable = 1;
        ppp->local_enable = 1;
        ppp->syslog_enable = 1;
        char *p = "hello world!";
        ppp->write(ppp,"context is %s\n",p)
        return 0;
}
#endif