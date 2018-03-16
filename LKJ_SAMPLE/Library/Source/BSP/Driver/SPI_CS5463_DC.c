/******************** (C) COPYRIGHT 2013 www.armjishu.com  ********************
 * 文件名  ：SPI_CS5463_DC.c
 * 描述    ：实现CS5463的底层函数
 * 实验平台：STM32神舟开发板
 * 标准库  ：STM32F10x_StdPeriph_Driver V3.5.0
 * 作者    ：zw
*******************************************************************************/

/* Includes ------------------------------------------------------------------*/
#include "SPI_CS5463_DC.h"
#include "SZ_STM32F107VC_LIB.h"
#include "DELAY.h"
#ifdef PHOTOELECTRIC_VELOCITY_MEASUREMENT
#include <Speed_sensor.h>
#include "app_task_mater.h"
#else
#include <power_macro.h>
#endif


#define SPI_CS5463_DC                   SPI1

#define SPI_CS5463_DC_CLK               RCC_APB2Periph_SPI1
#define SPI_CS5463_DC_SCK_PIN           GPIO_Pin_5              /* PA.05 */
#define SPI_CS5463_DC_SCK_GPIO_PORT     GPIOA
#define SPI_CS5463_DC_SCK_GPIO_CLK      RCC_APB2Periph_GPIOA

#define SPI_CS5463_DC_MISO_PIN          GPIO_Pin_6              /* PA.06 */
#define SPI_CS5463_DC_MISO_GPIO_PORT    GPIOA
#define SPI_CS5463_DC_MISO_GPIO_CLK     RCC_APB2Periph_GPIOA

#define SPI_CS5463_DC_MOSI_PIN          GPIO_Pin_7              /* PA.07 */
#define SPI_CS5463_DC_MOSI_GPIO_PORT    GPIOA
#define SPI_CS5463_DC_MOSI_GPIO_CLK     RCC_APB2Periph_GPIOA

#define SPI_CS5463_DC_CS_PIN_NUM        4                       /* PA.04 */
#define SPI_CS5463_DC_CS_PIN            GPIO_Pin_4  
#define SPI_CS5463_DC_CS_GPIO_PORT      GPIOA
#define SPI_CS5463_DC_CS_GPIO_CLK       RCC_APB2Periph_GPIOA

#define SPI_RST_DC_PIN_NUM              0
#define SPI_RST_DC_PIN                  GPIO_Pin_0   		    /* PB.00 */
#define SPI_RST_DC_GPIO_PORT            GPIOB
#define SPI_RST_DC_GPIO_CLK             RCC_APB2Periph_GPIOB

#define E1_DC_PIN_NUM                   0
#define E1_DC_PIN                       GPIO_Pin_0
#define E1_DC_TIMER_PORT                GPIOA
#define E1_DC_TIMER_CLK                 RCC_APB2Periph_GPIOA
//#define E1_DC_TIMER_CLK                 RCC_APB1Periph_TIM5

#define E2_DC_PIN_NUM                   9
#define E2_DC_PIN                       GPIO_Pin_9
#define E2_DC_TIMER_PORT                GPIOE
#define E2_DC_GPIO_CLK                  RCC_APB2Periph_GPIOE


/*寄存器读写*/

#define CONFIG_DATA_DC                  0x000001
#define STATUS_DATA_DC                  0x800000
#define INTMASK_DATA_DC                 0x000000
#define CTRL_DATA_DC                    0x000005
#define CYCLE_COUNT_DATA_DC             0x000FA0
#define PULSE_RATE_DATA_DC              0x00BE95
#define MODE_DATA_DC                    0x000000

CONFIGSFR   configsfr_DC;

/* Private macro -------------------------------------------------------------*/
/* Select SPI FLASH: Chip Select pin low  */

