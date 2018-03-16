/*******************************************************************************
*   Filename:       bsp_key.c
*   Revised:        All copyrights reserved to Roger.
*   Date:           2015-08-11
*   Revision:       v1.0
*   Writer:	        wumingshen.
*
*   Description:    按键驱动模块
*                   可以识别长按，短按，双击，旋转编码器的操作。
*
*
*   Notes:
*
*   All copyrights reserved to wumingshen
*******************************************************************************/

/*******************************************************************************
* INCLUDES
*/
#include <includes.h>
#include <global.h>
#include <app_lib.h>

#include "bsp_adc7682.h"

#define BSP_KEY_MODULE_EN 1
#if BSP_KEY_MODULE_EN > 0
/*******************************************************************************
 * CONSTANTS
 */

/*******************************************************************************
 * MACROS
 */
/***********************************************
* 描述： IO,时钟等定义
*/
#define SPI_SCK_PIN           GPIO_Pin_5             /* PA.05 */
#define SPI_SCK_PIN_NUM       5          
#define SPI_SCK_GPIO_PORT     GPIOA
#define SPI_SCK_GPIO_CLK      RCC_APB2Periph_GPIOA

#define SPI_MISO_PIN          GPIO_Pin_6             /* PA.06 */
#define SPI_MISO_PIN_NUM      6 
#define SPI_MISO_GPIO_PORT    GPIOA
#define SPI_MISO_GPIO_CLK     RCC_APB2Periph_GPIOA

#define SPI_MOSI_PIN          GPIO_Pin_7             /* PA.07 */
#define SPI_MOSI_PIN_NUM      7 
#define SPI_MOSI_GPIO_PORT    GPIOA
#define SPI_MOSI_GPIO_CLK     RCC_APB2Periph_GPIOA

#define SPI_CS_PIN            GPIO_Pin_4             /* PA.04 */
#define SPI_CS_PIN_NUM        4 
#define SPI_CS_GPIO_PORT      GPIOA
#define SPI_CS_GPIO_CLK       RCC_APB2Periph_GPIOA

/***********************************************
* 描述： IO操作定义
*/
#define SPI_CS_LOW()     	  GPIO_ResetBits(SPI_CS_GPIO_PORT, SPI_CS_PIN)
#define SPI_CS_HIGH()    	  GPIO_SetBits(SPI_CS_GPIO_PORT, SPI_CS_PIN)

#define SPI_SCK_LOW()    	  GPIO_ResetBits(SPI_SCK_GPIO_PORT,SPI_SCK_PIN)
#define SPI_SCK_HIGH()   	  GPIO_SetBits(SPI_SCK_GPIO_PORT,SPI_SCK_PIN)

#define SPI_MISO_LOW()   	  GPIO_ResetBits(SPI_MISO_GPIO_PORT,SPI_MISO_PIN)
#define SPI_MISO_HIGH()  	  GPIO_SetBits(SPI_MISO_GPIO_PORT,SPI_MISO_PIN)

#define SPI_MOSI_LOW()        GPIO_ResetBits(SPI_MOSI_GPIO_PORT,SPI_MOSI_PIN)
#define SPI_MOSI_HIGH()       GPIO_SetBits(SPI_MOSI_GPIO_PORT,SPI_MOSI_PIN)

/*******************************************************************************
 * TYPEDEFS
 */

/*******************************************************************************
 * LOCAL VARIABLES
 */

/*******************************************************************************
 * GLOBAL VARIABLES
 */

/*******************************************************************************
 * LOCAL FUNCTIONS
 */

/*******************************************************************************
 * GLOBAL FUNCTIONS
 */

/*******************************************************************************
 * EXTERN VARIABLES
 */

/*******************************************************************************
 * EXTERN FUNCTIONS
 */

