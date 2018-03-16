/*
*********************************************************************************************************
*                                            uC/Modbus
*
*                         (c) Copyright 2003-2009; Micrium, Inc.; Weston, FL
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
*                                      MODBUS CONFIGURATION
*
* Filename      : mb_cfg.h
* Version       : V2.12
* Programmer(s) : JJL
*********************************************************************************************************
* Note(s)       : (1) This file contains configuration constants for uC/Modbus
*********************************************************************************************************
*/

#ifndef  MB_DEF_MODULE_PRESENT
#define  MB_DEF_MODULE_PRESENT
/*
*********************************************************************************************************
*                                  MODBUS MODULES CONFIGURATION
*********************************************************************************************************
*/

#define  MB_AES_EN                        DEF_DISABLED          /* Enable or Disable  Modbus Slave                    */
#define  MB_NONMODBUS_EN                  DEF_ENABLED           /* Enable or Disable  非Modbus 通信                    */
#define  MB_IAPMODBUS_EN                  DEF_ENABLED           /* Enable or Disable  IAP升级                          */
#define  MBM_STAT_CTR_AUTO_CLR_EN         DEF_ENABLED           /* 自动清除计数器  pch->StatNoRespCtr    = 0;
                                                                                   pch->StatCRCErrCtr    = 0;
                                                                                   pch->StatExceptCtr    = 0;         */
/*
*********************************************************************************************************
*                                  MODBUS MODULES CONFIGURATION
*********************************************************************************************************
*/

#define  MODBUS_CFG_SLAVE_EN              DEF_ENABLED           /* Enable or Disable  Modbus Slave                    */
#define  MODBUS_CFG_MASTER_EN             DEF_DISABLED           /* Enable or Disable  Modbus Master                   */

/*
*********************************************************************************************************
*                                  MODBUS MODES CONFIGURATION
*********************************************************************************************************
*/

#define  MODBUS_CFG_ASCII_EN               DEF_DISABLED         /* Modbus ASCII is supported when DEF_ENABLED         */
#define  MODBUS_CFG_RTU_EN                 DEF_ENABLED          /* Modbus RTU   is supported when DEF_ENABLED         */

/*
*********************************************************************************************************
*                               MODBUS COMMUNICATION CONFIGURATION
*********************************************************************************************************
*/

#define  MODBUS_CFG_MAX_CH                           3          /* Maximum number of Modbus channels.                 */

#define  MODBUS_CFG_BUF_SIZE                       256          /* Maximum outgoing message size.                     */
#define  MODBUS_CFG_FAILD_MAX                        5          /* Maximum 通讯失败次数.                              */

/*
*********************************************************************************************************
*                                  MODBUS FLOATING POINT SUPPORT
*********************************************************************************************************
*/

#define  MODBUS_CFG_FP_EN                       DEF_DISABLED    /* Enable Floating-Point support.                     */

#if      MODBUS_CFG_FP_EN == DEF_ENABLED
#define  MODBUS_CFG_FP_START_IX                   5000          /* Start address of Floating-Point registers          */
#else
#define  MODBUS_CFG_FP_START_IX                   65500         /* Floating point is disabled, set start of ...       */
                                                                /*   ...FP very high                                  */
#endif

/*
*********************************************************************************************************
*                                  MODBUS DATA STRUCT
*********************************************************************************************************
*/
#define MB_DATA_NBR_REGS        125
#define MB_DATA_NBR_COILS       32
#define MB_DATA_ACC_PASSWORD   (6237)

/*
*********************************************************************************************************
*                                   MODBUS FUNCTIONS CONFIGURATION
*********************************************************************************************************
*/

#define  MODBUS_CFG_FC01_EN                DEF_ENABLED         /* Enable or Disable support for Modbus functions     */
#define  MODBUS_CFG_FC02_EN                DEF_DISABLED
#define  MODBUS_CFG_FC03_EN                DEF_ENABLED
#define  MODBUS_CFG_FC04_EN                DEF_DISABLED
#define  MODBUS_CFG_FC05_EN                DEF_ENABLED
#define  MODBUS_CFG_FC06_EN                DEF_ENABLED
#define  MODBUS_CFG_FC08_EN                DEF_ENABLED
#define  MODBUS_CFG_FC15_EN                DEF_ENABLED
#define  MODBUS_CFG_FC16_EN                DEF_ENABLED
#define  MODBUS_CFG_FC20_EN                DEF_ENABLED
#define  MODBUS_CFG_FC21_EN                DEF_ENABLED
#define  MODBUS_FC43_EN                    DEF_DISABLED
#define  MODBUS_FC100_EN                   DEF_DISABLED


/*
*********************************************************************************************************
*                                   MODBUS RX TASK ID CONFIGURATION
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*                                            TASK STACK SIZES
*                             Size of the task stacks (# of OS_STK entries)
*********************************************************************************************************
*/
#endif