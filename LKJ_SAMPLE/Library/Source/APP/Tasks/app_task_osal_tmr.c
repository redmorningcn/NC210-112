/*******************************************************************************
 *   Filename:       app_task_tmr.c
 *   Revised:        All copyrights reserved to wumingshen.
 *   Revision:       v1.0
 *   Writer:	     wumingshen.
 *
 *   Description:    双击选中 tmr 按 Ctrl + H, 钩选 Match the case, Replace with
 *                   输入您要的名字，点击 Replace All
 *                   双击选中 Tmr 按 Ctrl + H, 钩选 Match the case, Replace with
 *                   输入您要的名字，点击 Replace All
 *                   双击选中 TMR 按 Ctrl + H, 钩选 Match the case, Replace with
 *                   输入您要的名字，点击 Replace All
 *                   在 app_cfg.h 中指定本任务的 优先级  （ APP_TASK_TMR_PRIO ）
 *                                            和 任务堆栈（ APP_TASK_TMR_STK_SIZE ）大小
 *                   在 app.h 中声明本任务的     创建函数（ void  App_TaskTmrCreate(void) ）
 *                                            和 看门狗标志位 （ WDTFLAG_Tmr ）
 *
 *   Notes:
 *     				E-mail: shenchangwei945@163.com
 *
 *******************************************************************************/

/*******************************************************************************
 * INCLUDES
 */
#define  SNL_APP_SOURCE
#include <includes.h>
#include "DS3231.h"
#ifdef PHOTOELECTRIC_VELOCITY_MEASUREMENT
#include <Speed_sensor.h>
#include "app_task_mater.h"
#else
#include <power_macro.h>
#endif

#ifdef VSC_INCLUDE_SOURCE_FILE_NAMES
const  CPU_CHAR  *app_task_tmr__c = "$Id: $";
#endif

#define APP_TASK_TMR_EN     DEF_ENABLED
#if APP_TASK_TMR_EN == DEF_ENABLED
/*******************************************************************************
 * CONSTANTS
 */
#define CYCLE_TIME_TICKS            (OS_CFG_TICK_RATE_HZ * 1u)
#define CYCLE_SAMPLE_MSEC_TICKS     (OS_CFG_TICK_RATE_HZ / 5)
#define CYCLE_SAMPLE_SEC_TICKS      (OS_CFG_TICK_RATE_HZ * 5u)
#define CYCLE_SAMPLE_MIN_TICKS      (OS_CFG_TICK_RATE_HZ * 60u)
#define CYCLE_SAMPLE_TICKS          (OS_CFG_TICK_RATE_HZ * 2u)
     
/*******************************************************************************
 * MACROS
 */

/*******************************************************************************
 * TYPEDEFS
 */

/*******************************************************************************
 * LOCAL VARIABLES
 */

#if ( OSAL_EN == DEF_ENABLED )
#else
/***********************************************
* 描述： 任务控制块（TCB）
*/
static  OS_TCB   AppTaskTmrTCB;

/***********************************************
* 描述： 任务堆栈（STACKS）
*/
static  CPU_STK  AppTaskTmrStk[ APP_TASK_TMR_STK_SIZE ];

#endif
/*******************************************************************************
 * GLOBAL VARIABLES
 */

/*******************************************************************************
 * LOCAL FUNCTIONS
 */

/*******************************************************************************
 * GLOBAL FUNCTIONS
 */

/*******************************************************************************
 * EXTERN VARIABLES
 */

/*******************************************************************************/

/*******************************************************************************
 * 名    称： AppTaskTmr
 * 功    能： 控制任务
 * 入口参数： p_arg - 由任务创建函数传入
 * 出口参数： 无
 * 作　 　者： wumingshen.
 * 创建日期： 2015-02-05
 * 修    改：
 * 修改日期：
 *******************************************************************************/

