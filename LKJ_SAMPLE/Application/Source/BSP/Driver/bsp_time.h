
/*******************************************************************************
 *   Filename:       bsp_time.h
 *   Revised:        $Date: 2014-05-27$
 *   Revision:       $
 *	 Writer:		 Roger-WY.
 *
 *   Description:
 *   Notes:
 *
 *
 *   All copyrights reserved to Roger-WY.
 *
 *******************************************************************************/

#ifndef	__BSP_TIME_H__
#define	__BSP_TIME_H__

/*******************************************************************************
 * INCLUDES
 */
#include  "includes.h"
#include  "global.h"
#include  <time.h>   /*基于ANSI-C的标准time.h*/


/*******************************************************************************
 * 描述：  从tm_now结构体中读出实时时间
 */
extern struct tm   tm_now;


/*******************************************************************************
 * GLOBAL FUNCTIONS
 */
struct tm    TIME_ConvUnixToCalendar (time_t t);
time_t       TIME_ConvCalendarToUnix (struct tm t);
time_t       TIME_GetUnixTime        (void);
void         TIME_SetUnixTime        (time_t);


struct tm    TIME_GetCalendarTime    (void);          /*获取实时时间*/
void         TIME_SetCalendarTime    (struct tm t);   /*设置实时时间*/
void         BSP_TIME_Init           (void);          /*实时时间初始化*/



/*******************************************************************************
 *              end of file                                                    *
 *******************************************************************************/
#endif