//#define SPI_CS5463_DC_CS_LOW()   SPI_SSOutputCmd(SPI1, ENABLE)		   //NSS输出使能
//#define SPI_CS5463_DC_CS_HIGH()  SPI_SSOutputCmd(SPI1, DISABLE)		   //NSS输出禁止
#define SPI_CS5463_DC_CS_LOW()   (Periph_BB((uint32_t) & SPI_CS5463_DC_CS_GPIO_PORT->ODR, SPI_CS5463_DC_CS_PIN_NUM) = 0)
#define SPI_CS5463_DC_CS_HIGH()  (Periph_BB((uint32_t) & SPI_CS5463_DC_CS_GPIO_PORT->ODR, SPI_CS5463_DC_CS_PIN_NUM) = 1)

#define SPI_RST_DC_LOW()         GPIO_ResetBits(SPI_RST_DC_GPIO_PORT, SPI_RST_DC_PIN)
#define SPI_RST_DC_HIGH()        GPIO_SetBits(SPI_RST_DC_GPIO_PORT, SPI_RST_DC_PIN)



/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/


/**-----------------------------------------------------------------
  * @函数名 SPI_DC_INIT
  * @功能   初始化与CS5463接口的驱动函数
  * @参数   无
  * @返回值 无
***----------------------------------------------------------------*/
void SPI_DC_INIT(void)
{
    SPI_InitTypeDef  SPI_InitStructure;
    GPIO_InitTypeDef GPIO_InitStructure;

    /* Enable SPI1 and GPIO clocks */
    /*!< SPI_CS5463_DC_CS_GPIO, SPI_CS5463_DC_MOSI_GPIO, 
       SPI_CS5463_DC_MISO_GPIO, SPI_CS5463_DC_DETECT_GPIO 
       and SPI_CS5463_DC_SCK_GPIO Periph clock enable */
    RCC_APB2PeriphClockCmd(SPI_CS5463_DC_CS_GPIO_CLK | SPI_CS5463_DC_MOSI_GPIO_CLK |
                         SPI_CS5463_DC_MISO_GPIO_CLK | SPI_CS5463_DC_SCK_GPIO_CLK, ENABLE);

    /*!< SPI_CS5463_AC Periph clock enable */
    RCC_APB2PeriphClockCmd(SPI_CS5463_DC_CLK, ENABLE);
    
    /*!< AFIO Periph clock enable */
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);

    /*!< Configure SPI_CS5463_AC pins: SCK */
    GPIO_InitStructure.GPIO_Pin = SPI_CS5463_DC_SCK_PIN;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_Init(SPI_CS5463_DC_SCK_GPIO_PORT, &GPIO_InitStructure);

    /*!< Configure SPI_CS5463_DC pins: MOSI */
    GPIO_InitStructure.GPIO_Pin = SPI_CS5463_DC_MOSI_PIN;			//speed和mode无需修改，故无重新赋值
    GPIO_Init(SPI_CS5463_DC_MOSI_GPIO_PORT, &GPIO_InitStructure);

    /*!< Configure SPI_CS5463_DC pins: MISO */
    GPIO_InitStructure.GPIO_Pin = SPI_CS5463_DC_MISO_PIN;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;//GPIO_Mode_IN_FLOATING;//
	//GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_OD;
    GPIO_Init(SPI_CS5463_DC_MISO_GPIO_PORT, &GPIO_InitStructure);

    /*!< Configure SPI_CS5463_AC_CS_PIN pin: CS5463 Chip CS pin */
    GPIO_InitStructure.GPIO_Pin = SPI_CS5463_DC_CS_PIN;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	//GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_Init(SPI_CS5463_DC_CS_GPIO_PORT, &GPIO_InitStructure);

	/*!< Configure SPI_CS5463_AC pins: RST */
    RCC_APB2PeriphClockCmd(SPI_RST_DC_GPIO_CLK, ENABLE);
	GPIO_InitStructure.GPIO_Pin = SPI_RST_DC_PIN;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_Init(SPI_RST_DC_GPIO_PORT, &GPIO_InitStructure);

	RCC_APB1PeriphClockCmd(E1_DC_TIMER_CLK | E2_DC_GPIO_CLK , ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);
	GPIO_InitStructure.GPIO_Pin = E1_DC_PIN;
//    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPD;
    GPIO_Init(E1_DC_TIMER_PORT , &GPIO_InitStructure);
