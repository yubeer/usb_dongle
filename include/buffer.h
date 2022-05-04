/**
 * Copyright (c) 2021-10 Designed By WanZiLian@Nanjing.Jiangsu
 *	All rights reseverd.
 *
 *  FielName:	buffer.h
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
 */
#ifndef		__BUFFER_H__
#define		__BUFFER_H__

#include	<stdio.h>	
#include	<stdlib.h>
#include	<unistd.h>
#include	<ctype.h>
#include	<string.h>
#include	<stdint.h>
#include	<fcntl.h>
#include	<sys/time.h>
#include	<netinet/in.h>	
#include	<arpa/inet.h>
#include	<netinet/tcp.h>
#include    "config.h"

#if      BUF_SYSLOG_ENABLE
    #include	<syslog.h>
#endif

#define     BUF_SYSLOG_ENABLE                           APP_BUF_SYSLOG_ENABLE
#define     HARDWARE_USART_FIFO_ENABLE                  APP_HARDWARE_USART_FIFO_ENABLE
#define		HARDWARE_USART_FIFO_SIZE			        APP_HARDWARE_USART_FIFO_SIZE

#define         MODE_TCP                                1
#define         MODE_UDP                                0

#ifndef	PROGRAM_NAME
	#define		PROGRAM_NAME		"MuxUtils App"
#endif

typedef	enum
{
	BUF_OUT_BY_WRITE = 0,
	BUF_OUT_BY_SENDTO,
	BUF_IN_BY_READ,
	BUF_IN_BY_RECVFROM
}BufOutEnum;

struct sbuf {
    uint8_t		 *buf;
    unsigned int maxsize;
    unsigned int cursize;	/* current data size*/
    unsigned int pos;		/*  data read position*/
};

int buffer_init(struct sbuf *buf, uint8_t *data, unsigned int length );
int buffer_in( BufOutEnum flag, int mode,int fd,struct sbuf *buf,struct sockaddr *addr, socklen_t *addrlen );
int buffer_out( BufOutEnum flag,int mode,int fd,struct sbuf *buf, struct sockaddr *addr, socklen_t addrlen );
int buffer_flush(struct sbuf *buf);

#endif	/* BUFFER_H */
