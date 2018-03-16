/*******************************************************************************
 *   Filename:      bsp_rst.c
 *   Revised:       $Date: 2013-06-30
 *   Revision:      $V1.0
 *   Writer:        Roger-WY.
 *
 *   Description:
 *
 *   Notes:         系统重启驱动模块 头文件
 *
 *
 *   All copyrights reserved to Roger-WY.
 *
 *******************************************************************************/
#ifndef __BSP_WDT_H__
#define __BSP_WDT_H__

#include "stm32f10x.h"

/*******************************************************************************
 * 描述： 外部函数调用
 */
void  BSP_RST_SystemRst(void);

#endif