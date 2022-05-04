/**
 * Copyright (c) 2021-10 Designed By WanZiLian@Nanjing.Jiangsu
 *	All rights reseverd.
 *
 *  FielName:	muxSerial.h
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
 */
#ifndef		__MUX_SERIAL_H__
#define		__MUX_SERIAL_H__

#define     MUX_SERIAL_RECV_EXPIRED_SIZE        10000       //usec

// #ifndef	PROGRAM_NAME
// 	#define		PROGRAM_NAME		"MuxSerial App"
// #endif

struct MuxSerialTypeDef 
{
	struct SerialTypeDef *serial;
	struct MuxTypeDef	*mux;	/*复用方式*/
	struct sbuf *serial_recv_buf;
    struct sbuf *serial_send_buf;
    int (*init)( struct MuxSerialTypeDef *mstd );
    int (*read)( struct MuxSerialTypeDef *mstd );
    int (*write)( struct MuxSerialTypeDef *mstd );
};

int mux_serial_init_handler( struct MuxSerialTypeDef *mstd );

#endif /* __MUX_SERIAL_H__ */

