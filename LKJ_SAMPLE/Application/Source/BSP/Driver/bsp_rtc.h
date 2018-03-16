/*******************************************************************************
 *   Filename:      bsp_rtc.h
 *   Revised:       $Date: 2013-06-30
 *   Revision:      $V1.0
 *   Writer:        Roger-WY.
 *
 *   Description:
 *
 *   Notes:         STM32F103内部RTC驱动模块头文件
 *
 *
 *   All copyrights reserved to Roger-WY
 *
 *******************************************************************************/

#ifndef	BSP_RTC_H__
#define	BSP_RTC_H__
/*******************************************************************************
 * INCLUDES
 */
#include  <global.h>
#include  "includes.h"



/*******************************************************************************
 * 描述： 外部函数调用
 */
BOOL    BSP_RTC_Init            (void);
void    BSP_RTC_Main            (void);

BOOL    BSP_RTC_GetTime         (INT8U *time);   //数据从高位到低位//
BOOL    BSP_RTC_SetTime         (INT8U *time);   //数据从高位到低位//
void    BSP_RTC_DispTime        (void);

void    RTC_Configuration       (void);

/*******************************************************************************
 * EXTERN VARIABLES
 */


/*******************************************************************************
 *              end of file                                                    *
 *******************************************************************************/
#endif