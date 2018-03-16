#include "stm32f10x.h"
#include "stm32f10x_gpio.h"
#include <stdio.h>
#include "delay.h"


/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/

/**-----------------------------------------------------------------
  * @函数名 Delay
  * @功能   延时函数
  * @参数   无
  * @返回值 无
***----------------------------------------------------------------*/
void Delay(uint32_t i)
{
	uint32_t j ;
	while (i--) {
		for(j = 10;j > 0;j--);
	}
}


void Delay_ARMJISHU(uint32_t nCount)
{
  for (; nCount != 0; nCount--);
}