//	GPIO_Init(E2_DC_TIMER_PORT , &GPIO_InitStructure);          //E1、E2为同一个PORT，此处屏蔽

    /* Deselect the FLASH: Chip Select high */
    SPI_CS5463_DC_CS_HIGH();

	/* SPI1 configuration */
    // data input on the DIO pin is sampled on the rising edge of the CLK. 
    // Data on the DO and DIO pins are clocked out on the falling edge of CLK.
    SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
    SPI_InitStructure.SPI_Mode = SPI_Mode_Master;		 //设置SSI为1，即SPI为主模式，
    SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;
    SPI_InitStructure.SPI_CPOL = SPI_CPOL_High;			 //CPOL = 1,时钟极性为1，上升沿数据锁存
    SPI_InitStructure.SPI_CPHA = SPI_CPHA_2Edge;		 //CPHA = 1,相位为1，第2个边沿锁存
    SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;			 //设置SSM为1，启用软件从设备管理
    SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_4;
    SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;
    SPI_InitStructure.SPI_CRCPolynomial = 7;
    SPI_Init(SPI1, &SPI_InitStructure);

//	SPI_SSOutputCmd(SPI1, ENABLE);      //使能SPI1的NSS输出
    /* Enable SPI1  */
    SPI_Cmd(SPI1, ENABLE);

//  SPI_CS5463_DC_SOFT_RESET();
}

/**-----------------------------------------------------------------
  * @函数名 CS5463_DC_INIT
  * @功能   CS5463初始化
  *         
  * @参数   无
  * @返回值 无
***----------------------------------------------------------------*/
void CS5463_DC_INIT(void)
{
	uint32_t STATUS_VALUE = 0;
	SPI_RST_DC_LOW();
	SPI_RST_DC_LOW();
	SPI_RST_DC_LOW();
	Delay(1);
	SPI_RST_DC_HIGH() ;
	SPI_RST_DC_HIGH();
	SPI_RST_DC_HIGH();
	Delay(1000);

//	SPI_CS5463_DC_CS_LOW();

  /* Send  instruction */
  	SPI_CS5463_DC_SendByte(SYNC1);
	SPI_CS5463_DC_SendByte(SYNC1);
	SPI_CS5463_DC_SendByte(SYNC1);
  	SPI_CS5463_DC_SendByte(SYNC0);

	SPI_CS5463_DC_WriteRegister(CONFIG , CONFIG_DATA_DC);

/*   以下为150V 和 1A情况下校准，芯片输入引脚对地直接短接  */
	SPI_CS5463_DC_WriteRegister(IG , 0x488C31);
	SPI_CS5463_DC_WriteRegister(VG , 0x682E94);
	SPI_CS5463_DC_WriteRegister(IDCOFF , 0xFE396B);
	SPI_CS5463_DC_WriteRegister(VDCOFF , 0x072C2E);
	SPI_CS5463_DC_WriteRegister(IACOFF , 0xFFFC8A);
	SPI_CS5463_DC_WriteRegister(VACOFF , 0xFFF800);


/*   以下为187.5V 和 1.2A情况下校准，且只短接了输入线对地  */
//	SPI_CS5463_DC_WriteRegister(IDCOFF , 0xFE4F47);
//	SPI_CS5463_DC_WriteRegister(VDCOFF , 0x075074);
//	SPI_CS5463_DC_WriteRegister(IACOFF , 0xFFF100);
//	SPI_CS5463_DC_WriteRegister(VACOFF , 0xFFF100);
//	SPI_CS5463_DC_WriteRegister(IG , 0x3C764F);
//	SPI_CS5463_DC_WriteRegister(VG , 0x531018);

	STATUS_VALUE = SPI_CS5463_DC_ReadRegister_INT32(STATUS);		    //读取状态寄存器
	SPI_CS5463_DC_WriteRegister(STATUS , STATUS_VALUE);					//写状态寄存器

	SPI_CS5463_DC_WriteRegister(INTMASK , INTMASK_DATA_DC);				//屏蔽中断

	SPI_CS5463_DC_WriteRegister(CTRL , CTRL_DATA_DC);		  			//写控制寄存器

	SPI_CS5463_DC_WriteRegister(CYCLE_COUNT , CYCLE_COUNT_DATA_DC);		//A/D转换周期数

	SPI_CS5463_DC_WriteRegister(PULSE_RATE , PULSE_RATE_DATA_DC);			//能量脉冲速率

	SPI_CS5463_DC_WriteRegister(PAGE , PAGE_1);						//写寄存器1页
	SPI_CS5463_DC_WriteRegister(PULSE_WIDTH , 0x000002);			//设置脉冲宽度，10ms
	SPI_CS5463_DC_WriteRegister(PAGE , PAGE_0);						//恢复对寄存器0页操作

	STATUS_VALUE = SPI_CS5463_DC_ReadRegister_INT32(STATUS);		    //读取状态寄存器
	SPI_CS5463_DC_WriteRegister(STATUS , STATUS_VALUE);					//写状态寄存器

	SPI_CS5463_DC_SendByte(CONTINUOUS_CYCLE);				            //启动连续计算周期

    /* Deselect the CS5463: Chip Select high */
    SPI_CS5463_DC_CS_HIGH();
	Delay(10);
}

