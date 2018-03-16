/*******************************************************************************
*   Filename:       bsp_control.h
*   Revised:        All copyrights reserved to Roger.
*   Date:           2014-08-18
*   Revision:       v1.0
*   Writer:	        wumingshen.
*
*   Description:    继电器控制 头文件
*
*
*   Notes:
*
*******************************************************************************/
#ifndef __BSP_CONTROL_H__
#define __BSP_CONTROL_H__

#include "stm32f10x.h"
#include "bsp_pwm.h"


#define BEEP_RCC                RCC_APB2Periph_GPIOA
#define BEEP_PROT               GPIOA
#define BEEP_PIN                GPIO_Pin_1

#define BEEP_ON()               GPIO_SetBits(BEEP_PROT, BEEP_PIN);
#define BEEP_OFF()              GPIO_ResetBits(BEEP_PROT, BEEP_PIN);

// 风机档位	运行条件
enum __FAN_LEVEL {
    FAN_LEVEL_OFF               =   0,      // 关机/待机状态
    FAN_LEVEL_SILENT            =   1,	    // 静音模式
    FAN_LEVEL_AQL1_LOW          =   2,	    // 智能模式下空气优/手动模式低速
    FAN_LEVEL_AQL2_MID          =   3,	    // 智能模式下空气良/手动模式中速
    FAN_LEVEL_AQL3_HIG          =   4,	    // 智能模式下空气差/手动模式高速
    FAN_LEVEL_MAX               =   5,	    // 极速模式
};

// 状态及模式定义
enum __FAN_MODE {
    FAN_MODE_PWR_OFF            =   0,      // 断电	未上电状态
    //FAN_MODE_PWR_ON             =   0,      // 待机	上电自动运行，风机档位为0，其它正常运作
    FAN_MODE_SILENT             =   1,      // 静音模式	风机档位1，LED灯蓝色呼吸灯效果
    FAN_MODE_SMART              =   2,      // 智能模式	风机档位根据传感器反馈的空气质量动态变化2-4，LED随空气质量改变
    FAN_MODE_MAX_SPEED          =   3,      // 极速模式	风机档位5，LED灯绿色
    FAN_MODE_HAND               =   4,      // 手动模式	手动切换模式2-4，LED灯随空气质量改变
};

// LED颜色对应空气质量定义
enum __LED_COLOR {
    LED_COLOR_BLE               =   0,      // 蓝色	智能模式/手动模式下的空气优，档位2
    LED_COLOR_YEL               =   1,      // 黄色	智能模式/手动模式下的空气良，档位3
    LED_COLOR_RED               =   2,      // 红色	智能模式/手动模式下的空气差，档位4
    LED_COLOR_GRE               =   3,      // 绿色	极速模式，档位5
    LED_COLOR_BLE_BRE           =   4,      // 蓝色呼吸灯	静音模式，档位1
    LED_COLOR_BLK               =   5,      // 熄灭	待机状态/断电状态
};

//enum __fanlevel {
//    OFFMODE = 0,        //关机/待机模式
//    QUIETMODE,          //静音模式
//    LOWSPEEDMODE,       //低速模式
//    MEDIUMSPEEDMODE,    //中速模式
//    HIGHSPEEDMODE,      //高速模式
//    EXTREMESPEEDMODE,   //极速模式
//};
/***********************************************
* 描述： 函数声明
*/
void ControlInit(void);
void FanSpeedControl(uint8_t fanlevel);
void RGBLedControl(uint8_t r_duty,uint8_t g_duty,uint8_t b_duty);



#endif