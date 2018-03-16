#include "stm32f10x.h"
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "SZ_STM32F107VC_LIB.h"
#include "DS3231.h"
#include "I2C_CLK.h"
#include "Display.h"
#include "DATA_STORAGE.h"
#include "DELAY.h"
#include "MX25.h"
#include "FM24CL64.h"
#ifdef PHOTOELECTRIC_VELOCITY_MEASUREMENT
#include <Speed_sensor.h>
#include "app_task_mater.h"
#else
#include <power_macro.h>
#endif
#include "crccheck.h"
#include "WatchDog.h"


#include  <global.h>
#include  <includes.h>

/****************************************Copyright (c)**************************************************
**                               Guangzou ZLG-MCU Development Co.,LTD.
**                                      graduate school
**                                 http://www.zlgmcu.com
**
**--------------File Info-------------------------------------------------------------------------------
** File name:            MX25L1602.c
** Descriptions:        SPI下的SST25VF016B操作函数库 
**
**------------------------------------------------------------------------------------------------------
** Created by:            Litiantian
** Created date:        2007-04-16
** Version:                1.0
** Descriptions:        The original version
**
**------------------------------------------------------------------------------------------------------
** Modified by:            
** Modified date:    
** Version:
** Descriptions:        
********************************************************************************************************/
#define SPI_FLASH_SPI                   SPI2
#define SPI_FLASH_SPI_CLK               RCC_APB1Periph_SPI2

#define SPI_FLASH_SPI_SCK_PIN           GPIO_Pin_13              /* PB.13 */
#define SPI_FLASH_SPI_SCK_GPIO_PORT     GPIOB
#define SPI_FLASH_SPI_SCK_GPIO_CLK      RCC_APB2Periph_GPIOB

#define SPI_FLASH_SPI_MISO_PIN          GPIO_Pin_14              /* PB.14 */
#define SPI_FLASH_SPI_MISO_GPIO_PORT    GPIOB
#define SPI_FLASH_SPI_MISO_GPIO_CLK     RCC_APB2Periph_GPIOB

#define SPI_FLASH_SPI_MOSI_PIN          GPIO_Pin_15              /* PB.15 */
#define SPI_FLASH_SPI_MOSI_GPIO_PORT    GPIOB
#define SPI_FLASH_SPI_MOSI_GPIO_CLK     RCC_APB2Periph_GPIOB

#define SPI_FLASH_CS_PIN_NUM            12                       /* PB.12 */
#define SPI_FLASH_CS_PIN                GPIO_Pin_12  
#define SPI_FLASH_CS_GPIO_PORT          GPIOB
#define SPI_FLASH_CS_GPIO_CLK           RCC_APB2Periph_GPIOB

/* Private typedef -----------------------------------------------------------*/
#define SPI_FLASH_PageSize              256
#define SPI_FLASH_PerWritePageSize      256


/* Private define ------------------------------------------------------------*/
#define W25X_WriteEnable                0x06 
#define W25X_WriteDisable               0x04 
#define W25X_ReadStatusReg              0x05 
#define W25X_WriteStatusReg             0x01 
#define W25X_ReadData                   0x03 
#define W25X_FastReadData               0x0B 
#define W25X_FastReadDual               0x3B 
#define W25X_PageProgram                0x02 
#define W25X_BlockErase                 0xD8 
#define W25X_SectorErase                0x20 
#define W25X_ChipErase                  0xC7 
#define W25X_PowerDown                  0xB9 
#define W25X_ReleasePowerDown           0xAB 
#define W25X_DeviceID                   0xAB 
#define W25X_ManufactDeviceID           0x90 
#define W25X_JedecDeviceID              0x9F 

#define WIP_FlagMask                    0x01  /* Write In Progress (WIP) flag */

#define Dummy_Byte1                     0xA5


