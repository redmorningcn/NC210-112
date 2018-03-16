/******************** (C) COPYRIGHT 2013 www.nanche.com  ********************
* 文件名  ：SPI_CS5463_AC.c
* 描述    ：实现CS5463的底层函数
* 实验平台：STM32神舟开发板
* 标准库  ：STM32F10x_StdPeriph_Driver V3.5.0
* 作者    ：zw
*******************************************************************************/

/* Includes ------------------------------------------------------------------*/
#include "SZ_STM32F107VC_LIB.h"
#include "DELAY.h"
#ifdef PHOTOELECTRIC_VELOCITY_MEASUREMENT
#include <Speed_sensor.h>
#include "app_task_mater.h"
#else
#include <power_macro.h>
#endif
#include "SPI_CS5463_AC.h"
#include "FM24CL64.h"
#include <includes.h>

#define SPI_CS5463_AC_SCK_PIN           GPIO_Pin_2             /* PE.02 */
#define SPI_CS5463_AC_SCK_PIN_NUM       2          
#define SPI_CS5463_AC_SCK_GPIO_PORT     GPIOE
#define SPI_CS5463_AC_SCK_GPIO_CLK      RCC_APB2Periph_GPIOE

#define SPI_CS5463_AC_MISO_PIN          GPIO_Pin_3             /* PE.03 */
#define SPI_CS5463_AC_MISO_PIN_NUM      3 
#define SPI_CS5463_AC_MISO_GPIO_PORT    GPIOE
#define SPI_CS5463_AC_MISO_GPIO_CLK     RCC_APB2Periph_GPIOE

#define SPI_CS5463_AC_MOSI_PIN          GPIO_Pin_5             /* PE.05 */
#define SPI_CS5463_AC_MOSI_PIN_NUM      5 
#define SPI_CS5463_AC_MOSI_GPIO_PORT    GPIOE
#define SPI_CS5463_AC_MOSI_GPIO_CLK     RCC_APB2Periph_GPIOE

#define SPI_CS5463_AC_CS_PIN            GPIO_Pin_4              /* PE.04 */
#define SPI_CS5463_AC_CS_PIN_NUM        4 
#define SPI_CS5463_AC_CS_GPIO_PORT      GPIOE
#define SPI_CS5463_AC_CS_GPIO_CLK       RCC_APB2Periph_GPIOE

#define SPI_RST_AC_PIN                  GPIO_Pin_6  		    /* PE.06 */
#define SPI_RST_AC_PIN_NUM              6
#define SPI_RST_AC_GPIO_PORT            GPIOE
#define SPI_RST_AC_GPIO_CLK             RCC_APB2Periph_GPIOE

#define E1_AC_PIN                       GPIO_Pin_0				/* PE.00 */
#define E1_AC_PIN_NUM                   0
#define E1_AC_TIMER_PORT                GPIOE
#define E1_AC_TIMER_CLK                 RCC_APB2Periph_GPIOE

#define E2_AC_PIN                       GPIO_Pin_2				 /* PD.02 */
#define E2_AC_PIN_NUM                   2
#define E2_AC_TIMER_PORT                GPIOD
#define E2_AC_TIMER_CLK                 RCC_APB2Periph_GPIOD

/*寄存器读写*/

#define CONFIG_DATA_AC                  0x000001
#define STATUS_DATA_AC                  0x800000
#define INTMASK_DATA_AC                 0x000000
#define CTRL_DATA_AC                    0x000005
#define CYCLE_COUNT_DATA_AC             0x000FA0

#define PULSE_RATE_DATA_AC              0x027818				   //1号样机
//#define PULSE_RATE_DATA_AC              0x027019				   //1号样机
//#define PULSE_RATE_DATA_AC              0x027D00				   //2号样机
//#define PULSE_RATE_DATA_AC              0x027819				   //型式试验
#define MODE_DATA_AC                    0x000001
//#define PULSE_RATE_DATA_AC              0x027D00				   //2号样机

CONFIGSFR   configsfr_AC , configsfr_AC_RD;

/* Private macro -------------------------------------------------------------*/
/* Select SPI FLASH: Chip Select pin low  */

//#define SPI_CS5463_AC_CS_LOW()     (Periph_BB((uint32_t) & SPI_CS5463_AC_CS_GPIO_PORT->ODR, SPI_CS5463_AC_CS_PIN_NUM) = 0)
//#define SPI_CS5463_AC_CS_HIGH()    (Periph_BB((uint32_t) & SPI_CS5463_AC_CS_GPIO_PORT->ODR, SPI_CS5463_AC_CS_PIN_NUM) = 1)

#define SPI_CS5463_AC_CS_LOW(n)    {GPIO_ResetBits(SPI_CS5463_AC_CS_GPIO_PORT, SPI_CS5463_AC_CS_PIN),Delay(n);}
#define SPI_CS5463_AC_CS_HIGH(n)   {GPIO_SetBits(SPI_CS5463_AC_CS_GPIO_PORT, SPI_CS5463_AC_CS_PIN),Delay(n);}

#define SPI_RST_AC_LOW()           GPIO_ResetBits(SPI_RST_AC_GPIO_PORT, SPI_RST_AC_PIN)
#define SPI_RST_AC_HIGH()          GPIO_SetBits(SPI_RST_AC_GPIO_PORT, SPI_RST_AC_PIN)

#define SPI_CS5463_AC_SCK_LOW()    GPIO_ResetBits(SPI_CS5463_AC_SCK_GPIO_PORT,SPI_CS5463_AC_SCK_PIN)
#define SPI_CS5463_AC_SCK_HIGH()   GPIO_SetBits(SPI_CS5463_AC_SCK_GPIO_PORT,SPI_CS5463_AC_SCK_PIN)

#define SPI_CS5463_AC_MISO_LOW()   GPIO_ResetBits(SPI_CS5463_AC_MISO_GPIO_PORT,SPI_CS5463_AC_MISO_PIN)
#define SPI_CS5463_AC_MISO_HIGH()  GPIO_SetBits(SPI_CS5463_AC_MISO_GPIO_PORT,SPI_CS5463_AC_MISO_PIN)

#define SPI_CS5463_AC_MOSI_LOW()   GPIO_ResetBits(SPI_CS5463_AC_MOSI_GPIO_PORT,SPI_CS5463_AC_MOSI_PIN)
#define SPI_CS5463_AC_MOSI_HIGH()  GPIO_SetBits(SPI_CS5463_AC_MOSI_GPIO_PORT,SPI_CS5463_AC_MOSI_PIN)

