/*******************************************************************************
 *   Filename:       bsp_sht11.h
 *   Revised:        All copyrights reserved to Roger-WY.
 *   Revision:       v1.0
 *   Writer:	     Roger-WY.
 *
 *   Description:    STH11温湿度传感器驱动模块
 *                   该驱动同样适用 与SHT10。
 *   Notes:
 *
 *     				 E-mail:261313062@qq.com
 *
 *******************************************************************************/
#ifndef __BSP_SHT11_H__
#define __BSP_SHT11_H__

#include "stm32f10x.h"
#include "global.h"
#include "math.h"


/**************** 硬件平台依赖 ************************************************/
extern  GPIO_InitTypeDef    GPIO_InitStructure;


#define RCC_GPIO_SHT        RCC_APB2Periph_GPIOC
#define SHT_PORT            GPIOC
#define GPIO_Pin_SHT_DAT    GPIO_Pin_10
#define GPIO_Pin_SHT_CLK    GPIO_Pin_11


#define SHT_DATA_H()        GPIO_SetBits(SHT_PORT,GPIO_Pin_SHT_DAT)
#define SHT_DATA_L()        GPIO_ResetBits(SHT_PORT,GPIO_Pin_SHT_DAT)
#define SHT_CLK_H()         GPIO_SetBits(SHT_PORT,GPIO_Pin_SHT_CLK)
#define SHT_CLK_L()         GPIO_ResetBits(SHT_PORT,GPIO_Pin_SHT_CLK)

#define SHT_DATA_INPUT()  st(   GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_SHT_DAT;\
                                GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;\
                                GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_IPU;\
                                GPIO_Init(SHT_PORT, &GPIO_InitStructure);\
                            )

#define SHT_DATA_OUPUT()  st(   GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_SHT_DAT;\
                                GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;\
                                GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_Out_PP;\
                                GPIO_Init(SHT_PORT, &GPIO_InitStructure);\
                            )

#define SHT_CLK_OUPUT()   st(   GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_SHT_CLK;\
                                GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;\
                                GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_Out_PP;\
                                GPIO_Init(SHT_PORT, &GPIO_InitStructure);\
                            )

#define SHT_INIT()        st(   RCC_APB2PeriphClockCmd(RCC_GPIO_SHT,ENABLE);\
                                SHT_CLK_OUPUT();\
                                SHT_DATA_OUPUT();\
                                SHT_CLK_L();\
                                SHT_DATA_H();\
                            )


/******************************************************************************/
enum {TEMP,HUMI};

#define noACK 0                          // 不应答标志 用于判断是否结束通讯
#define ACK   1                          // 应答标志   结束数据传输
                                                            // adr  command  r/w
#define SHTCMD_WRITE_STATUS     0x06     // "写状态寄存器"命令 000   0011    0
#define SHTCMD_READ_STATUES     0x07     // "读状态寄存器"命令 000   0011    1
#define SHTCMD_MEASURE_TEMP     0x03     // "测量温度"命令     000   0001    1
#define SHTCMD_MEASURE_HUMI     0x05     // "测量相对湿度"命令 000   0010    1
#define SHTCMD_RESET            0x1e     // "复位"命令         000   1111    0

/***********************************************
* 描述： 定义一个值共用体
*/
typedef union
{
  long unsigned int i;
  float f;
} value;


/***********************************************
* 描述： SHTxx测量数据结果结构体
*/
typedef struct
{
    /***********************************************
    * 描述： SHTxx测量数据结果放大10倍之后的值（整型）
    */
    int16_t Temperature;
    int16_t Humidity;
    int16_t DewPoint;
    /***********************************************
    * 描述： SHTxx测量数据结果（浮点型）
    */
    float fTemp;
    float fHumi;
    float fDewpoint;
}SHT_DATA_TYPE;


/***********************************************
* 描述： 供外部调用的函数声明
*/
void Bsp_ShtInit(void);
u8   Bsp_ShtMeasureOnce(SHT_DATA_TYPE *t_shtval);


#endif