/* Private macro -------------------------------------------------------------*/
/* Select SPI FLASH: Chip Select pin low  */
#define CE_Low()       (Periph_BB((uint32_t)&SPI_FLASH_CS_GPIO_PORT->ODR, SPI_FLASH_CS_PIN_NUM) = 0)

/* Deselect SPI FLASH: Chip Select pin high */
#define CE_High()      (Periph_BB((uint32_t)&SPI_FLASH_CS_GPIO_PORT->ODR, SPI_FLASH_CS_PIN_NUM) = 1)

#define  UCOS_EN            DEF_ENABLED

/***********************************************
* 描述： OS接口
*/
#if UCOS_EN     == DEF_ENABLED
    #if OS_VERSION > 30000U
    static  OS_SEM                   Bsp_EepSem;    // 信号量
    #else
    static  OS_EVENT                *Bsp_EepSem;    // 信号量
    #endif
#endif
  
    
/*******************************************************************************
* 名    称： EEP_WaitEvent
* 功    能： 等待信号量
* 入口参数： 无
* 出口参数：  0（操作有误），1（操作成功）
* 作　 　者： 无名沈
* 创建日期： 2015-08-18
* 修    改：
* 修改日期：
* 备    注： 仅在使用UCOS操作系统时使用
*******************************************************************************/
static uint8_t EEP_WaitEvent(void)
{
    /***********************************************
    * 描述： OS接口
    */
#if OS_VERSION > 30000U
    return BSP_OS_SemWait(&Bsp_EepSem,0);           // 等待信号量
#else
    uint8_t       err;
    OSSemPend(Bsp_EepSem,0,&err);                   // 等待信号量
    if ( err = OS_ERR_NONE )
      return TRUE;
    else
      return FALSE;
#endif
}


/*******************************************************************************
* 名    称： EEP_SendEvent
* 功    能： 释放信号量
* 入口参数： 无
* 出口参数： 无
* 作　 　者： 无名沈
* 创建日期： 2015-08-18
* 修    改：
* 修改日期：
* 备    注： 仅在使用UCOS操作系统时使用
*******************************************************************************/
static void EEP_SendEvent(void)
{
    /***********************************************
    * 描述： OS接口
    */
#if OS_VERSION > 30000U
    BSP_OS_SemPost(&Bsp_EepSem);                        // 发送信号量
#else
    uint8_t       err;
    OSSemPost(Bsp_EepSem);                              // 发送信号量
#endif
}