/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/
extern void uartprintf(MODBUS_CH  *pch,const char *fmt, ...);
/**-----------------------------------------------------------------
* @函数名 SPI_CS5463_AC_INIT
* @功能   初始化与CS5463接口的驱动函数
* @参数   无
* @返回值 无
***----------------------------------------------------------------*/
void SPI_AC_INIT(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    
    RCC_APB2PeriphClockCmd(SPI_CS5463_AC_CS_GPIO_CLK | SPI_CS5463_AC_MOSI_GPIO_CLK |
                           SPI_CS5463_AC_MISO_GPIO_CLK | SPI_CS5463_AC_SCK_GPIO_CLK, ENABLE);
    
    /*!< Configure SPI_CS5463_AC pins: SCK */
    GPIO_InitStructure.GPIO_Pin = SPI_CS5463_AC_SCK_PIN;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_Init(SPI_CS5463_AC_SCK_GPIO_PORT, &GPIO_InitStructure);
    
    /*!< Configure SPI_CS5463_AC pins: MOSI */
    GPIO_InitStructure.GPIO_Pin = SPI_CS5463_AC_MOSI_PIN;			//speed和mode无需修改，故无重新赋值
    GPIO_Init(SPI_CS5463_AC_MOSI_GPIO_PORT, &GPIO_InitStructure);
    
    /*!< Configure SPI_CS5463_AC pins: MISO */
    GPIO_InitStructure.GPIO_Pin = SPI_CS5463_AC_MISO_PIN;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;          //GPIO_Mode_IPU;
    GPIO_Init(SPI_CS5463_AC_MISO_GPIO_PORT, &GPIO_InitStructure);
    
    /*!< Configure SPI_CS5463_AC_CS_PIN pin: CS5463 Chip CS pin */
    GPIO_InitStructure.GPIO_Pin = SPI_CS5463_AC_CS_PIN;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_Init(SPI_CS5463_AC_CS_GPIO_PORT, &GPIO_InitStructure);
    
	/*!< Configure SPI_CS5463_AC pins: RST */
    RCC_APB2PeriphClockCmd(SPI_RST_AC_GPIO_CLK, ENABLE);
	GPIO_InitStructure.GPIO_Pin = SPI_RST_AC_PIN;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_Init(SPI_RST_AC_GPIO_PORT, &GPIO_InitStructure);
    
    //	RCC_APB2PeriphClockCmd(E1_AC_TIMER_CLK | E2_AC_TIMER_CLK , ENABLE);
    //	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);
    //	GPIO_InitStructure.GPIO_Pin = E1_AC_PIN;
    //    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    //    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPD;
    //    GPIO_Init(E1_AC_TIMER_PORT , &GPIO_InitStructure);
    //	GPIO_Init(E2_AC_TIMER_PORT , &GPIO_InitStructure); 
    /* Deselect the FLASH: Chip Select high */
    SPI_CS5463_AC_CS_HIGH(1);
}

/**-----------------------------------------------------------------
* @函数名 SPI_CS5463_AC_SendByte
* @功能   通过SPI总线发送一个字节数据(同时接收一个字节数据)
*         Sends a byte through the SPI interface and return the byte
*         received from the SPI bus.
* @参数   要写入的一个字节数据
* @返回值 在发数据时，MISO信号线上接收的一个字节
***----------------------------------------------------------------*/
void SPI_CS5463_AC_SendByte(uint8_t byte)
{
    uint8_t i;
    
    for(i=0;i<8;i++)
	{		
		SPI_CS5463_AC_SCK_LOW();
		Delay(10);
		if(byte & 0x80)
			SPI_CS5463_AC_MOSI_HIGH();
		else
			SPI_CS5463_AC_MOSI_LOW();
		byte = byte << 1;
		Delay(5);
		SPI_CS5463_AC_SCK_HIGH();
		Delay(10);
	}
}	

/**-----------------------------------------------------------------
* @函数名 SPI_CS5463_AC_ReadByte
* @功能   读取CS5463的一个字节，未包含发送读命令和起始地址
* @参数   无
* @返回值 从CS5463读取的一个字节
***----------------------------------------------------------------*/
uint8_t SPI_CS5463_AC_ReadByte(uint8_t Command)
{
    uint8_t i ,result = 0x00;
	unsigned char SDI;
	for(i=0;i<8;i++)
	{
		SPI_CS5463_AC_SCK_LOW();
		Delay(2);
		if (Command & 0x80)
		{
			SPI_CS5463_AC_MOSI_HIGH();
		}
		else 
			SPI_CS5463_AC_MOSI_LOW();
		Command = Command << 1;
		SPI_CS5463_AC_SCK_HIGH();
		Delay(2);
		SDI =  GPIO_ReadInputDataBit(SPI_CS5463_AC_MISO_GPIO_PORT, SPI_CS5463_AC_MISO_PIN);
		result = result << 1;
		if(SDI)
			result = result | 0x01;
	}
	return result;
}

/**-----------------------------------------------------------------
* @函数名 SPI_CS5463_AC_Write
* @功能   往CS5463中写任意长度数据
* @参数   *buf 数组，len 数组长度(以字节为最小单位)
***----------------------------------------------------------------*/
void SPI_CS5463_AC_Write(uint8_t *buf , uint8_t len)
{
	uint8_t i;
	/* Select the CS5463: Chip Select low */
	SPI_CS5463_AC_CS_LOW(1);//Delay(2);
	buf[0]|=0x40;
	for(i=0;i<len;i++)
		SPI_CS5463_AC_SendByte(buf[i]);
	Delay(1);
	SPI_CS5463_AC_CS_HIGH(1);
}

/**-----------------------------------------------------------------
* @函数名 SPI_CS5463_WriteRegister
* @功能   写CS5463的寄存器
* @参数   Command为命令，DATA为数据
***----------------------------------------------------------------*/
void SPI_CS5463_AC_WriteRegister(uint8_t Command , uint32_t DATA)
{
	uint8_t buf[4];
	/* Select the CS5463: Chip Select low */
	buf[0] = Command;
	buf[1] = (DATA >> 16) & 0xFF;
	buf[2] = (DATA >> 8) & 0xFF;
	buf[3] = DATA & 0xFF;
	SPI_CS5463_AC_Write(buf , 4);
}

/**-----------------------------------------------------------------
* @函数名 SPI_CS5463_AC_Read
* @功能   读取CS5463的寄存器
* @参数   Command：单字节命令,buf为读取到的数组
***----------------------------------------------------------------*/
void SPI_CS5463_AC_Read(uint8_t Command , uint8_t *buf)
{
	uint8_t i;
    /* Select the CS5463: Chip Select low */
    SPI_CS5463_AC_CS_LOW(2);//Delay(2);
    /* Send  instruction */
    SPI_CS5463_AC_SendByte(Command);
	for(i=1;i<4;i++)
		buf[i] = SPI_CS5463_AC_ReadByte(0xFE);
	buf[0] = Command;
	Delay(1);
	SPI_CS5463_AC_CS_HIGH(2);//Delay(2);  
}

