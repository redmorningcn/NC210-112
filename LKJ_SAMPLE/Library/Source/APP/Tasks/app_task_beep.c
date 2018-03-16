/*******************************************************************************
 *   Filename:       app_task_beep.c
 *   Revised:        All copyrights reserved to Roger-WY.
 *   Revision:       v1.0
 *   Writer:	     Roger-WY.
 *
 *   Description:    双击选中 beep 按 Ctrl + H, 钩选 Match the case, Replace with
 *                   输入您要的名字，点击 Replace All
 *                   双击选中 Beep 按 Ctrl + H, 钩选 Match the case, Replace with
 *                   输入您要的名字，点击 Replace All
 *                   双击选中 BEEP 按 Ctrl + H, 钩选 Match the case, Replace with
 *                   输入您要的名字，点击 Replace All
 *                   在 app_cfg.h 中指定本任务的 优先级  （APP_TASK_BEEP_PRIO）
 *                                            和 任务堆栈（APP_TASK_BEEP_STK_SIZE）大小
 *                   在 app.h 中声明本任务的     创建函数（ void  App_TaskBeepCreate(void) ）
 *                                            和 看门狗标志位 （ WDTFLAG_Beep ）
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
#include <bsp_pwm.h>
#include <bsp_os.h>

#ifdef VSC_INCLUDE_SOURCE_FILE_NAMES
const  CPU_CHAR  *app_task_beep__c = "$Id: $";
#endif

#define APP_TASK_BEEP_EN     DEF_DISABLED
#if APP_TASK_BEEP_EN == DEF_ENABLED
/*******************************************************************************
 * CONSTANTS
 */

/*******************************************************************************
 * MACROS
 */
//该任务的控制周期   (OS_CFG_TICK_RATE_HZ)为1S
#define BEEP_CYCLE_TIME_TICKS       (OS_CFG_TICK_RATE_HZ * 1u)

#define BEEP_FREQ_HIGH              3000.0
#define BEEP_FREQ_MID               2500.0
#define BEEP_FREQ_LOW               2000.0 
     
#define BEEP_FREQ_NORMAL            2500.0
#define BEEP_FREQ_LOCK              2500.0
#define BEEP_FREQ_PWR_ON            2500.0
     
#define BEEP_DUTY_ON_OFF            40.0
#define BEEP_DUTY_LOCK              40.0
#define BEEP_DUTY_NORMAL            40.0
#define BEEP_DUTY_PWR_ON            40.0
     
#define BEEP_DLY_NORMAL             200
#define BEEP_DLY_SHORT              10
#define BEEP_DLY_LOCK               100
#define BEEP_DLY_PWR_ON             10
     
/*******************************************************************************
 * TYPEDEFS
 */
OS_SEM          App_BeepSem;
OS_FLAG_GRP     BeepFlagGrp;                           //蜂鸣器标志组
OS_FLAGS        BeepFlags;

/***********************************************
* 描述： 任务控制块（TCB）
*/
static  OS_TCB   AppTaskBeepTCB;

/***********************************************
* 描述： 任务堆栈（STACKS）
*/
static  CPU_STK  AppTaskBeepStk[ APP_TASK_BEEP_STK_SIZE ];

/*******************************************************************************
 * LOCAL VARIABLES
 */

/*******************************************************************************
 * GLOBAL VARIABLES
 */
/*******************************************************************************
 * LOCAL FUNCTIONS
 */
static  void    AppTaskBeep             (void *p_arg);
static  void    APP_BeepInit            (void);

void            BeepOn                  (void);
void            BeepOff                 (void);
void            BeepLock                (void);
/*******************************************************************************
 * GLOBAL FUNCTIONS
 */

/*******************************************************************************
 * EXTERN VARIABLES
 */

 /*******************************************************************************
 * EXTERN FUNCTIONS
 */

/*******************************************************************************/

/*******************************************************************************
 * 名    称： App_TaskBeepCreate
 * 功    能： **任务创建
 * 入口参数： 无
 * 出口参数： 无
 * 作　　者： Roger-WY.
 * 创建日期： 2015-02-05
 * 修    改：
 * 修改日期：
 * 备    注： 任务创建函数需要在app.h文件中声明
 *******************************************************************************/
