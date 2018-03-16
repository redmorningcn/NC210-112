/*******************************************************************************
 *   Filename:       bsp_adc.c
 *   Revised:        All copyrights reserved to Roger.
 *   Date:           2014-08-11
 *   Revision:       v1.0
 *   Writer:	     wumingshen.
 *
 *   Description:    ADC模数转换模块（使用DMA传输数据）
 *
 *
 *   Notes:
 *
 *   All copyrights reserved to wumingshen
 *******************************************************************************/
#include "bsp_dac.h"
#include "bsp.h"
#include "app_ctrl.h"

/*****************************************************************************************************/
/* EXTERN VARIABLES*/


//============================================================================//

/*******************************************************************************
 * 名    称： DAC_gpio_config
 * 功    能： 
 * 入口参数： 无
 * 出口参数： 无
 * 作　 　者： fouth peng
 * 创建日期： 2017-05-11
 * 修    改：
 *******************************************************************************/
void DAC_gpio_config()
{
	GPIO_InitTypeDef GPIO_InitStructure;

	/* Once the DAC channel is enabled, the corresponding GPIO pin is automatically 
	connected to the DAC converter. In order to avoid parasitic consumption, 
	the GPIO pin should be configured in analog */
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA , ENABLE);	     //使能DAC时钟
	GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_4 | GPIO_Pin_5;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
}
/*******************************************************************************
 * 名    称： DAC_gpio_config
 * 功    能： 
 * 入口参数： 无
 * 出口参数： 无
 * 作　 　者： fouth peng
 * 创建日期： 2017-05-11
 * 修    改：
 *******************************************************************************/
void DAC_mode_config()
{
	DAC_InitTypeDef            DAC_InitStructure;  
	  RCC_APB1PeriphClockCmd(RCC_APB1Periph_DAC , ENABLE);	     //使能DAC时钟
	/* DAC channel1 Configuration */
	DAC_InitStructure.DAC_Trigger = DAC_Trigger_None;
	DAC_InitStructure.DAC_WaveGeneration = DAC_WaveGeneration_None;
	DAC_InitStructure.DAC_LFSRUnmask_TriangleAmplitude =  DAC_TriangleAmplitude_4095  ;
	DAC_InitStructure.DAC_OutputBuffer = DAC_OutputBuffer_Disable;
	DAC_Init(DAC_Channel_1, &DAC_InitStructure);

	/* DAC channel2 Configuration */
	DAC_InitStructure.DAC_LFSRUnmask_TriangleAmplitude = DAC_TriangleAmplitude_4095;
	DAC_Init(DAC_Channel_2, &DAC_InitStructure);

	/* Enable DAC Channel1: Once the DAC channel1 is enabled, PA.04 is 
	automatically connected to the DAC converter. */
	DAC_Cmd(DAC_Channel_1, ENABLE);

	/* Enable DAC Channel2: Once the DAC channel2 is enabled, PA.05 is 
	automatically connected to the DAC converter. */
	DAC_Cmd(DAC_Channel_2, ENABLE);
	//DAC_SetDualChannelData(DAC_Align_12b_R, 1010, 129);
    DAC_SetDualChannelData(DAC_Align_12b_R, 4096/6, 4096/6);
}

/*******************************************************************************
 * 名    称： BSP_dac_init
 * 功    能： 
 * 入口参数： 无
 * 出口参数： 无
 * 作　 　者： fouth peng
 * 创建日期： 2017-05-11
 * 修    改：
 *******************************************************************************/
void BSP_dac_init()
{
	DAC_gpio_config();
	DAC_mode_config();
}

/*******************************************************************************
 * 名    称： BSP_dac_init
 * 功    能： 
 * 入口参数： 无
 * 出口参数： 无
 * 作　 　者： fouth peng
 * 创建日期： 2017-05-11
 * 修    改：
 *******************************************************************************/
void bsp_dac_set_dual_channel_voltage(uint16 ch1_vol, uint16 ch2_vol)
{
	
    //DAC_SetDualChannelData(DAC_Align_12b_R, ch1_vol, ch2_vol);
}


