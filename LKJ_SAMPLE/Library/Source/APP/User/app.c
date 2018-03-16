/*******************************************************************************
 *   Filename:      app.c
 *   Revised:       $Date: 2013-06-30$
 *   Revision:      $
 *	 Writer:		Wuming Shen.
 *
 *   Description:
 *   Notes:
 *					QQ:276193028
 *     				E-mail:shenchangwei945@163.com
 *
 *   All copyrights reserved to Wuming Shen.
 *
 *******************************************************************************/

/*******************************************************************************
 * INCLUDES
 */
#define  SNL_APP_SOURCE
#include <app.h>
#include <includes.h>
#include <iap.h>

#ifdef VSC_INCLUDE_SOURCE_FILE_NAMES
const  CPU_CHAR  *app__c = "$Id: $";
#endif

/*******************************************************************************
 * CONSTANTS
 */
/****************************************************
* 描述： 喂狗主任务的执行周期（单位:MS）
*/
#define CYCLE_TIME_TICKS            (OS_CFG_TICK_RATE_HZ)

//#define DEBUG_SIM                   DEF_ENABLED    // 软件仿真开关
/*******************************************************************************
 * MACROS
 */

/*******************************************************************************
 * TYPEDEFS
 */

/*******************************************************************************
 * LOCAL VARIABLES
 */
/***************************************************
* 描述： uC/OS-III 任务控制块（TCB）
*
*       类型        TCB名称
*/
static  OS_TCB      AppTaskStartTCB;

/***************************************************
* 描述： uC/OS 任务控制块（STACKS）
*
*       类型        堆栈名称                堆栈大小
*/
static  CPU_STK     AppTaskStartStk         [ APP_TASK_START_STK_SIZE ];

/***********************************************
* 描述： 创建标志组
*/
OS_FLAG_GRP         WdtFlagGRP;                     //看门狗标志组
OS_FLAGS            WdtFlags;

/***************************************************
* 描述： 软定时器声明
*/
OS_TMR              OSTmr0;                         // 定时器1
/****************************************************
* 描述： 超过WdtTimeoutSec的时间还没有喂狗，系统复位。
*        单位为 秒（S）
*/
#define             WdtTimeoutSec           60

/*******************************************************************************
 * GLOBAL VARIABLES
 */

/*******************************************************************************
 * LOCAL FUNCTIONS
 *
 * 返回类型         函数名称                入口参数
 */
static  void        AppTaskCreate           (void);
static  void        AppObjCreate            (void);
/***********************************************
* 描述： 函数申明
*/
static  void        App_Init                (void);
static  void        AppTaskStart            (void *p_arg);
/***********************************************
* 描述： 软定时器回调函数
*/
void                OSTmr0_callback         (OS_TMR *ptmr,void *p_arg);

/*******************************************************************************
 * GLOBAL FUNCTIONS
 */

/*******************************************************************************
 * EXTERN VARIABLES
 */

/*******************************************************************************
 * EXTERN FUNCTIONS
 */
extern  void        APP_TempEventProcess    (void);
extern  void        App_InitStartHook       (void);
extern  void        App_InitEndHook         (void);
extern  void        AppTaskStart            (void *p_arg);
extern  void        App_Main                (void);

