/*******************************************************************************
 *   Filename:       app_task_key.c
 *   Revised:        All copyrights reserved to Roger-WY.
 *   Revision:       v1.0
 *   Writer:	     Roger-WY.
 *
 *   Description:    双击选中 key 按 Ctrl + H, 钩选 Match the case, Replace with
 *                   输入您要的名字，点击 Replace All
 *                   双击选中 Key 按 Ctrl + H, 钩选 Match the case, Replace with
 *                   输入您要的名字，点击 Replace All
 *                   双击选中 KEY 按 Ctrl + H, 钩选 Match the case, Replace with
 *                   输入您要的名字，点击 Replace All
 *                   在 app_cfg.h 中指定本任务的 优先级  （ APP_TASK_KEY_PRIO ）
 *                                            和 任务堆栈（ APP_TASK_KEY_STK_SIZE ）大小
 *                   在 app.h 中声明本任务的     创建函数（ void  App_TaskKeyCreate(void) ）
 *                                            和 看门狗标志位 （ WDTFLAG_Key ）
 *
 *   Notes:
 *     				E-mail:261313062@qq.com
 *
 *******************************************************************************/

/*******************************************************************************
 * INCLUDES
 */
#define  SNL_APP_SOURCE
#include <includes.h>
#include "bsp_key.h"
#include <bsp_control.h>

#ifdef VSC_INCLUDE_SOURCE_FILE_NAMES
const  CPU_CHAR  *app_task_key__c = "$Id: $";
#endif

#define APP_TASK_KEY_EN     DEF_DISABLED
#if APP_TASK_KEY_EN == DEF_ENABLED
/*******************************************************************************
 * CONSTANTS
 */

/*******************************************************************************
 * MACROS
 */
//该任务的控制周期   (OS_CFG_TICK_RATE_HZ)为1S
#define  KEY_CYCLE_TIME_TICKS     (OS_CFG_TICK_RATE_HZ * 1u)

/*******************************************************************************
 * TYPEDEFS
 */
/***********************************************
* 描述： 任务控制块（TCB）
*/
static  OS_TCB   AppTaskKeyTCB;

/***********************************************
* 描述： 任务堆栈（STACKS）
*/
static  CPU_STK  AppTaskKeyStk[ APP_TASK_KEY_STK_SIZE ];

/*******************************************************************************
 * LOCAL VARIABLES
 */

/*******************************************************************************
 * GLOBAL VARIABLES
 */
/*******************************************************************************
 * LOCAL FUNCTIONS
 */
static  void    AppTaskKey          (void *p_arg);
static  void    APP_KeyInit         (void);
static  void    KeyValueHandle      (StrCtrlDCB *dcb);


/*******************************************************************************
 * GLOBAL FUNCTIONS
 */

/*******************************************************************************
 * EXTERN VARIABLES
 */
extern OS_SEM   App_BeepSem;

/*******************************************************************************
 * EXTERN FUNCTIONS
 */

/*******************************************************************************/

/*******************************************************************************
 * 名    称： App_TaskKeyCreate
 * 功    能： **任务创建
 * 入口参数： 无
 * 出口参数： 无
 * 作　　者： Roger-WY.
 * 创建日期： 2015-02-05
 * 修    改：
 * 修改日期：
 * 备    注： 任务创建函数需要在app.h文件中声明
 *******************************************************************************/