/*******************************************************************************
* 名    称： SPI_GpioInit
* 功    能： ADC GPIO初始化
* 入口参数： 无
* 出口参数： 无
* 作　　者： wumingshen.
* 创建日期： 2015-12-07
* 修    改：
* 修改日期：
* 备    注：
*******************************************************************************/
static void SPI_GpioInit(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;

    RCC_APB2PeriphClockCmd(SPI_CS_GPIO_CLK | SPI_MOSI_GPIO_CLK |
                         SPI_MISO_GPIO_CLK | SPI_SCK_GPIO_CLK, ENABLE);

    /*!< Configure SPI pins: SCK */
    GPIO_InitStructure.GPIO_Pin = SPI_SCK_PIN;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_Init(SPI_SCK_GPIO_PORT, &GPIO_InitStructure);

    /*!< Configure SPI pins: MOSI */
    GPIO_InitStructure.GPIO_Pin = SPI_MOSI_PIN;			//speed和mode无需修改，故无重新赋值
    GPIO_Init(SPI_MOSI_GPIO_PORT, &GPIO_InitStructure);

    /*!< Configure SPI pins: MISO */
    GPIO_InitStructure.GPIO_Pin = SPI_MISO_PIN;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;          //GPIO_Mode_IPU;
    GPIO_Init(SPI_MISO_GPIO_PORT, &GPIO_InitStructure);

    /*!< Configure SPI pin: Chip CS pin */
    GPIO_InitStructure.GPIO_Pin = SPI_CS_PIN;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_Init(SPI_CS_GPIO_PORT, &GPIO_InitStructure);

    SPI_CS_HIGH();
}

/*******************************************************************************
* 名    称： SPI_SendByte
* 功    能： 通过SPI总线发送一个字节数据(同时接收一个字节数据)
* 入口参数： 要写入的一个字节数据
* 出口参数： 在发数据时，MISO信号线上接收的一个字节
* 作　　者： wumingshen.
* 创建日期： 2015-12-07
* 修    改：
* 修改日期：
* 备    注：
*******************************************************************************/
//static void SPI_SendByte(uint8_t byte)
//{
//    uint8_t i;
//    
//    /***********************************************
//    * 描述： 
//    */
//    for(i=0;i<8;i++) {		
//		SPI_SCK_LOW();
//		Delay_Nus(10);
//		if(byte & 0x80)
//			SPI_MOSI_HIGH();
//		else
//			SPI_MOSI_LOW();
//		byte = byte << 1;
//		Delay_Nus(5);
//		SPI_SCK_HIGH();
//		Delay_Nus(10);
//	}
//}	

/*******************************************************************************
* 名    称： SPI_ReadByte
* 功    能： 读取一个字节，未包含发送读命令和起始地址
* 入口参数： 无
* 出口参数： 从目标读取的一个字节
* 作　　者： wumingshen.
* 创建日期： 2015-12-07
* 修    改：
* 修改日期：
* 备    注：
*******************************************************************************/
//static uint8_t SPI_ReadByte(uint8_t Command)
//{
//    uint8_t i ,result = 0x00;
//	unsigned char SDI;
//    /***********************************************
//    * 描述： 
//    */
//	for(i=0;i<8;i++) {
//		SPI_SCK_LOW();
//		Delay_Nus(2);
//		if (Command & 0x80) {
//			SPI_MOSI_HIGH();
//		} else {
//			SPI_MOSI_LOW();
//        }
//        
//		Command = Command << 1;
//		SPI_SCK_HIGH();
//		Delay_Nus(2);
//		SDI =  GPIO_ReadInputDataBit(SPI_MISO_GPIO_PORT, SPI_MISO_PIN);
//		result = result << 1;
//        
//		if(SDI)
//			result = result | 0x01;
//	}
//    
//	return result;
//}

/*******************************************************************************
* 名    称： SPI_SendCommand
* 功    能： 通过SPI总线发送一个字节命令
* 入口参数： 要写入的一个字节命令
* 出口参数： 无
* 作　　者： wumingshen.
* 创建日期： 2015-12-07
* 修    改：
* 修改日期：
* 备    注：
*******************************************************************************/
//static void SPI_SendCommand(uint8_t Command)
//{
//	SPI_CS_LOW();
//	Delay_Nus(5);
//	SPI_SendByte(Command);
//	SPI_CS_HIGH();
//	Delay_Nus(5);
//}

