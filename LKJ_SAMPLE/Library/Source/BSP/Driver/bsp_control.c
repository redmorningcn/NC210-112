/*******************************************************************************
*   Filename:       bsp_control.c
*   Revised:        All copyrights reserved to Roger.
*   Date:           2014-08-18
*   Revision:       v1.0
*   Writer:	        wumingshen.
*
*   Description:    控制信号（风机/RGB三色板）驱动模块
*
*
*   Notes:
*
*******************************************************************************/
#include "includes.h"
#include "global.h"
#include "bsp_control.h"



//============================================================================//

/*******************************************************************************
* 名    称： BeepGpioInit
* 功    能： 蜂鸣器引脚初始化
* 入口参数： 无
* 出口参数： 无
* 作　 　者： 无名沈
* 创建日期： 2014-08-18
* 修    改：
* 修改日期：
* 备    注：
*******************************************************************************/
void BeepGpioInit(void)
{
    //GPIO_InitTypeDef  GPIO_InitStructure;
	////GPIOA Periph clock enable
	//RCC_APB2PeriphClockCmd(BEEP_RCC, ENABLE);
    ///* Set PA.1 GPIO_Mode_AF_PP*/
    //GPIO_InitStructure.GPIO_Pin     = BEEP_PIN;
    //GPIO_InitStructure.GPIO_Speed   = GPIO_Speed_50MHz;
    //GPIO_InitStructure.GPIO_Mode    = GPIO_Mode_Out_PP;     // 通用推挽输出
    //GPIO_Init(BEEP_PROT, &GPIO_InitStructure);
}

/*******************************************************************************
* 名    称： ControlInit
* 功    能： 控制信号相关初始化
* 入口参数： 无
* 出口参数： 无
* 作　 　者： 无名沈
* 创建日期： 2014-08-18
* 修    改：
* 修改日期：
* 备    注：
*******************************************************************************/
void ControlInit(void)
{
   LedPWM_Init();
   FanPWM_Init();
}

/*******************************************************************************
* 名    称： FanSpeedControl
* 功    能： 风机等级控制
* 入口参数： 无
* 出口参数： 无
* 作　 　者： 无名沈
* 创建日期： 2014-08-18
* 修    改：
* 修改日期：
* 备    注： 共六挡 0-5 风速越来越快
*******************************************************************************/
void FanSpeedControl(uint8_t fanlevel)
{
//    static uint8_t lastlevel    = 0;

    switch(fanlevel) {
        /***********************************************
        * 描述： 关机/待机模式
        */
        case FAN_LEVEL_OFF:{
            FanPWM_SetDuty(0,0);
        };break;
        /***********************************************
        * 描述： 静音模式
        */
        case FAN_LEVEL_SILENT:{
            FanPWM_SetDuty(0,68);
        };break;
        /***********************************************
        * 描述： 低速模式
        */
        case FAN_LEVEL_AQL1_LOW:{
             FanPWM_SetDuty(0,72);
        };break;
        /***********************************************
        * 描述： 中速模式
        */
        case FAN_LEVEL_AQL2_MID:{
             FanPWM_SetDuty(0,82);
        };break;
        /***********************************************
        * 描述： 高速模式
        */
        case FAN_LEVEL_AQL3_HIG:{
             FanPWM_SetDuty(0,85);
        };break;
        /***********************************************
        * 描述： 极速模式
        */
        case FAN_LEVEL_MAX:{
             FanPWM_SetDuty(0,90);
        };break;
        default:break;
    }
}

/*******************************************************************************
* 名    称： FanSpeedControl
* 功    能： 风机等级控制
* 入口参数： 无
* 出口参数： 无
* 作　 　者： 无名沈
* 创建日期： 2014-08-18
* 修    改：
* 修改日期：
* 备    注： 控制RGB的占空比
*******************************************************************************/
void RGBLedControl(uint8_t r_duty,uint8_t g_duty,uint8_t b_duty)
{
    LedPWM_SetDuty(0,r_duty);
    LedPWM_SetDuty(1,g_duty);
    LedPWM_SetDuty(2,b_duty);
}





/*************end of file***************/