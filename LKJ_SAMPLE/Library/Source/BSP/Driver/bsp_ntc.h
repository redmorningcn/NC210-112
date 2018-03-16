/*******************************************************************************
 *   Filename:      bsp_ntc.h
 *   Revised:       $Date: 2015-03-11
 *   Revision:      $
 *   Writer:        Roger-WY.
 *
 *   Description:   NTC温度测量  （25摄氏度时阻值为10K）
 *
 *   Notes:
 *
 *
 *   All copyrights reserved to Roger-WY.
 *
 *******************************************************************************/
#ifndef __BSP_NTC_H__
#define __BSP_NTC_H__

#include  <stm32f10x.h>

/***********************************************
* 描述： 函数申明
*/
int ntctemp(u32 res);


#endif