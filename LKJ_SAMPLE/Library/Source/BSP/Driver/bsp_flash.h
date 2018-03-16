/*******************************************************************************
 *   Filename:       bsp_flash.h
 *   Revised:        $Date: 2015-08-23
 *   Revision:       $V1.0
 *	 Writer:		 Roger_WY.
 *
 *   Description:    STM32内部FLASH驱动模块头文件
 *
 *   Notes:
 *
 *
 *   All copyrights reserved to Roger_WY.
 *
 *******************************************************************************/
#ifndef __BSP_FLASH_H__
#define __BSP_FLASH_H__

#include "stm32f10x.h"
//改在工程中定义
#if !defined( STM32_FLASH_SIZE )
#if defined( STM32F103RB )
#define  STM32_FLASH_SIZE       128         // flash容量 根据自己选的芯片来定义
#elif defined( STM32F103RC )
#define  STM32_FLASH_SIZE       256         // flash容量 根据自己选的芯片来定义
#endif
#endif
     
#define  STM32_FLASH_BASE       0X08000000  // flash基地址

#if STM32_FLASH_SIZE < 256
#define STM_SECTOR_SIZE         1024        // 字节
#else
#define STM_SECTOR_SIZE	        2048        // 字节
#endif

#define KB						*1024

#if !defined ( STM32_FLASH_SIZE )
#define	STM32_FLASH_SIZE		(256KB)											// FLASH大小
#endif
#if !defined ( FLASH_SIZE_EEP )
#define	FLASH_SIZE_EEP			(16KB)	
#endif

#define STM_FLASH_PAGES         (STM32_FLASH_SIZE * 1024 / STM_SECTOR_SIZE )
#define EEPROM_START_PAGE       (STM_FLASH_PAGES-FLASH_SIZE_EEP/STM_SECTOR_SIZE)                             //你想存的页数
#define PARAM_START_ADDR        (STM32_FLASH_BASE + STM_SECTOR_SIZE * EEPROM_START_PAGE) //flash页首地址


typedef union {
    u8  buf1[STM_SECTOR_SIZE / 1];          //最多是2K字节
    u16 buf2[STM_SECTOR_SIZE / 2];
    u32 buf3[STM_SECTOR_SIZE / 4];
} UnionFlashBuf;

extern UnionFlashBuf   FlashBuf;
/***********************************************
* 描述：外部函数调用
*/
void    BSP_FlashRead           (u32 addr,u16 *pbuffer,u16 num);
void    BSP_FlashWrite          (u32 addr,u16 *pbuffer,u16 num);

u16     BSP_FlashReadBytes      (u32 addr, u8 *buf, u16 len);
u16     BSP_FlashWriteBytes     (u32 addr, u8 *buf, u16 len);

u8     FlashWriteNoCheck       (u32 addr,u16 *pbuffer,u16 num);


#endif