/**-----------------------------------------------------------------
* @函数名 SPI_CS5463_AC_ReadRegister_INT32
* @功能   读取CS5463的寄存器，得到1个3字节数据
* @参数   Command：单字节命令
* @返回   Data，3字节数据
***----------------------------------------------------------------*/
uint32_t SPI_CS5463_AC_ReadRegister_INT32(uint8_t Command)
{
	uint32_t Data = 0;
	uint8_t Data0 = 0,Data1 = 0,Data2 = 0;
    /* Select the CS5463: Chip Select low */
  	SPI_CS5463_AC_CS_LOW(1);
    /* Send  instruction */
  	SPI_CS5463_AC_SendByte(Command);
    
	Data0 = SPI_CS5463_AC_ReadByte(0xFE);
	Data1 = SPI_CS5463_AC_ReadByte(0xFE);
	Data2 = SPI_CS5463_AC_ReadByte(0xFE);
	Data = Data0<<16 | Data1<<8 | Data2;
	SPI_CS5463_AC_CS_HIGH(5);
	return Data;
}

/**-----------------------------------------------------------------
* @函数名 SPI_CS5463_AC_Read_Temp_FLOAT
* @功能   读取CS5463的寄存器，得到1个浮点型数据
* @参数   Command：单字节命令
* @返回   Data，浮点型数据
* @备注   返回的的字节为-2^7,2^6到2^-16
***----------------------------------------------------------------*/
float SPI_CS5463_AC_Read_Temp_FLOAT(uint8_t Command)
{
	float Data = 0;
	uint8_t Data0 = 0,Data1 = 0,Data2 = 0;
    /* Select the CS5463: Chip Select low */
  	SPI_CS5463_AC_CS_LOW(1);
    /* Send  instruction */
  	SPI_CS5463_AC_SendByte(Command);
	
	Data0 = SPI_CS5463_AC_ReadByte(0xFE);
	Data1 = SPI_CS5463_AC_ReadByte(0xFE);
	Data2 = SPI_CS5463_AC_ReadByte(0xFE);
	Data = ((uint8_t)((Data0>>7) & 0x01))*(-128) + (Data0 & 0x7F) + (float)Data1/256 + (float)Data2/65536;
	SPI_CS5463_AC_CS_HIGH(5);
	return Data;
}

/**-----------------------------------------------------------------
* @函数名 SPI_CS5463_AC_Read_VIrms_FLOAT
* @功能   读取CS5463的电压/电流有效值，得到1个浮点型数据
* @参数   Command：单字节命令
* @返回   Data，浮点型数据				  
* @备注   返回的的字节为2^-1到2^-24
***----------------------------------------------------------------*/
float SPI_CS5463_AC_Read_VIrms_FLOAT(uint8_t Command)
{
	float Data = 0;
	uint8_t Data0 = 0,Data1 = 0,Data2 = 0;
    /* Select the CS5463: Chip Select low */
  	SPI_CS5463_AC_CS_LOW(1);
    /* Send  instruction */
  	SPI_CS5463_AC_SendByte(Command);
	
	Data0 = SPI_CS5463_AC_ReadByte(0xFE);
	Data1 = SPI_CS5463_AC_ReadByte(0xFE);
	Data2 = SPI_CS5463_AC_ReadByte(0xFE);
	Data = (float)(Data0)/256 + (float)Data1/256/256 + (float)Data2/256/256/256;
	SPI_CS5463_AC_CS_HIGH(5);
	return Data;
}

/**-----------------------------------------------------------------
* @函数名 SPI_CS5463_AC_Read_VIgain_FLOAT
* @功能   读取CS5463的电压/电流增益，得到1个浮点型数据
* @参数   Command：单字节命令
* @返回   Data，浮点型数据
* @备注   返回的的字节为2^1到2^0 + 2^-1到2^-22 (2^1到2^-22)
***----------------------------------------------------------------*/
float SPI_CS5463_AC_Read_VIgain_FLOAT(uint8_t Command)
{
	float Data = 0;
	uint8_t Data0 = 0,Data1 = 0,Data2 = 0;
    /* Select the CS5463: Chip Select low */
  	SPI_CS5463_AC_CS_LOW(1);
    /* Send  instruction */
  	SPI_CS5463_AC_SendByte(Command);
	
	Data0 = SPI_CS5463_AC_ReadByte(0xFE);
	Data1 = SPI_CS5463_AC_ReadByte(0xFE);
	Data2 = SPI_CS5463_AC_ReadByte(0xFE);
	Data = ((Data0>>6)&0x03)+(float)(Data0&0x3F)/64 + (float)Data1/64/256 + (float)Data2/64/256/256;
	SPI_CS5463_AC_CS_HIGH(5);
	return Data;
}

/**-----------------------------------------------------------------
* @函数名 SPI_CS5463_AC_Read_Else_FLOAT
* @功能   读取CS5463的电压电流DC偏移，得到1个浮点型数据
* @参数   Command：单字节命令
* @返回   Data，浮点型数据
* @备注   返回的的字节为2^-1到2^-23，再加上最高位负值-2^0
***----------------------------------------------------------------*/
float SPI_CS5463_AC_Read_Else_FLOAT(uint8_t Command)
{
	float Data = 0;
	uint8_t Data0 = 0,Data1 = 0,Data2 = 0;
    /* Select the CS5463: Chip Select low */
  	SPI_CS5463_AC_CS_LOW(1);
    /* Send  instruction */
  	SPI_CS5463_AC_SendByte(Command);
	
	Data0 = SPI_CS5463_AC_ReadByte(0xFE);
	Data1 = SPI_CS5463_AC_ReadByte(0xFE);
	Data2 = SPI_CS5463_AC_ReadByte(0xFE);
	Data = (float)((Data0>>7)&0x01)*(-1)+(float)(Data0&0x7F)/128 + (float)Data1/32768 + (float)Data2/256/32768;
	SPI_CS5463_AC_CS_HIGH(5);
	return Data;
}


/**-----------------------------------------------------------------
* @函数名 SPI_CS5463_AC_SendCommand
* @功能   通过SPI总线发送一个字节命令
*         Sends a byte through the SPI interface
* @参数   要写入的一个字节命令
* @返回值 无
***----------------------------------------------------------------*/
void SPI_CS5463_AC_SendCommand(u8 Command)
{
	SPI_CS5463_AC_CS_LOW(5);
	SPI_CS5463_AC_SendByte(Command);
	SPI_CS5463_AC_CS_HIGH(5);
}	

