#ifndef		__UCI_PROCESS_H__
#define		__UCI_PROCESS_H__

#include	<stdio.h>
#include	<stdlib.h>
#include	<stdint.h>
#include	<unistd.h>
#include	<uci.h>
#include	<string.h>
#include	<stdbool.h>

#ifndef	PROGRAM_NAME
	#define		PROGRAM_NAME		"UciProcess App"
#endif

bool	uci_get_string( char *cfgName, char *optionName, char *value );

bool	uci_set_string( char *cfgName, char *sectionName, char *optionName, const char *value );

bool	uci_set_by_popen( char *setKey, char *commitKey, char *value );

bool	uci_get_by_popen( char *getKey,  char *value );

#endif