void  App_TaskBeepCreate(void)
{
    OS_ERR  err;

    /***********************************************
    * 描述： 任务创建
    */
    OSTaskCreate((OS_TCB     *)&AppTaskBeepTCB,               // 任务控制块  （当前文件中定义）
                 (CPU_CHAR   *)"App Task Beep",               // 任务名称
                 (OS_TASK_PTR ) AppTaskBeep,                  // 任务函数指针（当前文件中定义）
                 (void       *) 0,                              // 任务函数参数
                 (OS_PRIO     ) APP_TASK_BEEP_PRIO,           // 任务优先级，不同任务优先级可以相同，0 < 优先级 < OS_CFG_PRIO_MAX - 2（app_cfg.h中定义）
                 (CPU_STK    *)&AppTaskBeepStk[0],            // 任务栈顶
                 (CPU_STK_SIZE) APP_TASK_BEEP_STK_SIZE / 10,  // 任务栈溢出报警值
                 (CPU_STK_SIZE) APP_TASK_BEEP_STK_SIZE,       // 任务栈大小（CPU数据宽度 * 8 * size = 4 * 8 * size(字节)）（app_cfg.h中定义）
                 (OS_MSG_QTY  ) 5u,                             // 可以发送给任务的最大消息队列数量
                 (OS_TICK     ) 0u,                             // 相同优先级任务的轮循时间（ms），0为默认
                 (void       *) 0,                              // 是一个指向它被用作一个TCB扩展用户提供的存储器位置
                 (OS_OPT      )(OS_OPT_TASK_STK_CHK |           // 允许堆栈检查该任务
                                OS_OPT_TASK_STK_CLR),           // 创建任务时堆栈清零
                 (OS_ERR     *)&err);                           // 指向错误代码的指针，用于创建结果处理
}

/*******************************************************************************
 * 名    称： AppTaskBeep
 * 功    能： 控制任务
 * 入口参数： p_arg - 由任务创建函数传入
 * 出口参数： 无
 * 作　　者： Roger-WY.
 * 创建日期： 2015-02-05
 * 修    改：
 * 修改日期：
 *******************************************************************************/
static  void  AppTaskBeep (void *p_arg)
{
    OS_ERR          err;
    INT32S          dly = 1000;
    INT16S          ctr = 0;
    extern StrPwm   BeepPwm;
    /***********************************************
    * 描述： 任务初始化
    */
    APP_BeepInit();

    /***********************************************
    * 描述：Task body, always written as an infinite loop.
    */
    while (DEF_TRUE) {
        /***********************************************
        * 描述： 本任务看门狗标志置位
        */
        OS_FlagPost ((OS_FLAG_GRP *)&WdtFlagGRP,
                     (OS_FLAGS     ) WDT_FLAG_BEEP,
                     (OS_OPT       ) OS_OPT_POST_FLAG_SET,
                     (CPU_TS       ) 0,
                     (OS_ERR      *) &err);

        /********************************************************************
        * 描述： 独立看门狗标志组检查， 判断是否所有任务已喂狗
        */
        OS_FLAGS    flags =
        OSFlagPend(( OS_FLAG_GRP *)&BeepFlagGrp,
                   ( OS_FLAGS     ) BeepFlags,
                   ( OS_TICK      ) dly,
                   ( OS_OPT       ) OS_OPT_PEND_FLAG_SET_ANY,   //任意
                   ( CPU_TS      *) NULL,
                   ( OS_ERR      *)&err);

        if(err == OS_ERR_NONE) {
            switch ( flags ) {
            /***********************************************
            * 描述： 开机音
            */
            case BEEP_FLAG_PWR_ON:
                ctr = 0;
                dly = 1000;
                BeepOn();
                // 开机启动滤网寿命倒计时定时器
                osal_start_timerEx( OS_TASK_ID_CTRL, 
                                   OS_EVT_CTRL_FILTER, 
                                   (OS_TICKS_PER_SEC * 60 * FILTER_LIFE_SAVE_TIME) );
                // 统计出风量
                osal_start_timerEx( OS_TASK_ID_CTRL, 
                                   OS_EVT_CTRL_AIR_VOLUME, 
                                   (OS_TICKS_PER_SEC * 60) );
                break;
            /***********************************************
            * 描述： 关机音
            */
            case BEEP_FLAG_PWR_OFF:
                ctr = 0;
                dly = 1000;
                BeepOff();
                // 关机停止滤网寿命倒计时定时器
                osal_stop_timerEx( OS_TASK_ID_CTRL, OS_EVT_CTRL_FILTER );
                osal_stop_timerEx( OS_TASK_ID_CTRL, OS_EVT_CTRL_AIR_VOLUME );
                break;
            /***********************************************
            * 描述： 儿童锁音
            */
            case BEEP_FLAG_LOCK_EVT:
                ctr = 0;
                dly = 1000;
                BeepLock();
                break;
            /***********************************************
            * 描述： 按键音
            */
            case BEEP_FLAG_KEY_EVT:
            default:
                PWM_SetProperty(&BeepPwm, PWM_CH_BEEP, BEEP_FREQ_NORMAL, BEEP_DUTY_NORMAL);
                ctr = 100;
                dly = 10;
                break;
            }
            /***********************************************
            * 描述： 清除未知标志位
            */
            OSFlagPost( ( OS_FLAG_GRP  *)&BeepFlagGrp,
                        ( OS_FLAGS      )flags,
                        ( OS_OPT        )OS_OPT_POST_FLAG_CLR,
                        ( OS_ERR       *)&err);
        }

        /***********************************************
        * 描述： 播放按键音
        */
        if ( ctr >= 0 ) {
            PWM_SetDuty(&BeepPwm, PWM_CH_BEEP, ctr / ( 100 / BEEP_DUTY_NORMAL ));
            ctr--;
            dly = 7;
        } else {
            dly = 1000;
        }
    }
}