/*******************************************************************************
 * @fn      &:main
 * @brief   &:First function called after startup.
 * @return  &:don't care
 ******************************************************************************/
    int main (void)
{
    OS_ERR  err;

    /***********************************************
    * 描述： 第一个调用函数，可以不返回
    */
    App_Main();
#if DEBUG_SIM != DEF_ENABLED
//#if !defined (LIBRARY)
    /***********************************************
    * 描述： 仅限64K芯片可用，其他容量芯片不让使用
    */
    INT16U FlashSize    = *(INT16U*)(0x1FFFF7E0);  // 闪存容量寄存器   
    if ( FlashSize != STM32_FLASH_SIZE )           // STM32F103RC  256K，在option中定义
        while(1);
//#endif
#endif
    /***********************************************
    * 描述： Init uC/OS-III.
    */
    OSInit(&err);

    /***********************************************
    * 描述： 创建起始任务
    */
    OSTaskCreate((OS_TCB     *)&AppTaskStartTCB,                // 任务控制块（当前文件中定义）
                 (CPU_CHAR   *)"App Task Start",                // 任务名称
                 (OS_TASK_PTR ) AppTaskStart,                   // 任务函数指针（当前文件中定义）
                 (void       *) 0,                              // 任务函数参数
                 (OS_PRIO     ) APP_TASK_START_PRIO,            // 任务优先级，不同任务优先级可以相同，0 < 优先级 < OS_CFG_PRIO_MAX - 2（app_cfg.h中定义）
                 (CPU_STK    *)&AppTaskStartStk[0],             // 任务栈顶
                 (CPU_STK_SIZE) APP_TASK_START_STK_SIZE / 10,   // 任务栈溢出报警值
                 (CPU_STK_SIZE) APP_TASK_START_STK_SIZE,        // 任务栈大小（CPU数据宽度 * 8 * size = 4 * 8 * size(字节)）（app_cfg.h中定义）
                 (OS_MSG_QTY  ) 0u,                             // 可以发送给任务的最大消息队列数量
                 (OS_TICK     ) 0u,                             // 相同优先级任务的轮循时间（ms），0为默认
                 (void       *) 0,                              // 是一个指向它被用作一个TCB扩展用户提供的存储器位置
                 (OS_OPT      )(OS_OPT_TASK_STK_CHK |           // 允许堆栈检查该任务
                                OS_OPT_TASK_STK_CLR),           // 创建任务时堆栈清零
                 (OS_ERR     *)&err);                           // 指向错误代码的指针，用于创建结果处理

    /***********************************************
    * 描述： 启动 uC/OS-III
    */
    OSStart(&err);
}

