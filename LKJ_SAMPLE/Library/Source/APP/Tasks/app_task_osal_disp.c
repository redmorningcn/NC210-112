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
#include <iap.h>
#include "DS3231.h"
#ifdef PHOTOELECTRIC_VELOCITY_MEASUREMENT
#include <Speed_sensor.h>
#include "app_task_mater.h"
#else
#include <power_macro.h>
#endif

#include "app_voltage_detect.h"
#include "dtc.h"
#include "display.h"

#ifdef VSC_INCLUDE_SOURCE_FILE_NAMES
const  CPU_CHAR  *app_task_disp__c = "$Id: $";
#endif

#define APP_TASK_DISP_EN     DEF_ENABLED
#if APP_TASK_DISP_EN == DEF_ENABLED
/*******************************************************************************
 * CONSTANTS
 */
#define CYCLE_TIME_TICKS    (OS_CFG_TICK_RATE_HZ * 1u)

/*******************************************************************************
 * MACROS
 */

#define DISP_STARTUP			0
#define DISP_ID					1
#define DISP_HW_VERSION		2
#define DISP_SW_VERSION		3
#define DISP_PRIMARY_VOL		4
#define DISP_PRIMARY_CURRENT	5
#define DISP_DATE				6
#define DISP_TIME				7
#define DISP_ALL_CLEAR			8
#define DISP_NORMAL			9
#define DISP_LOCOTYPE			10
#define DISP_LOCONBR			11

/*******************************************************************************
 * TYPEDEFS
 */

typedef struct st_display_sequence
{
struct st_display_sequence *pNext;
const uint8 display_index;
}ST_DISPLAY_SEQUENCE;



/*******************************************************************************
 * LOCAL VARIABLES
 */
static INT32U  DispCycle           = CYCLE_TIME_TICKS;

static uint8 phase_display_index = 0;
static uint8 duty_ratio_display_index =0;
static uint8 dtc_display_index = 0;

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

static ST_DISPLAY_SEQUENCE display_sequence_arry[DISPLAY_STEP_MAX] = 
{
{NULL,	DISPLAY_TIME},
{NULL,	DISPLAY_BAT_VOL},
{NULL,	DISPLAY_SPEED},
{NULL,	DISPLAY_PHASE_DIFF},
{NULL,	DISPLAY_DUTY_RATIO},
{NULL,	DISPLAY_DTC},
{NULL,	DISPLAY_HISTORY_NUMBER}
};

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
extern ST_para_storage system_para;
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
extern void APP_MaterDispHandler(INT08U *step);


const uint8 * dtc_display_str_table[MAX_DTC_NUM]=
{
"E-81    ",	//COLLECTOR_I_CONNECT_FAIL ,
"E-82    ",	//COLLECTOR_II_CONNECT_FAIL,
"E-83    ",	//COLLECTOR_III_CONNECT_FAIL,
"E-91    ",	//COLLECTOR_I_PHASE_ABNORMAL,
"E-92    ",	//COLLECTOR_II_PHASE_ABNORMAL,
"E-93    ",	//COLLECTOR_III_PHASE_ABNORMAL,
"E-71    ",	//SENSOR_I_IS_ABNORMAL,
"E-72    ",	//SENSOR_II_IS_ABNORMAL,
"E-73    ",	//SENSOR_III_IS_ABNORMAL,
"E-74    ",	//SENSOR_IV_IS_ABNORMAL,
"E-75    ",	//SENSOR_V_IS_ABNORMAL,
"E-76    ",	//SENSOR_VI_IS_ABNORMAL,
"E-31    ",	//BATTERY_VOLTAGE_LOW_FAULT,
"E-36    ",	//OUTPUT_STORAGE_ERROR,
"E-37    ",	//PARAMETER_STORAGE_ERROR,
};


/*******************************************************************************
 * 名    称： app_speed_display_handler
 * 功    能： 刷新显示
 * 入口参数： 无
 * 出口参数： 无
 * 作    者： fourth.peng
 * 创建日期： 2017-05-2
 * 修    改：
 * 修改日期：
 *******************************************************************************/
