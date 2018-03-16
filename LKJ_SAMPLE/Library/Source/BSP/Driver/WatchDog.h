#ifndef		__WATCHGDOG_H__
#define		__WATCHGDOG_H__


/**-------------------------------------------------------
  * @函数名 IO-PORT_Config
  * @功能   初始化SPI_ISD的端口
  * @参数   无
  * @返回值 无
***------------------------------------------------------*/
extern void WDG_Init(void);


extern void FeedDog_LOW(void);


extern void FeedDog_HIGH(void);

#endif