/*******************************************************************************
* 名    称： AD7682_spi
* 功    能： reg寄存器内容,data转换数据
* 入口参数： 无
* 出口参数： 无
* 作　　者： wumingshen.
* 创建日期： 2015-12-07
* 修    改：
* 修改日期：
* 备    注：
*******************************************************************************/
static uint16_t AD7682_spi(uint8_t ch)
{
	uint16_t    reg     = 0;
	uint16_t    data    = 0;
	uint8_t     i       = 0;
	u8          SDI     = 0;
	
    /***********************************************
    * 描述： 
    */
	SPI_CS_LOW();
	Delay_Nus(2);
	SPI_CS_HIGH();
	Delay_Nus(2);
	SPI_CS_LOW();
    
    /***********************************************
    * 描述： 7682配置寄存器
    *        13  12   11   10   9   8   7   6  5   4   3   2   1   0
             CFG INCC INCC INCC INx INx INx BW REF REF REF SEQ SEQ RB
    */
    reg = (1 <<13)                                  // 1    = 覆盖寄存器的内容
        | (6 <<10)                                  // 110  = 单极性；INx以COM=GND±0.1V为参考
        | (ch<<7)                                   // 1XX  = 采集通道选择
        | (0 <<6)                                   // 0    = 1/4带宽（62.5K）；1   = 全带宽
        | (2 <<3)                                   // 010  = 外部基准，温度传感器使能
        | (0 <<1)                                   // 00   = 禁用序列器
        | (1 <<0);                                  // 1    = 不回读配置内容
    //reg = b1 110 100 0 010 00 1
	//reg = (ch<<7)|0x3811;
	reg <<=2;                                       //寄存器只用到了14位，所以要先移出高两位	
	Delay_Nus(2);
	
    /***********************************************
    * 描述： 
    */
	for(i = 0; i < 16; i++) {
		if(reg & 0x8000)   
            SPI_MOSI_HIGH();
		else               
            SPI_MOSI_LOW();
		
		Delay_Nus(2);
		SPI_SCK_LOW();
		Delay_Nus(2);
		reg <<=1;
		SPI_SCK_HIGH();
		data <<=1;
		Delay_Nus(1);
		SDI = GPIO_ReadInputDataBit(SPI_MISO_GPIO_PORT,  SPI_MISO_PIN); 
		if(SDI)      
			data = data | 0x01;	
	}
    
    /***********************************************
    * 描述： 
    */
	Delay_Nus(2);
	SPI_SCK_LOW();
	Delay_Nus(2);
	SPI_CS_HIGH();
	return (data);
}

/*******************************************************************************
* 名    称： BSP_ADCInit
* 功    能： ADC7682初始化
* 入口参数： 无
* 出口参数： 无
* 作　　者： wumingshen.
* 创建日期： 2015-12-07
* 修    改：
* 修改日期：
* 备    注：
*******************************************************************************/
void BSP_ADCInit( void )
{
	SPI_GpioInit(); 
    
	SPI_CS_HIGH();
	SPI_MISO_HIGH();
	SPI_SCK_HIGH();
    
	Delay_Nus(30);
	AD7682_spi(0);
	AD7682_spi(0); 
}


/*******************************************************************************
* 名    称： Get_ADC_10times_Value
* 功    能： 采样 ch 选择通道  前三次采集无效。 采集10次，去掉最大值和最小值
* 入口参数： ch - 通道序号（0~3）
* 出口参数： 无
* 作　　者： wumingshen.
* 创建日期： 2015-12-07
* 修    改：
* 修改日期：
* 备    注：
*******************************************************************************/
 uint16_t BSP_ADCGetValue(uint8_t ch)
{
#define SAMPLE_TIMES            100
#define FILTER_LEN              10
#if (FILTER_LEN * 2 + 1) > SAMPLE_TIMES
#error  "FILTER_LEN 太大，过滤参数太多"
#endif
	uint16      x;    
	long       sum             = 0;
    
    uint16      buf[SAMPLE_TIMES];
    uint16      After[SAMPLE_TIMES];
    /***********************************************
    * 描述： 参数过虑
    */
    if ( ch > 3 )
        return  0;
    
    /***********************************************
    * 描述： 丢掉前前一次采样值 
    */
	AD7682_spi(ch);
	
    /***********************************************
    * 描述： 开始采集
    */
	for(u8 i = 0; i < SAMPLE_TIMES; i++) {
		buf[i]      = AD7682_spi(ch);
	}
    
    /***********************************************
    * 描述： 一次滤波：排序
    */
    App_BubbleAscendingArray(buf, SAMPLE_TIMES, After);
    u16 *p  = After;
    
    /***********************************************
    * 描述： 一次滤波：排序
    */
    for(u8 i = FILTER_LEN; i < SAMPLE_TIMES - FILTER_LEN;i++) {
        sum += p[i];
    }
    
    x = (uint16)(sum/(float)(SAMPLE_TIMES - FILTER_LEN*2));
    
	return (x);
}

/*******************************************************************************
* 				                end of file                                    *
*******************************************************************************/
#endif