/**-----------------------------------------------------------------
  * @函数名 SPI_FLASH_Init
  * @功能   初始化与外部SPI FLASH接口的驱动函数
  *         Initializes the peripherals used by the SPI FLASH driver.
  * @参数   无
  * @返回值 无
***----------------------------------------------------------------*/
void SPI_FLASH_Init(void)
{
    SPI_InitTypeDef  SPI_InitStructure;
    GPIO_InitTypeDef GPIO_InitStructure;

    /* Enable SPI2 and GPIO clocks */
    /*!< SPI_FLASH_SPI_CS_GPIO, SPI_FLASH_SPI_MOSI_GPIO, 
       SPI_FLASH_SPI_MISO_GPIO, SPI_FLASH_SPI_DETECT_GPIO 
       and SPI_FLASH_SPI_SCK_GPIO Periph clock enable */
    RCC_APB2PeriphClockCmd(SPI_FLASH_CS_GPIO_CLK | SPI_FLASH_SPI_MOSI_GPIO_CLK |
                         SPI_FLASH_SPI_MISO_GPIO_CLK | SPI_FLASH_SPI_SCK_GPIO_CLK, ENABLE);

    /*!< SPI_FLASH_SPI Periph clock enable */
    RCC_APB1PeriphClockCmd(SPI_FLASH_SPI_CLK, ENABLE);
    
    /*!< AFIO Periph clock enable */
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);

    /*!< Configure SPI_FLASH_SPI pins: SCK */
    GPIO_InitStructure.GPIO_Pin = SPI_FLASH_SPI_SCK_PIN;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	//GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;			   //不能设置为推挽输出，？？
    GPIO_Init(SPI_FLASH_SPI_SCK_GPIO_PORT, &GPIO_InitStructure);

    /*!< Configure SPI_FLASH_SPI pins: MOSI */
    GPIO_InitStructure.GPIO_Pin = SPI_FLASH_SPI_MOSI_PIN;
    GPIO_Init(SPI_FLASH_SPI_MOSI_GPIO_PORT, &GPIO_InitStructure);

    /*!< Configure SPI_FLASH_SPI pins: MISO */
    GPIO_InitStructure.GPIO_Pin = SPI_FLASH_SPI_MISO_PIN;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;                //GPIO_Mode_IN_FLOATING;
	//GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;				   //设置为复用PP或OD均可
    GPIO_Init(SPI_FLASH_SPI_MISO_GPIO_PORT, &GPIO_InitStructure);

    /*!< Configure SPI_FLASH_SPI_CS_PIN pin: SPI_FLASH Card CS pin */
    GPIO_InitStructure.GPIO_Pin = SPI_FLASH_CS_PIN;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_Init(SPI_FLASH_CS_GPIO_PORT, &GPIO_InitStructure);

    /* Deselect the FLASH: Chip Select high */
    CE_High();

    /* SPI2 configuration */
    // W25X16: data input on the DIO pin is sampled on the rising edge of the CLK. 
    // Data on the DO and DIO pins are clocked out on the falling edge of CLK.
    SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
    SPI_InitStructure.SPI_Mode = SPI_Mode_Master;
    SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;
    SPI_InitStructure.SPI_CPOL = SPI_CPOL_High;
    SPI_InitStructure.SPI_CPHA = SPI_CPHA_2Edge;
    SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;
    SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_4;
    SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;
    SPI_InitStructure.SPI_CRCPolynomial = 7;
    SPI_Init(SPI2, &SPI_InitStructure);

    /* Enable SPI2  */
    SPI_Cmd(SPI2, ENABLE);

    SPI_Flash_WAKEUP();
    
    /***********************************************
    * 描述： OS接口
    */
#if (UCOS_EN     == DEF_ENABLED)
#if OS_VERSION > 30000U
    BSP_OS_SemCreate(&Bsp_EepSem,1, "Bsp EepSem");      // 创建信号量
#else
    Bsp_EepSem     = OSSemCreate(1);                    // 创建信号量
#endif
#endif
}

/**-----------------------------------------------------------------
  * @函数名 Send_Byte
  * @功能   通过SPI总线发送一个字节数据(顺便接收一个字节数据)
  *         Sends a byte through the SPI interface and return the byte
  *         received from the SPI bus.
  * @参数   要写入的一个字节数据
  * @返回值 在发数据时，MISO信号线上接收的一个字节
***----------------------------------------------------------------*/
uint8_t Send_Byte(uint8_t byte)
{
    /* Loop while DR register in not empty */
    while (SPI_I2S_GetFlagStatus(SPI2, SPI_I2S_FLAG_TXE) == RESET);

    /* Send byte through the SPI2 peripheral */
    SPI_I2S_SendData(SPI2, byte);

    /* Wait to receive a byte */
    while (SPI_I2S_GetFlagStatus(SPI2, SPI_I2S_FLAG_RXNE) == RESET);

    /* Return the byte read from the SPI bus */
    return SPI_I2S_ReceiveData(SPI2);
}

/**-----------------------------------------------------------------
  * @函数名 Get_Byte
  * @功能   读取SPI FLASH的一个字节，未包含发送读命令和起始地址
  * @参数   无
  * @返回值 从SPI_FLASH读取的一个字节
***----------------------------------------------------------------*/
uint8_t Get_Byte(void)
{
    return (Send_Byte(Dummy_Byte1));
}

