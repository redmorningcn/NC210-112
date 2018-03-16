/*******************************************************************************
*   Filename:       bsp_eeprom.h
*   Revised:        All copyrights reserved to Roger.
*   Date:           2014-08-18
*   Revision:       v1.0
*   Writer:	        wumingshen.
*
*   Description:    AT24Cxxx系列EEPROM驱动模块头文件
*
*
*   Notes:
*
*******************************************************************************/
#ifndef	__BSP_EEPROM_H__
#define	__BSP_EEPROM_H__


/*********************************************************************
 * INCLUDES
 */
#include  <global.h>
#include  <includes.h>

//============================================================================//
//              EEPROM具体型号的选择和使用引脚配置（根据硬件修改）            //
//============================================================================//
/***********************************************
* 描述： 宏定义该驱动使用的AT24xx系列的具体型号
*/
#define AT_DEV_TYPE             AT24C64         // 器件型号
#define AT_DEV_NO_SEL_PIN       DEF_DISABLED     // 是否有地址选择引脚

/***********************************************
* 描述： 宏定义AT24xx系列一个字节的大小
*/
#define AT_BYTE_SIZE            8

/***********************************************
* 描述： 宏定义AT24xx系列EEPROM的容量
*/
#define AT24C01                 (1 * 1024)
#define AT24C02                 (2 * 1024)
#define AT24C04                 (4 * 1024)
#define AT24C08                 (8 * 1024)
#define AT24C16                 (16 * 1024)
#define AT24C32                 (32 * 1024)
#define AT24C64                 (64 * 1024)
#define AT24C128                (128 * 1024)
#define AT24C256                (256 * 1024)
#define AT24C512                (512 * 1024)
#define AT24C1024               (1024 * 1024)

/***********************************************
* 描述： 如果没有定义设备类型，默认为AT24C02
*/
#ifndef AT_DEV_TYPE
#define AT_DEV_TYPE             AT24C02
#endif


/***********************************************
* 描述： 计算AT24xx系列不同型号的页数
*/
#if     AT_DEV_TYPE == AT24C01
#define AT_MAX_BYTES            ( AT24C01 / AT_BYTE_SIZE )      // 总字节数
#define AT_PAGES                16u                             // 页数
#define AT_PAGE_SIZE            ( AT_MAX_BYTES / AT_PAGES )     // 一页字节数
#elif   AT_DEV_TYPE == AT24C02
#define AT_MAX_BYTES            ( AT24C02 / AT_BYTE_SIZE )      // 总字节数
#define AT_PAGES                32u                             // 页数
#define AT_PAGE_SIZE            ( AT_MAX_BYTES / AT_PAGES )     // 一页字节数
#elif   AT_DEV_TYPE == AT24C04
#define AT_MAX_BYTES            ( AT24C04 / AT_BYTE_SIZE )      // 总字节数
#define AT_PAGES                32u                             // 页数
#define AT_PAGE_SIZE            ( AT_MAX_BYTES / AT_PAGES )     // 一页字节数
#elif   AT_DEV_TYPE == AT24C08
#define AT_MAX_BYTES            ( AT24C08 / AT_BYTE_SIZE )      // 总字节数
#define AT_PAGES                64u                             // 页数
#define AT_PAGE_SIZE            ( AT_MAX_BYTES / AT_PAGES )     // 一页字节数
#elif   AT_DEV_TYPE == AT24C16
#define AT_MAX_BYTES            (AT24C16 / AT_BYTE_SIZE )       // 总字节数
#define AT_PAGES                128u                            // 页数
#define AT_PAGE_SIZE            ( AT_MAX_BYTES / AT_PAGES )     // 一页字节数
#elif   AT_DEV_TYPE == AT24C32
#define AT_MAX_BYTES            (AT24C32 / AT_BYTE_SIZE )       // 总字节数
#define AT_PAGES                128u                            // 页数
#define AT_PAGE_SIZE            ( AT_MAX_BYTES / AT_PAGES )     // 一页字节数
#elif   AT_DEV_TYPE == AT24C64
#define AT_MAX_BYTES            (AT24C64 / AT_BYTE_SIZE )       // 总字节数
#define AT_PAGES                256u                            // 页数
#define AT_PAGE_SIZE            ( AT_MAX_BYTES / AT_PAGES )     // 一页字节数
#elif   AT_DEV_TYPE == AT24C128
#define AT_MAX_BYTES            (AT24C128 / AT_BYTE_SIZE )      // 总字节数
#define AT_PAGES                128u                            // 页数
#define AT_PAGE_SIZE            ( AT_MAX_BYTES / AT_PAGES )     // 一页字节数
#elif   AT_DEV_TYPE == AT24C256
#define AT_MAX_BYTES            (AT24C256 / AT_BYTE_SIZE )      // 总字节数
#define AT_PAGES                256u                            // 页数
#define AT_PAGE_SIZE            ( AT_MAX_BYTES / AT_PAGES )     // 一页字节数
#elif   AT_DEV_TYPE == AT24C512
#define AT_MAX_BYTES            (AT24C512 / AT_BYTE_SIZE )      // 总字节数
#define AT_PAGES                512u                            // 页数
#define AT_PAGE_SIZE            ( AT_MAX_BYTES / AT_PAGES )     // 一页字节数
#elif   AT_DEV_TYPE == AT24C1024
#define AT_MAX_BYTES            (AT24C1024 / AT_BYTE_SIZE )     // 总字节数
#define AT_PAGES                512u                            // 页数
#define AT_PAGE_SIZE            ( AT_MAX_BYTES / AT_PAGES )     // 一页字节数
#endif

/***********************************************
* 描述： 宏定义AT24xx系列的I2C地址
*/
#define BSP_EEP_I2C_ADDR        0xA0


/*********************************************************************
 * GLOBAL FUNCTIONS
 */
BOOL     BSP_EEP_Init              (void);
void    BSP_EEP_Main              (void);

uint8_t  BSP_EEP_WriteByte         (uint16_t suba, uint8_t byte );
uint16_t BSP_EEP_WriteBytes        (uint16_t suba, uint8_t *buf, uint16_t len);
uint8_t  BSP_EEP_QuickReadByte     (uint8_t *dat);
uint8_t  BSP_EEP_ReadByte          (uint16_t suba, uint8_t *byte);
uint16_t BSP_EEP_ReadBytes         (uint16_t suba, uint8_t *buf, uint16_t len);
uint8_t  BSP_EEP_WriteByteChk      (uint16_t suba, uint8_t byte );

uint8_t  BSP_EEP_WriteINT16U       (uint16_t suba, uint16_t dat );
uint8_t  BSP_EEP_WriteINT32U       (uint16_t suba, uint32_t dat );

uint8_t  BSP_EEP_ReadINT16U        (uint16_t suba, uint16_t *dat );
uint8_t  BSP_EEP_ReadINT32U        (uint16_t suba, uint32_t *dat );
void    BSP_I2CSetPort              (INT08U port);
#endif
/*********************************************************************
 * 				end of file
 *********************************************************************/