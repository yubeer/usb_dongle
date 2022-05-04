/**
 * Copyright (c) 2021-10 Designed By WanZiLian@Nanjing.Jiangsu

 *	All rights reseverd.
 *
 *  FielName:	muxUtils.h
 *
 *	Function:	Application select utility api
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
#ifndef		__MUX_UTILS_H__
#define		__MUX_UTILS_H__

#include	"config.h"

#define		MUX_UTILS_SYSLOG_ENABLE					APP_MUX_UTILS_SYSLOG_ENABLE
#define		MUX_UTILS_EPOLL_ENABLE					APP_MUX_UTILS_EPOLL_ENABLE
#define		MUX_UTILS_SELECT_ENABLE					APP_MUX_UTILS_SELECT_ENABLE
#define		MUX_UTILS_SELECT_NONBLOCK_ENABLE		APP_MUX_UTILS_SELECT_NONBLOCK_ENABLE

#define		MUX_UTILS_MAX_SIZE						APP_MUX_UTILS_MASX_SIZE
#define		MUX_READ								0x01
#define		MUX_WRITE								0x02
#define		MUX_EXCEPT								0x04

#if		MUX_UTILS_EPOLL_ENABLE
	#include	<sys/epoll.h>
#endif

#if		MUX_UTILS_SELECT_ENABLE
	#include	<sys/select.h>
#endif

#ifndef	PROGRAM_NAME
	#define		PROGRAM_NAME		"MuxUtils App"
#endif

struct MuxTypeDef
{
	#if	MUX_UTILS_EPOLL_ENABLE
	int		epfd;
	struct	epoll_event ev;
	struct	epoll_event events[MUX_UTILS_MAX_SIZE];
	#endif
	
	#if MUX_UTILS_SELECT_ENABLE
	fd_set	rfds;
	fd_set	wfds;
	fd_set	efds;
	#endif
};

int mux_utils_init( struct MuxTypeDef *mux );

int mux_utils_add(struct MuxTypeDef *mux , int fd, int mode ) ;

int mux_utils_del(struct MuxTypeDef *mux , int fd, int mode ) ;

int mux_utils_wait(struct MuxTypeDef *mux , int fd ,struct timeval *tv, int mode );

int mux_utils_isset( struct MuxTypeDef *mux, int fd, int mode );

#endif	/* __MUX_UTILS_H__ */

