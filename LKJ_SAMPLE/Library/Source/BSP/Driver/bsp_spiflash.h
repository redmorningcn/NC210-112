/******************** (C) COPYRIGHT 2008 STMicroelectronics ********************
* File Name          : spi_flash.h
* Author             : MCD Application Team
* Version            : V2.0.3
* Date               : 09/22/2008
* Description        : Header for spi_flash.c file.
********************************************************************************
* THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
* WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE TIME.
* AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY DIRECT,
* INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING FROM THE
* CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE CODING
* INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
*******************************************************************************/

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __BSP_SPIFLASH_H
#define __BSP_SPIFLASH_H

#define _USING_AT25DF161_
/* Private typedef -----------------------------------------------------------*/



#include "stm32f10x.h"
#include "includes.h"


typedef  union __unionflashstorage{
   u8  buf1[512];
   u16 buf2[256];
   u32 buf3[128];
}unionflashstorage;

//============================================================================//
/***********************************************
* 描述： 首先配置是否使用硬件SPI
     *   BSP_USE_SPI_MODE 为 1  使用硬件SPI
     *   BSP_USE_SPI_MODE 为 0  使用IO模拟SPI
*/
#define  BSP_USE_SPI_MODE  1

//============================================================================//


/***********************************************
* 描述： FLASH引脚定义
*/
#if (BSP_USE_SPI_MODE == 1)
#define BSP_FLASH_SPIx                          SPI1
#define BSP_FLASH_SPIx_RCC                      RCC_APB2Periph_SPI1
#endif

#define BSP_FLASH_CS_PIN                        GPIO_Pin_4
#define BSP_FLASH_CS_GPIO_PORT                  GPIOA
#define BSP_FLASH_CS_GPIO_RCC                   RCC_APB2Periph_GPIOA

#define BSP_FLASH_SPI_SCK_PIN                   GPIO_Pin_5
#define BSP_FLASH_SPI_SCK_GPIO_PORT             GPIOA
#define BSP_FLASH_SPI_SCK_GPIO_RCC              RCC_APB2Periph_GPIOA

#define BSP_FLASH_SPI_MISO_PIN                  GPIO_Pin_6
#define BSP_FLASH_SPI_MISO_GPIO_PORT            GPIOA
#define BSP_FLASH_SPI_MISO_GPIO_RCC             RCC_APB2Periph_GPIOA

#define BSP_FLASH_SPI_MOSI_PIN                  GPIO_Pin_7
#define BSP_FLASH_SPI_MOSI_GPIO_PORT            GPIOA
#define BSP_FLASH_SPI_MOSI_GPIO_RCC             RCC_APB2Periph_GPIOA

#define BSP_FLASH_WP_PIN                        GPIO_Pin_5
#define BSP_FLASH_WP_GPIO_PORT                  GPIOC
#define BSP_FLASH_WP_GPIO_RCC                   RCC_APB2Periph_GPIOC

#define BSP_FLASH_RST_PIN                       GPIO_Pin_4
#define BSP_FLASH_RST_GPIO_PORT                 GPIOC
#define BSP_FLASH_RST_GPIO_RCC                  RCC_APB2Periph_GPIOC

/* CS片选 低电平有效 */
#define BSP_FLASH_CS_LOW()      GPIO_ResetBits(BSP_FLASH_CS_GPIO_PORT, BSP_FLASH_CS_PIN)
#define BSP_FLASH_CS_HIGH()     GPIO_SetBits  (BSP_FLASH_CS_GPIO_PORT, BSP_FLASH_CS_PIN)
/* WP写保护 低电平有效 */
#define BSP_FLASH_WP_LOW()      GPIO_ResetBits(BSP_FLASH_WP_GPIO_PORT, BSP_FLASH_WP_PIN)
#define BSP_FLASH_WP_HIGH()     GPIO_SetBits  (BSP_FLASH_WP_GPIO_PORT, BSP_FLASH_WP_PIN)
/* RST复位 低电平有效 */
#define BSP_FLASH_RST_LOW()     GPIO_ResetBits(BSP_FLASH_RST_GPIO_PORT, BSP_FLASH_RST_PIN)
#define BSP_FLASH_RST_HIGH()    GPIO_SetBits  (BSP_FLASH_RST_GPIO_PORT, BSP_FLASH_RST_PIN)

