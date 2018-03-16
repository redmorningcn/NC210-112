/*******************************************************************************
 *   Filename:       iap.h
 *   Revised:        $Date: 2014-04-23
 *   Revision:       $
 *	 Writer:		 Wuming Shen.
 *
 *   Description:    
 *
 *   Notes:        
 *					   
 *			
 *   All copyrights reserved to Wuming Shen.
 *
 *******************************************************************************/

#ifndef __IAP_H__
#define __IAP_H__
/*******************************************************************************
 * INCLUDES
 */
 #include "stm32f10x.h"

/*******************************************************************************
 * CONSTANTS
 */
#define KB						*1024

#define	FLASH_SIZE				(256KB)											// FLASH大小
#define	FLASH_SIZE_BOOT			(80KB)											// BOOT区大小
#define	FLASH_SIZE_EEP			(16KB)											// 参数区大小

#define	FLASH_SIZE_A			(FLASH_SIZE-FLASH_SIZE_BOOT-FLASH_SIZE_EEP)/2	// 程序A大小
#define	FLASH_SIZE_B			(FLASH_SIZE-FLASH_SIZE_BOOT-FLASH_SIZE_EEP)/2	// 程序B大小

#define FLASH_BOOT_ADDR			0x8000000  								        // 引导程序起始地址(存放在FLASH)
#define FLASH_APP_ADDR_A		(0x8000000 + FLASH_SIZE_BOOT)					// 第一个应用程序起始地址(存放在FLASH)
#define FLASH_APP_ADDR_B		(0x8000000 + FLASH_SIZE_BOOT + FLASH_SIZE_A)  	// 第二个应用程序起始地址(存放在FLASH)
#define FLASH_APP_ADDR			FLASH_BOOT_ADDR  								// 应用程序起始地址(存放在FLASH)

#define FLASH_APP_ADDR_SAVE		(FLASH_SIZE_EEP - 4)  	                    // 保存下次运行的程序地址
#define FLASH_APP_STATUS		(FLASH_SIZE_EEP - 5)  	                    // 程序升级状态

#if defined     ( IMAGE_A )
#define IAP_SELF_APP_ADDR      FLASH_APP_ADDR_A 
#elif defined   ( IMAGE_B )
#define IAP_SELF_APP_ADDR      FLASH_APP_ADDR_B 
#else
#define IAP_SELF_APP_ADDR      FLASH_BOOT_ADDR
#endif 
    
     
#define IAP_STS_DEF             0
#define IAP_STS_RST             1
#define IAP_STS_START           2
#define IAP_STS_PROGRAMING      3
#define IAP_STS_FAILED          4
#define IAP_STS_SUCCEED         5
     
/*******************************************************************************
 * MACROS
 */

/*******************************************************************************
 * TYPEDEFS
 */
typedef struct {
    INT32U      AppAddr;                            // 应用程序地址    INT08U      Status              : 4;            // // 0未升级，1开始，2升级中，3失败，4成功，其他：无程序
    INT08U      Status;                             // 0未升级，1开始，2升级中，3失败，4成功，其他：无程序
    INT08U      Step;
    INT16U      FrameIdx;                           // 接收到数据帧序号
    INT16U      TimeOut;                            // 超时退出IAP
    
    INT32U      FileSize;                           // 文件大小
    INT32U      WrittenSize;                        // 已写入文件大小
    INT32U      SectorAddr;                         // 当前写入的扇区地址
    INT32U      SectorAddrLast;                     // 上次写入的扇区地址
    INT32U      Sectors;                            // 总扇区数
    INT08U      WriteCtr;                           // 16帧128字节组成一个2K扇区扇区
    INT16U     *pBuf;                               // 数据缓冲区指针
    
} StrIapState;

extern StrIapState Iap;

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
/***********************************************
 * 描述：
 */
void        IAP_DevDeInit       (void);
u8          IAP_GetAppAStatus   (void);
u8          IAP_GetBppAStatus   (void);
u32     	IAP_GetAppAddr  	(void);
u8    	    IAP_SetAppAddr  	(u32 appAddr);
u8      	IAP_JumpToApp   	(void);
u8      	IAP_JumpToAppA   	(void);
u8      	IAP_JumpToAppB   	(void);
u8          IAP_JumpToAddr      (u32 appAddr);

u8          IAP_GetStatus       (void);
void        IAP_SetStatus       (u8);

void        IAP_Reset           (void);
void        IAP_Finish          (void);
void        IAP_Exit            (u8 timeout);
void        IAP_Programing      (void);
BOOL        IAP_GetFileInfo    (void);
BOOL        IAP_Program        (StrIapState *iap, INT16U *buf, INT16U len );
/*******************************************************************************
 * 				end of file
 *******************************************************************************/ 
#endif