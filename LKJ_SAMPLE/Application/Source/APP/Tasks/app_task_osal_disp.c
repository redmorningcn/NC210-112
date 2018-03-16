/*******************************************************************************
 *   Filename:       app_task_disp.c
 *   Revised:        All copyrights reserved to wumingshen.
 *   Revision:       v1.0
 *   Writer:	     wumingshen.
 *
 *   Description:    双击选中 disp 按 Ctrl + H, 钩选 Match the case, Replace with
 *                   输入您要的名字，点击 Replace All
 *                   双击选中 Disp 按 Ctrl + H, 钩选 Match the case, Replace with
 *                   输入您要的名字，点击 Replace All
 *                   双击选中 DISP 按 Ctrl + H, 钩选 Match the case, Replace with
 *                   输入您要的名字，点击 Replace All
 *                   在 app_cfg.h 中指定本任务的 优先级  （ APP_TASK_DISP_PRIO ）
 *                                            和 任务堆栈（ APP_TASK_DISP_STK_SIZE ）大小
 *                   在 app.h 中声明本任务的     创建函数（ void  App_TaskDispCreate(void) ）
 *                                            和 看门狗标志位 （ WDTFLAG_Disp ）
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
#include <global.h>
#include <bsp_adc7682.h>
#include <bsp_max7219.h>

#ifdef VSC_INCLUDE_SOURCE_FILE_NAMES
const  CPU_CHAR  *app_task_disp__c = "$Id: $";
#endif

#define APP_TASK_DISP_EN     DEF_ENABLED
#if APP_TASK_DISP_EN == DEF_ENABLED
/*******************************************************************************
 * CONSTANTS
 */
#define CYCLE_TIME_TICKS     (OS_CFG_TICK_RATE_HZ * 2u)

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
static  OS_TCB   AppTaskDispTCB;

/***********************************************
* 描述： 任务堆栈（STACKS）
*/
static  CPU_STK  AppTaskDispStk[ APP_TASK_DISP_STK_SIZE ];

#endif
/*******************************************************************************
 * GLOBAL VARIABLES
 */

/*******************************************************************************
 * LOCAL FUNCTIONS
 */
#if ( OSAL_EN == DEF_ENABLED )
#else
static  void    AppTaskDisp           (void *p_arg);
#endif

/*******************************************************************************
 * GLOBAL FUNCTIONS
 */

/*******************************************************************************
 * EXTERN VARIABLES
 */

/*******************************************************************************/

/*******************************************************************************
 * 名    称： App_TaskDispCreate
 * 功    能： **任务创建
 * 入口参数： 无
 * 出口参数： 无
 * 作　 　者： wumingshen.
 * 创建日期： 2015-02-05
 * 修    改：
 * 修改日期：
 * 备    注： 任务创建函数需要在app.h文件中声明
 *******************************************************************************/
void  App_TaskDispCreate(void)
{

#if ( OSAL_EN == DEF_ENABLED )
#else
    OS_ERR  err;

    /***********************************************
    * 描述： 任务创建
    */
    OSTaskCreate((OS_TCB     *)&AppTaskDispTCB,                 // 任务控制块  （当前文件中定义）
                 (CPU_CHAR   *)"App Task Disp",                 // 任务名称
                 (OS_TASK_PTR ) AppTaskDisp,                    // 任务函数指针（当前文件中定义）
                 (void       *) 0,                              // 任务函数参数
                 (OS_PRIO     ) APP_TASK_DISP_PRIO,             // 任务优先级，不同任务优先级可以相同，0 < 优先级 < OS_CFG_PRIO_MAX - 2（app_cfg.h中定义）
                 (CPU_STK    *)&AppTaskDispStk[0],              // 任务栈顶
                 (CPU_STK_SIZE) APP_TASK_DISP_STK_SIZE / 10,    // 任务栈溢出报警值
                 (CPU_STK_SIZE) APP_TASK_DISP_STK_SIZE,         // 任务栈大小（CPU数据宽度 * 8 * size = 4 * 8 * size(字节)）（app_cfg.h中定义）
                 (OS_MSG_QTY  ) 5u,                             // 可以发送给任务的最大消息队列数量
                 (OS_TICK     ) 0u,                             // 相同优先级任务的轮循时间（ms），0为默认
                 (void       *) 0,                              // 是一个指向它被用作一个TCB扩展用户提供的存储器位置
                 (OS_OPT      )(OS_OPT_TASK_STK_CHK |           // 允许堆栈检查该任务
                                OS_OPT_TASK_STK_CLR),           // 创建任务时堆栈清零
                 (OS_ERR     *)&err);                           // 指向错误代码的指针，用于创建结果处理
#endif
}