#if (BSP_USE_SPI_MODE == 0)
/* SPI  SCK时钟 */
#define BSP_FLASH_SCK_LOW()      GPIO_ResetBits(BSP_FLASH_SPI_SCK_GPIO_PORT, BSP_FLASH_SPI_SCK_PIN)
#define BSP_FLASH_SCK_HIGH()     GPIO_SetBits  (BSP_FLASH_SPI_SCK_GPIO_PORT, BSP_FLASH_SPI_SCK_PIN)
/* SPI  MISO */
#define BSP_FLASH_MISO_READ()    GPIO_ReadInputDataBit(BSP_FLASH_SPI_MISO_GPIO_PORT, BSP_FLASH_SPI_MISO_PIN)
/* SPI  MOSI */
#define BSP_FLASH_MOSI_LOW()     GPIO_ResetBits(BSP_FLASH_SPI_MOSI_GPIO_PORT, BSP_FLASH_SPI_MOSI_PIN)
#define BSP_FLASH_MOSI_HIGH()    GPIO_SetBits  (BSP_FLASH_SPI_MOSI_GPIO_PORT, BSP_FLASH_SPI_MOSI_PIN)
#endif





#ifdef _USING_AT25DF161_

typedef enum
{
  SECTOR_UNPROTECT = 0,
  SECTOR_PROTECT = 1
}FlashPretectStatue;
/***********************************************
* 描述： FLASH固有参数
*/
#define  AT45DB161E_FLASH_ID            0x1F260001  //Flash的芯片ID
#define  FLASH_ID                       AT45DB161E_FLASH_ID
#define  BSP_FLASH_PageSize             512         //页大小默认528个字节，可修改为512，每个芯片只能修改一次
#define  BSP_FLASH_PageNum              4096

/***********************************************
* 描述： FLASH指令定义
*/
#define Dummy_Byte                      0xA5     //无意义字节,用于SPI读取数据

#define FLASH_IDREAD                    0x9F     //读取芯片ID指令
#define FLASH_STATUSREAD                0xD7     //读取芯片状态寄存器指令  也可以使用指令码0x57

#define FLASH_CHREAD                    0x0B     //连续读 高速模式 可以达到66MHZ
#define FLASH_CLREAD                    0x03     //连续读 低速模式 可以达到33MHZ
#define FLASH_PAGEREAD	                0xD2     //主存储器页读，主存储器页读允许直接从 4096 个页中某一个页中读取数据， 而不影响缓冲区中的数据
#define FLASH_BUF1READ                  0x54     //读取缓冲区1的数据
#define FLASH_BUF2READ                  0x56     //读取缓冲区2的数据

#define FLASH_PAGEERASE                 0x81     //页擦除指令
#define FLASH_BLOCKERASE                0x50     //块擦除指令
#define FLASH_SECTORERASE               0x7C     //扇区擦除指令

#define FLASH_BUFWRITE1                 0x84     //缓冲区1写 往缓冲区1写数据
#define FLASH_BUFWRITE2                 0x87     //缓冲区2写 往缓冲区2写数据
#define B1_TO_MM_PAGE_PROG_WITH_ERASE   0x83	 // 将第一缓冲区的数据写入主存储器（擦除模式）
#define B2_TO_MM_PAGE_PROG_WITH_ERASE   0x86	 // 将第二缓冲区的数据写入主存储器（擦除模式）


#define MM_PAGE_TO_B1_XFER              0x53	 // 将主存储器的指定页数据加载到第一缓冲区
#define MM_PAGE_TO_B2_XFER              0x55	 // 将主存储器的指定页数据加载到第二缓冲区


