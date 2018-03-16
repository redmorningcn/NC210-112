/*******************************************************************************
 *   Filename:       bsp_adc.h
 *   Revised:        All copyrights reserved to Roger.
 *   Date:           2015-08-11
 *   Revision:       v1.0
 *   Writer:	     Roger-WY.
 *
 *   Description:    ADC模数转换模块（使用DMA传输数据）头文件
 *
 *
 *   Notes:
 *
 *   All copyrights reserved to Roger-WY
 *******************************************************************************/
#ifndef __BSP_ADC_H__
#define	__BSP_ADC_H__


#include "stm32f10x.h"

#define ADC1_DR_Address    ((u32)0x40012400+0x4c)

/*******************************************************************************
 * 描述： 宏定义采集通道个数和每个通道采集的次数
 */
#define Channel_Number 3    //2个通道
#define Channel_Times  1    //每个通到采集1次


extern __IO uint16_t ADC_Value[Channel_Times][Channel_Number];
extern __IO uint16_t ADC_AverageValue[Channel_Number];


/*******************************************************************************
 * 描述： 外部函数调用
 */
void Bsp_ADC_Init(void);
void ADC1Convert_Begin(void);
void ADC1Convert_Stop(void);
void Get_AD_AverageValue(void);


#endif /* __ADC_H */