/*******************************************************************************
 * 名    称： AppTaskDisp
 * 功    能： 控制任务
 * 入口参数： p_arg - 由任务创建函数传入
 * 出口参数： 无
 * 作　 　者： wumingshen.
 * 创建日期： 2015-02-05
 * 修    改：
 * 修改日期：
 *******************************************************************************/

#if ( OSAL_EN == DEF_ENABLED )
osalEvt  TaskDispEvtProcess(INT8U task_id, osalEvt task_event)
#else
static  void  AppTaskDisp (void *p_arg)
#endif
{
    OS_ERR      err;
    INT32U      ticks;
    INT32S      dly;
    CPU_SR_ALLOC();
    
    /***********************************************
    * 描述：Task body, always written as an infinite loop.
    */
#if ( OSAL_EN == DEF_ENABLED )
#else
    TaskInitDisp();
    
    while (DEF_TRUE) {
#endif
        /***********************************************
        * 描述： 本任务看门狗标志置位
        */
        OS_FlagPost ((OS_FLAG_GRP *)&WdtFlagGRP,
                     (OS_FLAGS     ) WDT_FLAG_DISP,
                     (OS_OPT       ) OS_OPT_POST_FLAG_SET,
                     (CPU_TS       ) 0,
                     (OS_ERR      *) &err);
        /***********************************************
        * 描述： 得到系统当前时间
        */
        ticks = OSTimeGet(&err);
        
#if ( OSAL_EN == DEF_ENABLED )
        if( task_event & OS_EVT_DISP_TICKS ) {
#else
#endif
            static INT8U       step    = 0;
            static INT16S      *pValue;
                    int         DispValue ;
            
            /***********************************************
            * 描述： 清显示
            */
            BSP_DispClrAll();
                
            /***********************************************
            * 描述： 根据显示模式显示
            */
            switch(Ctrl.Disp.Mode) {
            /*******************************************************************
            * 描述： 显示程序类型
            *        时间：开机显示一次，显示2s；
            *        内容：“SF-X”；
            *        释义：X=S：引导程序；X=A：应用程序A；X=B：应用程序B
            */
            case 0: 
                Ctrl.Disp.Mode++;
                BSP_DispClrAll(); 
#if defined     (IMAGE_A)
                /***********************************************
                * 描述： 程序A
                */
                uprintf("SF-A");
#elif defined   (IMAGE_B)
                /***********************************************
                * 描述： 程序B
                */
                uprintf("SF-B");
#else
                /***********************************************
                * 描述： 标准程序
                */
                uprintf("SF-S");
#endif
                break;
            /*******************************************************************
            * 描述： 数码管自检
            *        时间：开机显示一次，显示2s；
            *        内容：“8.8.8.8.”；
            *        释义：8.8.8.8.为全点.
            */
            case 1: 
                Ctrl.Disp.Mode++;                  
                uprintf("ID-%d", Ctrl.Comm.pch->NodeAddr);
                break;
            /*******************************************************************
            * 描述： 显示硬件版本
            *        时间：开机显示一次，显示2s；
            *        内容：内容：“HV21”；
            *        释义：HV为硬件21为版本号。
            */
            case 2: 
                Ctrl.Disp.Mode++;
                uprintf("HV22");
                
                break;
            /*******************************************************************
            * 描述： 显示软件版本
            *        时间：开机显示一次，显示2s。
            *        内容：内容：“SV25” 
            *        释义：SV为软件件25为版本号。
            */
            case 3: 
                Ctrl.Disp.Mode++;
                uprintf("SV22");                
                
                break;
            /*******************************************************************
            * 描述： 显示高度差
            *        时间：开机显示一次，显示2s；
            *        内容：“XX.Xd”；
            *        释义：XX.X为高度值，单位.1mm。
            */
            case 4: 
                Ctrl.Disp.Mode++;
                DispValue   = (INT16U)Ctrl.Sen.Para.dat.ChkedDiffHight;// 800;
                /***********************************************
                * 描述： 
                */
                uprintf("d%d",DispValue);
                //BSP_DispWrite((int)DispValue,"%D",RIGHT,0,(1<<2),1,20,0);
                
                break;
            /*******************************************************************
            * 描述： 故障代码显示
            *        时间：故障时显示，有故障时，显示模块为循环显示时4s刷新一次；
            *        内容：“E-XX”；
            *        释义：XX故障代码。
            */
            case 5:  
                Ctrl.Disp.Mode++;
                DispValue   = Ctrl.Para.dat.SenSts.Dat[1];
                /***********************************************
                * 描述： 
                */
                uprintf("E-%02X",DispValue);                
                break;
            /*******************************************************************
            * 描述： 油量、高度、故障代码显示
            */
            case 6: {
                INT08U  err = 0;
                INT08U  dot = 0;
                /***********************************************
                * 描述： 如果与统计装置连接成功，则显示统计装置发过来的值
                */
                switch ( Ctrl.Para.dat.Sel.udat.DispSel ) {
                    
                /***********************************************
                * 描述： 显示模式：0：连接显示油量，断开显示高度
                */
                case 0:
                    /***********************************************
                    * 描述： 与统计模块连接成功显示油量
                    */
                    if ( Ctrl.Comm.ConnectFlag ) {
                        pValue  = (INT16S *)&Ctrl.Comm.Rd.Data.Oil;
                    /***********************************************
                    * 描述： 如果与统计装置连接失败，则显示当前测量高度
                    */
                    } else {
                        pValue  = (INT16S *)&Ctrl.Sen.C.Hight;
                        dot     = 1;
                    }
                    break;
                /***********************************************
                * 描述： 显示模式：1：显示高度
                */
                case 1:
                    pValue  = (INT16S *)&Ctrl.Sen.C.Hight;
                    dot     = 1;
                    break;
                /***********************************************
                * 描述： 显示模式：2；循环显示
                */
                case 2: 
                    /***********************************************
                    * 描述： 显示油量
                    */
                    if ( step < 2 ) {
                        pValue  = (INT16S *)&Ctrl.Comm.Rd.Data.Oil;
                    /***********************************************
                    * 描述： 显示高度，传感器故障时显示故障代码
                    */
                    } else if ( step < 4 ) {
                        dot     = 1;
                        /***********************************************
                        * 描述： 高位传感器有故障
                        */
                        if (Ctrl.Para.dat.SenSts.Udat.Err.HSenBrokenErr || 
                            Ctrl.Para.dat.SenSts.Udat.Err.HSenShortErr ||
                        /***********************************************
                        * 描述： 低位传感器有故障
                        */
                            Ctrl.Para.dat.SenSts.Udat.Err.LSenBrokenErr || 
                            Ctrl.Para.dat.SenSts.Udat.Err.LSenShortErr ) {
                            err = 1;
                            DispValue   = Ctrl.Para.dat.SenSts.Dat[1];
                        } else {                        
                            pValue  = (INT16S *)&Ctrl.Sen.C.Hight;
                        }
                    }
                    break;
                /***********************************************
                * 描述： 显示模式：3：关闭显示
                */
                case 3:
                    BSP_DispOff();
                    goto exit;
                    break;
                /***********************************************
                * 描述： 显示模式：其他值恢复默认值
                */
                default:
                    Ctrl.Para.dat.Sel.udat.DispSel  = 0;
                }
                
                BSP_DispClrAll();
                /***********************************************
                * 描述： 
                */
                if ( err ) {
                    uprintf("E-%02X",DispValue);
                } else {
                    CPU_CRITICAL_ENTER();
                    DispValue  = *pValue;
                    CPU_CRITICAL_EXIT();
                    
                    if ( DispValue > 9999 ) {
                        DispValue   /= 10;
                    }
                    BSP_DispWrite((int)DispValue,"%",RIGHT,0,(dot<<1),1,20,0);
                }
                
                /***********************************************
                * 描述： 
                */  
                if (++step >= 4)
                    step    = 0;
            } break;
            /*******************************************************************
            * 描述： 调度显示
            */
            case 7:
                /***********************************************
                * 描述： 
                */
                if ( step < 3 ) {
                    pValue  = (INT16S *)&Ctrl.Sen.H.HightSec;
                 } else if ( step < 6 ) {
                    pValue  = (INT16S *)&Ctrl.Sen.L.HightSec;
                } else if ( step < 9 ) {
                    pValue  = (INT16S *)&Ctrl.Sen.C.CalcDiffHightSec;
                }
            
                BSP_DispClrAll();
                    
                DispValue  = *pValue;
                /***********************************************
                * 描述： 
                */
                //BSP_DispWrite((u16)*pValue,"%",RIGHT,0x00,1,20,0); 
                if ( DispValue > 9999 ) {
                    DispValue   /= 10;
                    BSP_DispWrite((int)DispValue,"%",RIGHT,0,0x01,1,20,0);
                } else {
                    if ( ( step < 3 ) ) {
                        DispValue   += Ctrl.Sen.C.ChkedDiffHight;
                        //DispValue   /= 10;
                        BSP_DispWrite((int)DispValue,"%",RIGHT,0,(0<<0),1,20,0);
                    } else if ( step < 6 ) {
                        //DispValue   /= 10;
                        BSP_DispWrite((int)DispValue,"%",RIGHT,0,(1<<0),1,20,0);                
                    } else {
                        BSP_DispWrite((int)DispValue,"%d",RIGHT,0,(1<<2),1,20,0);                 
                    }
                }
                if (++step >= 9)
                    step    = 0;
                break;
            default:
                Ctrl.Disp.Mode  = 0;
                break;
            }
            /*******************************************************************
            * 描述： 更新显示内容
            */
            BSP_DispEvtProcess();
        exit:
            /***********************************************
            * 描述： 去除任务运行的时间，等到一个控制周期里剩余需要延时的时间
            */
            dly   = CYCLE_TIME_TICKS - ( OSTimeGet(&err) - ticks );
            if ( dly  <= 0 ) {
                dly   = 1;
            }
#if ( OSAL_EN == DEF_ENABLED )
            osal_start_timerEx( OS_TASK_ID_DISP,
                                OS_EVT_DISP_TICKS,
                                dly);
            
            return ( task_event ^ OS_EVT_SEN_SAMPLE );
        }
#endif
        
#if ( OSAL_EN == DEF_ENABLED )
        return 0;
#else
        BSP_OS_TimeDly(dly);
    }
#endif
}

