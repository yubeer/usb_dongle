#include	"uciProcess.h"
#include	<string.h>
static	FILE * uci_fp;
static	char uci_param[128];
/*
bool	uci_get_string( char *cfgName, char *optionName, char *value )
{
	struct	uci_context *ctx = NULL;
	struct	uci_ptr *ptr = NULL;
	struct  uci_package *pkg = NULL;
	struct	uci_element *element = NULL;
	struct	uci_section	*section = NULL;
	struct	uci_option	*option = NULL;

	bool ret = false;

	const char *pValue = NULL;
	ctx = uci_alloc_context();

	if( UCI_OK != uci_load( ctx, cfgName, &pkg) )
		goto clean;
	
    uci_foreach_element(&pkg->sections, element)  
    {  
        section = uci_to_section(element);  
        if (NULL != ( pValue = uci_lookup_option_string(ctx, section, optionName)))  
		{
			memcpy(value,pValue,strlen(pValue));
			ret = true;
			goto clean;
		}
    }  
	goto clean;

clean:
	uci_unload( ctx, pkg );
	uci_free_context(ctx);
	return ret;
}

bool	uci_set_string( char *cfgName, char *sectionName, char *optionName,const char *value )
{
	struct	uci_context *ctx = NULL;
	struct	uci_ptr *ptr = NULL;
//	struct	uci_ptr ptr;
	struct  uci_package *pkg = NULL;
	struct	uci_section	*section = NULL;
	struct	uci_element *element = NULL;
	struct	uci_option	*option = NULL;

	ptr = ( struct uci_ptr *)malloc(sizeof(struct uci_ptr));
	bool ret = false;
	ctx = uci_alloc_context();

	if( UCI_OK != uci_load( ctx, cfgName, &pkg) )
		goto clean;

	ptr->package = cfgName;
	ptr->section = sectionName;
	ptr->option = optionName;
	ptr->value = value;
	ptr.package = cfgName;
	ptr.section = sectionName;
	ptr.option = optionName;
	ptr.value = value;

	//if ( UCI_OK != uci_set( ctx, &ptr) )
	if ( UCI_OK != uci_set( ctx, ptr) )
		goto clean;
	
	if ( UCI_OK != uci_commit( ctx, &ptr->p,true) )
	//if ( UCI_OK != uci_commit( ctx, &ptr.p,true) )
		goto clean;

	ret = true;
	free(ptr);
	goto clean;

clean:
	uci_unload( ctx, pkg );
	free(ptr);
	uci_free_context(ctx);
	return ret;
}
*/

bool	uci_get_string( char *cfgName, char *optionName, char *value )
{

	 struct	uci_context *ctx = NULL;
	 struct	uci_element *e;
	 struct	uci_package *pkg = NULL;
	 const char *pValue = NULL;


    ctx = uci_alloc_context(); // ????????????UCI?????????.
	if (UCI_OK != uci_load(ctx, cfgName, &pkg))
    //if (UCI_OK != uci_load(ctx, cfgName, &ptr->p))
        goto cleanup; //????????????UCI????????????,??????????????? ?????? UCI ?????????.


    /*??????UCI???????????????*/
    uci_foreach_element(&pkg->sections, e)
    {
        struct uci_section *s = uci_to_section(e);
        // ????????? element ????????? section??????, ?????????????????????,??? s->anonymous ??? false.
        // ???????????? s->e->name ?????????.
        // ?????? ??????????????? uci_lookup_option()????????? ????????????????????????.
        if (NULL != (pValue = uci_lookup_option_string(ctx, s, optionName)))
        {
            //value = strdup(pValue); //???????????????????????????????????????????????????????????? pkg?????????value????????????????????????
			memcpy(value,pValue,strlen(pValue));
			return true;
        }
        // ????????????????????? string?????? ??????????????? uci_lookup_option() ????????????Option ???????????????.
        // Option ???????????? UCI_TYPE_STRING ??? UCI_TYPE_LIST ??????.


    }
    uci_unload(ctx, pkg); // ?????? pkg
cleanup:
    uci_free_context(ctx);
    ctx = NULL;
	return false;
}


bool	uci_set_string( char *cfgName, char *sectionName, char *optionName, const char *value )
{
	uint8_t ret = 0;
	struct	uci_context *ctx = uci_alloc_context(); //???????????????  
	struct	uci_ptr ptr;

	memset( &ptr, 0, sizeof(struct uci_ptr) );

	ptr.package = cfgName;
	ptr.section = sectionName;
	ptr.option = optionName;
	ptr.value = value;
	  
    if( UCI_OK != uci_set(ctx,&ptr) ) //????????????  
	{
		#ifdef	_DEBUG
		printf("???%s???set %s failure!\n", PROGRAM_NAME,cfgName);
		#endif
		return false;
	}
    if ( UCI_OK != (ret = uci_commit(ctx, &ptr.p, false)) )//??????????????????  
	{
		#ifdef	_DEBUG
		printf("???%s???commit %s failure!\n", PROGRAM_NAME,cfgName);
		#endif
		return false;
	}
	uci_unload(ctx,ptr.p);
    uci_free_context(ctx); //???????????????  
	ctx = NULL;
	return true;
}

bool	uci_set_by_popen( char *setKey, char *commitKey, char *value )
{
	memset(uci_param,0,sizeof(char)*128);
	memcpy(uci_param,setKey,strlen(setKey));
	strcat(uci_param,value);
	strcat(uci_param,"\"");
	uci_fp = popen( uci_param, "w");
	pclose(uci_fp);	

	uci_fp = popen(commitKey,"w");
	pclose(uci_fp);	
	#ifdef	_DEBUG
	printf("???%s???uci_param commit is %s\n", PROGRAM_NAME,commitKey);
	#endif
	return true;
}

bool	uci_get_by_popen( char *getKey, char *value )
{
	bool ret = true;
	memset(uci_param,0,sizeof(char)*128);
	memcpy(uci_param,getKey,strlen(getKey));
	//strcat(uci_param,value);
	uci_fp = popen( uci_param, "r");
	#ifdef	_DEBUG
	printf("???%s???uci_param commit is %s\n", PROGRAM_NAME,getKey);
	#endif
	if ( fgets(value,128,uci_fp) != NULL )
	{
	#ifdef	_DEBUG
	printf("???%s???uci_param key value is %s\n", PROGRAM_NAME,getKey);
	#endif
		ret = true;
	}
	pclose(uci_fp);
	return ret;
}