uint32_t    CS5463Sts   = 0;
/**-----------------------------------------------------------------
* @函数名 SPI_CS5463_AC_ClearDrdy
* @功能   CS5463进入清除数据就绪状态位
* @参数   无
* @返回值 无
***----------------------------------------------------------------*/
void SPI_CS5463_AC_ClearDrdy(void)
{
	SPI_CS5463_AC_WriteRegister(STATUS , 0xFFFFFF);
    CS5463Sts   = SPI_CS5463_AC_ReadRegister_INT32(STATUS);
}

/**-----------------------------------------------------------------
* @函数名 SPI_CS5463_AC_WaitDrdy
* @功能   CS5463等待数据就绪状态位
* @参数   无
* @返回值 无
***----------------------------------------------------------------*/
void SPI_CS5463_AC_WaitDrdy(void)
{
    
	while (SPI_CS5463_AC_ReadRegister_INT32(STATUS) & 0x800000 == 0); 	
    return;
    
    uint32_t    timeout = 0;
	do {
        CS5463Sts   = SPI_CS5463_AC_ReadRegister_INT32(STATUS);
        if ( (0x800000 & CS5463Sts) != 0x800000 )
            BSP_OS_TimeDly(5);
        else
            break;
    } while ( ++timeout < 2000 );
}

uint8_t SPI_CS5463_AC_GetDrdy(void)
{
    CS5463Sts   = SPI_CS5463_AC_ReadRegister_INT32(STATUS);
	if (( CS5463Sts & 0x800000 ) == 0x800000)
        return FALSE;
    else
        return TRUE;
}
/**-----------------------------------------------------------------
* @函数名 SPI_SPI_CS5463_AC_SLEEP
* @功能   CS5463进入睡眠模式
* @参数   无
* @返回值 无
***----------------------------------------------------------------*/
void SPI_CS5463_AC_SLEEP(void)   
{ 
	/* Select the CS5463: Chip Select low */
	SPI_CS5463_AC_CS_LOW(1);
    
	/* Send "SLEEP" instruction */
	SPI_CS5463_AC_SendCommand(SLEEP);
	
	/* Deselect the CS5463: Chip Select high */
	SPI_CS5463_AC_CS_HIGH(1);
}   

/**-----------------------------------------------------------------
* @函数名 SPI_SPI_CS5463_AC_Wait
* @功能   CS5463进入等待模式
* @参数   无
* @返回值 无
***----------------------------------------------------------------*/
void SPI_CS5463_AC_WAIT(void)   
{ 
	/* Select the CS5463: Chip Select low */
	SPI_CS5463_AC_CS_LOW(1);
	
	/* Send "WAIT" instruction */
	SPI_CS5463_AC_SendCommand(WAIT);
	
	/* Deselect the CS5463: Chip Select high */
	SPI_CS5463_AC_CS_HIGH(1);
}   

