/**
 * Copyright (c) 2021-10 Designed By WanZiLian@Nanjing.Jiangsu

 *	All rights reseverd.
 *
 *  FielName:	muxUtils.c
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
#include	<unistd.h>
#include	<errno.h>
#include	<stdio.h>
#include	<string.h>
#include	"muxUtils.h"

int mux_utils_init( struct MuxTypeDef *mux ) {
	/*epoll create*/
	#if	MUX_UTILS_EPOLL_ENABLE
	if( (mux->epfd = epoll_create1(0)) < 0 ) {
		#ifdef	DEBUG
		printf("【%s】epoll create1 error: %s\n",PROGRAM_NAME,strerror(errno));
		#endif
		return -1;
	}
	#endif

	#if	MUX_UTILS_SELECT_ENABLE	
	FD_ZERO( &mux->rfds );
	FD_ZERO( &mux->wfds );
	FD_ZERO( &mux->efds );
	#endif

	return 0;
}

int mux_utils_add(struct MuxTypeDef *mux , int fd, int mode ) {
	#if	MUX_UTILS_EPOLL_ENABLE
	mux->ev.data.fd = fd;
	switch( mode )
	{
		case MUX_READ:
			mux->ev.events = EPOLLIN;
			break;

		case MUX_WRITE:
			mux->ev.events = EPOLLOUT;
			break;

		case MUX_EXCEPT:
			mux->ev.events = EPOLLERR;
			break;

		case MUX_READ | MUX_WRITE:
			mux->ev.events = EPOLLIN | EPOLLOUT;
			break;

		case MUX_READ | MUX_EXCEPT:
			mux->ev.events = EPOLLIN | EPOLLERR;
			break;

		case MUX_WRITE | MUX_EXCEPT:
			mux->ev.events = EPOLLOUT| EPOLLERR;
			break;

		case MUX_READ | MUX_WRITE | MUX_EXCEPT:
			mux->ev.events = EPOLL|EPOLLOUT| EPOLLERR;
			break;
	}
	/* add fd-readable to epoll*/
	if( epoll_ctl( mux->epfd, EPOLL_CTL_ADD, fd, &mux->ev) == -1 ) {
	#ifdef	DEBUG
		printf("【%s】epoll ctl add fd = %d, error: %s\n", PROGRAM_NAME,fd, strerror(errno));
	#endif
		return -1;
	}
	#endif

	#if	MUX_UTILS_SELECT_ENABLE	
	switch( mode )
	{
		case MUX_READ:
			FD_SET( fd, &mux->rfds );
			break;

		case MUX_WRITE:
			FD_SET( fd, &mux->wfds );
			break;

		case MUX_EXCEPT:
			FD_SET( fd, &mux->efds );
			break;

		case MUX_READ | MUX_WRITE:
			FD_SET( fd, &mux->rfds );
			FD_SET( fd, &mux->wfds );
			break;

		case MUX_READ | MUX_EXCEPT:
			FD_SET( fd, &mux->rfds );
			FD_SET( fd, &mux->efds );
			break;

		case MUX_WRITE | MUX_EXCEPT:
			FD_SET( fd, &mux->wfds );
			FD_SET( fd, &mux->efds );
			break;

		case MUX_READ | MUX_WRITE | MUX_EXCEPT:
			FD_SET( fd, &mux->rfds );
			FD_SET( fd, &mux->wfds );
			FD_SET( fd, &mux->efds );
			break;
	}
	#endif
	return 0;
}

int mux_utils_del(struct MuxTypeDef *mux , int fd, int mode ) {
/* del fd-readable to epoll*/
	#if	MUX_UTILS_EPOLL_ENABLE
	if( epoll_ctl( mux->epfd, EPOLL_CTL_DEL, fd, &mux->ev) == -1 ) {
		#ifdef	DEBUG
		printf("【%s】epoll ctl fd = %d del error: %s\n",PROGRAM_NAME,fd,strerror(errno));
		#endif
		return -1;
	}
	#endif

	#if	MUX_UTILS_SELECT_ENABLE	
	switch( mode )
	{
		case MUX_READ:
			//FD_ZERO( &mux->rfds );
			FD_CLR( fd, &mux->rfds );
			break;

		case MUX_WRITE:
			FD_CLR( fd, &mux->wfds );
			break;

		case MUX_EXCEPT:
			FD_CLR( fd, &mux->efds );
			break;

		case MUX_READ | MUX_WRITE:
			FD_CLR( fd, &mux->rfds );
			FD_CLR( fd, &mux->wfds );
			break;

		case MUX_READ | MUX_EXCEPT:
			FD_CLR( fd, &mux->rfds );
			FD_CLR( fd, &mux->efds );
			break;

		case MUX_WRITE | MUX_EXCEPT:
			FD_CLR( fd, &mux->wfds );
			FD_CLR( fd, &mux->efds );
			break;

		case MUX_READ | MUX_WRITE | MUX_EXCEPT:
			FD_CLR( fd, &mux->rfds );
			FD_CLR( fd, &mux->wfds );
			FD_CLR( fd, &mux->efds );
			break;
	}
	#endif
	return 0;
}

