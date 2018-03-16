/*******************************************************************************
 *   Filename:       app_task_sen.c
 *   Revised:        All copyrights reserved to wumingshen.
 *   Revision:       v1.0
 *   Writer:	     wumingshen.
 *
 *   Description:    双击选中 sen 按 Ctrl + H, 钩选 Match the case, Replace with
 *                   输入您要的名字，点击 Replace All
 *                   双击选中 Sen 按 Ctrl + H, 钩选 Match the case, Replace with
 *                   输入您要的名字，点击 Replace All
 *                   双击选中 SEN 按 Ctrl + H, 钩选 Match the case, Replace with
 *                   输入您要的名字，点击 Replace All
 *                   在 app_cfg.h 中指定本任务的 优先级  （ APP_TASK_SEN_PRIO ）
 *                                            和 任务堆栈（ APP_TASK_SEN_STK_SIZE ）大小
 *                   在 app.h 中声明本任务的     创建函数（ void  App_TaskSenCreate(void) ）
 *                                            和 看门狗标志位 （ WDTFLAG_Sen ）
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

#ifdef VSC_INCLUDE_SOURCE_FILE_NAMES
const  CPU_CHAR  *app_task_sen__c = "$Id: $";
#endif

#define APP_TASK_SEN_EN     DEF_ENABLED
#if APP_TASK_SEN_EN == DEF_ENABLED
/*******************************************************************************
 * CONSTANTS
 */
#define CYCLE_TIME_TICKS            (OS_CFG_TICK_RATE_HZ * 1u)
#define CYCLE_SAMPLE_MSEC_TICKS     (OS_CFG_TICK_RATE_HZ / ADC_SAMPLE_CNTS_PER_SEC/2)
#define CYCLE_SAMPLE_SEC_TICKS      (OS_CFG_TICK_RATE_HZ * 5u)
#define CYCLE_SAMPLE_MIN_TICKS      (OS_CFG_TICK_RATE_HZ * 60u)
#define CYCLE_SAMPLE_TICKS          (OS_CFG_TICK_RATE_HZ * 2u)
#define CYCLE_UPDATE_DENSITY        (OS_CFG_TICK_RATE_HZ * 3u*60u)
#define CYCLE_UPDATE_TICKS          (OS_CFG_TICK_RATE_HZ * 30u)                 // 密度检测周期

#define STOP_UPDATE_TICKS_CNT       (5 * 60 / (CYCLE_UPDATE_TICKS / OS_CFG_TICK_RATE_HZ))   // 停车检测时间间隔

#define CYCLE_UPDATE_TICKS_CNT      (30 * 60 * CYCLE_UPDATE_TICKS / CYCLE_UPDATE_TICKS)     // 静止检测时间间隔

#define CYCLE_SEN_ERR_CHK           (OS_CFG_TICK_RATE_HZ * 2u)                  // 传感器故障检测传感器
     
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
static  OS_TCB   AppTaskSenTCB;

/***********************************************
* 描述： 任务堆栈（STACKS）
*/
static  CPU_STK  AppTaskSenStk[ APP_TASK_SEN_STK_SIZE ];

#endif
/*******************************************************************************
 * GLOBAL VARIABLES
 */

/*******************************************************************************
 * LOCAL FUNCTIONS
 */
#if ( OSAL_EN == DEF_ENABLED )
#else
static  void    AppTaskSen           (void *p_arg);
#endif

float           App_fParaFilter     (float para, float def, float min, float max);
long            App_lParaFilter     (long para, long def, long min, long max);

void            APP_ParaInit        (void);

/*******************************************************************************
 * GLOBAL FUNCTIONS
 */

/*******************************************************************************
 * EXTERN VARIABLES
 */

/*******************************************************************************/

/*******************************************************************************
 * 名    称： App_TaskSenCreate
 * 功    能： **任务创建
 * 入口参数： 无
 * 出口参数： 无
 * 作　 　者： wumingshen.
 * 创建日期： 2015-02-05
 * 修    改：
 * 修改日期：
 * 备    注： 任务创建函数需要在app.h文件中声明
 *******************************************************************************/
void  App_TaskSenCreate(void)
{

#if ( OSAL_EN == DEF_ENABLED )
#else
    OS_ERR  err;

    /***********************************************
    * 描述： 任务创建
    */
    OSTaskCreate((OS_TCB     *)&AppTaskSenTCB,                  // 任务控制块  （当前文件中定义）
                 (CPU_CHAR   *)"App Task Sen",                  // 任务名称
                 (OS_TASK_PTR ) AppTaskSen,                     // 任务函数指针（当前文件中定义）
                 (void       *) 0,                              // 任务函数参数
                 (OS_PRIO     ) APP_TASK_SEN_PRIO,              // 任务优先级，不同任务优先级可以相同，0 < 优先级 < OS_CFG_PRIO_MAX - 2（app_cfg.h中定义）
                 (CPU_STK    *)&AppTaskSenStk[0],               // 任务栈顶
                 (CPU_STK_SIZE) APP_TASK_SEN_STK_SIZE / 10,     // 任务栈溢出报警值
                 (CPU_STK_SIZE) APP_TASK_SEN_STK_SIZE,          // 任务栈大小（CPU数据宽度 * 8 * size = 4 * 8 * size(字节)）（app_cfg.h中定义）
                 (OS_MSG_QTY  ) 5u,                             // 可以发送给任务的最大消息队列数量
                 (OS_TICK     ) 0u,                             // 相同优先级任务的轮循时间（ms），0为默认
                 (void       *) 0,                              // 是一个指向它被用作一个TCB扩展用户提供的存储器位置
                 (OS_OPT      )(OS_OPT_TASK_STK_CHK |           // 允许堆栈检查该任务
                                OS_OPT_TASK_STK_CLR),           // 创建任务时堆栈清零
                 (OS_ERR     *)&err);                           // 指向错误代码的指针，用于创建结果处理
#endif
}