/*******************************************************************************
 * 名    称： APP_BeepInit
 * 功    能： 任务初始化
 * 入口参数： 无
 * 出口参数： 无
 * 作　　者： Roger-WY.
 * 创建日期： 2015-03-28
 * 修    改：
 * 修改日期：
 *******************************************************************************/
static void APP_BeepInit(void)
{
    OS_ERR err;
    /***********************************************
    * 描述： 初始化本任务用到的相关硬件
    */
    BeepPWM_Init();

    /***********************************************
    * 描述： 上电响一声
    */
    PWM_SetProperty(&BeepPwm, PWM_CH_BEEP, BEEP_FREQ_PWR_ON, BEEP_DUTY_PWR_ON);
    BSP_OS_TimeDly(BEEP_DLY_NORMAL);

    /***********************************************
    * 描述： 余音，慢慢消失
    */
    for ( int i = BEEP_DUTY_PWR_ON; i > 0 ; i-- ) {
        PWM_SetDuty(&BeepPwm, PWM_CH_BEEP, i);
        BSP_OS_TimeDly(BEEP_DLY_PWR_ON);
    }
    PWM_SetProperty(&BeepPwm, PWM_CH_BEEP, BEEP_FREQ_NORMAL, 0.0);

    /***********************************************
    * 描述：创建一个串口到一包数据信号量
    */
    OSSemCreate ((OS_SEM      *)&App_BeepSem,
                 (CPU_CHAR    *)"Beep On",
                 (OS_SEM_CTR   )0,
                 (OS_ERR      *)&err);
    /***********************************************
    * 描述： 创建发送消息类型标志组
    */
    OSFlagCreate(( OS_FLAG_GRP  *)&BeepFlagGrp,
                 ( CPU_CHAR     *)"BeepFlags",
                 ( OS_FLAGS      )0,
                 ( OS_ERR       *)&err);
    
    /***********************************************
    * 描述： 添加标志位
    */
    BeepFlags   = BEEP_FLAG_PWR_ON
                + BEEP_FLAG_PWR_OFF
                + BEEP_FLAG_KEY_EVT
                + BEEP_FLAG_LOCK_EVT;

    /***********************************************
    * 描述： 在看门狗标志组注册本任务的看门狗标志
    */
    WdtFlags |= WDT_FLAG_BEEP;
}

/*******************************************************************************
 * 名    称： BeepOn
 * 功    能：
 * 入口参数： 无
 * 出口参数： 无
 * 作　　者： Roger-WY.
 * 创建日期： 2015-03-28
 * 修    改：
 * 修改日期：
 *******************************************************************************/
void BeepOn(void)
{
    PWM_SetProperty(&BeepPwm, PWM_CH_BEEP, BEEP_FREQ_LOW, BEEP_DUTY_ON_OFF);
    BSP_OS_TimeDly(BEEP_DLY_NORMAL);

    PWM_SetProperty(&BeepPwm, PWM_CH_BEEP, BEEP_FREQ_MID, BEEP_DUTY_ON_OFF);
    BSP_OS_TimeDly(BEEP_DLY_NORMAL);

    PWM_SetProperty(&BeepPwm, PWM_CH_BEEP, BEEP_FREQ_HIGH, BEEP_DUTY_ON_OFF);
    BSP_OS_TimeDly(BEEP_DLY_NORMAL);

    for ( int i = BEEP_DUTY_ON_OFF; i > 5; i-- ) {
        PWM_SetDuty(&BeepPwm, PWM_CH_BEEP, i);
        BSP_OS_TimeDly(BEEP_DLY_SHORT);
    }

    PWM_SetProperty(&BeepPwm, PWM_CH_BEEP, BEEP_FREQ_NORMAL, 0.0);
}

