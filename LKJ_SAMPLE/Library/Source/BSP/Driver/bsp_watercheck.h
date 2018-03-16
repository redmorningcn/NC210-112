#ifndef __BSP_WATERCHECK_H__
#define __BSP_WATERCHECK_H__

#include "stm32f10x.h"
#include "global.h"

typedef struct {
    uint16 ShuiWei;         /*水位*/
    uint16 ShuiXiangTemp;   /*水箱温度*/
    uint16 JiReQiTemp;      /*集热器温度*/
    uint16 NeiXunHuanTemp;  /*内循环温度*/
    uint16 WaiXunHuanTemp;  /*外循环温度*/
    uint8  TemperatureError;/*温度测量超量程错误指示*/
}SensorTypeDef;

void WaterCheck_GPIO_Config(void);
unsigned char WaterCheck_Height(void);

#endif