/**-----------------------------------------------------------------
  * @函数名 CS5463_DC_Adjust
  * @功能   CS5463校准
  *         
  * @参数   无
  * @返回值 无
***----------------------------------------------------------------*/
void CS5463_DC_Adjust(void)						//5463校准
{
	uint32_t i;
	float x;
	uint8_t mode;
    SPI_CS5463_DC_SendByte(SYNC1);
	SPI_CS5463_DC_SendByte(SYNC1);
	SPI_CS5463_DC_SendByte(SYNC1);
	SPI_CS5463_DC_SendByte(SYNC0);   										
 	SPI_CS5463_DC_ClearDrdy();    
 	SPI_CS5463_DC_WaitDrdy();  
    SPI_CS5463_DC_SendCommand(STOP); 									//发送POWER_HALT 命令
   
    mode = IACGAINC;
    //电流  交流偏置校准：
 	SPI_CS5463_DC_ClearDrdy();    
    switch(mode)
    {
    case IDCOFFC:														//直流零点校准
	 	SPI_CS5463_DC_ClearDrdy();        								 
    	SPI_CS5463_DC_WriteRegister(IDCOFF , 0x000000);				    //电压增益校准寄存器**
		SPI_CS5463_DC_SendCommand(IDCOFFC);								//写校准命令寄存器
		for(i=0;i<100000000;i++);
    	SPI_CS5463_DC_WaitDrdy();     									//等待校准完成 										
    	configsfr_DC.Idcoff = SPI_CS5463_DC_ReadRegister_INT32(IDCOFF)|0xAA000000;	//读Idcoff 寄存器
    	printf("\r\n 直流电流偏移 = %x\n", configsfr_DC.Idcoff);
		break;
    case VDCOFFC:														//直流零点校准
	 	SPI_CS5463_DC_ClearDrdy();    
        SPI_CS5463_DC_WriteRegister(VDCOFF , 0x000000); 				//电压增益校准寄存器** 
   		SPI_CS5463_DC_SendCommand(VDCOFFC);								//写校准命令寄存器
    	for(i=0;i<100000000;i++);
		SPI_CS5463_DC_WaitDrdy();     									//等待校准完成
    	configsfr_DC.Vdcoff = SPI_CS5463_DC_ReadRegister_INT32(VDCOFF)|0xAA000000;	//读Idcoff 寄存器
    	printf("\r\n 直流电压偏移 = %x\n", configsfr_DC.Vdcoff);
		break;
    case IACOFFC:														//交流零点校准
	 	SPI_CS5463_DC_ClearDrdy();    
        SPI_CS5463_DC_WriteRegister(IACOFF , 0x000000); 				//电压增益校准寄存器** 
  		SPI_CS5463_DC_SendCommand(IACOFFC);								//写校准命令寄存器
    	for(i=0;i<100000000;i++);
		SPI_CS5463_DC_WaitDrdy();     									//等待校准完成
		configsfr_DC.Iacoff = SPI_CS5463_DC_ReadRegister_INT32(IACOFF)|0xAA000000;	//读Iacoff 寄存器
    	printf("\r\n 交流电流偏移 = %x\n", configsfr_DC.Iacoff);
		break;
    case VACOFFC:														//交流零点校准
		SPI_CS5463_DC_ClearDrdy();    
        SPI_CS5463_DC_WriteRegister(VACOFF , 0x000000); 				//电压增益校准寄存器** 
    	SPI_CS5463_DC_SendCommand(VACOFFC);								//写校准命令寄存器
    	for(i=0;i<100000000;i++);
		SPI_CS5463_DC_WaitDrdy();     									//等待校准完成
		configsfr_DC.Vacoff = SPI_CS5463_DC_ReadRegister_INT32(VACOFF)|0xAA000000; //读Idcoff 寄存器
    	printf("\r\n 交流电压偏移 = %x\n", configsfr_DC.Vacoff);
		break;
	case VACGAINC:														//电压增益校准
		SPI_CS5463_DC_ClearDrdy();    
      	SPI_CS5463_DC_WriteRegister(VG , 0x400000); 					//电压增益校准寄存器** 
		SPI_CS5463_DC_SendCommand(VACGAINC);								//写校准命令寄存器
    	for(i=0;i<50000000;i++);
		SPI_CS5463_DC_WaitDrdy();     									  //等待校准完成
		configsfr_DC.Vgain = SPI_CS5463_DC_ReadRegister_INT32(VG)|0xAA000000; //读Igain 寄存器
//    	x = SPI_CS5463_DC_Read_VIgain_FLOAT(VG);
		SPI_CS5463_DC_ClearDrdy();
		SPI_CS5463_DC_SendByte(CONTINUOUS_CYCLE);
		SPI_CS5463_DC_WaitDrdy();
		x = SPI_CS5463_DC_Read_VIrms_FLOAT(VRMS);
		printf("\r\n 交流电压增益 = %x\n", configsfr_DC.Vgain);
		printf("\r\n 交流电压有效值 = %f\n", x);
		break;
	case IACGAINC:														//电流增益校准
		SPI_CS5463_DC_ClearDrdy();    
	    SPI_CS5463_DC_WriteRegister(IG , 0x400000); 					//电流增益校准寄存器** 
    	SPI_CS5463_DC_SendCommand(IACGAINC);								//写校准命令寄存器
    	for(i=0;i<50000000;i++);
		SPI_CS5463_DC_WaitDrdy();     									  //等待校准完成
		configsfr_DC.Igain = SPI_CS5463_DC_ReadRegister_INT32(IG)|0xAA000000; //读Igain 寄存器
//    	x = SPI_CS5463_DC_Read_VIgain_FLOAT(VG);
		SPI_CS5463_DC_ClearDrdy();
		SPI_CS5463_DC_SendByte(CONTINUOUS_CYCLE);
		SPI_CS5463_DC_WaitDrdy();
		x = SPI_CS5463_DC_Read_VIrms_FLOAT(IRMS);
		printf("\r\n 交流电流增益 = %x\n", configsfr_DC.Igain);
		printf("\r\n 交流电流有效值 = %f\n", x);
		break;   
    }
	CS5463_DC_INIT();			 												//启动测量
}

