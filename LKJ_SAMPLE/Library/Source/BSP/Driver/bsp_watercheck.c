/*******************************************************************************
 *   Filename:       bsp_watercheck.c
 *   Revised:        All copyrights reserved to Snlion.
 *   Date:           2014-08-18
 *   Revision:       v1.0
 *   Writer:	     Snlion-WY.
 *
 *   Description:    使用四个IO的状态检测水位
 *
 *
 *   Notes:          IO读到高电平，说明水位到达现在IO监测的高度。
 *
 *******************************************************************************/

#include "bsp_watercheck.h"


/*******************************************************************************
 * 名    称： WaterCheck_GPIO_Config
 * 功    能： 水位检测IO口配置
 * 入口参数： 无
 * 出口参数： 无
 * 作　　者： Snlion-WY
 * 创建日期： 2014-08-18
 * 修    改：
 * 修改日期：
 * 备    注：
 *******************************************************************************/
void WaterCheck_GPIO_Config(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;

    RCC_APB2PeriphClockCmd( RCC_APB2Periph_GPIOC, ENABLE);

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_2 |GPIO_Pin_3;

    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;

//    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;

    GPIO_Init(GPIOC, &GPIO_InitStructure);
}


/*******************************************************************************
 * 名    称： WaterCheck_Height
 * 功    能： 检测水位的高度
 * 入口参数： 无
 * 出口参数： 对应高度的值
 * 作　　者： Snlion-WY
 * 创建日期： 2014-08-18
 * 修    改：
 * 修改日期：
 * 备    注： 定义端口1位最下面的测量点。返回值越大，水位越高。
 *******************************************************************************/
unsigned char WaterCheck_Height(void)
{
    uint8_t Port1Status = 0x00;
    uint8_t Port2Status = 0x00;
    uint8_t Port3Status = 0x00;
    uint8_t Port4Status = 0x00;

    uint8_t ReturnValue = 0;

    Port1Status = (uint8_t)GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_0);
    Port2Status = (uint8_t)GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_1);
    Port3Status = (uint8_t)GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_2);
    Port4Status = (uint8_t)GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_3);


    if((Port1Status == 1)&&(Port2Status == 0)&&(Port3Status == 0)&&(Port4Status == 0))
    {
        ReturnValue = 2;
    }else if((Port1Status == 1)&&(Port2Status == 1)&&(Port3Status == 0)&&(Port4Status == 0))
    {
        ReturnValue = 3;
    }else if((Port1Status == 1)&&(Port2Status == 1)&&(Port3Status == 1)&&(Port4Status == 0))
    {
        ReturnValue = 4;
    }else if((Port1Status == 1)&&(Port2Status == 1)&&(Port3Status == 1)&&(Port4Status == 1))
    {
        ReturnValue = 5;
    }else
    {
        ReturnValue = 1;
    }

    return (ReturnValue);

}