/**-----------------------------------------------------------------
* @函数名 SPI_CS5463_AC_SOFT_RESET
* @功能   软件复位CS5463
* @参数   无
* @返回值 无
***----------------------------------------------------------------*/
void SPI_CS5463_AC_SOFT_RESET(void)   
{
	/* Select the CS5463: Chip Select low */
	SPI_CS5463_AC_CS_LOW(1);
	
	/* Send "SOFT_RESET" instruction */
	SPI_CS5463_AC_SendCommand(SOFT_RESET);
	
	/* Deselect the CS5463: Chip Select high */
	SPI_CS5463_AC_CS_HIGH(1);
}   
union {
float   x;
uint32_t y;
} sx;
uint32_t y;
/**-----------------------------------------------------------------
* @函数名 CS5463_AC_INIT
* @功能   CS5463初始化
*         
* @参数   无
* @返回值 无
***----------------------------------------------------------------*/
void CS5463_AC_INIT(void)
{
	uint8_t buf[4];
	uint8_t temp[4];
	uint32_t STATUS_VALUE = 0; 
    
    //sx.x = 0.01929;
    //y = sx.x * 0x800000;
    //sx.x = (float)y / 0x800000;
    
	SPI_RST_AC_LOW();
	SPI_RST_AC_LOW();
	SPI_RST_AC_LOW();
	Delay(2000);
	SPI_RST_AC_HIGH() ;
	SPI_RST_AC_HIGH();
	SPI_RST_AC_HIGH();
	Delay(4000);
    
    /* Send  instruction */    
	SPI_CS5463_AC_CS_LOW(1);
	SPI_CS5463_AC_SendByte(SYNC1);
	SPI_CS5463_AC_SendByte(SYNC1);
	SPI_CS5463_AC_SendByte(SYNC1);
	SPI_CS5463_AC_SendByte(SYNC0);
	SPI_CS5463_AC_CS_HIGH(1);
    //配置寄存器设置为0x000001，K=1
	SPI_CS5463_AC_WriteRegister(CONFIG , CONFIG_DATA_AC);
    //操作模式寄存器设置为0x000000
	SPI_CS5463_AC_WriteRegister(MODE   , MODE_DATA_AC);
    //E2为电能方向，E3为无功电能										 
    
    //	SPI_CS5463_AC_WriteRegister(IG , 0x400000);
    //	SPI_CS5463_AC_WriteRegister(VG , 0x400000);
    //	SPI_CS5463_AC_WriteRegister(IDCOFF , 0x000000);
    //	SPI_CS5463_AC_WriteRegister(VDCOFF , 0x000000);
    //	SPI_CS5463_AC_WriteRegister(IACOFF , 0x000000);
    //	SPI_CS5463_AC_WriteRegister(VACOFF , 0x000000);
    
    //	SPI_CS5463_AC_WriteRegister(IG , 0x406538);
    //	SPI_CS5463_AC_WriteRegister(VG , 0x41B96C);
    
    /*型式试验样机*/
    //	SPI_CS5463_AC_WriteRegister(IDCOFF , 0xFE3014);	
    //	SPI_CS5463_AC_WriteRegister(VDCOFF , 0x06C0FD);
    //	SPI_CS5463_AC_WriteRegister(IACOFF , 0xFFFC00);
    //	SPI_CS5463_AC_WriteRegister(VACOFF , 0x000000);
    //	SPI_CS5463_AC_WriteRegister(IG , 0x429ec1);
    //	SPI_CS5463_AC_WriteRegister(VG , 0x6b661c);	                       
    ////	SPI_CS5463_AC_WriteRegister(POFF , 0xFA9E2B);
	
	ReadFM24CL64(32, temp,4);
	configsfr_AC_RD.Vdcoff =  temp[3]<<24 |  temp[2]<<16 | temp[1]<<8 | temp[0];
	ReadFM24CL64(36, temp,4);
	configsfr_AC_RD.Idcoff =  temp[3]<<24 |  temp[2]<<16 | temp[1]<<8 | temp[0];
	ReadFM24CL64(40, temp,4);
	configsfr_AC_RD.Vacoff =  temp[3]<<24 |  temp[2]<<16 | temp[1]<<8 | temp[0];
	ReadFM24CL64(44, temp,4);
	configsfr_AC_RD.Iacoff =  temp[3]<<24 |  temp[2]<<16 | temp[1]<<8 | temp[0];
	ReadFM24CL64(48, temp,4);
	configsfr_AC_RD.Vgain =  temp[3]<<24 |  temp[2]<<16 | temp[1]<<8 | temp[0];
	ReadFM24CL64(52, temp,4);
	configsfr_AC_RD.Igain =  temp[3]<<24 |  temp[2]<<16 | temp[1]<<8 | temp[0];	
	ReadFM24CL64(56, temp,4);
	configsfr_AC_RD.P_rate =  temp[3]<<24 |  temp[2]<<16 | temp[1]<<8 | temp[0];	
    //	printf("\r\n 直流电压偏移 = %x\n", configsfr_AC_RD.Vdcoff);
    //	printf("\r\n 直流电流偏移 = %x\n", configsfr_AC_RD.Idcoff);
    //	printf("\r\n 交流电压偏移 = %x\n", configsfr_AC_RD.Vacoff);
    //	printf("\r\n 交流电流偏移 = %x\n", configsfr_AC_RD.Iacoff);
    //	printf("\r\n 交流电压增益 = %x\n", configsfr_AC_RD.Vgain);
    //	printf("\r\n 交流电流增益 = %x\n", configsfr_AC_RD.Igain);
    
	if((configsfr_AC_RD.Vdcoff&0xFF000000)==0xAA000000) {
	    buf[0]=VDCOFF;
    	buf[1]=(configsfr_AC_RD.Vdcoff&0xFF0000)>>16;           
    	buf[2]=(configsfr_AC_RD.Vdcoff&0xFF00)>>8;           
     	buf[3]=configsfr_AC_RD.Vdcoff&0xFF;
        //电压偏移校准寄存器** 
	    SPI_CS5463_AC_Write(buf,4);
    } else {
	    buf[0]=VDCOFF;          
     	buf[1]=0x00;
    	buf[2]=0x00;           
    	buf[3]=0x00; 
        //电压偏移校准寄存器** 
	    SPI_CS5463_AC_Write(buf,4);     										
    }
    
	if((configsfr_AC_RD.Idcoff&0xFF000000)==0xAA000000) {
	    buf[0]=IDCOFF;          
    	buf[1]=(configsfr_AC_RD.Idcoff&0xFF0000)>>16;  
    	buf[2]=(configsfr_AC_RD.Idcoff&0xFF00)>>8; 
    	buf[3]=configsfr_AC_RD.Idcoff&0xFF; 
        //电流偏移校准寄存器** 
	    SPI_CS5463_AC_Write(buf,4);     										
	} else {
	    buf[0]=IDCOFF;          
     	buf[1]=0x00;
    	buf[2]=0x00;           
    	buf[3]=0x00;
        //电流偏移校准寄存器** 
	    SPI_CS5463_AC_Write(buf,4);     										
	}
	if((configsfr_AC_RD.Vacoff&0xFF000000)==0xAA000000) {
	    buf[0]=VACOFF;
    	buf[1]=(configsfr_AC_RD.Vacoff>>16)&0xFF; 
    	buf[2]=(configsfr_AC_RD.Vacoff>>8)&0xFF; 
     	buf[3]=configsfr_AC_RD.Vacoff&0xFF;
        //电压偏移校准寄存器** 
	    SPI_CS5463_AC_Write(buf,4);
    } else {
	    buf[0]=VACOFF;          
     	buf[1]=0x00;
    	buf[2]=0x00;           
    	buf[3]=0x00;
        //电压偏移校准寄存器** 
	    SPI_CS5463_AC_Write(buf,4); 
    }
	if((configsfr_AC_RD.Iacoff&0xFF000000)==0xAA000000) {
	    buf[0]=IACOFF;
		buf[1]=(configsfr_AC_RD.Iacoff&0xFF0000)>>16;             
    	buf[2]=(configsfr_AC_RD.Iacoff&0xFF00)>>8; 
    	buf[3]=configsfr_AC_RD.Iacoff&0xFF;
        //电流偏移校准寄存器** 
	    SPI_CS5463_AC_Write(buf,4);     										
	} else {
	    buf[0]=IACOFF;          
     	buf[1]=0x00;
    	buf[2]=0x00;           
    	buf[3]=0x00;
        //电流偏移校准寄存器** 
	    SPI_CS5463_AC_Write(buf,4);     										
	}
	if((configsfr_AC_RD.Vgain&0xFF000000)==0xAA000000) {
	    buf[0]=VG;          
    	buf[1]=(configsfr_AC_RD.Vgain&0xFF0000)>>16;
    	buf[2]=(configsfr_AC_RD.Vgain&0xFF00)>>8; 
     	buf[3]=configsfr_AC_RD.Vgain&0xFF;
        //电压增益校准寄存器** 
	    SPI_CS5463_AC_Write(buf,4);     										
    } else {
	    buf[0]=VG;          
     	buf[1]=0x40;
    	buf[2]=0x00;           
    	buf[3]=0x00;
        //电压增益校准寄存器** 
	    SPI_CS5463_AC_Write(buf,4);     										
    }
	if((configsfr_AC_RD.Igain&0xFF000000)==0xAA000000) {
	    buf[0]=IG;
		buf[1]=(configsfr_AC_RD.Igain&0xFF0000)>>16;
		buf[2]=(configsfr_AC_RD.Igain&0xFF00)>>8;                      
     	buf[3]=configsfr_AC_RD.Igain&0xFF;
        //电压增益校准寄存器** 
	    SPI_CS5463_AC_Write(buf,4); 
    } else {
	    buf[0]=IG;          
     	buf[1]=0x40;
    	buf[2]=0x00;           
    	buf[3]=0x00;
        //电压增益校准寄存器** 
	    SPI_CS5463_AC_Write(buf,4);     										
    }
    //状态寄存器（读）
	STATUS_VALUE = SPI_CS5463_AC_ReadRegister_INT32(STATUS);
    //写状态寄存器
	SPI_CS5463_AC_WriteRegister(STATUS , STATUS_VALUE);
    //屏蔽中断
	SPI_CS5463_AC_WriteRegister(INTMASK , INTMASK_DATA_AC);
    //控制寄存器初始化
	SPI_CS5463_AC_WriteRegister(CTRL , CTRL_DATA_AC);
    //转换周期数寄存器设置为4000
	SPI_CS5463_AC_WriteRegister(CYCLE_COUNT , CYCLE_COUNT_DATA_AC);	
    
	if((configsfr_AC_RD.P_rate&0xFF000000)==0xAA000000) {
	    buf[0]=PULSE_RATE;                    
    	buf[1]=(configsfr_AC_RD.P_rate&0xFF0000)>>16;
		buf[2]=(configsfr_AC_RD.P_rate&0xFF00)>>8;
		buf[3]=configsfr_AC_RD.P_rate&0xFF;
        //默认设置，1秒钟计算一次           
	    SPI_CS5463_AC_Write(buf,4);
    } else {
		configsfr_AC.P_rate = PULSE_RATE_DATA_AC; 
        //脉冲速率设置，约为10Hz
		SPI_CS5463_AC_WriteRegister(PULSE_RATE , PULSE_RATE_DATA_AC);
	}
        
	//设置操作模式，E2为电能方向、E3为无功电能
	SPI_CS5463_AC_WriteRegister(MODE , 0x000000); 					
    //写寄存器1页
	SPI_CS5463_AC_WriteRegister(PAGE , PAGE_1);	
    //设置脉冲宽度，5ms					
	SPI_CS5463_AC_WriteRegister(PULSE_WIDTH , 0x000014);			
    //恢复对寄存器0页操作
	SPI_CS5463_AC_WriteRegister(PAGE , PAGE_0);	
    //状态寄存器（读）
	STATUS_VALUE = SPI_CS5463_AC_ReadRegister_INT32(STATUS);	    
    //写状态寄存器
	SPI_CS5463_AC_WriteRegister(STATUS , STATUS_VALUE);			    
    
	SPI_CS5463_AC_CS_LOW(1);	
    //启动连续计算周期
	SPI_CS5463_AC_SendByte(CONTINUOUS_CYCLE);
	SPI_CS5463_AC_CS_HIGH(1);
}