/**-----------------------------------------------------------------
  * @函数名 SPI_Flash_WAKEUP
  * @功能   唤醒SPI FLASH
  * @参数   无
  * @返回值 无
***----------------------------------------------------------------*/
void SPI_Flash_WAKEUP(void)   
{
  /* Select the FLASH: Chip Select low */
  CE_Low();

  /* Send "Power Down" instruction */
  Send_Byte(W25X_ReleasePowerDown);

  /* Deselect the FLASH: Chip Select high */
  CE_High();
}   
 
/* 以下函数在移植时无需修改 */
/************************************************************************
** 函数名称:MX25L1602_RD                                                                                                        
** 函数功能:MX25L1602的读函数,可选择读ID和读数据操作                
** 入口参数:
**            uint32_t Dst：目标地址,范围 0x0 - MAX_ADDR（MAX_ADDR = 0x1FFFFF）                
**          uint32_t NByte:    要读取的数据字节数
**            uint8_t* RcvBufPt:接收缓存的指针            
** 出口参数:操作成功则返回OK,失败则返回ERROR        
** 注     意:若某功能下,某一入口参数无效,可在该入口参数处填Invalid，该参数将被忽略
************************************************************************/
uint8_t MX25L3206_RD(uint32_t Dst, uint32_t NByte,uint8_t* RcvBufPt)
{
    uint32_t i = 0;
	uint32_t adr;
    
    EEP_WaitEvent();
    
	adr=Dst&MAX_ADDR ;
    CE_Low();            
    Send_Byte(0x0B);                                // 发送读命令
    Send_Byte(((adr & 0xFFFFFF) >> 16));            // 发送地址信息:该地址由3个字节组成
    Send_Byte(((adr & 0xFFFF) >> 8));
    Send_Byte(adr & 0xFF);
    Send_Byte(0xFF);                                // 发送一个哑字节以读取数据
    for (i = 0; i < NByte; i++)        
    {
        RcvBufPt[i] = Get_Byte();        
    }
    CE_High();   
    EEP_SendEvent();
    return (1);
}

/************************************************************************
** 函数名称:MX25L1602_RdID                                                                                                        
** 函数功能:MX25L1602的读ID函数,可选择读ID和读数据操作                
** 入口参数:
**            idtype IDType:ID类型。用户可在Jedec_ID,Dev_ID,Manu_ID三者里选择
**            uint32_t* RcvbufPt:存储ID变量的指针
** 出口参数:操作成功则返回OK,失败则返回ERROR        
** 注     意:若填入的参数不符合要求，则返回ERROR
************************************************************************/
uint32_t MX25L3206_RdID(void)
{
    uint32_t temp = 0;
    CE_Low();            
    Send_Byte(0x9F);                            // 发送读JEDEC ID命令(9Fh)
    temp = (temp | Get_Byte()) << 8;            // 接收数据 
    temp = (temp | Get_Byte()) << 8;    
    temp = (temp | Get_Byte());                 // 在本例中,temp的值应为0xBF2541
    CE_High();            
	return temp;
}

/************************************************************************
** 函数名称:MX25L1602_WR                                            
** 函数功能:MX25L1602的写函数，可写1个和多个数据到指定地址                                    
** 入口参数:
**            uint32_t Dst：目标地址,范围 0x0 - MAX_ADDR（MAX_ADDR = 0x1FFFFF）                
**            uint8_t* SndbufPt:发送缓存区指针
**          uint32_t NByte:要写的数据字节数
** 出口参数:操作成功则返回OK,失败则返回ERROR        
** 注     意:若某功能下,某一入口参数无效,可在该入口参数处填Invalid，该参数将被忽略
************************************************************************/
uint8_t MX25L3206_WR(uint32_t Dst, uint8_t* SndbufPt, uint32_t NByte)
{
    uint8_t StatRgVal = 0;
    uint32_t i = 0;
	uint32_t adr;
    
    EEP_WaitEvent();
    
	adr=Dst&MAX_ADDR ;
    
    CE_Low();            
    Send_Byte(0x06);                                                    /* 发送写使能命令               */
    CE_High();        
    //写数据 
    CE_Low();            
    Send_Byte(0x02);                                                    /* 发送字节数据烧写命令         */
    Send_Byte((((adr+i) & 0xFFFFFF) >> 16));                            /* 发送3个字节的地址信息        */
    Send_Byte((((adr+i) & 0xFFFF) >> 8));
    Send_Byte((adr+i) & 0xFF);
    for(i = 0; i < NByte; i++)
    {
        Send_Byte(SndbufPt[i]);                                     /* 发送被烧写的数据             */
    }
    CE_High();    
    
    do
    {
        CE_Low();             
        Send_Byte(0x05);                                                /* 发送读状态寄存器命令         */
        StatRgVal = Get_Byte();                                         /* 保存读得的状态寄存器值       */
        CE_High();                                
    }
    while (StatRgVal == 0x03);                                          /* 一直等待，直到芯片空闲       */
    EEP_SendEvent();
    return (1);        
}