//#define FLASH_ID  0x1F2601
//
//#define AT25DF161_FLASH_ID    FLASH_ID
//
//#define SPI_FLASH_PageSize    512
//
//#define WREN        0x06  /* Write enable instruction */
//#define READ        0x03  /* Read from Memory instruction */
//#define WRITE       0x84  /* Write to Memory instruction */
//#define RDID        0x9F  /* Read identification */
//#define SE          0x7C  /* Sector Erase instruction */
//#define PE          0x81  /* Page Erase instruction */
//#define RDSR        0xD7  /* Read Statue instruction */
//#define WIP_Flag    0x80  /* Write In Progress (WIP) flag */
//#define BE          0x60  /* Bulk Erase instruction */
//#define BLE         0x20  /* Block(size:4K/block) Erase instruction */
//#define RA          0x1b  /* Read arrary */
//
//#define Dummy_Byte 0xA5
#endif

#ifdef _USING_M25P64_

#define SPI_FLASH_PageSize    0x100
/* Private define ------------------------------------------------------------*/
#define WRITE      0x02  /* Write to Memory instruction */
#define WRSR       0x01  /* Write Status Register instruction */
#define WREN       0x06  /* Write enable instruction */

#define READ       0x03  /* Read from Memory instruction */
#define RDSR       0x05  /* Read Status Register instruction  */
#define RDID       0x9F  /* Read identification */
#define SE         0xD8  /* Sector Erase instruction */
#define BE         0xC7  /* Bulk Erase instruction */

#define WIP_Flag   0x01  /* Write In Progress (WIP) flag */

#define Dummy_Byte 0xA5

#endif

/***********************************************
* 描述： 函数申明
*/
/*----- Low layer function -----*/
#if (BSP_USE_SPI_MODE == 1)
uint8_t BSP_FLASH_ReadByte(void);
uint8_t BSP_FLASH_SendByte(uint8_t byte);
uint16_t BSP_FLASH_SendHalfWord(uint16_t HalfWord);
#else
u8   BSP_FLASH_ReadByte(void);
void BSP_FLASH_SendByte(uint8_t byte);
#endif

/* Exported functions ------------------------------------------------------- */
/*----- High layer function -----*/
void SPI_FLASH_Init(void);
void SPI_FLASH_SectorErase(u32 SectorAddr);
void SPI_FLASH_BulkErase(void);
void SPI_FLASH_PageWrite(u8* pBuffer, u32 WriteAddr, u16 NumByteToWrite);
void SPI_FLASH_BufferWrite(u8* pBuffer, u32 WriteAddr, u16 NumByteToWrite);
void SPI_FLASH_Ram1Read(u8* pBuffer, u32 ReadAddr, u16 NumByteToRead);
u32  SPI_FLASH_ReadID(void);
void SPI_FLASH_StartReadSequence(u32 ReadAddr);


FlashPretectStatue SPI_FLASH_GetProtectStatue(u8 sectorNo);
void SPI_FLASH_BlockErase(u32 BlockAddr);
void SPI_FLASH_ArraryRead(u8* pBuffer, u32 ReadAddr, u16 NumByteToRead);//wisure
void SPI_FLASH_ProtectSet(u8 setValue);//wisure added

/*----- Low layer function -----*/
u8 SPI_FLASH_ReadByte(void);
u8 SPI_FLASH_SendByte(u8 byte);
u16 SPI_FLASH_SendHalfWord(u16 HalfWord);
void SPI_FLASH_WriteEnable(void);
void SPI_FLASH_WaitForWriteEnd(void);
u8 SPI_FLASH_GetStatueByte(void);
void SPI_FLASH_SetAt45DB161_512BytesOnePage(void);
void SPI_FLASH_EnableSecProtect(void);
void SPI_FLASH_DisableSecProtect(void);
void SPI_FLASH_Ram1Write(u8* pBuffer, u32 WriteAddr, u16 NumByteToWrite);
void SPI_FLASH_PageErase(u32 Addr);





extern unionflashstorage FlashStorageBuf;




