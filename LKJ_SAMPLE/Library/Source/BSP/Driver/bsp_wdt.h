/*******************************************************************************
 *   Filename:       bsp_wdt.c
 *   Revised:        All copyrights reserved to Roger.
 *   Date:           2015-08-11
 *   Revision:       v1.0
 *   Writer:	     wumingshen.
 *
 *   Description:    看门狗驱动模块头文件
 *
 *
 *   Notes:
 *   独立看门狗工作原理：在键值寄存器（IWDG_KR）中写入0XCCCC，开始启用独立看门狗，
 *   此时计数器开始从其复位值OXFFF递减计数，当计数器计数到末尾0X000的时候，会产生
 *   一个复位信号（IWDG_RESET），无论何时，只要寄存器IWDG_KR中被写入0XAAAA，IWDG_RLR
 *   中的值就会被重新加载到计数器中从而避免产生看门狗复位。
 *
 *   All copyrights reserved to wumingshen
 *******************************************************************************/

#ifndef	__BSP_WDT_H__
#define	__BSP_WDT_H__

/*******************************************************************************
 * INCLUDES
 */
#include  <global.h>

/*******************************************************************************
 * CONSTANTS
 */
#define BSP_WDT_MODE_NONE   0       //不使用看门狗
#define BSP_WDT_MODE_EXT    1       //仅使用外部看门狗
#define BSP_WDT_MODE_INT    2       //仅使用内部看门狗
#define BSP_WDT_MODE_ALL    3       //外部和内部看门狗同时使用


#define WDI_GPIO_PIN        GPIO_Pin_8             /* PC.00 */
#define WDI_GPIO_PORT       GPIOA
#define WDI_GPIO_RCC        RCC_APB2Periph_GPIOA


/*******************************************************************************
 * GLOBAL VARIABLES
 */
extern INT8U BSP_WdtMode;          // 0:禁止；1：外部看门狗 2：内部看门狗；3：同时使用内部和外部看门狗


/*******************************************************************************
 * GLOBAL FUNCTIONS
 */
uint8_t   BSP_WDT_Init      (uint8_t mode);
void      BSP_WDT_Rst       (void);
uint8_t   BSP_WDT_GetMode   (void);


#endif
/*******************************************************************************
 *                        end of file                                          *
 *******************************************************************************/