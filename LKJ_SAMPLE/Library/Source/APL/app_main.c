

#include <app.h>
#include <iap.h>
#include "task_comm.h"


/*******************************************************************************
* Description  : 系统参数
* Author       : 2018/3/14 星期三, by redmorningcn
*******************************************************************************/
strSysPara  sys;


extern uint16	collector_update_count_down_tmr;
OS_RATE_HZ     const  OSCfg_TickRate_Hz          = (OS_RATE_HZ  )OS_CFG_TICK_RATE_HZ;

extern int32 get_pulse_count();

extern  uint32 get_sys_tick();

 /*******************************************************************************
 * 名    称：     polling
 * 功    能：     系统时钟查询函数
 * 入口参数：    无
 * 出口参数：    无
 * 作　 　者：   无名沈
 * 创建日期：    2014-08-08
 * 修    改：
 * 修改日期：
 *******************************************************************************/
void polling()
{
//	static uint32 tick;
//	static uint16 polling_tick_cnt;
//	static uint16 led_blink_cnt = 0;
//
//	if (tick != get_sys_tick())
//	{
//		tick = get_sys_tick();
//		polling_tick_cnt++;
//		if (0 == polling_tick_cnt%100)
//		{
//			//100ms
//			system_send_msg(DETECT_TASK_ID, DETECT_CALC_DETECT_INFO, NULL, 0);
//			if ( 0 == led_blink_cnt )
//			{
//				BSP_LED_On(1);
//			}
//			else
//			{
//				BSP_LED_Off(1);
//			}
//			led_blink_cnt++;
//			if(0 == get_pulse_count())
//			{
//				led_blink_cnt %= 20;
//			}
//			else
//			{
//				led_blink_cnt %= 2;
//			}
////			system_send_msg(COMM_TASK_ID, COMM_TASK_COLLECTION_INFO_UPDATE, NULL, 0);
//		}
//		if (collector_update_count_down_tmr)
//		{
//			collector_update_count_down_tmr--;
//			if (0 == collector_update_count_down_tmr)
//			{
//                /*******************************************************************************
//                * Description  : 串口操作定时器到，进行串口处理
//                * Author       : 2018/1/30 星期二, by redmorningcn
//                *******************************************************************************/
//
//				system_send_msg(COMM_TASK_ID, COMM_TASK_COLLECTION_INFO_UPDATE, NULL, 0);
//			}
//		}
//	}
}

void  OS_CPU_SysTickInit (CPU_INT32U  cnts)
{
    CPU_INT32U  prio;


    CPU_REG_NVIC_ST_RELOAD = cnts - 1u;

                                                            /* Set SysTick handler prio.                              */
    prio  = CPU_REG_NVIC_SHPRI3;
    prio &= DEF_BIT_FIELD(24, 0);
    prio |= DEF_BIT_MASK(OS_CPU_CFG_SYSTICK_PRIO, 24);

    CPU_REG_NVIC_SHPRI3 = prio;

                                                            /* Enable timer.                                          */
    CPU_REG_NVIC_ST_CTRL |= CPU_REG_NVIC_ST_CTRL_CLKSOURCE |
                            CPU_REG_NVIC_ST_CTRL_ENABLE;
                                                            /* Enable timer interrupt.                                */
    CPU_REG_NVIC_ST_CTRL |= CPU_REG_NVIC_ST_CTRL_TICKINT;
}

 /*******************************************************************************
 * Description  : 程序框架说明
    1、实时性要求高，未进行有操作系统
    2、程序运行采用事件驱动，通过事件回调函数运行（system_get_msg）；
       system_send_msg,用于设置事件，入口参数（任务ID，事件ID，数据区指针，数据长度）；
    3、system_get_msg的入口参数是任务ID（任务中可定义多种事件），
       返回事件组指针（包含：事件号、数据长度、及数据指针）；
    4、app_XXXX_task_event,循环执行，有对应事件产生，
       则从任务事件app_XXX_task_event_table表中，找到对应回调函数处理事件。

 * Author       : 2018/1/30 星期二, by redmorningcn
 *******************************************************************************/

void main (void)
{
	CPU_INT32U  cpu_clk_freq;
	CPU_INT32U  cnts;
	BSP_Init();                                                 /* Initialize BSP functions                             */
	CPU_TS_TmrInit();

	/***********************************************
	* 描述： 初始化滴答定时器，即初始化系统节拍时钟。
	*/
	cpu_clk_freq = BSP_CPU_ClkFreq();                           /* Determine SysTick reference freq.                    */
    
    sys.cpu_freq = cpu_clk_freq;                                //时钟频率
    
	cnts = cpu_clk_freq / (CPU_INT32U)OSCfg_TickRate_Hz;        /* Determine nbr SysTick increments                     */
	//OS_CPU_SysTickInit(cnts);
	
	App_ModbusInit();
	APP_Detect_Init();
    init_ch_timepara_detect();

	while(1)
	{
        /*******************************************************************************
        * Description  : 串口通讯处理，调用在MOUDB底层设置事件的处理函数
        * Author       : 2018/3/9 星期五, by redmorningcn
        *******************************************************************************/
		AppTaskComm();          //串口通讯时间处理。
        
        //和AppTaskComm一起完成串口通讯
		mod_bus_rx_task();
        
        /*******************************************************************************
          * Description  : 信号检测
          * Author       : 2018/3/9 星期五, by redmorningcn
          *******************************************************************************/
		//AppTaskDetect();
        
		polling();
        
        app_calc_ch_timepara();
	}
}


