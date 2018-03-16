#ifndef __BSP_HT1621__
#define __BSP_HT1621__

/***********************************************
 * 描述：需要添加的头文件
 */
#include "stm32f10x.h"

#ifndef  u8
#define  u8    unsigned char
#endif

/***********************************************
 * 描述：是否使能HT1621B的测试函数 0为不使能 1为使能
 */
#define  HT1621B_TEST_EN  1

/***********************************************
 * 描述：延时函数
 */
#define _Nop() __NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP()


/***********************************************
 * 描述： 定义HT1621的命令
 */
#define BIAS    0x52    //0b1000 0101 0010 1/3bis 4com

#define SYSDIS  0X00    //0b1000 0000 0000 关振系统荡器和LCD偏压发生器
#define SYSEN   0X02    //0b1000 0000 0010 打开系统振荡器

#define LCDOFF  0X04    //0b1000 0000 0100 关LCD偏压
#define LCDON   0X06    //0b1000 0000 0110 打开LCD偏压

#define XTAL    0x28    //0b1000 0010 1000 外部接时钟
#define RC256   0X30    //0b1000 0011 0000 内部时钟

#define TONEON  0X12    //0b1000 0001 0010 打开声音输出
#define TONEOFF 0X10    //0b1000 0001 0000 关闭声音输出

#define WDTDIS  0X0A    //0b1000 0000 1010 禁止看门狗


/***********************************************
 * 描述：HT1621B的引脚定义
 */
/*----------引脚使用端口定义----------------*/
#define HT1621B_CS_RCC            RCC_APB2Periph_GPIOD
#define HT1621B_CS_PORT           GPIOD
#define HT1621B_CS_PIN            GPIO_Pin_2

#define HT1621B_WR_RCC            RCC_APB2Periph_GPIOC
#define HT1621B_WR_PORT           GPIOC
#define HT1621B_WR_PIN            GPIO_Pin_11

#define HT1621B_DATA_RCC          RCC_APB2Periph_GPIOC
#define HT1621B_DATA_PORT         GPIOC
#define HT1621B_DATA_PIN          GPIO_Pin_10

/*----------引脚输出电平定义----------------*/
#define HT1621B_CS_HIGH()         GPIO_SetBits  (HT1621B_CS_PORT,HT1621B_CS_PIN)
#define HT1621B_CS_LOW()          GPIO_ResetBits(HT1621B_CS_PORT,HT1621B_CS_PIN)

#define HT1621B_WR_HIGH()         GPIO_SetBits  (HT1621B_WR_PORT,HT1621B_WR_PIN)
#define HT1621B_WR_LOW()          GPIO_ResetBits(HT1621B_WR_PORT,HT1621B_WR_PIN)

#define HT1621B_DATA_HIGH()       GPIO_SetBits  (HT1621B_DATA_PORT,HT1621B_DATA_PIN)
#define HT1621B_DATA_LOW()        GPIO_ResetBits(HT1621B_DATA_PORT,HT1621B_DATA_PIN)



/***********************************************
 * 描述：函数申明
 */
void HT1621B_Init(void);
void HT1621BWrCmd(u8 Cmd);
void HT1621BWrOneData(u8 Addr,u8 Data);
void HT1621BWrStrData(u8 Addr,u8 *p,u8 cnt);
void HT1821B_AllClean(void);

#if  HT1621B_TEST_EN > 0
void HT1621B_TEST(void);
#endif



#endif /* end of ht1621b*/