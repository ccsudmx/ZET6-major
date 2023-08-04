#ifndef __DEBUG_PRINTF_H__
#define __DEBUG_PRINTF_H__
#include <stdio.h>
#include "systick.h"

#define filename(x) strrchr(x,'\\')?strrchr(x,'\\')+1:x


#define __FILE_NAME__	filename(__FILE__)

#define _PREFIX_I	"[INFO]: "
#define _PREFIX_W	"[WARN]: "
#define _PREFIX_E	"[ERRO]: "

#define _APPEND_FIX "\r\n"

#define _FILE_INFO	"[%ld][%8s][%8s][%-3d]"
#define _FILE_INFO_ARG	SYS_MS_FUNC,__FILE_NAME__,__FUNCTION__,__LINE__

#define COLOR_RED  "\033[1;31;40m"
#define COLOR_GRE  "\033[1;32;40m"
#define COLOR_BLU  "\033[1;34;40m"
#define COLOR_WHI  "\033[1;37;40m"
#define COLOR_YEL  "\033[1;33;40m"
#define COLOR_END  "\033[0m"

#define flag_enable_cpf 0 

#define ENABLE_PROTOCOL 1
#define DISABLE_PROTOCOL 0

#define ENABLE_SD_LOG 1
#define DISABLE_SD_LOG 0
#define SYS_MS_FUNC  SYSTICK_get_time()
#define FIX_PARAM           ENABLE_PROTOCOL,ENABLE_SD_LOG


#define DBG_G_INFO(fmt,args...) 								  	do{ if(flag_enable_cpf==0) {printf(_FILE_INFO 	_PREFIX_I	fmt _APPEND_FIX,_FILE_INFO_ARG,##args);} \
else                   {printf("%s"_FILE_INFO 	_PREFIX_I	fmt COLOR_END _APPEND_FIX,	COLOR_GRE,_FILE_INFO_ARG,##args);}}while(0)


#define DBG_B_INFO(fmt,args...) 								  	do{ if(flag_enable_cpf==0) {printf(_FILE_INFO 	_PREFIX_I	fmt _APPEND_FIX,_FILE_INFO_ARG,##args);} \
else                   {printf("%s"_FILE_INFO 	_PREFIX_I	fmt COLOR_END _APPEND_FIX,	COLOR_BLU,_FILE_INFO_ARG,##args);}}while(0)



#define DBG_Y_WARN(fmt,args...) 									do{ if(flag_enable_cpf==0) {printf(_FILE_INFO 	_PREFIX_W	fmt _APPEND_FIX,_FILE_INFO_ARG,##args);} \
else                   {printf("%s"_FILE_INFO 	_PREFIX_W	fmt COLOR_END _APPEND_FIX,	COLOR_YEL,_FILE_INFO_ARG,##args);}}while(0)


#define DBG_R_E(fmt,args...) 									    do{ if(flag_enable_cpf==0) {printf(_FILE_INFO 	_PREFIX_E	fmt _APPEND_FIX,_FILE_INFO_ARG,##args);} \
else                   {printf("%s"_FILE_INFO 	_PREFIX_E	fmt COLOR_END _APPEND_FIX,	COLOR_RED,_FILE_INFO_ARG,##args);}}while(0)
#define DBG_E_LIMIT(ms,fmt,args...) RUN_BY_LIMIT_BLOCK(ms,	DBG_R_E(fmt, ##args))


#define _PREFIX_ASSERT	"[ASSERT]: "

#endif //__DEBUG_H__