/*
********************************************************************************
*                                          STARTUP TASK
*
* Description : This is an example of a startup task.  As mentioned in the book's 
*               text, you MUST initialize the ticker only once multitasking has started.
*
* Arguments   : p_arg   is the argument passed to 'AppTaskStart()' by 'OSTaskCreate()'.
*
* Returns     : none
*
* Notes       : 1) The first line of code is used to prevent a compiler warning 
*               because 'p_arg' is not used.  The compiler should not generate 
*               any code for this statement.
********************************************************************************
*/
static  void  AppTaskStart (void *p_arg)
{
    CPU_INT32U  cpu_clk_freq;
    CPU_INT32U  cnts;
    OS_ERR      err;
    INT32U      ticks;
    INT32S      dly;
    INT16U  	TimeOutCnt     = 0;                 //看门狗超时计数器
    uint8_t     ReadRTCTimeCnt = 0;
   (void)p_arg;

    /***********************************************
    * 描述： 设置STM32的系统时钟，I/O口设置，定时器等
    */
#if DEBUG_SIM != DEF_ENABLED
    BSP_Init();                                                 /* Initialize BSP functions                             */
#endif
    CPU_Init();
    
    /***********************************************
    * 描述： 初始化滴答定时器，即初始化系统节拍时钟。
    */
    cpu_clk_freq = BSP_CPU_ClkFreq();                           /* Determine SysTick reference freq.                    */
    cnts = cpu_clk_freq / (CPU_INT32U)OSCfg_TickRate_Hz;        /* Determine nbr SysTick increments                     */
    OS_CPU_SysTickInit(cnts);                                   /* Init uC/OS periodic time src (SysTick).              */
    
    /***********************************************
    * 描述： 设置UCOS钩子函数
    */
    App_OS_SetAllHooks();
    
    /***********************************************
    * 描述： 初始化内存管理模块
    */
#if (LIB_MEM_CFG_ALLOC_EN == DEF_ENABLED)
    Mem_Init();                                                 /* Initialize Memory Management Module                  */
#endif
    
    /***********************************************
    * 描述： 看门狗初始化
    */
#if DEBUG_SIM != DEF_ENABLED
#if defined     (RELEASE)
    BSP_WDT_Init(BSP_WDT_MODE_ALL);
#else
    BSP_WDT_Init(BSP_WDT_MODE_NONE);
#endif  
#endif
    
    /***********************************************
    * 描述： 先关闭所有的LED后，闪烁3次指示灯
    */
    BSP_LED_Flash( 1, 5, 100, 50);
    
    WdtReset();
    /***********************************************
    * 描述： E2PROM初始化
    */
#if DEBUG_SIM != DEF_ENABLED
    //App_Init();
#endif
    
    AppTaskCreate();                                            /* Create Application Tasks                             */

    /***********************************************
    * 描述： 喂狗
    */
    WdtReset();
    
    /***********************************************
    * 描述： 估算在没有运行任务时CPU的能力
    */
#if OS_CFG_STAT_TASK_EN > 0u
    OSStatTaskCPUUsageInit(&err);                               /* Compute CPU capacity with no task running            */
#endif

    /***********************************************
    * 描述：用于测量禁止中断的时间
    */
    CPU_IntDisMeasMaxCurReset();

#if (APP_CFG_SERIAL_EN == DEF_ENABLED)
    BSP_Ser_Init(115200);                                       /* Enable Serial Interface                              */
#endif
    
    /***********************************************
    * 描述： 创建对象（事件）,标志组
    */
    AppObjCreate();                                             /* Create Application Objects                           */

    /***********************************************
    * 描述： 创建创建任务
    */


    Ctrl.Com.SlaveAddr     = 0xA3;
    /***************************************************************************
    * 描述：每一个任务在创建时分配了一个喂狗标志位
    *	    每个任务在执行过和中，在未超出看门狗超时时间内
    *	    将相应的标志位置位，该任务将每秒钟查询一次
    *	    已经注册到喂狗标志位组的标志位，如果当前标志
    *	    位与注册的标志位相等，则表示每个任务当前都处于
    *	    活动状态，此时将超时计数器清零，重新开始超时计数
    *	    如果喂狗标志位没有全部置位，则将超时计数器+1，当
    *	    计数值超出设定的计数值时，系统进行重启。
    */
    while (DEF_TRUE) {
        /***********************************************
        * 描述： 得到系统当前时间
        */
        ticks = OSTimeGet(&err);
        ReadRTCTimeCnt  ++;

        /***********************************************
        * 描述： 喂狗
        */
        WdtReset();
               
        /***********************************************************************
        * 描述： 独立看门狗标志组检查， 判断是否所有任务已喂狗
        */
        OSFlagPend(( OS_FLAG_GRP *)&WdtFlagGRP,
                   ( OS_FLAGS     ) WdtFlags,
                   ( OS_TICK      ) 50,
                   ( OS_OPT       ) OS_OPT_PEND_FLAG_SET_ALL,   //全部置一
                   ( CPU_TS      *) NULL,
                   ( OS_ERR      *)&err);
        
        if(err == OS_ERR_NONE) {                                //所有任务已喂狗
            TimeOutCnt = 0;                                     //超时计数器清零
            BSP_LED_Flash( 1, 1, 40, 40);
            OS_FlagPost ((OS_FLAG_GRP *)&WdtFlagGRP,            //清零所有标志
                         (OS_FLAGS     ) WdtFlags,
                         (OS_OPT       ) OS_OPT_POST_FLAG_CLR,
                         (CPU_TS       ) 0,
                         (OS_ERR      *) &err);
        } else {                                                //不是所有任务都喂狗
            TimeOutCnt++;                                       //超时计数器加1
            if(TimeOutCnt > WdtTimeoutSec) {                    //喂狗超时
                /***********************************************
                * 描述： 如果程序处在升级模式
                */
                if ( ( Iap.Status != IAP_STS_DEF ) && ( Iap.Status != IAP_STS_SUCCEED ) ) {
                } else {
                    SystemReset();							    //系统重启
                    while(1){
                        /*******************************************************
                        * 描述： 长时间等不到某任务的看门狗标志位注册，说明有任务死了，
                        *        指示灯快速闪烁，进入死循环等待系统复位
                        */
                        BSP_LED_Toggle(1);
                        Delay_Nms(100);
                    };
                }
            } else {
                BSP_LED_Flash( 1, 1, 500, 500); 
            }
        }
        /***********************************************
        * 描述： 去除任务运行的时间，等到一个控制周期里剩余需要延时的时间
        */
        dly   = CYCLE_TIME_TICKS - ( OSTimeGet(&err) - ticks );
        if ( dly  <= 0 ) {
            dly   = 0;
        }
        OSTimeDly(dly, OS_OPT_TIME_DLY, &err);
    }
}
/*******************************************************************************
 * 名    称： App_Init
 * 功    能： 用户应用初始化
 * 入口参数： 无
 * 出口参数： 无
 * 作　 　者： 无名沈.
 * 创建日期： 2015-03-19
 * 修    改：
 * 修改日期：
 *******************************************************************************/
