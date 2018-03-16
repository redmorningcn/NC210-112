/*******************************************************************************
 *   Filename:      app_voltage_detect.c
 *   Create date: 2017-07-11
 *
 *   Author:        fourth peng
 *
 *   Description:  LKJ检测装置电池电压以及工况信号采集
 *   Notes:
 *
 *
 *   All copyrights reserved to changsha nanche electronics LTD.
 *
 *******************************************************************************/
#include "global.h"
#include "bsp_pm25.h"
#include "bsp_adc.h"
#include "app_voltage_detect.h"

/*******************************************************************************
 * 名    称： app_voltage_detect_init
 * 功    能： 电压检测设定初始化
 * 入口参数： 无
 * 出口参数： 无
 * 作    者：	 fourth.peng
 * 创建日期： 2017-05-3
 * 修    改记录:
 *******************************************************************************/
void app_voltage_detect_init()
{
	Bsp_ADC_Init();
}


/*******************************************************************************
 * 名    称： app_get_voltage
 * 功    能：定时器100ms 回调函数
 * 入口参数： 无
 * 出口参数： 无
 * 作    者：	 fourth.peng
 * 创建日期： 2017-05-3
 * 修    改记录:
 *******************************************************************************/
void app_get_voltage(uint8 ch)
{
	
}