/**-----------------------------------------------------------------
  * @函数名 SPI_CS5463_AC_SendByte
  * @功能   通过SPI总线发送一个字节数据(同时接收一个字节数据)
  *         Sends a byte through the SPI interface and return the byte
  *         received from the SPI bus.
  * @参数   要写入的一个字节数据
  * @返回值 在发数据时，MISO信号线上接收的一个字节
***----------------------------------------------------------------*/
uint8_t SPI_CS5463_DC_SendByte(uint8_t byte)
{
    SPI_CS5463_DC_CS_LOW();
//	Delay(10);
    /* Loop while DR register in not emplty */
    while (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_TXE) == RESET);

    /* Send byte through the SPI1 peripheral */
    SPI_I2S_SendData(SPI1, byte);

    /* Wait to receive a byte */
    while (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_RXNE) == RESET);

//    SPI_CS5463_DC_CS_HIGH();
		/* Return the byte read from the SPI bus */
    return SPI_I2S_ReceiveData(SPI1);	
//	Delay(10);
}

/**-----------------------------------------------------------------
  * @函数名 SPI_CS5463_DC_ReadByte
  * @功能   读取CS5463的一个字节，未包含发送读命令和起始地址
  * @参数   无
  * @返回值 从CS5463读取的一个字节
***----------------------------------------------------------------*/
uint8_t SPI_CS5463_DC_ReadByte(void)
{
    return (SPI_CS5463_DC_SendByte(Dummy_Byte));
}