void  App_TaskKeyCreate(void)
{
    OS_ERR  err;

    /***********************************************
    * 描述： 任务创建
    */
    OSTaskCreate((OS_TCB     *)&AppTaskKeyTCB,                  // 任务控制块  （当前文件中定义）
                 (CPU_CHAR   *)"App Task Key",                  // 任务名称
                 (OS_TASK_PTR ) AppTaskKey,                     // 任务函数指针（当前文件中定义）
                 (void       *) 0,                              // 任务函数参数
                 (OS_PRIO     ) APP_TASK_KEY_PRIO,              // 任务优先级，不同任务优先级可以相同，0 < 优先级 < OS_CFG_PRIO_MAX - 2（app_cfg.h中定义）
                 (CPU_STK    *)&AppTaskKeyStk[0],               // 任务栈顶
                 (CPU_STK_SIZE) APP_TASK_KEY_STK_SIZE / 10,     // 任务栈溢出报警值
                 (CPU_STK_SIZE) APP_TASK_KEY_STK_SIZE,          // 任务栈大小（CPU数据宽度 * 8 * size = 4 * 8 * size(字节)）（app_cfg.h中定义）
                 (OS_MSG_QTY  ) 5u,                             // 可以发送给任务的最大消息队列数量
                 (OS_TICK     ) 0u,                             // 相同优先级任务的轮循时间（ms），0为默认
                 (void       *) 0,                              // 是一个指向它被用作一个TCB扩展用户提供的存储器位置
                 (OS_OPT      )(OS_OPT_TASK_STK_CHK |           // 允许堆栈检查该任务
                                OS_OPT_TASK_STK_CLR),           // 创建任务时堆栈清零
                 (OS_ERR     *)&err);                           // 指向错误代码的指针，用于创建结果处理

}

/*******************************************************************************
 * 名    称： AppTaskKey
 * 功    能： 控制任务
 * 入口参数： p_arg - 由任务创建函数传入
 * 出口参数： 无
 * 作　　者： Roger-WY.
 * 创建日期： 2015-02-05
 * 修    改：
 * 修改日期：
 *******************************************************************************/
static  void  AppTaskKey (void *p_arg)
{
    OS_ERR      err;
    /***********************************************
    * 描述： 任务初始化
    */
    APP_KeyInit();

    /***********************************************
    * 描述：Task body, always written as an infinite loop.
    */
    while (DEF_TRUE) {

        /***********************************************
        * 描述： 本任务看门狗标志置位
        */
        OS_FlagPost ((OS_FLAG_GRP *)&WdtFlagGRP,
                     (OS_FLAGS     ) WDT_FLAG_KEY,
                     (OS_OPT       ) OS_OPT_POST_FLAG_SET,
                     (CPU_TS       ) 0,
                     (OS_ERR      *) &err);
        /***********************************************
        * 描述： 当有按键事件时获取键值和按键事件
        */
        if( KEY_GetKey(Ctrl.Key) != KEY_STATUS_NULL ) {
            KeyValueHandle(&Ctrl);
        }
    }
}

/*******************************************************************************
 * 名    称： APP_KeyInit
 * 功    能： 任务初始化
 * 入口参数： 无
 * 出口参数： 无
 * 作　　者： Roger-WY.
 * 创建日期： 2015-03-28
 * 修    改：
 * 修改日期：
 *******************************************************************************/
static void APP_KeyInit(void)
{
    /***********************************************
    * 描述： 初始化本任务用到的相关硬件
    */
    KEY_Init();

    /***********************************************
    * 描述： 在看门狗标志组注册本任务的看门狗标志
    */
    WdtFlags |= WDT_FLAG_KEY;
}

/*******************************************************************************
 * 名    称： KeyValueHandle
 * 功    能： 按键处理函数
 * 入口参数： 无
 * 出口参数： 无
 * 作　　者： Roger-WY.
 * 创建日期： 2015-03-28
 * 修    改：
 * 修改日期：
 *******************************************************************************/
