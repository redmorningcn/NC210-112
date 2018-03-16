/*******************************************************************************
 *   Filename:       bsp_ds3231.h
 *   Revised:        All copyrights reserved to Roger.
 *   Date:           2016-11-06
 *   Revision:       v1.0
 *   Writer:	     wumingshen.
 *
 *   Description:    实时时钟模块 头文件
 *                   
 *
 *
 *   Notes:
 *
 *   All copyrights reserved to wumingshen
 *******************************************************************************/
#ifndef	__BSP_DS3231_H__
#define	__BSP_DS3231_H__

/*******************************************************************************
 * INCLUDES
 */
 #include  <global.h>

/*******************************************************************************
 * MACROS
 */

/***********************************************
 * 描述：键值定义
 */

/***********************************************
 * 描述：
 */

/*******************************************************************************
 * TYPEDEFS
 */
 /***********************************************
 * 描述：数据类型定义
 */
typedef struct
{
	uint8_t				Year;							//?üá??úèY
	uint8_t				Month;							//?üá??úèY
	uint8_t				Day;							//?üá??úèY
	uint8_t				Hour;							//?üá??úèY
	uint8_t				Min;							//?üá??úèY
	uint8_t				Sec;							//?üá??úèY
}TIME;



/*******************************************************************************
 * 描述： 外部函数调用
 */
extern void     GPIO_RST_CLK_Configuration  (void);
extern uint8_t  BCD2HEX                     (uint8_t Bcd);
extern uint8_t  HEX2BCD                     (uint8_t Hex);
extern uint8_t  ReadDS3231Byte              (uint8_t addr);
extern void     WriteDS3231Byte             (uint8_t Addr,uint8_t Data);
extern  float   ReadTemp                    (void);
extern void     WriteTime                   (TIME  sTime);
extern void     SetTime                     (uint8_t Year,uint8_t Month,uint8_t Day,uint8_t Hour,uint8_t Min,uint8_t Sec);  
extern void     InitDS3231                  (void);  
extern void     DisplayTime                 (void);
extern void     GetTime                     (TIME *t);

#endif
 /*******************************************************************************
 * 				end of file
 *******************************************************************************/