#endif /* __SPI_FLASH_H */
/*
|------------------------------------------------------------------------------------------------------------------------------|
|    符号	|                名称与功能	                                                         |  有效电平   |	  类型     |
|------------------------------------------------------------------------------------------------|-------------|---------------|
|#CS	    |    片选：#CS用以选中芯片。当#CS被设置为无效状态时，芯片则不被选中，                |             |               |
|           |    并且处于闲置状态（不是深度睡眠状态），输出引脚SO处于高阻态。                    |             |               |
|           |    当芯片未被选中时，从输入引脚SI输入的数据将不被接受。                            |    LOW      |   	输入       |
|           |    #CS引脚上的下降沿将会启动一个操作，而上跳沿则会结束一个操作。                   |             |               |
|           |    在一个内部操作如芯片内部的编程或擦除周期内，芯片不会进入闲置状态，直到操作完毕。|	           |               |
|------------------------------------------------------------------------------------------------|-------------|---------------|
|SCK	    |    串行时钟：此引脚用来向芯片提供时钟信号，有来控制数据流的出入。                  |             |               |
|           |    SI引脚上的命令、地址与输入数据在时钟SCK的上升沿被写入，                         |      -      |   输入        |
|           |    而SO引脚上的输出数据则在时钟的下降沿变化。		                                 |             |               |
|------------------------------------------------------------------------------------------------|-------------|---------------|
|SI	        |    串行输入：SI引脚用来向芯片以移位方式写入数据。                                  |             |               |
|           |    SI引脚上的所有数据输入包括命令与地址。                                          |      -      |   输入        |
|           |    SI上的数据在时钟的上升沿写入芯片。                                              |             |               |
|------------------------------------------------------------------------------------------------|-------------|---------------|
|SO	        |    串行输出：SO引脚用来从芯片以移位方式输出数据。                                  |      -      |   输出        |
|           |    SI上的数据在时钟的下降沿变化。	                                                 |             |               |
|------------------------------------------------------------------------------------------------|-------------|---------------|
|#WP	    |    写保护：当#WP被设置为有效时芯片的扇区将被保护起来，                             |             |               |
|           |    以防止编程与擦除对数据的破坏。                                                  |     LOW     |   	输入       |
|           |    但是扇区保护使能与扇区死锁命令仍然可以被芯片识别。                              |             |               |
|           |    #WP引脚在内部被拉高，可以悬空。但是仍然建议在外部接到VCC。	                     |             |               |
|------------------------------------------------------------------------------------------------|-------------|---------------|
|#RESET	    |    复位：#RESET引脚上的低电平会终止正在处理的操作并复位内部状态机到闲置状态。      |             |               |
|           |    #RESET引脚上的低电平会使芯片一直处于复位状态。                                  |     LOW     |  	输入       |
|           |    当#RESET上转为高电平后，才能进行正常的操作。                                    |             |               |
|           |    芯片内部设置上电复位电路。当此引脚不用时，外部接到高电平。                      |             |               |
|------------------------------------------------------------------------------------------------|-------------|---------------|
|RDY/#BUSY	|    就绪/忙碌状态指示：此引脚是漏极开路的输出引脚。                                 |             |               |
|           |    当芯片处于忙状态时（内部操作过程中）此引脚为低电平，                            |             |               |
|           |    此引脚在正常状态下为高电平（外部接上拉电阻）。                                  |      -      |   输出        |
|           |    当正在进行编程/擦除操作，比较操作与页-缓冲区传送时，被拉低。                    |             |               |
|           |    忙状态指示FLASH储存阵列与某一个缓冲区不能被操作，                               |             |               |
|           |    而对另一个缓冲区的读与写操作仍然可以进行。	                                     |             |               |
|------------------------------------------------------------------------------------------------|-------------|---------------|
|VCC	    |    芯片电源供给	                                                                 |      -      |   电源        |
|------------------------------------------------------------------------------------------------|-------------|---------------|
|GND	    |    地：此引脚应与系统地接在一起。                                                  |      -      |   地          |
|------------------------------------------------------------------------------------------------|-------------|---------------|


*/
/******************* (C) COPYRIGHT 2008 STMicroelectronics *****END OF FILE****/
