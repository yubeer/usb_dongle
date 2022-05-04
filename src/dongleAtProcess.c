/**
 * Copyright (c) 2021-10 Designed By WanZiLian@Nanjing.Jiangsu

 *	All rights reseverd.
 *
 *  FielName:	dongle_at_process.c
 *
 *	Function:	at-command recevie process. 
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

#include		<string.h>
#include		<stdio.h>
#include		<stdlib.h>
#include		<ctype.h>
#include		<math.h>
#include		<stdbool.h>
#include		"dongleAtProcess.h"

static	char temp[32];
static	uint8_t	ipAddr[4];

void dongle_init_handler(DongleTypeDef *dtd)
{
	memset(dtd,0,sizeof( DongleTypeDef ));
}

/* getIpaddr From String*/
static int getIpaddr(char *ret,uint32_t *Ip)
{
	/*
	char temp[4];
	uint8_t ipAddr[4];
	*/
	uint8_t len = 0;
	uint8_t i = 0;
	
	memset(temp,0,sizeof(temp));
	memset(ipAddr,0,sizeof(ipAddr));
	
	while( !isdigit(*ret) || (*ret == '"') || (*ret == ':') )
	{
		ret++;
		len++;
	}
	
	while( isdigit(*ret) )
	{
		*(temp+(i++)) = *(ret++);
		len++;
	}
	temp[i]='\0';
	i = 0;
	if( strlen(temp) != 0 )
		ipAddr[0] = strtoul(temp,NULL,10);
	else
		ipAddr[0] = 0;
	ret++;
	memset(temp,0,sizeof(temp));
	while( isdigit(*ret) )
	{
		*(temp+(i++)) = *(ret++);
		len++;
	}
	temp[i]='\0';
	i = 0;
	if(strlen(temp)!=0)
		ipAddr[1] = strtoul(temp,NULL,10);			
	else
		ipAddr[1] = 0;
	ret++;
	while( isdigit(*ret) )
	{
		*(temp+(i++)) = *(ret++);
		len++;
	}
	temp[i]='\0';
	i = 0;
	if(strlen(temp) != 0)
		ipAddr[2] = strtoul(temp,NULL,10);	
	else
		ipAddr[2] = 0;
	ret++;
	while( isdigit(*ret) )
	{
		*(temp+(i++)) = *(ret++);
		len++;
	}
	temp[i]='\0';
	if( strlen(temp) != 0 )
		ipAddr[3] = strtoul(temp,NULL,10);
	else
		ipAddr[3] = 0;
	if(ipAddr[0] != 0 )
	{
		*Ip = (ipAddr[0]<<24) | (ipAddr[1]<<16) | (ipAddr[2]<<8) | (ipAddr[3]);
		len+=3;
	}
	else
		*Ip = 0;
	return len;	
}

/* Get Data From String */
static uint8_t getNumData(char *ret,void *data)
{
	//char temp[32];

	int *pdata = (int *)data;

	uint8_t i = 0;
	uint8_t len = 0;

	memset(temp,0,sizeof(temp));

	while(!isdigit(*ret) && (*ret != '-'))
	{
		ret++;
		len++;
	}

	while(isspace(*ret))
	{
		ret++;
		len++;
	}

	while(!isspace(*ret) && (*ret != ','))
	{
		temp[i++] = *(ret++);
		len++;
	}

	temp[i] = '\0';
	if( strlen(temp) != 0 )
		*pdata = strtoul(temp,NULL,10);
	return len;
}