/************************************************************************
** 函数名称:MX25L1602_Erase                                                
** 函数功能:根据指定的扇区号选取最高效的算法擦除                                
** 入口参数:
**            uint32_t sec1：起始扇区号,范围(0~499)
**            uint32_t sec2：终止扇区号,范围(0~499)
** 出口参数:操作成功则返回OK,失败则返回ERROR        
************************************************************************/

uint8_t MX25L3206_Erase(uint32_t sec1, uint32_t sec2)
{
    uint8_t  temp2 = 0,StatRgVal = 0;
    uint32_t SecnHdAddr = 0;                  
    uint32_t no_SecsToEr = 0;                       // 要擦除的扇区数目
    uint32_t CurSecToEr = 0;                        // 当前要擦除的扇区号
    
    /*  检查入口参数 */
    if ((sec1 > SEC_MAX)||(sec2 > SEC_MAX))    
    {
        return (0);    
    }       
    
    CE_Low();            
    Send_Byte(0x06);                                // 发送写使能命令
    CE_High();            

    /* 如果用户输入的起始扇区号大于终止扇区号，则在内部作出调整 */
    if (sec1 > sec2)
    {
       temp2 = sec1;
       sec1  = sec2;
       sec2  = temp2;
    } 
    /* 若起止扇区号相等则擦除单个扇区 */
    if (sec1 == sec2)    
    {
        SecnHdAddr = SEC_SIZE * sec1;               // 计算扇区的起始地址
        CE_Low();    
        Send_Byte(0x20);                            // 发送扇区擦除指令
        Send_Byte(((SecnHdAddr & 0xFFFFFF) >> 16)); // 发送3个字节的地址信息
           Send_Byte(((SecnHdAddr & 0xFFFF) >> 8));
           Send_Byte(SecnHdAddr & 0xFF);
          CE_High();            
        do
        {
              CE_Low();             
            Send_Byte(0x05);                        // 发送读状态寄存器命令
            StatRgVal = Get_Byte();                 // 保存读得的状态寄存器值
            CE_High();                                
          }
        while (StatRgVal == 0x03);                  // 一直等待，直到芯片空闲
        return (1);            
    }
    
                     /*         根据起始扇区和终止扇区间距调用最快速的擦除功能                             */    
    
    if (sec2 - sec1 == SEC_MAX)    
    {
        CE_Low();            
        Send_Byte(0x60);                            // 发送芯片擦除指令(60h or C7h)
        CE_High();            
        do
        {
            CE_Low();             
            Send_Byte(0x05);                        // 发送读状态寄存器命令
            StatRgVal = Get_Byte();                 // 保存读得的状态寄存器值
            CE_High(); 
			Delay(10);								//此处延时是否考虑更改
        }
        while (StatRgVal == 0x03);                  // 一直等待，直到芯片空闲
        return (1);
    }
    
    no_SecsToEr = sec2 - sec1 +1;                   // 获取要擦除的扇区数目
    CurSecToEr  = sec1;                             // 从起始扇区开始擦除
    
    /* 若两个扇区之间的间隔够大，则采取16扇区擦除算法 */
    while (no_SecsToEr >= 16)
    {
        SecnHdAddr = SEC_SIZE * CurSecToEr;         // 计算扇区的起始地址
        CE_Low();    
        Send_Byte(0xD8);                            // 发送64KB块擦除指令
        Send_Byte(((SecnHdAddr & 0xFFFFFF) >> 16)); // 发送3个字节的地址信息
        Send_Byte(((SecnHdAddr & 0xFFFF) >> 8));
        Send_Byte(SecnHdAddr & 0xFF);
        CE_High();            
        do
        {
            CE_Low();             
            Send_Byte(0x05);                        // 发送读状态寄存器命令
            StatRgVal = Get_Byte();                 // 保存读得的状态寄存器值
            CE_High();                                
          }
        while (StatRgVal == 0x03);                  // 一直等待，直到芯片空闲
        CurSecToEr  += 16;                          // 计算擦除了16个扇区后,和擦除区域相邻的待擦除扇区号
        no_SecsToEr -=  16;                         // 对需擦除的扇区总数作出调整
    }
    /* 若两个扇区之间的间隔够大，则采取8扇区擦除算法 */
    while (no_SecsToEr >= 8)
    {
        SecnHdAddr = SEC_SIZE * CurSecToEr;         // 计算扇区的起始地址
        CE_Low();    
        Send_Byte(0x52);                            // 发送32KB擦除指令
        Send_Byte(((SecnHdAddr & 0xFFFFFF) >> 16)); // 发送3个字节的地址信息
        Send_Byte(((SecnHdAddr & 0xFFFF) >> 8));
        Send_Byte(SecnHdAddr & 0xFF);
        CE_High();            
        do
        {
            CE_Low();             
            Send_Byte(0x05);                        // 发送读状态寄存器命令
            StatRgVal = Get_Byte();                 // 保存读得的状态寄存器值
            CE_High();                                
          }
        while (StatRgVal == 0x03);                  // 一直等待，直到芯片空闲    
        CurSecToEr  += 8;
        no_SecsToEr -=  8;
    }
    /* 采用扇区擦除算法擦除剩余的扇区 */
    while (no_SecsToEr >= 1)
    {
        SecnHdAddr = SEC_SIZE * CurSecToEr;         // 计算扇区的起始地址
        CE_Low();    
        Send_Byte(0x20);                            // 发送扇区擦除指令
        Send_Byte(((SecnHdAddr & 0xFFFFFF) >> 16)); // 发送3个字节的地址信息
           Send_Byte(((SecnHdAddr & 0xFFFF) >> 8));
           Send_Byte(SecnHdAddr & 0xFF);
          CE_High();            
        do
        {
              CE_Low();             
            Send_Byte(0x05);                        // 发送读状态寄存器命令
            StatRgVal = Get_Byte();                 // 保存读得的状态寄存器值
            CE_High();                                
          }
        while (StatRgVal == 0x03);                  // 一直等待，直到芯片空闲
        CurSecToEr  += 1;
        no_SecsToEr -=  1;
    }
    return (1);
}


