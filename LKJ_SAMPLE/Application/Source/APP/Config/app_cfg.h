/*
*********************************************************************************************************
*                                              EXAMPLE CODE
*
*                           (c) Copyright 2009-2013; Micrium, Inc.; Weston, FL
*
*               All rights reserved.  Protected by international copyright laws.
*               Knowledge of the source code may NOT be used to develop a similar product.
*               Please help us continue to provide the Embedded community with the finest
*               software available.  Your honesty is greatly appreciated.
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*                                      APPLICATION CONFIGURATION
*
*                                     ST Microelectronics STM32
*                                              on the
*
*                                     Micrium uC-Eval-STM32F107
*                                         Evaluation Board
*
* Filename      : app_cfg.h
* Version       : V1.00
* Programmer(s) : JJL
*                 EHS
*                 DC
*********************************************************************************************************
*/

#ifndef  __APP_CFG_H__
#define  __APP_CFG_H__


/*
*********************************************************************************************************
*                                       MODULE ENABLE / DISABLE
*********************************************************************************************************
*/

#define  APP_CFG_SERIAL_EN                          DEF_DISABLED


/****************************************************
* 描述： 系统使用到的ModBus从机节点站号
*/
#define ModbusNode                                  2u

/*
*********************************************************************************************************
*                                           配置任务的优先级
*                                            TASK PRIORITIES
*                设置任务优先级，uCOS-III中优先级可以相同，使用时间片转轮询完成相同优先级任务的执行
*                           !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
*                        !!!任务优先级分配很重要，优先级会影响整个系统的运行!!!
*                           !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
*********************************************************************************************************
*/


#define  MB_OS_CFG_RX_TASK_PRIO     (OS_CFG_PRIO_MAX - 14)  // ModBus任务的优先级（没有用到ModBus则不需要关注）
#define  APP_TASK_START_PRIO        (OS_CFG_PRIO_MAX - 13)  
#define  APP_TASK_WIFI_PRIO         (OS_CFG_PRIO_MAX - 12) 
#define  APP_TASK_COMM_PRIO         (OS_CFG_PRIO_MAX - 12)  
#define  APP_TASK_SENSOR_PRIO       (OS_CFG_PRIO_MAX - 11)  
#define  APP_TASK_BEEP_PRIO         (OS_CFG_PRIO_MAX - 10)  
#define  APP_TASK_KEY_PRIO          (OS_CFG_PRIO_MAX -  9)  
#define  APP_TASK_CONTROL_PRIO      (OS_CFG_PRIO_MAX -  8)  

#define  APP_TASK_OSAL_PRIO         (OS_CFG_PRIO_MAX -  3)  // 例程任务优先级，可以随意修改。

#define  APP_TASK_SAMPLE_PRIO       (OS_CFG_PRIO_MAX -  2)  // 例程任务优先级，可以随意修改。
/*
*********************************************************************************************************
*                                           配置任务堆栈的大小
*                                            TASK STACK SIZES
*                             Size of the task stacks (# of OS_STK entries)
*                      堆栈的字节大小 = CPU数据宽度 * 8 * size = 4 * 8 * size(字节)
*                           如：APP_TASK_SAMPLE_STK_SIZE = 128u，实际为 128u * 4 = 512（字节）
*                             可以在IAR仿真中查看堆栈大小分配是否充足。
*********************************************************************************************************
*/

#define  MB_OS_CFG_RX_TASK_STK_SIZE                 512u        // ModBus任务堆栈的大小
#define  APP_TASK_START_STK_SIZE                    128u        // 给起始任务分配堆栈
#define  APP_TASK_SAMPLE_STK_SIZE                    60u        // 例程任务堆栈大小，用户更换成自己相应的名称
#define  APP_TASK_WIFI_STK_SIZE                     128u
#define  APP_TASK_COMM_STK_SIZE                     256u
#define  APP_TASK_SENSOR_STK_SIZE                   256u
#define  APP_TASK_KEY_STK_SIZE                      128u
#define  APP_TASK_CONTROL_STK_SIZE                  128u
#define  APP_TASK_BEEP_STK_SIZE                     128u
#define  APP_TASK_OSAL_STK_SIZE                     512u


/*
*********************************************************************************************************
*                                    BSP CONFIGURATION: RS-232
*********************************************************************************************************
*/

#define  BSP_CFG_SER_COMM_SEL             			BSP_SER_COMM_UART_02
#define  BSP_CFG_TS_TMR_SEL                         2


/*
*********************************************************************************************************
*                                     TRACE / DEBUG CONFIGURATION
*********************************************************************************************************
*/
#if 0
#define  TRACE_LEVEL_OFF                            0
#define  TRACE_LEVEL_INFO                           1
#define  TRACE_LEVEL_DEBUG                          2
#endif

#define  APP_TRACE_LEVEL                            TRACE_LEVEL_INFO
#define  APP_TRACE                                  BSP_Ser_Printf

#define  APP_TRACE_INFO(x)            ((APP_TRACE_LEVEL >= TRACE_LEVEL_INFO)  ? (void)(APP_TRACE x) : (void)0)
#define  APP_TRACE_DEBUG(x)           ((APP_TRACE_LEVEL >= TRACE_LEVEL_DEBUG) ? (void)(APP_TRACE x) : (void)0)


#endif