/*******************************************************************************
 * 名    称： AppTaskSen
 * 功    能： 控制任务
 * 入口参数： p_arg - 由任务创建函数传入
 * 出口参数： 无
 * 作　 　者： wumingshen.
 * 创建日期： 2015-02-05
 * 修    改：
 * 修改日期：
 *******************************************************************************/

#if ( OSAL_EN == DEF_ENABLED )
osalEvt  TaskSenEvtProcess(INT8U task_id, osalEvt task_event)
#else
static  void  AppTaskSen (void *p_arg)
#endif
{
    OS_ERR      err;
    static BOOL     chang_flag = FALSE;
    
    /***********************************************
    * 描述： 本任务看门狗标志置位
    */
    OS_FlagPost ((OS_FLAG_GRP *)&WdtFlagGRP,
                 (OS_FLAGS     ) WDT_FLAG_SEN,
                 (OS_OPT       ) OS_OPT_POST_FLAG_SET,
                 (CPU_TS       ) 0,
                 (OS_ERR      *) &err);    
  
    /***************************************************************************
    * 描述： 1秒钟一次
    */
    if( task_event & OS_EVT_SEN_SEC ) {
        
        return ( task_event ^ OS_EVT_SEN_SEC );
    }
    
    /***************************************************************************
    * 描述： 1分钟一次
    */
    if( task_event & OS_EVT_SEN_MIN ) {        
        OS_FlagPost(( OS_FLAG_GRP *)&Ctrl.Os.MaterEvtFlagGrp,
                    ( OS_FLAGS     ) COMM_EVT_FLAG_REPORT,
                    ( OS_OPT       ) OS_OPT_POST_FLAG_SET,
                    ( CPU_TS       ) 0,
                    ( OS_ERR      *) &err);
        return ( task_event ^ OS_EVT_SEN_MIN );
    }
    
    return 0;
}

/*******************************************************************************
 * 名    称： App_fParaFilter
 * 功    能： 浮点参数过滤
 * 入口参数： 无
 * 出口参数： 无
 * 作　　者： wumingshen.
 * 创建日期： 2016-04-20
 * 修    改：
 * 修改日期：
 *******************************************************************************/
float App_fParaFilter(float para, float def, float min, float max)
{
    if ( para < min ) 
        para   = def;
    else if ( para > max )
        para   = def;
    else if ( (INT16U)para == 0XFFFF ) {
        para   = def;
    }
    
    return para;
}

/*******************************************************************************
 * 名    称： App_lParaFilter
 * 功    能： 整型参数过滤
 * 入口参数： 无
 * 出口参数： 无
 * 作　　者： wumingshen.
 * 创建日期： 2016-04-20
 * 修    改：
 * 修改日期：
 *******************************************************************************/
long App_lParaFilter(long para, long def, long min, long max)
{
    if ( para < min ) 
        para   = def;
    else if ( para > max )
        para   = def; 
    
    return para;
}

/*******************************************************************************
 * 名    称： APP_ParaInit
 * 功    能： 应用参数初始化
 * 入口参数： 无
 * 出口参数： 无
 * 作　　者： wumingshen.
 * 创建日期： 2016-06-03
 * 修    改：
 * 修改日期：
 *******************************************************************************/
void APP_ParaInit(void)
{
    
}

/*******************************************************************************
 * 名    称： APP_SenInit
 * 功    能： 任务初始化
 * 入口参数： 无
 * 出口参数： 无
 * 作　　者： wumingshen.
 * 创建日期： 2015-03-28
 * 修    改：
 * 修改日期：
 *******************************************************************************/
void TaskInitSen(void)
{
    /***********************************************
    * 描述： 初始化ADC
    */
    //BSP_ADCInit();
    
       
    /***********************************************
    * 描述： 初始化参数
    */
    //APP_ParaInit();
    
    /***********************************************
    * 描述： 在看门狗标志组注册本任务的看门狗标志
    */
    //WdtFlags |= WDT_FLAG_SEN;
    /*************************************************
    * 描述：启动事件查询
    */
#if ( OSAL_EN == DEF_ENABLED )
    //osal_start_timerRl( OS_TASK_ID_SEN, OS_EVT_SEN_TICKS,           CYCLE_TIME_TICKS);
    //osal_start_timerRl( OS_TASK_ID_SEN, OS_EVT_SEN_MSEC ,           CYCLE_SAMPLE_MSEC_TICKS);
    osal_start_timerRl( OS_TASK_ID_SEN, OS_EVT_SEN_MIN ,           60*OS_TICKS_PER_SEC);
    //osal_start_timerEx( OS_TASK_ID_SEN, OS_EVT_SEN_UPDATE_DENSITY , CYCLE_UPDATE_DENSITY);  
    //osal_start_timerRl( OS_TASK_ID_SEN, OS_EVT_SEN_ERR_CHK ,        CYCLE_SEN_ERR_CHK);  
#else
#endif
}

/*******************************************************************************
 * 				                    end of file                                *
 *******************************************************************************/
#endif