uint8_t SaveOneREcord(uint32_t adr,uint8_t *buf,uint32_t len)
{
	if((adr&0x0FFF)==0)							//在块边界，需要擦除块
	{
        EEP_WaitEvent();
		MX25L3206_Erase((adr>>12)&0x3FF,(adr>>12)&0x3FF);
        EEP_SendEvent();
	}
	return MX25L3206_WR(adr, buf, len);
}
void FASTREAD(uint32_t adr,uint8_t *buf,uint32_t len)
{
	MX25L3206_RD(adr, len,buf);
	
}
uint8_t RDSR(void)
{
	uint8_t dat;
    CE_Low();             
    Send_Byte(0x05);                                // 发送读状态寄存器命令
    dat = Get_Byte();                             // 保存读得的状态寄存器值
    CE_High();                                
	return dat;
}
void WRSR(uint8_t dat)
{
    CE_Low();             
    Send_Byte(0x06);                                // 发送读状态寄存器命令
    CE_High();                                
    CE_Low();             
    Send_Byte(0x01);                                // 发送读状态寄存器命令
    Send_Byte(dat);                                // 发送读状态寄存器命令
    CE_High();                                
}

void SaveRecord(void)
{
	uint8_t   AddressBuf[4] = {0};
	uint32_t  Address = 0;
	
	GetTime((TIME *)&recordsfr.Time[0]);
    
	recordsfr.Crc16 = GetCrc16Check((uint8_t *)&recordsfr.Time[0],sizeof(SDAT)-2);
   	SaveOneREcord(g_Flash_Adr,(uint8_t *)&recordsfr.Time[0],sizeof(SDAT));
    // 	printf("g_Flash_Adr = %d",g_Flash_Adr);
	g_Flash_Adr += sizeof(SDAT);
    //	printf("g_Flash_Adr = %d",g_Flash_Adr);
	Address = (MAX_ADDR & g_Flash_Adr)|0xAA000000;	 //FLASH大小，128Mbit
	TO_Arayy(AddressBuf , Address);
	WriteFM24CL64(60, AddressBuf, 4);
}


