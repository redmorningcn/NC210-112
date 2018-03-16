/*
*********************************************************************************************************
*                                              uC/Modbus
*                                       The Embedded Modbus Stack
*
*                          (c) Copyright 2003-2009; Micrium, Inc.; Weston, FL
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
*                                     MODBUS uC/OS-II LAYER INTERFACE
*
* Filename      : mb_os.h
* Version       : V2.12
* Programmer(s) : JJL
*********************************************************************************************************
* Note(s)       :
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*                                               MODULE
*********************************************************************************************************
*/

#ifndef  MB_OS_MODULE_PRESENT
#define  MB_OS_MODULE_PRESENT

/*
*********************************************************************************************************
*                                            INCLUDE FILES
*
*********************************************************************************************************
*/

#include <mb_cfg.h>
#include <mb.h>
#include <os.h>

/*
*********************************************************************************************************
*                                               EXTERNS
*********************************************************************************************************
*/

#ifdef   MB_OS_MODULE
#define  MB_OS_EXT
#else
#define  MB_OS_EXT  extern
#endif


/*
*********************************************************************************************************
*                                        DEFAULT CONFIGURATION
*********************************************************************************************************
*/


/*
*********************************************************************************************************
*                                               DEFINES
*********************************************************************************************************
*/
#if OS_VERSION   < 30000u
#ifndef MB_OS_CFG_RX_TASK_PRIO
#define  MB_OS_CFG_RX_TASK_PRIO                     (OS_CFG_PRIO_MAX - 13)
#endif
#ifndef MB_OS_CFG_RX_TASK_STK_SIZE
#define  MB_OS_CFG_RX_TASK_STK_SIZE                 512u
#endif
#define  MB_OS_CFG_RX_TASK_ID                       MB_OS_CFG_RX_TASK_PRIO
#endif

/*
*********************************************************************************************************
*                                             DATA TYPES
*********************************************************************************************************
*/


/*
*********************************************************************************************************
*                                          GLOBAL VARIABLES
*********************************************************************************************************
*/


/*
*********************************************************************************************************
*                                              MACRO'S
*********************************************************************************************************
*/


/*
*********************************************************************************************************
*                                         FUNCTION PROTOTYPES
*********************************************************************************************************
*/


/*
*********************************************************************************************************
*                                        CONFIGURATION ERRORS
*********************************************************************************************************
*/

#if OS_VERSION > 30000u
#if      (OS_CFG_Q_EN == 0)
#error  "MODBUS Slave requires uC/OS-III Message Queue Services."
#endif

#ifndef  MB_OS_CFG_RX_TASK_PRIO
#error  "MODBUS Missing Rx Task's MB_OS_CFG_RX_TASK_PRIO."
#endif

#ifndef  MB_OS_CFG_RX_TASK_STK_SIZE
#error  "MODBUS Missing Rx Task's MB_OS_CFG_RX_TASK_STK_SIZE."
#endif

#else

#if      (OS_Q_EN == 0)
#error  "MODBUS Slave requires uC/OS-II Message Queue Services."
#endif

#ifndef  MB_OS_CFG_RX_TASK_ID
#error  "MODBUS Missing Rx Task's MB_OS_CFG_RX_TASK_ID."
#endif


#ifndef  MB_OS_CFG_RX_TASK_PRIO
#error  "MODBUS Missing Rx Task's MB_OS_CFG_RX_TASK_PRIO."
#endif


#ifndef  MB_OS_CFG_RX_TASK_STK_SIZE
#error  "MODBUS Missing Rx Task's MB_OS_CFG_RX_TASK_STK_SIZE."
#endif

#endif

#if      (MODBUS_CFG_MASTER_EN == DEF_ENABLED)
#if OS_VERSION > 30000u
#if      (OS_CFG_SEM_EN            == 0          )
#error  "MODBUS Master requires uC/OS-III Semaphore Services."
#error  "... It needs MODBUS_CFG_MAX_CH semaphores."
#endif
#else
#if      (OS_SEM_EN            == 0          )
#error  "MODBUS Master requires uC/OS-II Semaphore Services."
#error  "... It needs MODBUS_CFG_MAX_CH semaphores."
#endif
#endif
#endif


/*
*********************************************************************************************************
*                                             MODULE END
*********************************************************************************************************
*/

#endif                                                          /* End of MB_OS module                                */

