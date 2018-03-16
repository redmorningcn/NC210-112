
#include "bsp_tim.h"
#include "bsp.h"
#include "app_ctrl.h"
/*****************************************************************************************************/
/* EXTERN VARIABLES*/



//============================================================================//

void TIM3_init()
{

TIM_TimeBaseInitTypeDef    Tim_Handle;

  /* Initialize TIMx peripheral as follows:
       + Period = 10000 - 1
       + Prescaler = (SystemCoreClock/10000) - 1
       + ClockDivision = 0
       + Counter direction = Up
  */
	Tim_Handle.TIM_Period= 65535;
	Tim_Handle.TIM_Prescaler= TIM_ICPSC_DIV1;
	Tim_Handle.TIM_ClockDivision= 0;
	Tim_Handle.TIM_CounterMode= TIM_CounterMode_Up;
	Tim_Handle.TIM_RepetitionCounter = 0;
	TIM_TimeBaseInit(TIM3 ,&Tim_Handle)


  /*##-2- Start the TIM Base generation in interrupt mode ####################*/
  /* Start Channel1 */
  if (HAL_TIM_Base_Start_IT(&Tim3_Handle) != HAL_OK)
  {
    /* Starting Error */
  }
}

void BSP_tim_init()
{
}