/*  数据转储                    */
/*  数据低字节在前，高字节在后  */
void DOWNLOAD(uint32_t NRecord)
{	
	uint32_t i;
	static uint8_t  DATA_OUT[32] = {0};
    
	uint32_t PP_Power = 0;
	uint32_t NP_Power = 0;
	uint32_t PQ_Power = 0;
	uint32_t NQ_Power = 0;
	uint16_t CRC_sum1 = 0;
	uint16_t CRC_sum2 = 0;
    
	for(i=0;i<NRecord;i++) {
		FeedDog_HIGH();
		MX25L3206_RD(i*32, 32 ,DATA_OUT);				  //每次读取32字节，即一条记录
		FeedDog_LOW();
		
		CRC_sum1 = GetCrc16Check((uint8_t *)&DATA_OUT[0],sizeof(SDAT)-2);		
		CRC_sum2 = DATA_OUT[30] + (DATA_OUT[31] << 8);
		
        if(CRC_sum1 == CRC_sum2) {						   //进行累加和计算
			printf("\r\n第%05d条记录：20%02d-%02d-%02d %02d:%02d:%02d  ", (i+1),
                   DATA_OUT[0],DATA_OUT[1],DATA_OUT[2],
                   DATA_OUT[3],DATA_OUT[4],DATA_OUT[5]);
			FeedDog_HIGH();
			PP_Power =  DATA_OUT[11]<<24 |  DATA_OUT[10]<<16 | DATA_OUT[9]<<8  | DATA_OUT[8];
			FeedDog_LOW();
			printf("正有功电量：%10dkW.h  ", PP_Power);
			FeedDog_HIGH();
			NP_Power =  DATA_OUT[15]<<24 |  DATA_OUT[14]<<16 | DATA_OUT[13]<<8 | DATA_OUT[12];
			FeedDog_LOW();
			printf("负有功电量：%10dkW.h  ", NP_Power);
			PQ_Power =  DATA_OUT[19]<<24 |  DATA_OUT[18]<<16 | DATA_OUT[17]<<8 | DATA_OUT[16];
			FeedDog_HIGH();
			printf("正无功电量：%10dkvar.h  ", PQ_Power);
			FeedDog_LOW();
			NQ_Power =  DATA_OUT[23]<<24 |  DATA_OUT[22]<<16 | DATA_OUT[21]<<8 | DATA_OUT[20];
			FeedDog_HIGH();
			printf("负无功电量：%10dkvar.h  ", NQ_Power);
			FeedDog_LOW();
		}
		else 
			printf("累加和错误！");
	}
}

