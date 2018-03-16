/*
*********************************************************************************************************
*                                              EXAMPLE CODE
*
*                          (c) Copyright 2003-2006; Micrium, Inc.; Weston, FL
*
*               All rights reserved.  Protected by international copyright laws.
*               Knowledge of the source code may NOT be used to develop a similar product.
*               Please help us continue to provide the Embedded community with the finest
*               software available.  Your honesty is greatly appreciated.
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*
*                                           MASTER INCLUDES
*
*                                     ST Microelectronics STM32
*                                              with the
*                                   STM3210E-EVAL Evaluation Board
*
* Filename      : includes.h
* Version       : V1.00
* Programmer(s) : BAN
*********************************************************************************************************
*/

#ifndef  __INCLUDES_H__
#define  __INCLUDES_H__

/***********************************************
* √Ë ˆ£∫
*/
#include  <stdio.h>
#include  <string.h>
#include  <ctype.h>
#include  <stdlib.h>
#include  <stdarg.h>
#include  <math.h>

/***********************************************
* √Ë ˆ£∫
*/
#include  <stm32f10x_conf.h>
#include  <stm32f10x.h>

/***********************************************
* √Ë ˆ£∫
*/
#include  <os.h>
#include  <os_cfg_app.h>
#include  <os_app_hooks.h>

/***********************************************
* √Ë ˆ£∫
*/
#include  <osal.h>
#include  <osal_cpu.h>
#include  <osal_event.h>
#include  <osal_timer.h>

/***********************************************
* √Ë ˆ£∫
*/
#include  <cpu.h>
#include  <lib_def.h>
#include  <lib_mem.h>
#include  <lib_str.h>

/***********************************************
* √Ë ˆ£∫uC/MODBUS
*/
#include  <MB_DEF.H>
#include  <MB.h>
#include  <MB_OS.h>
#include  <MB_CFG.H>

/***********************************************
* √Ë ˆ£∫ APP
*/
#include  <app.h>
#include  <app_cfg.h>

#include  <app_lib.h>
#include  <app_calc.h>
#include  <app_comm_protocol.h>


/***********************************************
* √Ë ˆ£∫ BSP
*/
#include  <bsp.h>
#include  <bsp_wdt.h>

#include  "bsp_filteralgorithm.h"
#include  "bsp_dataconversion.h"

#include  "Protocol.h"
#include  "bsp_usart.h"
#include  "bsp_eeprom.h"
#include  "bsp_time.h"
#include  "bsp_flash.h"

#include  <bsp_adc7682.h>
#include  <bsp_max7219.h>

/***********************************************
* √Ë ˆ£∫
*/
#if (APP_OS_PROBE_EN == DEF_ENABLED)
#include  <os_probe.h>
#endif

#if (APP_PROBE_COM_EN == DEF_ENABLED)
#include  <probe_com.h>

#if (PROBE_COM_METHOD_RS232 == DEF_ENABLED)
#include  <probe_rs232.h>
#endif
#endif



#endif