extern OS_SEM  SemReportStatus;            //主动上报状态信号量
static void KeyValueHandle(StrCtrlDCB *dcb)
{
    OS_ERR err;
    static INT08U  LongPressedFlag  = 0;
    /***********************************************
    * 描述： 获取按键事件
    */
    switch(dcb->Key->evt){
    /***************************************************************************
    * 描述： 按键按下事件
    */
    case KEY_EVT_DOWN: {
        switch(dcb->Key->val) {
        /***********************************************
        * 描述： WIFI复位键
        */
        case KEY_NRESET: 
            break;
        /***********************************************
        * 描述： WIFI配置键
        */
        case KEY_NRELOAD:
            osal_stop_timerEx( OS_TASK_ID_SEN,
                               OS_EVT_SEN_PM2D5_CHK);
            break;
        /***********************************************
        * 描述： 电源键
        */
        case KEY_PWR: 
            break;
        }
    };break;
    
    /***************************************************************************
    * 描述： 按键弹起事件
    */
    case KEY_EVT_UP: {        
        /***********************************************
        * 描述： 清除长按标志
        */
        if ( LongPressedFlag ) {
            LongPressedFlag    = 0;
        } else {
            switch(dcb->Key->val) {
            /***********************************************
            * 描述： WIFI复位键
            */
            case KEY_NRESET: 
                if ( Ctrl.Sys.FirstPwrFlag == TRUE ) {
                    if ( ++Ctrl.PM2D5.ChkVal > 35 ) {
                        Ctrl.PM2D5.ChkVal   = 20;
                        BeepExt(3, 2000, 100);
                    } else {
                        BeepExt(1, 1500, 200);                        
                    }
                } else {
                    /***********************************************
                    * 描述： 发送按键事件标志位
                    */
                    Beep( BEEP_FLAG_KEY_EVT );
                }
                break;
            /***********************************************
            * 描述： WIFI配置键
            */
            case KEY_NRELOAD: 
                if ( Ctrl.Sys.FirstPwrFlag == TRUE ) {
                    osal_stop_timerEx( OS_TASK_ID_SEN,
                                       OS_EVT_SEN_PM2D5_CHK);
                    osal_start_timerEx( OS_TASK_ID_SEN,
                                       OS_EVT_SEN_PM2D5_CHK,
                                        OS_TICKS_PER_SEC * 1);
                    
                    BeepExt(2, 1500, 200);
                /***********************************************
                * 描述： 发送按键事件标志位
                */
                } else {
                    /***********************************************
                    * 描述： PM2.5低值校验
                    */
                    osal_start_timerEx( OS_TASK_ID_SEN,
                                       OS_EVT_SEN_CHK_TIMEOUT,
                                       OS_TICKS_PER_SEC);
                    
                    if ( ++Ctrl.PM2D5.ChkCtr > 10 ) {  
                        osal_start_timerEx( OS_TASK_ID_SEN,
                                           OS_EVT_SEN_PM2D5_CHK,
                                           OS_TICKS_PER_SEC * 10); 
                        BeepExt(3, 1500, 200);
                    } else {                
                        /***********************************************
                        * 描述： WIFI配置键
                        */
                        Ctrl.Wifi.W2dWifiSts.WifiStatus.ustatus.SoftApMode  = 1;
                        Ctrl.Wifi.W2dWifiSts.WifiStatus.ustatus.ConnRouter  = 0;
                        Ctrl.Sys.WifiSts = WIFI_STS_CONFIG;
                        
                        osal_set_event( OS_TASK_ID_CTRL, (OS_EVT_CTRL_WIFI_CFG) );
                        osal_start_timerEx( OS_TASK_ID_CTRL, (OS_EVT_CTRL_WIFI_TMO), 60*OS_TICKS_PER_SEC);
                        /***********************************************
                        * 描述： 发送按键事件标志位
                        */
                        Beep( BEEP_FLAG_KEY_EVT );
                        /***********************************************
                        * 描述： 通知WIFI模组进入配置模式
                        */
                        OSFlagPost(( OS_FLAG_GRP  *)&Ctrl.Os.WifiEvtFlagGrp,
                                   ( OS_FLAGS      )WIFI_EVT_FLAG_CONFIG,
                                   ( OS_OPT        )OS_OPT_POST_FLAG_SET,
                                   ( OS_ERR       *)&err);
                    }
                }
                break;
            /***********************************************
            * 描述： 电源键
            */
            case KEY_PWR:
                /***********************************************
                * 描述： 在没有开启儿童锁的时候，单机修改模式
                */
                if( !Ctrl.Wifi.Wr.ChildSecurityLock ) {
                    /***********************************************
                    * 描述： 如果是关机状态，则开机
                    */
                    if( SYS_PWR_ON != Ctrl.Sys.SysSts ) {
                        /***********************************************
                        * 描述： 开机音
                        */
                        Power(ON);
                    /***********************************************
                    * 描述： 如果是开机状态，改变模式
                    */
                    } else {
                        if ( ++Ctrl.Fan.Mode > FAN_MODE_MAX_SPEED ){
                            Ctrl.Fan.Mode  = FAN_MODE_SILENT;
                        }
                        /***********************************************
                        * 描述： 发送按键事件标志位
                        */
                        Beep( BEEP_FLAG_KEY_EVT );
                    }
                /***********************************************
                * 描述： 在有开启儿童锁的时候，不修改模式，播放锁定音
                */
                } else {
                    /***********************************************
                    * 描述： 发送儿童锁事件标志位
                    */
                    OSFlagPost(( OS_FLAG_GRP  *)&BeepFlagGrp,
                               ( OS_FLAGS      )BEEP_FLAG_LOCK_EVT,
                               ( OS_OPT        )OS_OPT_POST_FLAG_SET,
                               ( OS_ERR       *)&err);
                }
                break;
            }
        }
    } break;
    
    /***************************************************************************
    * 描述： 按键按住事件
    */
    case KEY_EVT_PRESSED: {
        switch(dcb->Key->val) {
        /***********************************************
        * 描述： WIFI复位键
        */
        case KEY_NRESET:
            if ( !LongPressedFlag++ ) {
                BeepExt(3, 1500, 200);
            }
            break;
        /***********************************************
        * 描述： WIFI配置键
        */
        case KEY_NRELOAD:{
            if ( !LongPressedFlag++ ) {
                Ctrl.Sys.WifiSts = WIFI_STS_CONFIG;
                osal_set_event( OS_TASK_ID_CTRL, (OS_EVT_CTRL_WIFI_CFG) );
                osal_start_timerEx( OS_TASK_ID_CTRL, (OS_EVT_CTRL_WIFI_TMO), 60*OS_TICKS_PER_SEC);
                /***********************************************
                * 描述： 发送按键事件标志位
                */
                Beep( BEEP_FLAG_KEY_EVT );
                /***********************************************
                * 描述： 通知WIFI模组进入配置模式
                */
                OSFlagPost(( OS_FLAG_GRP  *)&Ctrl.Os.WifiEvtFlagGrp,
                           ( OS_FLAGS      )WIFI_EVT_FLAG_CONFIG,
                           ( OS_OPT        )OS_OPT_POST_FLAG_SET,
                           ( OS_ERR       *)&err);
                BSP_OS_TimeDly(OS_TICKS_PER_SEC);           // 延时，等待声音播放完毕
            }
        } break;
        
        /***********************************************
        * 描述： 电源键
        */
        case KEY_PWR:{
            /***********************************************
            * 描述： 置长按标志位
            */
            if ( !LongPressedFlag ) {
                LongPressedFlag++;
                /***********************************************
                * 描述： 当开启儿童锁时，长按为解锁
                */
                if( Ctrl.Wifi.Wr.ChildSecurityLock == 1 ) {
                    Ctrl.Wifi.Wr.ChildSecurityLock = 0;
                    /***********************************************
                    * 描述： 发送按键事件标志位
                    */
                    Beep( BEEP_FLAG_KEY_EVT );
                /***********************************************
                * 描述： 长按
                */
                } else {
                    /***********************************************
                    * 描述： 开机状态，长按关机
                    */
                    if( Ctrl.Sys.SysSts == SYS_PWR_ON) {
                        /***********************************************
                        * 描述： 关机
                        */
                        Power(OFF);
                        BSP_OS_TimeDly(OS_TICKS_PER_SEC);           // 延时，等待声音播放完毕
                    /***********************************************
                    * 描述： 关机状态，长按不处理
                    */
                    } else {
                        /***********************************************
                        * 描述： 
                        */
                        //BeepExt(2, 1500, 100);
                    }
                }
            }
            /***********************************************
            * 描述： 主动上报状态
            */
            OSFlagPost(( OS_FLAG_GRP  *)&Ctrl.Os.WifiEvtFlagGrp,
                       ( OS_FLAGS      )WIFI_EVT_FLAG_REPORT,
                       ( OS_OPT        )OS_OPT_POST_FLAG_SET,
                       ( OS_ERR       *)&err);
        } break;
        }
    };break;
    
    default:break;
    }
}

/*******************************************************************************
 * 				end of file
 *******************************************************************************/
#endif