/**-----------------------------------------------------------------
  * @函数名 SPI_CS5463_DC_SendCommand
  * @功能   通过SPI总线发送一个字节命令
  *         Sends a byte through the SPI interface
  * @参数   要写入的一个字节命令
  * @返回值 无
***----------------------------------------------------------------*/
void SPI_CS5463_DC_SendCommand(uint8_t Command)
{
	SPI_CS5463_DC_CS_LOW();
	Delay(5);
	SPI_CS5463_DC_SendByte(Command);
	SPI_CS5463_DC_CS_HIGH();
	Delay(5);
}

/**-----------------------------------------------------------------
  * @函数名 SPI_CS5463_DC_Write
  * @功能   往CS5463中写任意长度数据
  * @参数   *buf 数组，len 数组长度(以字节为最小单位)
***----------------------------------------------------------------*/
void SPI_CS5463_DC_Write(uint8_t *buf , uint8_t len)
{
	uint8_t i;
	/* Select the CS5463: Chip Select low */
	SPI_CS5463_DC_CS_LOW();
	Delay(2);
	buf[0]|=0x40;
	for(i=0;i<len;i++)
		SPI_CS5463_DC_SendByte(buf[i]);
	Delay(1);
	SPI_CS5463_DC_CS_HIGH();
	Delay(1);
}

/**-----------------------------------------------------------------
  * @函数名 SPI_CS5463_DC_WriteRegister
  * @功能   写CS5463的寄存器
  * @参数   Command为命令，DATA为数据
***----------------------------------------------------------------*/
void SPI_CS5463_DC_WriteRegister(uint8_t Command , uint32_t DATA)
{
	uint8_t buf[4];
	/* Select the CS5463: Chip Select low */
	buf[0] = Command;
	buf[1] = (DATA >> 16) & 0xFF;
	buf[2] = (DATA >> 8) & 0xFF;
	buf[3] = DATA & 0xFF;
	SPI_CS5463_DC_Write(buf , 4);
}

/**-----------------------------------------------------------------
  * @函数名 SPI_CS5463_DC_Read
  * @功能   读取CS5463的寄存器
  * @参数   Command：单字节命令,buf为读取到的数组
***----------------------------------------------------------------*/
void SPI_CS5463_DC_Read(uint8_t Command , uint8_t *buf)
{
	uint8_t i;
  /* Select the CS5463: Chip Select low */
    SPI_CS5463_DC_CS_LOW();
	Delay(2);
  /* Send  instruction */
    SPI_CS5463_DC_SendByte(Command);
	for(i=1;i<4;i++)
		buf[i] = SPI_CS5463_DC_SendByte(0xFE);
	buf[0] = Command;
	Delay(1);
	SPI_CS5463_DC_CS_HIGH();
	Delay(2);
}

/**-----------------------------------------------------------------
  * @函数名 SPI_CS5463_DC_ReadRegister_INT32
  * @功能   读取CS5463的寄存器，得到1个3字节数据
  * @参数   Command：单字节命令
  * @返回   Data，3字节数据
***----------------------------------------------------------------*/
uint32_t SPI_CS5463_DC_ReadRegister_INT32(uint8_t Command)
{
	uint32_t Data = 0;
	uint8_t Data0 = 0,Data1 = 0,Data2 = 0;
  /* Select the CS5463: Chip Select low */
  	SPI_CS5463_DC_CS_LOW();
	Delay(1);
  /* Send  instruction */
  	SPI_CS5463_DC_SendByte(Command);

	Data0 = SPI_CS5463_DC_SendByte(0xFE);
	Data1 = SPI_CS5463_DC_SendByte(0xFE);
	Data2 = SPI_CS5463_DC_SendByte(0xFE);
	Data = Data0<<16 | Data1<<8 | Data2;
	SPI_CS5463_DC_CS_HIGH();
	Delay(5);
	return Data;
}

