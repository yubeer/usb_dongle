/**
 * Copyright (c) 2021-10 Designed By WanZiLian@Nanjing.Jiangsu

 *	All rights reseverd.
 *
 *  FielName:	config.h
 *
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

#ifndef		__CONFIG_H__
#define		__CONFIG_H__

#define     APP_VERSION                                     "V1.0.0"
#define     APP_VERSION_MAJOR                               1
#define     APP_VERSION_MINOR                               0
#define     APP_VERSION_MICRO                               0

//#define     PROGRAM_NAME                                    "usb_dongle"
/*Use uci library*/
#define		APP_CONFIG_UCI_ENABLE                           1
#define		APP_CONFIG_FILE_PATH_PREFIX                     "/etc/config/"
#define		APP_CONFIG_FILE_NAME                            "usb_dongle"
#define		APP_CONFIG_FILE_CONTEXT_MAX_SIZE                1024
#define		APP_CONFIG_FILE_SYSLOG_ENABLE                   0

#define		APP_DONGLE_SEARCH_CYCLE                         1

#define		APP_DONLEG_SERIAL_SELECT_TIME_SEC               5
#define		APP_DONLEG_SERIAL_SELECT_TIME_USEC              0
#define		APP_DONGLE_SERIAL_RECEIVE_BUF_SIZE              1024
#define		APP_DONGLE_SERIAL_SEND_BUF_SIZE                 512
#define     APP_DONGLE_SEND_WAIT_TIME                       2
#define     APP_DONGLE_SOFT_RESET_TIME_SEC                  600
#define     APP_DONGLE_AIR_MODE_TIME_SEC                    120

#define     APP_CRONTRAB_MAX_FILE_FD                                     3

#define		APP_POPEN_MAX_SIZE                              1024
#define		APP_USB_BUF_MAX_SIZE                            512

#define		APP_USB_DONGLE_HUAWEI_VID					    "12d1"
#define		APP_USB_DONGLE_GOSUN_VID						"305a"
#define		APP_USB_DONGLE_ZTE_VID						    "19d2"
#define     APP_USB_DONGLE_OTHER_VID                        ""

#define		APP_LOG_ENABLE                                  1
#define		APP_LOG_SYSLOG_ENABLE                           1
#define		APP_LOG_FILE_ENABLE                             0
#define     APP_LOG_CLASS_NAME_SIZE                         64
#define		APP_LOG_TIME_STAMP_MAX_SIZE                     64
#define		APP_LOG_FILE_NAME_MAX_SIZE                      64
#define		APP_LOG_FILE_PATH                               "/var/log/usb_dongle.log"

#define     APP_BUF_SYSLOG_ENABLE                           0
#define     APP_HARDWARE_USART_FIFO_ENABLE                  1
#define		APP_HARDWARE_USART_FIFO_SIZE                    16

#define     APP_MUX_UTILS_SYSLOG_ENABLE                     0
#define     APP_MUX_UTILS_EPOLL_ENABLE                      0
#define     APP_MUX_UTILS_SELECT_ENABLE                     1
#define     APP_MUX_UTILS_SELECT_NONBLOCK_ENABLE            1
#define     APP_MUX_UTILS_MASX_SIZE                         10

#endif  /*__CONFIG_H__*/