void app_dtc_display_handler(uint8 *step)
{
	uint8 i;

	if (MAX_DTC_NUM != dtc.fault_list[dtc_display_index])
	{
		uprintf("%s",dtc_display_str_table[dtc.fault_list[dtc_display_index]]);
	}
}

/*******************************************************************************
 * 名    称： app_display_step_sequence
 * 功    能： 刷新当前需要显示的序列
 * 入口参数： 无
 * 出口参数： 无
 * 作    者： fourth.peng
 * 创建日期： 2017-05-3
 * 修    改：
 * 修改日期：
 *******************************************************************************/
 void app_display_step_sequence()
{
	uint8 i;
	ST_DISPLAY_SEQUENCE *p_primary_step = NULL;
	ST_DISPLAY_SEQUENCE *p_curr_step = &display_sequence_arry[0];
	DISPLAY_SETTING *p_display = system_para.display_setting;
	for ( i = 0; i < DISPLAY_STEP_MAX; i++)
	{
		if(DEF_ON == p_display[i].enable)
		{
			if (NULL == p_primary_step)
			{
				p_primary_step = &display_sequence_arry[i];
			}
			p_curr_step->pNext = &display_sequence_arry[i];
			p_curr_step = p_curr_step->pNext;
		}
	}
	p_curr_step->pNext = p_primary_step;
}
/*******************************************************************************
 * 名    称： app_speed_display_handler
 * 功    能： 刷新显示
 * 入口参数： 无
 * 出口参数： 无
 * 作    者： fourth.peng
 * 创建日期： 2017-05-3
 * 修    改：
 * 修改日期：
 *******************************************************************************/
 uint8 app_display_step_handler(uint8 step)
{
	uint8 next_step = step;
	DISPLAY_SETTING *p_next_display_setting = NULL;
	uint8 i = 0;

	next_step = display_sequence_arry[step].pNext->display_index;
	
	switch(step)
	{
		case DISPLAY_PHASE_DIFF:
			phase_display_index++;
			
/*			if (TRUE == Mater.monitor.abnormal_info[phase_display_index].connected_fail)
			{
				phase_display_index++;
			}
*/			
			if ( 3 <= phase_display_index )
			{
				phase_display_index = 0;
			}
			else
			{
				next_step = step;
			}
			break;
			
		case DISPLAY_DUTY_RATIO:
			duty_ratio_display_index++;

//		if (TRUE == Mater.monitor.abnormal_info[duty_ratio_display_index>>1].connected_fail)

			if ( 6 <= duty_ratio_display_index )
			{
				duty_ratio_display_index = 0;
			}
			else
			{
				next_step = step;
			}
			break;
			
		case DISPLAY_DTC:
			dtc_display_index++;
			dtc_display_index %= MAX_DTC_NUM;
			if (MAX_DTC_NUM == dtc.fault_list[dtc_display_index])
			{
				dtc_display_index = 0;	
			}
			else
			{
				next_step = step;
			}
			break;
			
		default:
		case DISPLAY_TIME:
		case DISPLAY_BAT_VOL:
		case DISPLAY_SPEED:
			break;
	}
	return next_step;
}


/*******************************************************************************
 * 名    称： app_speed_display_handler
 * 功    能： 刷新显示
 * 入口参数： 无
 * 出口参数： 无
 * 作    者： fourth.peng
 * 创建日期： 2017-05-2
 * 修    改：
 * 修改日期：
 *******************************************************************************/