osalEvt  TaskTmrEvtProcess(INT8U task_id, osalEvt task_event)
{
    OS_ERR      err;
    static BOOL     chang_flag = FALSE;
    
    /***********************************************
    * 描述： 本任务看门狗标志置位
    */
    OS_FlagPost ((OS_FLAG_GRP *)&WdtFlagGRP,
                 (OS_FLAGS     ) WDT_FLAG_TMR,
                 (OS_OPT       ) OS_OPT_POST_FLAG_SET,
                 (CPU_TS       ) 0,
                 (OS_ERR      *) &err);    
  
    /***************************************************************************
    * 描述： 1秒钟一次
    */
    if( task_event & OS_EVT_TMR_TICKS ) {     
        /***************************************************
        * 描述： 读取时间
        */
        uint8_t time[6];   
        GetTime((TIME *)&time[0]);
        
        CPU_SR  cpu_sr;
        OS_CRITICAL_ENTER();
        Mater.Time.Year  = time[0];
        Mater.Time.Mon   = time[1];
        Mater.Time.Day   = time[2];
        Mater.Time.Hour  = time[3];
        Mater.Time.Min   = time[4];
        Mater.Time.Sec   = time[5];
        OS_CRITICAL_EXIT();
        /***************************************************
        * 描述： 置位电量保存数据标志位，启动电量数据保存
        */
        OS_FlagPost(( OS_FLAG_GRP *)&Ctrl.Os.MaterEvtFlagGrp,
                    ( OS_FLAGS     ) COMM_EVT_FLAG_HEART,
                    ( OS_OPT       ) OS_OPT_POST_FLAG_SET,
                    ( CPU_TS       ) 0,
                    ( OS_ERR      *) &err);
        return ( task_event ^ OS_EVT_TMR_TICKS );
    }
    
    /***************************************************************************
    * 描述： 1分钟一次
    */
    if( task_event & OS_EVT_TMR_MIN ) { 
        /***************************************************
        * 描述： 置位保存数据标志位，启动数据保存
        */
        OS_FlagPost(( OS_FLAG_GRP *)&Ctrl.Os.DtuEvtFlagGrp,
                    ( OS_FLAGS     ) COMM_EVT_FLAG_REPORT,
                    ( OS_OPT       ) OS_OPT_POST_FLAG_SET,
                    ( CPU_TS       ) 0,
                    ( OS_ERR      *) &err);
        
        return ( task_event ^ OS_EVT_TMR_MIN );
    }
    
    /***************************************************************************
    * 描述： 1分钟一次
    */
    if( task_event & OS_EVT_TMR_DEAL ) {        
#if 0
        Ctrl.PFreq          = (AC.PPpulse  - Ctrl.PPpulseLast)/60.0;
        Ctrl.PPpulseLast    = AC.PPpulse;
#endif        
        osal_start_timerRl( OS_TASK_ID_TMR, OS_EVT_TMR_MIN ,(osalTime)Mater.RecordTime);
                
        OS_FlagPost(( OS_FLAG_GRP *)&Ctrl.Os.MaterEvtFlagGrp,
                    ( OS_FLAGS     ) COMM_EVT_FLAG_CONFIG,
                    ( OS_OPT       ) OS_OPT_POST_FLAG_SET,
                    ( CPU_TS       ) 0,
                    ( OS_ERR      *) &err);
        return ( task_event ^ OS_EVT_TMR_DEAL );
    }
    return 0;
}

/*******************************************************************************
 * 名    称： APP_TmrInit
 * 功    能： 任务初始化
 * 入口参数： 无
 * 出口参数： 无
 * 作　　者： wumingshen.
 * 创建日期： 2015-03-28
 * 修    改：
 * 修改日期：
 *******************************************************************************/
void TaskInitTmr(void)
{
    /***********************************************
    * 描述： 初始化
    */    
       
    /***********************************************
    * 描述： 在看门狗标志组注册本任务的看门狗标志
    */
    WdtFlags |= WDT_FLAG_TMR;
    /*************************************************
    * 描述：启动事件查询
    */
    osal_start_timerRl( OS_TASK_ID_TMR, OS_EVT_TMR_TICKS, CYCLE_TIME_TICKS);
    osal_start_timerRl( OS_TASK_ID_TMR, OS_EVT_TMR_MIN ,  OS_TICKS_PER_SEC * 10);
    osal_start_timerRl( OS_TASK_ID_TMR, OS_EVT_TMR_DEAL , OS_TICKS_PER_SEC * 60);
}

/*******************************************************************************
 * 				                    end of file                                *
 *******************************************************************************/
#endif