int mux_utils_wait(struct MuxTypeDef *mux , int fd ,struct timeval *tv, int mode ) {

	int listen_num = 0;
	#if	MUX_UTILS_EPOLL_ENABLE
		#ifdef	MUX_EPOLL_NONBLOCK
			listen_num = epoll_wait( mux->epfd, mux->events, MUX_UTILS_MAX_SIZE,tv->tv_sec*1000 + tv->tv_usec/1000 );
		#else
			listen_num = epoll_wait( mux->epfd, mux->events,MUX_UTILS_MAX_SIZE,-1 );
		#endif
	#endif

	#if	MUX_UTILS_SELECT_ENABLE
		#if	MUX_UTILS_SELECT_NONBLOCK_ENABLE
			switch( mode )
			{
				case MUX_READ:
					listen_num = select( fd, &mux->rfds,NULL,NULL,tv) ;
					break;

				case MUX_WRITE:
					listen_num = select( fd, NULL,&mux->wfds,NULL,tv) ;
					break;

				case MUX_EXCEPT:
					listen_num = select( fd, NULL,NULL, &mux->efds,tv) ;
					break;

				case MUX_READ | MUX_WRITE:
					listen_num = select( fd, &mux->rfds, &mux->wfds, NULL,tv) ;
					break;

				case MUX_READ | MUX_EXCEPT:
					listen_num = select( fd, &mux->rfds, NULL, &mux->efds,tv) ;
					break;

				case MUX_WRITE | MUX_EXCEPT:
					listen_num = select( fd, NULL, &mux->wfds, &mux->efds,tv) ;
					break;

				case MUX_READ | MUX_WRITE | MUX_EXCEPT:
					listen_num = select( fd, &mux->rfds,&mux->wfds,&mux->efds,tv) ;
					break;
			}
		#else
			switch( mode )
			{
				case MUX_READ:
					listen_num = select( fd, &mux->rfds,NULL,NULL,NULL) ;
					break;

				case MUX_WRITE:
					listen_num = select( fd, NULL,&mux->wfds,NULL,NULL) ;
					break;

				case MUX_EXCEPT:
					listen_num = select( fd, NULL,NULL, &mux->efds,NULL) ;
					break;

				case MUX_READ | MUX_WRITE:
					listen_num = select( fd, &mux->rfds, &mux->wfds, NULL,NULL) ;
					break;

				case MUX_READ | MUX_EXCEPT:
					listen_num = select( fd, &mux->rfds, NULL, &mux->efds,NULL) ;
					break;

				case MUX_WRITE | MUX_EXCEPT:
					listen_num = select( fd, NULL, &mux->wfds, &mux->efds,NULL) ;
					break;

				case MUX_READ | MUX_WRITE | MUX_EXCEPT:
					listen_num = select( fd, &mux->rfds,&mux->wfds,&mux->efds,NULL) ;
					break;
			}
		#endif
	#endif
	return listen_num;
}

int mux_utils_isset( struct MuxTypeDef *mux, int fd, int mode )
{
	int ret = -1;
	#if	MUX_UTILS_EPOLL_ENABLE

		;
	#endif

	#if	MUX_UTILS_SELECT_ENABLE
	switch( mode )
	{
		case MUX_READ:
			ret = FD_ISSET( fd, &mux->rfds);
			break;

		case MUX_WRITE:
			ret = FD_ISSET( fd, &mux->wfds);
			break;

		case MUX_EXCEPT:
			ret = FD_ISSET( fd, &mux->efds);
			break;
	}
	#endif
	return ret;
}