/**-----------------------------------------------------------------
  * @函数名 SPI_CS5463_DC_Read_Temp_FLOAT
  * @功能   读取CS5463的寄存器，得到1个浮点型数据
  * @参数   Command：单字节命令
  * @返回   Data，浮点型数据
  * @备注   返回的的字节为-2^7,2^6到2^-16
***----------------------------------------------------------------*/
float SPI_CS5463_DC_Read_Temp_FLOAT(uint8_t Command)
{
	float Data = 0;
	uint8_t Data0 = 0,Data1 = 0,Data2 = 0;
  /* Select the CS5463: Chip Select low */
  	SPI_CS5463_DC_CS_LOW();
	Delay(1);
  /* Send  instruction */
  	SPI_CS5463_DC_SendByte(Command);
	
	Data0 = SPI_CS5463_DC_SendByte(0xFE);
	Data1 = SPI_CS5463_DC_SendByte(0xFE);
	Data2 = SPI_CS5463_DC_SendByte(0xFE);
	Data = ((uint8_t)((Data0>>7) & 0x01))*(-128) + (Data0 & 0x7F) + (float)Data1/256 + (float)Data2/65536;
	SPI_CS5463_DC_CS_HIGH();
	Delay(5);
	return Data;
}

/**-----------------------------------------------------------------
  * @函数名 SPI_CS5463_DC_Read_VIrms_FLOAT
  * @功能   读取CS5463的电压/电流有效值，得到1个浮点型数据
  * @参数   Command：单字节命令
  * @返回   Data，浮点型数据				  
  * @备注   返回的的字节为2^-1到2^-24
***----------------------------------------------------------------*/
float SPI_CS5463_DC_Read_VIrms_FLOAT(uint8_t Command)
{
	float Data = 0;
	uint8_t Data0 = 0,Data1 = 0,Data2 = 0;
  /* Select the CS5463: Chip Select low */
  	SPI_CS5463_DC_CS_LOW();
	Delay(1);
  /* Send  instruction */
  	SPI_CS5463_DC_SendByte(Command);
	
	Data0 = SPI_CS5463_DC_SendByte(0xFE);
	Data1 = SPI_CS5463_DC_SendByte(0xFE);
	Data2 = SPI_CS5463_DC_SendByte(0xFE);
	Data = (float)(Data0)/256 + (float)Data1/256/256 + (float)Data2/256/256/256;
	SPI_CS5463_DC_CS_HIGH();
	Delay(5);
	return Data;
}

/**-----------------------------------------------------------------
  * @函数名 SPI_CS5463_DC_Read_VIgain_FLOAT
  * @功能   读取CS5463的电压/电流增益，得到1个浮点型数据
  * @参数   Command：单字节命令
  * @返回   Data，浮点型数据
  * @备注   返回的的字节为2^1到2^0 + 2^-1到2^-22 (2^1到2^-22)
***----------------------------------------------------------------*/
float SPI_CS5463_DC_Read_VIgain_FLOAT(uint8_t Command)
{
	float Data = 0;
	uint8_t Data0 = 0,Data1 = 0,Data2 = 0;
  /* Select the CS5463: Chip Select low */
  	SPI_CS5463_DC_CS_LOW();
	Delay(1);
  /* Send  instruction */
  	SPI_CS5463_DC_SendByte(Command);
	
	Data0 = SPI_CS5463_DC_SendByte(0xFE);
	Data1 = SPI_CS5463_DC_SendByte(0xFE);
	Data2 = SPI_CS5463_DC_SendByte(0xFE);
	Data = ((Data0>>6)&0x03)+(float)(Data0&0x3F)/64 + (float)Data1/64/256 + (float)Data2/64/256/256;
	SPI_CS5463_DC_CS_HIGH();
	Delay(5);
	return Data;
}