/*******************************************************************************
 * 名    称： BeepOff
 * 功    能：
 * 入口参数： 无
 * 出口参数： 无
 * 作　　者： Roger-WY.
 * 创建日期： 2015-03-28
 * 修    改：
 * 修改日期：
 *******************************************************************************/
void BeepOff(void)
{
    PWM_SetProperty(&BeepPwm, PWM_CH_BEEP, BEEP_FREQ_HIGH, BEEP_DUTY_ON_OFF);
    BSP_OS_TimeDly(BEEP_DLY_NORMAL);

    PWM_SetProperty(&BeepPwm, PWM_CH_BEEP, BEEP_FREQ_MID, BEEP_DUTY_ON_OFF);
    BSP_OS_TimeDly(BEEP_DLY_NORMAL);

    PWM_SetProperty(&BeepPwm, PWM_CH_BEEP, BEEP_FREQ_LOW, BEEP_DUTY_ON_OFF);
    BSP_OS_TimeDly(BEEP_DLY_NORMAL);
    
    for ( int i = BEEP_DUTY_ON_OFF; i > 5; i-- ) {
        PWM_SetDuty(&BeepPwm, PWM_CH_BEEP, i);
        BSP_OS_TimeDly(BEEP_DLY_SHORT);
    }

    PWM_SetProperty(&BeepPwm, PWM_CH_BEEP, BEEP_FREQ_NORMAL, 0.0);
}

/*******************************************************************************
 * 名    称： BeepLock
 * 功    能：
 * 入口参数： 无
 * 出口参数： 无
 * 作　　者： Roger-WY.
 * 创建日期： 2015-03-28
 * 修    改：
 * 修改日期：
 *******************************************************************************/
void BeepLock(void)
{
    PWM_SetProperty(&BeepPwm, PWM_CH_BEEP, BEEP_FREQ_LOCK, BEEP_DUTY_LOCK);
    BSP_OS_TimeDly(BEEP_DLY_LOCK);
    PWM_SetProperty(&BeepPwm, PWM_CH_BEEP, BEEP_FREQ_LOCK, 0.0);
    BSP_OS_TimeDly(BEEP_DLY_LOCK);
    
    PWM_SetProperty(&BeepPwm, PWM_CH_BEEP, BEEP_FREQ_LOCK, BEEP_DUTY_LOCK);
    BSP_OS_TimeDly(BEEP_DLY_LOCK);
    PWM_SetProperty(&BeepPwm, PWM_CH_BEEP, BEEP_FREQ_LOCK, 0.0);
    BSP_OS_TimeDly(BEEP_DLY_LOCK);
    
    PWM_SetProperty(&BeepPwm, PWM_CH_BEEP, BEEP_FREQ_LOCK, BEEP_DUTY_LOCK);
    BSP_OS_TimeDly(BEEP_DLY_LOCK);

    PWM_SetProperty(&BeepPwm, PWM_CH_BEEP, BEEP_FREQ_NORMAL, 0.0);
}

                     
/*******************************************************************************
 * 名    称： BeepLock
 * 功    能：
 * 入口参数： 无
 * 出口参数： 无
 * 作　　者： Roger-WY.
 * 创建日期： 2015-03-28
 * 修    改：
 * 修改日期：
 *******************************************************************************/
void BeepExt(u8 cnt, u16 freq, u16 dly)
{
    for ( int i = cnt; i > 0; i-- ) {
        PWM_SetProperty(&BeepPwm, PWM_CH_BEEP, freq, BEEP_DUTY_LOCK);
        BSP_OS_TimeDly(dly);
        if ( i == 1 )
            break;
        PWM_SetProperty(&BeepPwm, PWM_CH_BEEP, freq, 0.0);
        BSP_OS_TimeDly(dly);
    }

    PWM_SetProperty(&BeepPwm, PWM_CH_BEEP, BEEP_FREQ_NORMAL, 0.0);
}
/***********************************************
* 描述： 
*/
void Beep(INT32U flag)
{   
    OS_ERR  err;
    /***********************************************
    * 描述： 发送按键事件标志位
    */
    OSFlagPost(( OS_FLAG_GRP  *)&BeepFlagGrp,
               ( OS_FLAGS      )flag,
               ( OS_OPT        )OS_OPT_POST_FLAG_SET,
               ( OS_ERR       *)&err);
}
/*******************************************************************************
 * 				end of file
 *******************************************************************************/
#endif