/* AT-Command Receive Judge */
EnumAtRetStatus atCmdReceiveProcess( DongleTypeDef *dtd, char *pdata )
{
	EnumAtRetStatus status = AT_ERR;
	char *ret;
	int16_t i = 0;
	//char temp[32];
	memset( temp,0,sizeof(temp) );

	if( strlen(pdata) == 0 )
		return AT_NONE;
	else if( strstr(pdata, "ERROR") != NULL )
		return AT_NO_SUPPORT;

	switch( dtd->_cmd )
	{
		#if  CONFIG_DONGLE_USE_USB
		case _ATE:
			if( strstr(pdata,"OK") != NULL )
				status = AT_OK;
			else
				status = AT_ERR;	
			break;

		case _ATI:
			if( (ret = strstr(pdata,"Manufacturer")) != NULL )
			{
				ret += strlen("Manufacturer:");
				while( isspace(*ret) )
				{
					ret++;	
				}

				i = 0;

				while( (*ret != '\r') && (*ret != '\n') )
				{
					dtd->dongleGeneralStruct.factory[i++] = *(ret++);
				}
				if(i>0)
				{
					dtd->dongleGeneralStruct.factory[i] = '\0';
				#ifdef	_DEBUG
					printf("【%s】Manufacturer:%s\n",dtd->dongleGeneralStruct.factory);
				#endif
				}

				if( ( ret = strstr( pdata, "Model")) != NULL )
				{
					ret += strlen("Model:");	
					while( isspace(*ret) )
					{
						ret++;	
					}

					i = 0;

					while( (*ret != '\r') && (*ret != '\n') )
					{
						dtd->dongleGeneralStruct.model[i++] = *(ret++);
					}
					if(i>0)
					{
						dtd->dongleGeneralStruct.model[i] = '\0';
					#ifdef	_DEBUG
						printf("【%s】Model:%s\n",PROGRAM_NAME,dtd->dongleGeneralStruct.model);
					#endif
					}

				}

				if( ( ret = strstr( pdata, "Revision")) != NULL )
				{
					ret += strlen("Revision:");	
					while( isspace(*ret) )
					{
						ret++;	
					}

					i = 0;

					while( (*ret != '\r') && (*ret != '\n') )
					{
						dtd->dongleGeneralStruct.ver[i++] = *(ret++);
					}
					if(i>0)
					{
						dtd->dongleGeneralStruct.ver[i] = '\0';
					#ifdef	_DEBUG
						printf("【%s】Revision:%s\n",PROGRAM_NAME,dtd->dongleGeneralStruct.ver);
					#endif
					}
				}
				
				if( ( ret = strstr( pdata, "IMEI")) != NULL )
				{
					ret += strlen("IMEI:");	
					while( isspace(*ret) )
					{
						ret++;	
					}

					i = 0;

					while( (*ret != '\r') && (*ret != '\n') )
					{
						dtd->dongleGeneralStruct.imei[i++] = *(ret++);
					}
					if(i>0)
					{
						dtd->dongleGeneralStruct.imei[i] = '\0';
					#ifdef	_DEBUG
						printf("【%s】IMEI:%s\n",PROGRAM_NAME,dtd->dongleGeneralStruct.imei);
					#endif
					}
				}

				status = AT_OK;
			}
			else
				status = AT_ERR;	
			break;

		case _CMEE:
			if( strstr(pdata,"OK") != NULL)
				status = AT_OK;
			else
				status = AT_ERR;	
			break;

		case _COPS:
			if( strstr(pdata,"OK") != NULL)
				status = AT_OK;
			else
				status = AT_ERR;	
			//AT+COPS=0,2,"46021"
			break;

			/*
			 *  signal-			strength
			 *	no signal:		99
			 *	0				0-2
			 *	1				3-6
			 *	2				7-10
			 *	3				11-15
			 *	4				15+
			 */
		case _CSQ:
			if( (ret = strstr(pdata,"+CSQ")) != NULL )
			{
				dtd->dongleRegisterStruct.rssi = 99;
				ret += strlen("+CSQ");
				if( strchr(ret,',') != NULL )
				{
					getNumData(ret,&dtd->dongleRegisterStruct.rssi);
				}
				status = AT_OK;
			}
			else
				status = AT_ERR;
			break;
			
		case _SYSINFO:
			if((ret = strstr(pdata,"^SYSINFO")) != NULL )
			{
				memset(&dtd->dongleSearchStruct,0,sizeof(DongleSearchTypeDef));
				ret += strlen("^SYSINFO");
				ret += getNumData(ret,&dtd->dongleSearchStruct.srvStatus);
				ret += getNumData(ret,&dtd->dongleSearchStruct.srvDomain);
				ret += getNumData(ret,&dtd->dongleSearchStruct.roamStatus);
				ret += getNumData(ret,&dtd->dongleSearchStruct.sysMode);
				ret += getNumData(ret,&dtd->dongleSearchStruct.simStatus);
				status = AT_OK;
			}
			else
				status = AT_ERR;
			break;
		/*Huawei*/			
		case _SYSINFOEX:
			if((ret = strstr(pdata,"^SYSINFOEX")) != NULL )
			{
				memset(&dtd->dongleSearchStruct,0,sizeof(DongleSearchTypeDef));
				ret += strlen("^SYSINFOEX");
				ret += getNumData(ret,&dtd->dongleSearchStruct.srvStatus);
				ret += getNumData(ret,&dtd->dongleSearchStruct.srvDomain);
				ret += getNumData(ret,&dtd->dongleSearchStruct.roamStatus);
				ret += getNumData(ret,&dtd->dongleSearchStruct.sysMode);
				ret += getNumData(ret,&dtd->dongleSearchStruct.simStatus);
				status = AT_OK;
			}
			else
				status = AT_ERR;
			break;

		case _SPLMN:
		case _CURC:
			status = AT_OK;
			break;

		case _CPIN:
			if( dtd->_mode == DONGLE_AT_QUERY )
			{
				if( (ret = strstr(pdata,"READY")) != NULL )
				{
					status = AT_OK;
				}
				else
					status = AT_ERR;
			}
			else
			{
				if( strstr(pdata,"OK") != NULL)
					status = AT_OK;
				else
					status = AT_ERR;
			}
			break;


		case _CFUN:		
			if( dtd->_mode == DONGLE_AT_QUERY )
			{
				if( (ret = strstr(pdata,"+CFUN")) != NULL)
				{
					dtd->dongleSearchStruct.cfun = 0;
					ret += strlen("+CFUN");		//+CFUN=4,airplane,+CFUN=0,close radio
					getNumData(ret,&dtd->dongleSearchStruct.cfun);
					status = AT_OK;
				}
				else	if( strstr(pdata,"+CME") != NULL )
					status = AT_NO_SUPPORT;
				else
					status = AT_ERR;
			}
			else
			{
				if( strstr(pdata,"OK") != NULL )
					status = AT_OK;
				else
					status = AT_ERR;
			}
			break;

		case _CIMI:
			if( strlen(pdata) >= 15 )
			{
				i = 0;
				memset(&dtd->dongleUsimStruct.imsi,0,sizeof(dtd->dongleUsimStruct.imsi));
				ret = pdata;
				while(!isdigit(*ret))
				{
					ret++;
				}
				while(isdigit(*ret))
				{
					dtd->dongleUsimStruct.imsi[i++] = *(ret++);
					if(i==15)
						break;
				}
				dtd->dongleUsimStruct.imsi[i] = '\0';
				status = (strlen((char *)dtd->dongleUsimStruct.imsi) == 15 )? AT_OK:AT_ERR;
			}
			else
				status = AT_ERR;
			break;
		
		case _CGDCONT:
			if( strstr(pdata,"OK") != NULL)
				status = AT_OK;
			else
				status = AT_ERR;
			break;
		
		case _CGPADDR:
			if( (ret = strstr(pdata,"+CGPADDR")) != NULL )
			{
				//memset(&dtd->dongleRegisterStruct.ipAddr,0,sizeof(dtd->dongleRegisterStruct.ipAddr));
				dtd->dongleRegisterStruct.ipAddr = 0;
				ret += strlen("+CGPADDR");
				if( (ret=strstr(ret,",")) != NULL )
				{
					ret++;
					getIpaddr(ret,&dtd->dongleRegisterStruct.ipAddr);
					status = AT_OK;
				}		
				else
					status = AT_ERR;
			}
			else
				status = AT_ERR;
			break;
	
		case _CGATT:
			if( dtd->_mode == DONGLE_AT_QUERY )
			{
				if( (ret = strstr(pdata,"+CGATT")) != NULL )
				{
					dtd->dongleRegisterStruct.registerStatus= 0;				
					ret += strlen("+CGATT");
					ret += getNumData(ret,&dtd->dongleRegisterStruct.registerStatus);
					status = AT_OK;
				}
				else
					status = AT_ERR;
			}
			else
			{
				if( strstr(pdata,"OK") != NULL)
					status = AT_OK;
				else
					status = AT_ERR;
			}
			break;
		
		case _CGACT:
			if( dtd->_mode == DONGLE_AT_QUERY )
			{
				if ( (ret = strstr(pdata,"+CGACT")) != NULL )
				{
					dtd->dongleRegisterStruct.registerStatus = 0;				
					ret	+= strlen("+CGACT");
					if( (ret = strstr(ret,",")) != NULL )
					{
						if( isdigit(*(++ret)) )
							dtd->dongleRegisterStruct.registerStatus = *(ret) - '0';			//eps bear 0-de 1-act
					}
					status = AT_OK;
				}
				else
					status = AT_ERR;
			}
			else
			{
				if( strstr(pdata,"OK") != NULL)
					status = AT_OK;
				else
					status = AT_ERR;
			}
			break;
	
		case _CEREG:
				if(dtd->_mode == DONGLE_AT_QUERY)
				{
					if ( (ret = strstr(pdata,"+CEREG")) != NULL )
					{ 
						i = 0;
						dtd->dongleRegisterStruct.registerStatus = 0;
						dtd->dongleRegisterStruct.lac = 0;
						dtd->dongleRegisterStruct.cellId = 0;
						
						ret	+= strlen("+CEREG");
						if( (ret = strstr(ret,",")) != NULL )
						{
							/*0-no attach or search*/
							/*1-loca register*/
							/*2-attach and search*/
							/*3-attach reject*/
							/*4-unknow*/		
							/*5-register,roam*/
							ret++;
							ret += getNumData(ret,&dtd->dongleRegisterStruct.registerStatus);	
							status = AT_OK;
					}
					else
						status = AT_ERR;			
				}					
			}
			else
			{
				if( strstr(pdata,"OK") != NULL  )
					status = AT_OK;
				else
					status = AT_ERR;
			}
			break;

		case _CGREG:
			if(dtd->_mode == DONGLE_AT_QUERY)
			{
				if ( (ret = strstr(pdata,"+CGREG")) != NULL )
				{ 
					i = 0;
					dtd->dongleRegisterStruct.registerStatus = 0;
					dtd->dongleRegisterStruct.lac = 0;
					dtd->dongleRegisterStruct.cellId = 0;
					
					ret	+= strlen("+CGREG");
					if( (ret = strstr(ret,",")) != NULL )
					{
						/*0-no attach or search*/
						/*1-loca register*/
						/*2-attach and search*/
						/*3-attach reject*/
						/*4-unknow*/		
						/*5-register,roam*/
						ret++;
						ret += getNumData(ret,&dtd->dongleRegisterStruct.registerStatus);	
						if( strstr(ret,"\"") != NULL )
						{
							ret++;
							while( isspace(*ret) || *(ret) == '\"' )
							{
								ret++;	
							}

							memset(temp,0,sizeof(char)*32);

							while( *ret != '\"' )
							{
								temp[i++] = *(ret++);
							}

							if(i>0)
							{
								temp[i] = '\0';
								dtd->dongleRegisterStruct.lac = strtoul(temp,NULL,16);
								#ifdef	_DEBUG
								printf("【%s】LAC=%x\n",PROGRAM_NAME,dtd->dongleRegisterStruct.lac);
								#endif
								memset(temp,0,sizeof(char)*32);

								i = 0;

								if( strstr(ret,",") != NULL )
								{
									ret++;
									if( strstr(ret,"\"") != NULL )
									{
										ret++;
										while( isspace(*ret) || *(ret) == '\"' )
										{
											ret++;	
										}

										memset(temp,0,sizeof(char)*32);

										while( isdigit(*ret) || isalpha(*ret) )
										{
											temp[i++] = *(ret++);
										}

										if(i>0)
										{
											temp[i] = '\0';
											dtd->dongleRegisterStruct.cellId = strtoul(temp,NULL,16);
											#ifdef	_DEBUG
											printf("【%s】PCI=%x\n",PROGRAM_NAME,dtd->dongleRegisterStruct.cellId);
											#endif
											memset(temp,0,sizeof(char)*32);
										}
									}
								}
							}
						}
						status = AT_OK;
				}
				else
					status = AT_ERR;			
				}					
			}
			else
			{
				if( strstr(pdata,"OK") != NULL  )
					status = AT_OK;
				else
					status = AT_ERR;
			}
			break;

		/*Gosun 800*/
		case _ZCELLINFO:
			if( (ret = strstr(pdata,"+ZCELLINFO")) != NULL )
			{
				ret += strlen("+ZCELLINFO");

				if( ( ret = strstr( ret, "tac")) != NULL )
				{
					ret += strlen("tac");	

					if( (ret = strstr(ret,"0X")) != NULL )
						ret += strlen("0X");
					
					else if( (ret = strstr(ret,"0x")) != NULL )
						ret += strlen("0x");
					else
						break;

					i = 0;

					while( isdigit(*ret) || ( ( *ret >= 'A' ) && ( *ret <= 'F' )) || ( ( *ret >= 'a' ) && ( *ret <= 'f' ) ) )
					{
						temp[i++] = *(ret++);
					}

					temp[i] = '\0';
					if( i > 0 )
						dtd->dongleRegisterStruct.lac = strtoul(temp,NULL,16);
					memset(temp,0,sizeof(temp));
				}


				if(  ( ret = strstr( pdata, "cellid") )  != NULL )
				{
					ret += strlen("cellid");	
					if( ( ret = strstr( ret,"0X") ) != NULL )
						ret += strlen("0X");
					else if( ( ret = strstr( ret,"0x") ) != NULL )
						ret += strlen("0x");
					else
						break;

					i = 0;

					while( isdigit(*ret) || ( ( *ret >= 'A' ) && ( *ret <= 'F' )) || ( ( *ret >= 'a' ) && ( *ret <= 'f' ) ) )
					{
						temp[i++] = *(ret++);
					}

					temp[i] = '\0';
					if( i>0 )
						dtd->dongleRegisterStruct.cellId = strtoul(temp,NULL,16);
					memset(temp,0,sizeof(temp));
				}
				else if(  ( ret = strstr( pdata, "cell_id") )  != NULL )
				{
					ret += strlen("cell_id");	
					while( isspace(*ret) )
					{
						ret++;	
					}

					i = 0;

					while( isdigit(*ret) || ( ( *ret >= 'A' ) && ( *ret <= 'F' )) || ( ( *ret >= 'a' ) && ( *ret <= 'f' ) ) )
					{
						temp[i++] = *(ret++);
					}

					temp[i] = '\0';
					if( i>0 )
						dtd->dongleRegisterStruct.cellId = strtoul(temp,NULL,16);
					memset(temp,0,sizeof(temp));

				}
				else
					return AT_OK;

				if( (( ret = strstr( pdata, "band") ) != NULL ) || (( ret = strstr( pdata, "Band") ) != NULL) )
				{
					ret += strlen("band");	

					while( !isdigit(*ret) )
					{
						ret++;	
					}
					i = 0;
					while(isdigit(*ret))
					{
						temp[i++] = *(ret++);
					}

					temp[i] = '\0';
					
					if( i>=0 )
					{
						//ret += getNumData(ret,&mdtd->dongleFreqStruct.bandId)
						dtd->dongleFreqStruct.bandId = atoi(temp);	
					}
					memset(temp,0,sizeof(temp));
				}
/*
				if(  ( ret = strstr( ret, "cellid") )  != NULL )
				{
					ret += strlen("cellid");	
					if( ( ret = strstr( ret,"0X") ) != NULL )
						ret += strlen("0X");
					else if( ( ret = strstr( ret,"0x") ) != NULL )
						ret += strlen("0x");
					else
						break;

					i = 0;

					while( isdigit(*ret) || ( ( *ret >= 'A' ) && ( *ret <= 'F' )) || ( ( *ret >= 'a' ) && ( *ret <= 'f' ) ) )
					{
						temp[i++] = *(ret++);
					}

					temp[i] = '\0';
					if( i>0 )
						dtd->dongleRegisterStruct.cellId = strtoul(temp,NULL,16);
					memset(temp,0,sizeof(temp));
				}

				if( ( ret = strstr( ret, "band") ) != NULL )
				{
					ret += strlen("band");	

					while( !isdigit(*ret) )
					{
						ret++;	
					}
					i = 0;
					while(isdigit(*ret))
					{
						temp[i++] = *(ret++);
					}

					temp[i] = '\0';
					
					if( i>=0 )
					{
						ret += getNumData(ret,&dtd->dongleFreqStruct.bandId);	
					}
					memset(temp,0,sizeof(temp));
				}
				*/
				status = AT_OK;
			}
			break;

		case _ZECMCALL:
			status = AT_OK;
			break;
	
		case _ZGRST:
		case _ZSNT:
			status = AT_OK;
			break;
		case _ZPAS:
			if( strstr(pdata,"NO SERVICE") != NULL )
				status = AT_ERR;
			else if( strstr(pdata,"LIMITED SERVICE") != NULL )
				status = AT_ERR;
			else
				status = AT_OK;
			break;
			
		case _ZRST:
			if( strstr(pdata,"OK") != NULL )
				status = AT_OK;
			else
				status = AT_ERR;
			break;

		case _RESET:		//AT^RESET or AT+ZSOFTRESET
			if( strstr(pdata,"OK") != NULL )
				status = AT_OK;
			else
				status = AT_ERR;
			break;

		/*Huawei*/
		case _CSGQRY:
			if( (ret = strstr(pdata,"^CSGQRY")) != NULL )
			{
				dtd->dongleRegisterStruct.cellId = 0;				
				dtd->dongleRegisterStruct.freq = 0;				
				dtd->dongleRegisterStruct.rsrp = 0;				
				dtd->dongleRegisterStruct.rsrq = 0;				
				dtd->dongleRegisterStruct.rssi = 0;
				
				ret += strlen("^CSGQRY");
				ret+=getNumData(ret,&dtd->dongleRegisterStruct.cellId);
				ret+=getNumData(ret,&dtd->dongleRegisterStruct.freq);
				ret+=getNumData(ret,&dtd->dongleRegisterStruct.rsrp);
				ret+=getNumData(ret,&dtd->dongleRegisterStruct.rsrq);
				ret+=getNumData(ret,&dtd->dongleRegisterStruct.rssi);
				ret+=getNumData(ret,&dtd->dongleRegisterStruct.gtchBler);
				//ret++;
				ret+=getNumData(ret,&dtd->dongleRegisterStruct.gpcchBler);
				ret+=getNumData(ret,&dtd->dongleRegisterStruct.sinr);
				/*scanCell(ret, &dtd->dongleRegisterStruct.cellId,\
											&dtd->dongleRegisterStruct.freq,\
											&dtd->dongleRegisterStruct.rsrp,\
											&dtd->dongleRegisterStruct.rsrq,\
										    &dtd->dongleRegisterStruct.rssi,\
										    &dtd->dongleRegisterStruct.cellType);
											*/
				status = AT_OK;
			}
			else
				status = AT_ERR;
			break;

	
		case _NDISDUP:
			if( strstr(pdata,"OK") != NULL)
				status = AT_OK;
			else
				status = AT_ERR;			
			break;
		
		case _NDISSTATQRY:
			if( (ret = strstr(pdata,"^NDISSTATQRY") ) != NULL )
			{
				ret += strlen("^NDISSTATQRY");
				while( !isdigit( *ret) )
				{
					ret++;
				}
				if( isdigit(*ret) )
					dtd->dongleRegisterStruct.registerStatus = *ret - 30;
				status = AT_OK;
			}
			else
				status = AT_ERR;
			break;
			
		case _ABINFO:
			if( dtd->_mode == DONGLE_AT_QUERY )
			{
				if( (ret = strstr(pdata,"^ABINFO")) != NULL )
				{
					dtd->dongleFreqStruct.airBandId = 0;
					dtd->dongleFreqStruct.bandId = 0;
					dtd->dongleFreqStruct.freqLow = 0;
					dtd->dongleFreqStruct.freqHigh = 0;
					dtd->dongleFreqStruct.earfcnLow = 0;
					dtd->dongleFreqStruct.earfcnHigh = 0;

					ret += strlen("^ABINFO");
					ret += getNumData(ret,&dtd->dongleFreqStruct.airBandId);					
					ret += getNumData(ret,&dtd->dongleFreqStruct.bandId);
					ret += getNumData(ret,&dtd->dongleFreqStruct.freqLow);
					ret += getNumData(ret,&dtd->dongleFreqStruct.freqHigh);
					ret += getNumData(ret,&dtd->dongleFreqStruct.earfcnLow);
					ret += getNumData(ret,&dtd->dongleFreqStruct.earfcnHigh);
					status = AT_OK;
				}
				else
					status = AT_ERR;
			}
			else
			{
				if( strstr(pdata,"OK") != NULL )
					status = AT_OK;
				else
					status = AT_ERR;
			}
		break;
			
		case _LBINFO:
			if( (ret = strstr(pdata,"^LBINFO")) != NULL )
			{
				dtd->dongleFreqStruct.bandId = 0;
				dtd->dongleFreqStruct.freqLow = 0;
				dtd->dongleFreqStruct.freqHigh = 0;
				dtd->dongleFreqStruct.earfcnLow = 0;
				dtd->dongleFreqStruct.earfcnHigh = 0;
				ret += strlen("^LBINFO");
				ret += getNumData(ret,&dtd->dongleFreqStruct.bandId);
				ret += getNumData(ret,&dtd->dongleFreqStruct.freqLow);
				ret += getNumData(ret,&dtd->dongleFreqStruct.freqHigh);
				ret += getNumData(ret,&dtd->dongleFreqStruct.earfcnLow);		
				ret += getNumData(ret,&dtd->dongleFreqStruct.earfcnHigh);		
				status = AT_OK;
			}
			else
				status = AT_ERR;
			break;
		
		case _ANTPC:
			if( strstr(pdata,"OK") != NULL )
				status = AT_OK;
			else
				status = AT_ERR;		
			break;
			
		case _SSWEN:
			if( strstr(pdata,"OK") != NULL )
				status = AT_OK;
			else
				status = AT_ERR;
			break;

		case _SSIM:
			if( strstr(pdata,"OK") != NULL )
				status = AT_OK;
			else
				status = AT_ERR;			
			break;

		
		case _CGEV:
			if( strstr(pdata,"+CGEV:PDN ACT") != NULL )
			{
				dtd->dongleRegisterStruct.registerStatus = 1;
				status = AT_OK;
			}
			else if( strstr(pdata,"+CGEV:PDN DEACT") != NULL )
			{
				dtd->dongleRegisterStruct.registerStatus = 0;
				status = AT_OK;
			}
			else if( strstr(pdata,"+CME") != NULL )
				status = AT_NO_SUPPORT;
			else
				status = AT_ERR;
			break;
			
		case _ZSINR:
			if( (ret = strstr(pdata,"+ZSINR")) != NULL )
			{
					dtd->dongleRegisterStruct.sinr = 0;
					i = 0;
					ret += getNumData(ret,&i);
					ret += getNumData(ret,&dtd->dongleRegisterStruct.sinr);	
					dtd->dongleRegisterStruct.sinr = i/abs(i)*(dtd->dongleRegisterStruct.sinr+100*abs(i));
					status = AT_OK;
			}
			else
				status = AT_ERR;	
			break;

		#endif
			
			
		#if	CONFIG_MODEM_USE_USART
		/*Serial-Extension-Begin*/		
		case _DSMS:
			if( (ret = strstr(pdata,"+DSMS")) != NULL )
			{
				if( strstr( pdata,"0x00000004") != NULL )
				{
					dtd->dongleRegisterStruct.registerStatus= 1;
				}
				else
					dtd->dongleRegisterStruct.registerStatus= 0;
				status = AT_OK;
			}
			else
				status = AT_ERR;
			break;
			
			case _CCLK:
				if( (ret = strstr(pdata,"+CCLK")) != NULL )
				{
					i = 0;
					ret += strlen("+CCLK");
					
					while(!isdigit(*ret))
					{
						ret++;
					}				
					while((*ret != '\n')&&(*ret != '\r'))
					{
						;//dtd->dongleRegisterStruct.mNetTime[i++] = *(ret++);
					}
					;//dtd->dongleRegisterStruct.mNetTime[i] = '\0';
					status = AT_OK;
				}
				else
					status = AT_ERR;				
				break;
				
				case _RATE:
					if( strstr(pdata,"OK") != NULL )
						status = AT_OK;
					else
						status = AT_ERR;		
					break;
				
				case _OFF:
					if( strstr(pdata,"OK") != NULL)
						status = AT_OK;
					else
						status = AT_ERR;		
					break;
				
				case _VER:
					if( (ret = strstr(pdata,"+VER")) != NULL )
					{
						i = 0;
						ret += strlen("+VER");
						while(isspace(*ret) || (*ret == ':'))
						{
							ret++;
						}
						while((*ret != '\n')&&(*ret != '\r'))
						{
							dtd->dongleGeneralStruct.sver[i++] = *(ret++);
						}
						dtd->dongleGeneralStruct.sver[i] = '\0';
						status = (strlen((char *)dtd->dongleGeneralStruct.sver)==5)?AT_OK:AT_ERR;
					}
					else	if( strstr(pdata,"+CME") != NULL )
						status = AT_NO_SUPPORT;
					else
						status = AT_ERR;						
					break;
					
				case _USLP:
					if( dtd->_mode == DONGLE_AT_QUERY )
					{
						if( strstr(pdata,"OK") != NULL )
							status = AT_OK;
						else
							status = AT_ERR;
					}	
					else
					{
						if( strstr(pdata,"OK") != NULL )
							status = AT_OK;
						else
							status = AT_ERR;						
					}
					break;
				
				case _ZCAPN:
					if( strstr(pdata,"OK") != NULL)
						status = AT_OK;
					else
						status = AT_ERR;	
					break;
				
				case _ZIPCALL:
					if( (ret = strstr(pdata,"+ZIPCALL")) != NULL )
					{
						ret += strlen("+ZIPCALL");
						ret += getNumData(ret,&dtd->dongleRegisterStruct.registerStatus);
						getIpaddr(ret,&dtd->dongleRegisterStruct.ipAddr);
						if( dtd->dongleRegisterStruct.ipAddr == 0 )
							status = AT_ERR;
						else
							status = AT_OK;
					}
					else
						status = AT_ERR;
					break;
					
					case _ZIPOPEN:
						/* when open connection, Firstly check array*/
						if( (ret = strstr(pdata,"+ZIPSTAT")) != NULL)
						{
							ret += strlen("+ZIPSTAT");
							/*socket id	:	1-5*/
							/*type id		:	0->tcp	1->udp*/
							ret += getNumData(ret,&dtd->dongleSrvStruct.socketId);
							ret += getNumData(ret,&dtd->dongleSrvStruct.sendFlag);
							status = AT_OK;
						}
						else
							status = AT_ERR;
						break;
					
					case _ZIPSEND:
						if( (ret = strstr(pdata,"+ZIPSEND")) != NULL )
						{
							ret += strlen("+ZIPSEND");
							ret += getNumData(ret,&dtd->dongleSrvStruct.socketId);
							ret += getNumData(ret,&dtd->dongleSrvStruct.sendFlag);
							status = AT_OK;
						}
						else
							status = AT_ERR;
						break;
					
					case _ZIPCLOSE:
						if( strstr(pdata,"OK") != NULL )
							status = AT_OK;
						else
							status = AT_ERR;
						break;
					
					case _ZIPSLCFG:
						if( strstr(pdata,"OK") != NULL )
							status = AT_OK;
						else
							status = AT_ERR;
						break;
					
					case _ZIPLISTEN:
						if( dtd->_mode == DONGLE_AT_SET )
						{
							if( strstr(pdata,"OK") != NULL )
								status = AT_OK;
							else
								status = AT_ERR;							
						}
						else
						{
							if( (ret = strstr(pdata,"+ZIPLISTEN")) != NULL )
							{
								i = 0;
								ret += strlen("+ZIPLISTEN");
								ret += getNumData(ret,&i);
								ret += getNumData(ret,&i);		//type
								ret += getNumData(ret,&dtd->dongleSrvStruct.socketId);
								ret += getIpaddr(ret,&dtd->dongleSrvStruct.serverIp);
								ret += getNumData(ret,&dtd->dongleSrvStruct.serverPort);
								ret += getNumData(ret,&i);
								if( (ret = strstr(ret,",")) != NULL )
									ret++;
								if(i)
									memcpy(dtd->dongleSrvStruct.mRecvDataPtr,ret,i);
								status = AT_OK;
							}		
							else
								status = AT_ERR;								
						}
						break;
					
					case _ZIPRECV:
						if( (ret = strstr(pdata,"+ZIPRECV")) != NULL )
						{
							i = 0;
							ret += strlen("+ZIPRECV");
							ret += getNumData(ret,&dtd->dongleSrvStruct.socketId);
							ret += getIpaddr(ret,&dtd->dongleSrvStruct.serverIp);
							ret += getNumData(ret,&dtd->dongleSrvStruct.serverPort);
							ret += getNumData(ret,&i);
							if( (ret = strstr(ret,",")) != NULL )
								ret++;
							if(i)
								memcpy(dtd->dongleSrvStruct.mRecvDataPtr,ret,i);
							status = AT_OK;
						}
						else
							status = AT_ERR;	
						break;
						
				#endif
	/*Serial-Extension-Emd*/		
					
			default:
				status = AT_ERR;
				break;
		}
		return status;
}