void CS5463_AC_Adjust(MODBUS_CH  *pch, uint8_t mode)						//5463校准
{
	uint32_t i;
	uint8_t temp[4];
	uint8_t clear_buf[4] = {0};
    uint8_t cnts = CYCLE_COUNT_DATA_AC / 4000 + 1;
    
	float x;
    
    /***********************************************
    * 描述： 
    */
    SPI_CS5463_AC_SendByte(SYNC1);
	SPI_CS5463_AC_SendByte(SYNC1);
	SPI_CS5463_AC_SendByte(SYNC1);
	SPI_CS5463_AC_SendByte(SYNC0);
    //清转DRDY标志位
 	SPI_CS5463_AC_ClearDrdy();
    //等待DRDY标志位置位 
 	SPI_CS5463_AC_WaitDrdy();  
    //发送停止计数，中止A/D转换命令
    SPI_CS5463_AC_SendCommand(STOP);
    
    /***********************************************
    * 描述： 
    */
    //清转DRDY标志位
 	SPI_CS5463_AC_ClearDrdy();
    /***********************************************
    * 描述： 电流  交流偏置校准
    */
    switch(mode) {
        /***********************************************
        * 描述： 直流电压零点校准：
        *        周期计数器(N默认为4000) + 30 个AD转换周期
        *        在4.096MHz频率下约为1秒钟
        *        N越大，精度越高，
        */
    case VDCOFFC:
	 	SPI_CS5463_AC_ClearDrdy(); 
        //电压通道直流偏 
        SPI_CS5463_AC_WriteRegister(VDCOFF , 0x000000);
        //写校准命令寄存器
   		SPI_CS5463_AC_SendCommand(VDCOFFC);
    	for(i=0;i<50000000;i++);
		SPI_CS5463_AC_WaitDrdy();     									//等待校准完成
    	configsfr_AC.Vdcoff = SPI_CS5463_AC_ReadRegister_INT32(VDCOFF)|0xAA000000;	//读Idcoff 寄存器
    	TO_Arayy(temp , configsfr_AC.Vdcoff);
		WriteFM24CL64(32, temp, 4);
		uartprintf(pch,"\r\n 直流电压偏移 = %x\n", configsfr_AC.Vdcoff);
		break;    
        /***********************************************
        * 描述： 直流电流零点校准：
        *        周期计数器(N默认为4000) + 30 个AD转换周期
        *        在4.096MHz频率下约为1秒钟
        *        N越大，精度越高，
        */
    case IDCOFFC:
	 	SPI_CS5463_AC_ClearDrdy();
    	SPI_CS5463_AC_WriteRegister(IDCOFF , 0x000000);				    //电压增益校准寄存器**
		SPI_CS5463_AC_SendCommand(IDCOFFC);								//写校准命令寄存器
		for(i=0;i<50000000;i++);
    	SPI_CS5463_AC_WaitDrdy();     									//等待校准完成 										
    	configsfr_AC.Idcoff = SPI_CS5463_AC_ReadRegister_INT32(IDCOFF)|0xAA000000;	//读Idcoff 寄存器
    	TO_Arayy(temp , configsfr_AC.Idcoff);
		WriteFM24CL64(36, temp, 4);
		uartprintf(pch,"\r\n 直流电流偏移 = %x\n", configsfr_AC.Idcoff);
		break;
        /***********************************************
        * 描述： 交流电压零点校准：
        *        6 * 周期计数器(N默认为4000) + 30 个AD转换周期
        *        在4.096MHz频率下约为6秒钟
        *        N越大，精度越高，
        */
	case VACOFFC:
//		SPI_CS5463_AC_ClearDrdy(); 
//        //电压增益校准寄存器
//        SPI_CS5463_AC_WriteRegister(VACOFF , 0x000000);
//        //写校准命令寄存器
//    	SPI_CS5463_AC_SendCommand(VACOFFC);	
//		uartprintf(pch,"\r\n 交流电压零点校准中...\n");
//        //30个AD转换等待
//        //BSP_OS_TimeDly(OS_TICKS_PER_SEC / 10);          
        
        SPI_CS5463_AC_ClearDrdy();    
        SPI_CS5463_AC_WriteRegister(VACOFF , 0x000000); 				//μ??1??ò?D￡×???′??÷** 
    	SPI_CS5463_AC_SendCommand(VACOFFC);								//D′D￡×??üá???′??÷
    	for(i=0;i<100000000;i++);
		SPI_CS5463_AC_WaitDrdy();     									//μè′yD￡×?íê3é
		configsfr_AC.Vacoff = SPI_CS5463_AC_ReadRegister_INT32(VACOFF)|0xAA000000; //?áIdcoff ??′??÷
    	TO_Arayy(temp , configsfr_AC.Vacoff);
		WriteFM24CL64(40, temp, 4);
		uartprintf(pch,"\r\n 交流电压偏移 = %x\n", configsfr_AC.Vacoff);       
        
//        /***********************************************
//        * 描述： 等待校准结束
//        */
//        cnts = 10 * CYCLE_COUNT_DATA_AC / 4000 + 1;
//    	for(i=0;i<cnts;i++) {
//            uprintf("acvo%4d", i);
//            BSP_OS_TimeDly(OS_TICKS_PER_SEC);
//            //SPI_CS5463_AC_WaitDrdy(); 
//            if( TRUE == SPI_CS5463_AC_GetDrdy() );//break;
//        }
//        //读Idcoff 寄存器
//		configsfr_AC.Vacoff = SPI_CS5463_AC_ReadRegister_INT32(VACOFF)|0xAA000000; 
//    	TO_Arayy(temp , configsfr_AC.Vacoff);
//        //保存校准值
//		WriteFM24CL64(40, temp, 4);
//        //打印校准值
//		uartprintf(pch,"\r\n 交流电压偏移 = %x\n", configsfr_AC.Vacoff);
//        uprintf("acvo%4d", configsfr_AC.Vacoff & ~0xAA000000);
//        BSP_OS_TimeDly(OS_TICKS_PER_SEC);
		break;
        /***********************************************
        * 描述： 交流电流零点校准：
        *        6 * 周期计数器(N默认为4000) + 30 个AD转换周期
        *        在4.096MHz频率下约为6秒钟
        *        N越大，精度越高，
        */
	case IACOFFC:
        SPI_CS5463_AC_ClearDrdy();    
        SPI_CS5463_AC_WriteRegister(IACOFF , 0x000000); 				//μ??1??ò?D￡×???′??÷** 
  		SPI_CS5463_AC_SendCommand(IACOFFC);								//D′D￡×??üá???′??÷
    	for(i=0;i<100000000;i++);
		SPI_CS5463_AC_WaitDrdy();     									//μè′yD￡×?íê3é
		configsfr_AC.Iacoff = SPI_CS5463_AC_ReadRegister_INT32(IACOFF)|0xAA000000;	//?áIacoff ??′??÷
    	TO_Arayy(temp , configsfr_AC.Iacoff);
		WriteFM24CL64(44, temp, 4);
		uartprintf(pch,"\r\n 交流电流偏移 = %x\n", configsfr_AC.Iacoff);
        break;
//	 	SPI_CS5463_AC_ClearDrdy();    
//        //电压增益校准寄存器**
//        SPI_CS5463_AC_WriteRegister(IACOFF , 0x000000);
//        //写校准命令寄存器
//  		SPI_CS5463_AC_SendCommand(IACOFFC);	
//		uartprintf(pch,"\r\n 交流电流零点校准中...\n");
//        //30个AD转换等待
//        BSP_OS_TimeDly(OS_TICKS_PER_SEC / 10);        
//        /***********************************************
//        * 描述： 等待校准结束
//        */
//        cnts = 10 * CYCLE_COUNT_DATA_AC / 4000 + 1;
//    	for(i=0;i<cnts;i++) {
//            uprintf("acao%4d", i);
//            BSP_OS_TimeDly(OS_TICKS_PER_SEC);
//            //SPI_CS5463_AC_WaitDrdy(); 
//            if( TRUE == SPI_CS5463_AC_GetDrdy() ) ;//break;
//        }
//        //读Iacoff 寄存器
//		configsfr_AC.Iacoff = SPI_CS5463_AC_ReadRegister_INT32(IACOFF)|0xAA000000;
//    	TO_Arayy(temp , configsfr_AC.Iacoff);
//        //保存校准值
//		WriteFM24CL64(44, temp, 4);
//        //打印校准值
//		uartprintf(pch,"\r\n 交流电流偏移 = %x\n", configsfr_AC.Iacoff);
//        uprintf("acao%4d", configsfr_AC.Iacoff & ~0xAA000000);
//        BSP_OS_TimeDly(OS_TICKS_PER_SEC);
//		break;
        /***********************************************
        * 描述： 电压增益校准
        */
	case VACGAINC:
      	SPI_CS5463_AC_WriteRegister(VG , 0x400000); 					//μ??1??ò?D￡×???′??÷** 
		SPI_CS5463_AC_SendCommand(VACGAINC);								//D′D￡×??üá???′??÷
    	for(i=0;i<50000000;i++);
		SPI_CS5463_AC_WaitDrdy();     									  //μè′yD￡×?íê3é
		configsfr_AC.Vgain = SPI_CS5463_AC_ReadRegister_INT32(VG)|0xAA000000; //?áIgain ??′??÷
		TO_Arayy(temp , configsfr_AC.Vgain);
		WriteFM24CL64(48, temp, 4);
		SPI_CS5463_AC_ClearDrdy();
		SPI_CS5463_AC_SendByte(CONTINUOUS_CYCLE);
		SPI_CS5463_AC_WaitDrdy();
		x = SPI_CS5463_AC_Read_VIrms_FLOAT(VRMS);
		uartprintf(pch,"\r\n 交流电压增益 = %x\n", configsfr_AC.Vgain);
		uartprintf(pch,"\r\n 交流电压有效值 = %f\n", x);
        break;
//        //电压增益校准寄存器恢复默认值，即1.0倍增益
//      	SPI_CS5463_AC_WriteRegister(VG , 0x400000);
//        //写校准命令寄存器
//		SPI_CS5463_AC_SendCommand(VACGAINC);
//		uartprintf(pch,"\r\n 交流电压增益校准中...\n");
//        //30个AD转换等待
//        BSP_OS_TimeDly(OS_TICKS_PER_SEC / 10);        
//        /***********************************************
//        * 描述： 等待校准结束
//        */
//        cnts = 10 * CYCLE_COUNT_DATA_AC / 4000 + 1;
//    	for(i=0;i<cnts;i++) {
//            uprintf("acvv%4d", i);
//            BSP_OS_TimeDly(OS_TICKS_PER_SEC);
//            //SPI_CS5463_AC_WaitDrdy(); 
//            if( TRUE == SPI_CS5463_AC_GetDrdy() );//break;
//        }
//        //读Igain 寄存器
//		configsfr_AC.Vgain = SPI_CS5463_AC_ReadRegister_INT32(VG)|0xAA000000; 
//		TO_Arayy(temp , configsfr_AC.Vgain);
//        //保存校准值
//		WriteFM24CL64(48, temp, 4);
//        //清转DRDY标志位
//		SPI_CS5463_AC_ClearDrdy();
//        //启动连续计算周期
//		SPI_CS5463_AC_SendByte(CONTINUOUS_CYCLE);
//        //等待转换完成
//		SPI_CS5463_AC_WaitDrdy();
//        //读取电压有效值
//		x = SPI_CS5463_AC_Read_VIrms_FLOAT(VRMS);
//		uartprintf(pch,"\r\n 交流电压增益 = %x\n", configsfr_AC.Vgain);
//		uartprintf(pch,"\r\n 交流电压有效值 = %f\n", x);
//		break;
        /***********************************************
        * 描述： 电流增益校准
        */
	case IACGAINC:SPI_CS5463_AC_ClearDrdy();    
      	SPI_CS5463_AC_ClearDrdy();    
	    SPI_CS5463_AC_WriteRegister(IG , 0x400000); 					//μ?á÷??ò?D￡×???′??÷** 
    	SPI_CS5463_AC_SendCommand(IACGAINC);								//D′D￡×??üá???′??÷
    	for(i=0;i<50000000;i++);
		SPI_CS5463_AC_WaitDrdy();     									  //μè′yD￡×?íê3é
		configsfr_AC.Igain = SPI_CS5463_AC_ReadRegister_INT32(IG)|0xAA000000; //?áIgain ??′??÷
		TO_Arayy(temp , configsfr_AC.Igain);
		WriteFM24CL64(52, temp, 4);
		SPI_CS5463_AC_ClearDrdy();
		SPI_CS5463_AC_SendByte(CONTINUOUS_CYCLE);
		SPI_CS5463_AC_WaitDrdy();
		x = SPI_CS5463_AC_Read_VIrms_FLOAT(IRMS);
		uartprintf(pch,"\r\n 交流电流增益 = %x\n", configsfr_AC.Igain);
		uartprintf(pch,"\r\n 交流电流有效值 = %f\n", x);
        break;
//        //电流增益校准寄存器恢复默认值，即1.0倍增益
//	    SPI_CS5463_AC_WriteRegister(IG , 0x400000);
//        //写校准命令寄存器
//    	SPI_CS5463_AC_SendCommand(IACGAINC);
//		uartprintf(pch,"\r\n 交流电流增益校准中...\n");
//        //30个AD转换等待
//        BSP_OS_TimeDly(OS_TICKS_PER_SEC / 10);        
//        /***********************************************
//        * 描述： 等待校准结束
//        */
//        cnts = 10 * CYCLE_COUNT_DATA_AC / 4000 + 1;
//    	for(i=0;i<cnts;i++) {
//            uprintf("acav%4d", i);
//            BSP_OS_TimeDly(OS_TICKS_PER_SEC);
//            //SPI_CS5463_AC_WaitDrdy(); 
//            if( TRUE == SPI_CS5463_AC_GetDrdy() );//break;
//        }//等待校准完成
//		configsfr_AC.Igain = SPI_CS5463_AC_ReadRegister_INT32(IG)|0xAA000000; //读Igain 寄存器
//		TO_Arayy(temp , configsfr_AC.Igain);
//		WriteFM24CL64(52, temp, 4);
//        //清转DRDY标志位
//		SPI_CS5463_AC_ClearDrdy();
//        //启动连续计算周期
//		SPI_CS5463_AC_SendByte(CONTINUOUS_CYCLE);
//        //等待转换完成
//		SPI_CS5463_AC_WaitDrdy();
//        //读取电流有效值
//		x = SPI_CS5463_AC_Read_VIrms_FLOAT(IRMS);
//		uartprintf(pch,"\r\n 交流电流增益 = %x\n", configsfr_AC.Igain);
//		uartprintf(pch,"\r\n 交流电流有效值 = %f\n", x);
//		break;
        /***********************************************
        * 描述： 
        */
	case P_rate_ADD:
		configsfr_AC.P_rate += 50;                      //脉冲速率加5
		configsfr_AC.P_rate |=0xAA000000;
		TO_Arayy(temp , configsfr_AC.P_rate);
		WriteFM24CL64(56, temp, 4);
		SPI_CS5463_AC_ClearDrdy();
		SPI_CS5463_AC_SendByte(CONTINUOUS_CYCLE);
		SPI_CS5463_AC_WaitDrdy();
		uartprintf(pch,"\r\n 脉冲速率设置 = %x\n", configsfr_AC.P_rate);
		break;
        /***********************************************
        * 描述： 
        */
	case P_rate_SUB:
		configsfr_AC.P_rate -= 50;                      //脉冲速率减5
		configsfr_AC.P_rate |=0xAA000000;
		TO_Arayy(temp , configsfr_AC.P_rate);
		WriteFM24CL64(56, temp, 4);
		SPI_CS5463_AC_ClearDrdy();
		SPI_CS5463_AC_SendByte(CONTINUOUS_CYCLE);
		SPI_CS5463_AC_WaitDrdy();
		uartprintf(pch,"\r\n 脉冲速率设置 = %x\n", configsfr_AC.P_rate);
		break;
        /***********************************************
        * 描述： 
        */
    case P_rate_DEL:
	  	WriteFM24CL64(56, clear_buf, 4);
		ReadFM24CL64(56, temp,4);
		configsfr_AC_RD.P_rate =  temp[3]<<24 |  temp[2]<<16 | temp[1]<<8 | temp[0];
		uartprintf(pch,"\r\n 脉冲速率设置 = %x\n", configsfr_AC_RD.P_rate);
		break;
        /***********************************************
        * 描述： 
        */
    case P_rate_SET: {
        float x = (float)((pch->RxFrameData[4] & 0x0f)) * 0.1
                + (float)((pch->RxFrameData[5] >> 4) & 0x0f) * 0.01
                + (float)((pch->RxFrameData[5] >> 0) & 0x0f) * 0.001
                + (float)((pch->RxFrameData[6] >> 4) & 0x0f) * 0.0001
                + (float)((pch->RxFrameData[6] >> 0) & 0x0f) * 0.00001
                + (float)((pch->RxFrameData[7] >> 4) & 0x0f) * 0.000001
                + (float)((pch->RxFrameData[7] >> 0) & 0x0f) * 0.0000001
                + (float)((pch->RxFrameData[8] >> 4) & 0x0f) * 0.00000001
                + (float)((pch->RxFrameData[8] >> 0) & 0x0f) * 0.000000001;
        
		configsfr_AC.P_rate = x * 0x800000; 
        //脉冲速率设置
		SPI_CS5463_AC_WriteRegister(PULSE_RATE , configsfr_AC.P_rate);
        //configsfr_AC.P_rate | 0xAA000000;
        
        
	    temp[3]=0XAA;                    
    	temp[2]=(configsfr_AC.P_rate&0xFF0000)>>16;
		temp[1]=(configsfr_AC.P_rate&0xFF00)>>8;
		temp[0]=configsfr_AC.P_rate&0xFF;  
        
	  	WriteFM24CL64(56, temp, 4);
		ReadFM24CL64(56, temp,4);
		configsfr_AC_RD.P_rate =  temp[3]<<24 |  temp[2]<<16 | temp[1]<<8 | temp[0];
		uartprintf(pch,"\r\n 脉冲速率设置 = %x\n", configsfr_AC_RD.P_rate);
        }break;
	default:
		break;
	}
    
    /***********************************************
    * 描述： 启动测量
    */
	CS5463_AC_INIT();
}

/******************* (C) COPYRIGHT 2010 www.armjishu.com *****END OF FILE****/
