#ifndef __APP_DBG_H__
#define __APP_DBG_H__

#include <stdio.h>
#include "../sys/sys_dbg.h"

#define APP_PRINT_EN		1
#define APP_DBG_EN			1

#if (APP_PRINT_EN == 1)
#define APP_PRINT(fmt, ...)		printf(fmt, ##__VA_ARGS__)
#else
#define APP_PRINT(fmt, ...)
#endif

#if (APP_DBG_EN == 1)
#define APP_DBG(fmt, ...)		__LOG__(fmt, "APP_DBG", ##__VA_ARGS__)
#else
#define APP_DBG(fmt, ...)
#endif

#endif //__APP_DBG_H__