/*******************************************************************************
 * 名    称： APP_DispInit
 * 功    能： 任务初始化
 * 入口参数： 无
 * 出口参数： 无
 * 作　 　者： wumingshen.
 * 创建日期： 2015-12-08
 * 修    改：
 * 修改日期：
 *******************************************************************************/
void TaskInitDisp(void)
{
    /***********************************************
    * 描述： 初始化本任务用到的相关硬件
    */
    BSP_DispInit();
    Ctrl.Disp.Led   = (StrLedDisp  *)&LedDispCtrl;
    Ctrl.Disp.Mode  = 0;
    BSP_DispOff();
    BSP_DispSetBrightness(10);
    uprintf("8888");
    BSP_DispClrAll();
    BSP_DispEvtProcess();
    
    /***********************************************
    * 描述： 在看门狗标志组注册本任务的看门狗标志
    */
    WdtFlags |= WDT_FLAG_DISP;
    /*************************************************
    * 描述：启动事件查询
    */
#if ( OSAL_EN == DEF_ENABLED )
#if defined     (IMAGE_A)
    osal_start_timerEx( OS_TASK_ID_DISP,
                        OS_EVT_DISP_TICKS,
                        1);
#elif defined   (IMAGE_B)
    osal_start_timerEx( OS_TASK_ID_DISP,
                        OS_EVT_DISP_TICKS,
                        1);
#else
    osal_start_timerEx( OS_TASK_ID_DISP,
                        OS_EVT_DISP_TICKS,
                        CYCLE_TIME_TICKS);
#endif
#else
#endif
}

/*******************************************************************************
 * 				                    end of file                                *
 *******************************************************************************/
#endif