void app_display_handler()
{
        static uint8_t dispStep = DISPLAY_TIME; 
#ifndef SAMPLE_BOARD
	BSP_DispClrAll();
	switch(dispStep)
	{
		case DISPLAY_TIME:
		{
			uprintf("%02d-%02d-%02d",Mater.Time.Hour, Mater.Time.Min, Mater.Time.Sec);
			break;
		}

		case DISPLAY_BAT_VOL:
		{
			uprintf("V  %5d",Mater.monitor.voltage[ADC_CHANNEL_POWER]);
			break;
		}
		
		case DISPLAY_SPEED:
		{
			uprintf("S %5d.%1d", (uint32)Mater.monitor.real_speed, (uint32)(Mater.monitor.real_speed*10)%10);
			break;
		}

		case DISPLAY_PHASE_DIFF:
		{
			uprintf("p%d %4d", (phase_display_index+1), Mater.monitor.collector_board[phase_display_index].phase_diff);
			break;
		}

		case DISPLAY_DUTY_RATIO:
		{
			uprintf("ch%d %4d", (duty_ratio_display_index+1), Mater.monitor.collector_board[duty_ratio_display_index/2].channel[duty_ratio_display_index%2].duty_ratio);
			break;
		}

		case DISPLAY_DTC:
		{
			app_dtc_display_handler(&dispStep);
			break;
		}

		case DISPLAY_HISTORY_NUMBER:
		{
			uprintf("H %6d",Mater.RecordNbr);
			break;
		}
		default:
		{
			dispStep = DISPLAY_TIME;
			break;
		}
	}
	DispCycle = system_para.display_setting[dispStep].times;
	dispStep = app_display_step_handler(dispStep);
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
    //CPU_SR_ALLOC();
    
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
           // static INT16S      *pValue;
           //         int         DispValue ;
            
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
            case DISP_STARTUP: 
                Ctrl.Disp.Mode++;
                BSP_DispClrAll(); 
#if defined     (IMAGE_A)
                /***********************************************
                * 描述： 程序A
                */
#if defined ( DISPLAY )
                /***********************************************
                * 描述： 如果是显示模块
                */
                uprintf("    SD-A");
#endif
                uprintf("    SF-A");
#elif defined   (IMAGE_B)
                /***********************************************
                * 描述： 程序B
                */
#if defined ( DISPLAY )
                /***********************************************
                * 描述： 如果是显示模块
                */
                uprintf("    SD-B");
#endif
                uprintf("    SF-B");
#else
                /***********************************************
                * 描述： 标准程序
                */
#if defined ( DISPLAY )
                /***********************************************
                * 描述： 如果是显示模块
                */
                uprintf("    SD-S");
#endif
                uprintf("    SF-S");
#endif
                break;
            /*******************************************************************
            * 描述： 机车型号
            */
	    case DISP_LOCOTYPE:
			Ctrl.Disp.Mode = DISP_LOCONBR;
	                uprintf("%8d", Mater.LocoTyp);
			break;
            /*******************************************************************
            * 描述： 机车号
            */
	    case DISP_LOCONBR:
			Ctrl.Disp.Mode = DISP_ID;
	                uprintf("%8d", Mater.LocoNbr);
			break;

            /*******************************************************************
            * 描述： 显示设备ID
            */
            case DISP_ID: 
                Ctrl.Disp.Mode = DISP_DATE;
                uprintf("%8d", Ctrl.ID);
                break;
#if 0
            /*******************************************************************
            * 描述： 显示硬件版本
            *        时间：开机显示一次，显示2s；
            *        内容：内容：“HV21”；
            *        释义：HV为硬件21为版本号。
            */
            case DISP_HW_VERSION: 
                Ctrl.Disp.Mode++;
                uprintf("    HV12");
                
                break;
            /*******************************************************************
            * 描述： 显示软件版本
            *        时间：开机显示一次，显示2s。
            *        内容：内容：“SV25” 
            *        释义：SV为软件件25为版本号。
            */
            case DISP_SW_VERSION: 
//                Ctrl.Disp.Mode++;
		  Ctrl.Disp.Mode = DISP_ALL_CLEAR;
                uprintf("    SV20");
                
                break;
            /*******************************************************************
            * 描述： 
            */
            case DISP_PRIMARY_VOL: 
                Ctrl.Disp.Mode++;
//                uprintf("%7dU", AC.U_SCL);
                
                break;
            /*******************************************************************
            * 描述： 
            */
            case DISP_PRIMARY_CURRENT: 
                Ctrl.Disp.Mode++;
//                uprintf("%7dA", AC.I_SCL);
                
                break;
#endif
            /*******************************************************************
            * 描述： 显示高度差
            *        时间：开机显示一次，显示2s；
            *        内容：“XX.Xd”；
            *        释义：XX.X为高度值，单位.1mm。
            */
            case DISP_DATE: 
                Ctrl.Disp.Mode = DISP_ALL_CLEAR;
                osal_start_timerEx( OS_TASK_ID_DISP,
                                    OS_EVT_DISP_TICKS,
                                    CYCLE_TIME_TICKS*2);
                /***********************************************
                * 描述： 
                */
                uprintf("%02d-%02d-%02d",Mater.Time.Year, Mater.Time.Mon, Mater.Time.Day);
                //BSP_DispWrite((int)DispValue,"%D",RIGHT,0,(1<<2),1,20,0);
                
                break;
            /*******************************************************************
            * 描述： 故障代码显示
            *        时间：故障时显示，有故障时，显示模块为循环显示时4s刷新一次；
            *        内容：“E-XX”；
            *        释义：XX故障代码。
            */
            case DISP_TIME:  
                //DispCycle   = CYCLE_TIME_TICKS * 2;
                Ctrl.Disp.Mode++;
                /***********************************************
                * 描述： 此定时器应该无效
                */
                osal_start_timerEx( OS_TASK_ID_DISP,
                                    OS_EVT_DISP_TICKS,
                                    CYCLE_TIME_TICKS*2);
                //uprintf("    E-%02X",DispValue);
                uprintf("%02d-%02d-%02d",Mater.Time.Hour, Mater.Time.Min, Mater.Time.Sec);
                break;
            case DISP_ALL_CLEAR:  
                Ctrl.Disp.Mode++;
                /***********************************************
                * 描述： 
                */
                osal_start_timerEx( OS_TASK_ID_DISP,
                                    OS_EVT_DISP_TICKS,
                                    CYCLE_TIME_TICKS);
                uprintf("        ");
                break;
            /*******************************************************************
            * 描述： 油量、高度、故障代码显示
            */
            case DISP_NORMAL: {
                //INT08U  err = 0;
                //INT08U  dot = 0;
#if 1
		app_display_handler();
//		DispCycle   = CYCLE_TIME_TICKS * 2;
#else
                if ( ( dispStep < 8 ) && ( dispStep % 2 == 0 ) ) {
                    DispCycle   = CYCLE_TIME_TICKS;
                } else {
                    DispCycle   = CYCLE_TIME_TICKS * 2;
                }
                /***********************************************
                * 描述： 
                */
                APP_MaterDispHandler(&dispStep);
                //BSP_DispClrAll();
                /***********************************************
                * 描述： 
                *
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
                
                *//***********************************************
                * 描述： 
                */  
                if (++step >= 4)
                    step    = 0;
#endif
            } break;
            default:
                Ctrl.Disp.Mode  = 0;
                break;
            }
            
            /***********************************************
            * 描述： 显示程序下载进度
            */
            if ( Iap.Status == IAP_STS_PROGRAMING ) { 
                uprintf("    u%03d",Iap.FrameIdx);
                //BSP_DispSetBrightness(Ctrl.Para.dat.DispLevel);
                BSP_DispEvtProcess();	// 是否多余
            } else {
            
            }
            
            /*******************************************************************
            * 描述： 更新显示内容
            */
            BSP_DispSetBrightness(Ctrl.Para.dat.DispLevel);
            //BSP_DispEvtProcess();
        //exit:
            /***********************************************
            * 描述： 去除任务运行的时间，等到一个控制周期里剩余需要延时的时间
            */            
            if ( Iap.Status == IAP_STS_PROGRAMING ) { 
                dly   = OS_CFG_TICK_RATE_HZ / 5 - ( OSTimeGet(&err) - ticks );
            } else {
                dly   = DispCycle - ( OSTimeGet(&err) - ticks );            
            }
                
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
 * 作  　者： wumingshen.
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
    Ctrl.Disp.Mode  = DISP_LOCOTYPE;
    BSP_DispOff();
    BSP_DispSetBrightness(Ctrl.Para.dat.DispLevel);
    BSP_DispClrAll();
    uprintf("        ");
    
    //for(char i = 0; i < 10; i++) {
    //    uprintf("%08d",i*11111111);
    //    //BSP_DispEvtProcess();
    //    BSP_OS_TimeDly(OS_TICKS_PER_SEC/4);
    //}
    
    uprintf("8.8.8.8.8.8.8.8.");
    BSP_OS_TimeDly(OS_TICKS_PER_SEC);
    uprintf("        ");
    BSP_OS_TimeDly(OS_TICKS_PER_SEC/2);
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