static void App_Init(void)
{
    /********************************************************************
    * 描述： 初始化EEPROM
    */
    App_PraInit();
    WdtReset();
    Ctrl.Para.dat.Password  = 0; 
    /********************************************************************
    * 描述： 在发布RELEASE版本时，给FLASH加上读保护
    */
#ifndef DEBUG
    /***********************************************
    * 描述：Flash写保护和读保护，增加保护后，
    *       需使用JFLASH擦出芯片才可以进行下次下载与仿真。
    */
    if( FLASH_GetReadOutProtectionStatus() != SET ){
        FLASH_Unlock();
        FLASH_ReadOutProtection(ENABLE);
        FLASH_Lock();
    }
    /***********************************************
    * 描述：Flash写保护和读保护，增加保护后，
    *       需使用JFLASH擦出芯片才可以进行下次下载与仿真。
    */
#endif
}

/*
********************************************************************************
*                                      CREATE APPLICATION EVENTS
*
* Description:  This function creates the application kernel objects.
*
* Arguments  :  none
*
* Returns    :  none
********************************************************************************
*/

static  void  AppObjCreate (void)
{
    OS_ERR    err;

    /***********************************************
    * 描述：创建看门狗标志组
    */
    OSFlagCreate(( OS_FLAG_GRP  *)&WdtFlagGRP,
                 ( CPU_CHAR     *)"Wdt Flag",
                 ( OS_FLAGS      )0,
                 ( OS_ERR       *)&err);

    /***********************************************
    * 描述：创建一个软定时器 OS_OPT_TMR_ONE_SHOT, OS_OPT_TMR_PERIODIC
    */
    //OSTmrCreate ((OS_TMR               *)&OSTmr0,
    //             (CPU_CHAR             *)"tmr0",
    //             (OS_TICK               )60 * OS_CFG_TMR_TASK_RATE_HZ,
    //             (OS_TICK               )60 * OS_CFG_TMR_TASK_RATE_HZ,
    //             (OS_OPT                )OS_OPT_TMR_ONE_SHOT,
    //             (OS_TMR_CALLBACK_PTR   )OSTmr0_callback,
    //             (void                 *)NULL,
    //             (OS_ERR               *)&err);
}


/*******************************************************************************
 * 名    称： OSTmr0_callback
 * 功    能： 软定时器0的回调函数
 * 入口参数： 无
 * 出口参数： 无
 * 作　  者： 无名沈
 * 创建日期： 2015-03-28
 * 修    改：
 * 修改日期：
 * 备    注： 定时器回调函数不能使用延时函数
 *******************************************************************************/
void OSTmr0_callback(OS_TMR *ptmr,void *p_arg)
{
    /***********************************************
    * 描述：注意：回调函数中执行的时间不宜过长
    */

}

/*
********************************************************************************
*                                      CREATE APPLICATION TASKS
*
* Description:  This function creates the application tasks.
*
* Arguments  :  none
*
* Returns    :  none
********************************************************************************
*/
static  void  AppTaskCreate (void)
{
    /***********************************************
    * 描述： 在此处创建任务
    */
    OS_TaskCreateHook();

#ifndef DISABLE_OSAL_TASK
    /***********************************************
    * 描述： 在此处创建OSAL任务
    */
#if ( OSAL_EN == DEF_ENABLED )
    App_TaskOsalCreate();
    /***********************************************
    * 描述： 在此处创UCOS建任务
    */
#else
    App_TaskSensorCreate();
    //App_TaskControlCreate();
#endif
#endif
}

/*******************************************************************************
 * 				end of file
 *******************************************************************************/
