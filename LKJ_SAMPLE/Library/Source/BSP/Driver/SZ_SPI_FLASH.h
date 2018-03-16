/******************** (C) COPYRIGHT 2013 www.armjishu.com  ********************
 * 文件名  ：SZ_SPI_FLASH.h
 * 描述    ：实现STM32F107VC神舟IV号开发板的SPI存储器W25Q16_W2XQ16底层函数
 * 实验平台：STM32神舟开发板
 * 标准库  ：STM32F10x_StdPeriph_Driver V3.5.0
 * 作者    ：www.armjishu.com 
*******************************************************************************/

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __SZ_SPI_FLASH_H
#define __SZ_SPI_FLASH_H

/* Includes ------------------------------------------------------------------*/
#include "stm32f10x.h"
#include "stm32f10x_spi.h"

/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/

/**
  * @brief  SPI_FLASH SPI Interface pins
  */
#define  W25X16_FLASH_ID            0xEF3015
#define  W25Q16_FLASH_ID            0xEF4015

#define  MX25L12805                 0xC22018

/* Exported macro ------------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */
/*----- High layer function -----*/
void SPI_FLASH_Init(void);
void SPI_FLASH_SectorErase(u32 SectorAddr);
void SPI_FLASH_ChipErase(void);
void SPI_FLASH_PageWrite(u8* pBuffer, u32 WriteAddr, u16 NumByteToWrite);
void SPI_FLASH_BufferWrite(u8* pBuffer, u32 WriteAddr, u16 NumByteToWrite);
void SPI_FLASH_BufferRead(u8* pBuffer, u32 ReadAddr, u16 NumByteToRead);
u32 SPI_FLASH_Read_ManufactDeviceID(void);
u32 SPI_FLASH_ReadID(void);
u32 SPI_FLASH_ReadDeviceID(void);
void SPI_FLASH_StartReadSequence(u32 ReadAddr);
void SPI_Flash_PowerDown(void);
void SPI_Flash_WAKEUP(void);

/*----- Low layer function -----*/
u8 SPI_FLASH_ReadByte(void);
u8 SPI_FLASH_SendByte(u8 byte);
u16 SPI_FLASH_SendHalfWord(u16 HalfWord);
void SPI_FLASH_WriteEnable(void);
void SPI_FLASH_WriteDisable(void);
void SPI_FLASH_WaitForWriteEnd(void);

#endif /* __SZ_SPI_FLASH_H */

/******************* (C) COPYRIGHT 2010 www.armjishu.com *****END OF FILE****/