/**-----------------------------------------------------------------
  * @函数名 SPI_CS5463_DC_Read_Else_FLOAT
  * @功能   读取CS5463的电压电流DC偏移，得到1个浮点型数据
  * @参数   Command：单字节命令
  * @返回   Data，浮点型数据
  * @备注   返回的的字节为2^-1到2^-23，再加上最高位负值-2^0
***----------------------------------------------------------------*/
float SPI_CS5463_DC_Read_Else_FLOAT(uint8_t Command)
{
	float Data = 0;
	uint8_t Data0 = 0,Data1 = 0,Data2 = 0;
  /* Select the CS5463: Chip Select low */
  	SPI_CS5463_DC_CS_LOW();
	Delay(1);
  /* Send  instruction */
  	SPI_CS5463_DC_SendByte(Command);
	
	Data0 = SPI_CS5463_DC_SendByte(0xFE);
	Data1 = SPI_CS5463_DC_SendByte(0xFE);
	Data2 = SPI_CS5463_DC_SendByte(0xFE);
	Data = (float)((Data0>>7)&0x01)*(-1)+(float)(Data0&0x7F)/128 + (float)Data1/32768 + (float)Data2/256/32768;
	SPI_CS5463_DC_CS_HIGH();
	Delay(5);
	return Data;
}

/**-----------------------------------------------------------------
  * @函数名 SPI_CS5463_DC_ClearDrdy
  * @功能   CS5463进入清除数据就绪状态位
  * @参数   无
  * @返回值 无
***----------------------------------------------------------------*/
void SPI_CS5463_DC_ClearDrdy(void)
{
	SPI_CS5463_DC_WriteRegister(STATUS , 0xFFFFFF);
}

/**-----------------------------------------------------------------
  * @函数名 SPI_CS5463_AC_WaitDrdy
  * @功能   CS5463等待数据就绪状态位
  * @参数   无
  * @返回值 无
***----------------------------------------------------------------*/
void SPI_CS5463_DC_WaitDrdy(void)
{
	while (SPI_CS5463_DC_ReadRegister_INT32(STATUS) & 0x800000 == 0); 	
}


/**-----------------------------------------------------------------
  * @函数名 SPI_SPI_CS5463_DC_SLEEP
  * @功能   CS5463进入睡眠模式
  * @参数   无
  * @返回值 无
***----------------------------------------------------------------*/
void SPI_CS5463_DC_SLEEP(void)   
{ 
	/* Select the CS5463: Chip Select low */
	SPI_CS5463_DC_CS_LOW();
	//  Delay(10);
	/* Send "WAIT" instruction */
	SPI_CS5463_DC_SendByte(SLEEP);
	
	/* Deselect the CS5463: Chip Select high */
	SPI_CS5463_DC_CS_HIGH();
	//  Delay(10);
}   

/**-----------------------------------------------------------------
  * @函数名 SPI_SPI_CS5463_DC_Wait
  * @功能   CS5463进入等待模式
  * @参数   无
  * @返回值 无
***----------------------------------------------------------------*/
void SPI_CS5463_DC_WAIT(void)   
{ 
	/* Select the CS5463: Chip Select low */
	SPI_CS5463_DC_CS_LOW();
	Delay(5);
	/* Send "WAIT" instruction */
	SPI_CS5463_DC_SendByte(WAIT);
	
	/* Deselect the CS5463: Chip Select high */
	SPI_CS5463_DC_CS_HIGH();
	Delay(5);
}   

/**-----------------------------------------------------------------
  * @函数名 SPI_CS5463_DC_SOFT_RESET
  * @功能   软件复位CS5463
  * @参数   无
  * @返回值 无
***----------------------------------------------------------------*/
void SPI_CS5463_DC_SOFT_RESET(void)   
{
  /* Select the CS5463: Chip Select low */
  SPI_CS5463_DC_CS_LOW();
  Delay(100);
  /* Send "SOFT_RESET" instruction */
  SPI_CS5463_DC_SendByte(SOFT_RESET);

  /* Deselect the CS5463: Chip Select high */
  SPI_CS5463_DC_CS_HIGH();
  Delay(100);
}   

/******************* (C) COPYRIGHT 2010 www.armjishu.com *****END OF FILE****/
