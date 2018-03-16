/*******************************************************************************
 *   Filename:       bsp_info.h
 *   Revised:        $Date: 2015-08-24	 $
 *   Revision:       $V1.0
 *	 Writer:		 Roger-WY.
 *
 *   Description:    读取芯片唯一UID模块驱动头文件
 *
 *   Notes:          每个STM32芯片在出厂时都具有一个全球唯一的ID，该ID为96位，其地址分布如下：
 *                   Start Address : 0x1FFF F7E8
 *                   Address offset: 0×00 U_ID(00-31)
 *                   Address offset: 0×04 U_ID(32-63)
 *                   Address offset: 0×08 U_ID(64-96)
 *                   该ID的应用：
 *
 *                   ●用来作为序列号(例如USB字符序列号或者其他的终端应用)
 *                   ●用来作为密码，在编写闪存时，将此唯一标识与软件加解密算法结合使用，提高代码在闪存存储器内的安全性。
 *                   ●用来激活带安全机制的自举过程
 *
 *
 *   All copyrights reserved to Roger-WY.
 *
 *******************************************************************************/
#ifndef __BSP_INFO_H__
#define __BSP_INFO_H__

/*******************************************************************************
 * INCLUDES
 */
#include <global.h>

/*******************************************************************************
 * CONSTANTS
 */
#define APP_FLASH_DATA_ADDR          (0x08014800)                            // 放在APP FLASH第3页的前1K
#define APP_FLASH_DATA_LEN           (STM_SECTOR_SIZE)                      // 放在APP FLASH第3页的前1K

#define APP_CHECK_INFO_ADDR         (0x08014800 + STM_SECTOR_SIZE -32 )      // 放在APP FLASH第3页的前1K
#define APP_REG_INFO_ADDR           (APP_CHECK_INFO_ADDR + 16)              // 放在APP FLASH第3页的前1K
#define BOOT_CHECK_INFO_ADDR        0x08002000
#define APP_CTRL_ID_ADDR            (APP_CHECK_INFO_ADDR - 24)
#define APP_SENSOR_ID_ADDR          (APP_CHECK_INFO_ADDR - 48)

/*******************************************************************************
 * MACROS
 */

/*******************************************************************************
 * TYPEDEFS
 */
/***********************************************
* 描述：
*/
typedef union {
  uint8_t         buf1[12];
  uint16_t        buf2[6];
  uint32_t        buf3[3];
} UnionChipId;

/***********************************************
* 描述：
*/
#define CID_BUF_LEN         (8 * 3 + 2 + 1)
#define FLASH_BUF_LEN       (4 + 1 + 1)
#define DATE_BUF_LEN        (3 + 2 + 4 + 2 +1)
#define TIME_BUF_LEN        (2 + 2 + 2 + 2 +1)
#define VERSION_BUF_LEN     (9 +1)

typedef struct {
  UnionChipId   id;
  char          cid[CID_BUF_LEN];
  char          flash[FLASH_BUF_LEN];
  char          date[DATE_BUF_LEN];
  char          time[TIME_BUF_LEN];
  char          version[VERSION_BUF_LEN];
} StrChipInfo;

extern StrChipInfo  ChipInfo[3];
extern const u32    checkInfo[];


/*******************************************************************************
 * 描述： 外部函数调用
 */
uint8_t  BSP_GetChipID     (uint8_t devType, UnionChipId *pId );
uint8_t  BSP_GetChipInfo   (uint8_t devType, StrChipInfo *pInfo);

#endif
/*******************************************************************************
 * 				end of file
 *******************************************************************************/