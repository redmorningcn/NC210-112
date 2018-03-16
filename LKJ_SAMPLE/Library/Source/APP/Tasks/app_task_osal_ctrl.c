/*******************************************************************************
 *   Filename:       app_task_control.c
 *   Revised:        All copyrights reserved to Roger-WY.
 *   Revision:       v1.0
 *   Writer:	     Roger-WY.
 *
 *   Description:    双击选中 control 按 Ctrl + H, 钩选 Match the case, Replace with
 *                   输入您要的名字，点击 Replace All
 *                   双击选中 Control 按 Ctrl + H, 钩选 Match the case, Replace with
 *                   输入您要的名字，点击 Replace All
 *                   双击选中 CONTROL 按 Ctrl + H, 钩选 Match the case, Replace with
 *                   输入您要的名字，点击 Replace All
 *                   在 app_cfg.h 中指定本任务的 优先级  （ APP_TASK_CONTROL_PRIO ）
 *                                            和 任务堆栈（ APP_TASK_CONTROL_STK_SIZE ）大小
 *                   在 app.h 中声明本任务的     创建函数（ void  App_TaskControlCreate(void) ）
 *                                            和 看门狗标志位 （ WDTFLAG_Control ）
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
#include "bsp_control.h"
#include <app.h>
#include <Protocol.h>
#include <bsp_adc.h>
#include <math.h>
     
#ifdef VSC_INCLUDE_SOURCE_FILE_NAMES
const  CPU_CHAR  *app_task_ctrl__c = "$Id: $";
#endif

#define APP_TASK_CONTROL_EN     DEF_DISABLED
#if APP_TASK_CONTROL_EN == DEF_ENABLED
/*******************************************************************************
 * CONSTANTS
 */

/*******************************************************************************
 * MACROS
 */
#define CONTROL_CYCLE_TIME_TICKS    (OS_CFG_TICK_RATE_HZ / 50)
//static INT32U CYCLE_TIME_TICKS      = (OS_CFG_TICK_RATE_HZ / 50);

#define BREATHE_CYCEL               4000.0                                      // 呼吸周期(3-5秒)
#define BREATHE_TICK                10.0                                        // 
#define BREATHE_DUTY_MIN            5.0
#define BREATHE_TICK_NBR           (BREATHE_CYCEL / BREATHE_TICK)               // 
#define BREATHE_STEP               (2.0 * ( 100.0 - BREATHE_DUTY_MIN) / BREATHE_TICK_NBR)
/*******************************************************************************
 * TYPEDEFS
 */
/***********************************************
* 描述： 任务控制块（TCB）
*/
#if ( OSAL_EN == DEF_ENABLED )
#else
static  OS_TCB   AppTaskControlTCB;
#endif

/***********************************************
* 描述： 任务堆栈（STACKS）
*/
#if ( OSAL_EN == DEF_ENABLED )
#else
static  CPU_STK  AppTaskControlStk[ APP_TASK_CONTROL_STK_SIZE ];
#endif

/*******************************************************************************
 * LOCAL VARIABLES
 */

/*******************************************************************************
 * GLOBAL VARIABLES
 */
/*******************************************************************************
 * LOCAL FUNCTIONS
 */
#if ( OSAL_EN == DEF_ENABLED )
#else
static  void    AppTaskControl          (void *p_arg);
#endif
void            CalcFilterLife          (u8 level, u8 min);
void            CalcAirVolume           (u8 level, u8 min);
static  void    App_PwrOff              (StrCtrlDCB *dcb);
static  void    App_PwrOn               (StrCtrlDCB *dcb);
static  void    App_WifiConfig          (StrCtrlDCB *dcb);

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
 * 名    称： App_TaskControlCreate
 * 功    能： **任务创建
 * 入口参数： 无
 * 出口参数： 无
 * 作　　者： Roger-WY.
 * 创建日期： 2015-02-05
 * 修    改：
 * 修改日期：
 * 备    注： 任务创建函数需要在app.h文件中声明
 *******************************************************************************/
void  App_TaskControlCreate(void)
{
#if ( OSAL_EN == DEF_ENABLED )
#else
    OS_ERR  err;

    /***********************************************
    * 描述： 任务创建
    */
    OSTaskCreate((OS_TCB     *)&AppTaskControlTCB,              // 任务控制块  （当前文件中定义）
                 (CPU_CHAR   *)"App Task Control",              // 任务名称
                 (OS_TASK_PTR ) AppTaskControl,                 // 任务函数指针（当前文件中定义）
                 (void       *) 0,                              // 任务函数参数
                 (OS_PRIO     ) APP_TASK_CONTROL_PRIO,          // 任务优先级，不同任务优先级可以相同，0 < 优先级 < OS_CFG_PRIO_MAX - 2（app_cfg.h中定义）
                 (CPU_STK    *)&AppTaskControlStk[0],           // 任务栈顶
                 (CPU_STK_SIZE) APP_TASK_CONTROL_STK_SIZE / 10, // 任务栈溢出报警值
                 (CPU_STK_SIZE) APP_TASK_CONTROL_STK_SIZE,      // 任务栈大小（CPU数据宽度 * 8 * size = 4 * 8 * size(字节)）（app_cfg.h中定义）
                 (OS_MSG_QTY  ) 5u,                             // 可以发送给任务的最大消息队列数量
                 (OS_TICK     ) 0u,                             // 相同优先级任务的轮循时间（ms），0为默认
                 (void       *) 0,                              // 是一个指向它被用作一个TCB扩展用户提供的存储器位置
                 (OS_OPT      )(OS_OPT_TASK_STK_CHK |           // 允许堆栈检查该任务
                                OS_OPT_TASK_STK_CLR),           // 创建任务时堆栈清零
                 (OS_ERR     *)&err);                           // 指向错误代码的指针，用于创建结果处理
#endif
}

/*******************************************************************************
 * 名    称： AppTaskControl
 * 功    能： 控制任务
 * 入口参数： p_arg - 由任务创建函数传入
 * 出口参数： 无
 * 作　　者： Roger-WY.
 * 创建日期： 2015-02-05
 * 修    改：
 * 修改日期：
 *******************************************************************************/
#if ( OSAL_EN == DEF_ENABLED )
osalEvt  TaskCtrlEvtProcess(INT8U task_id, osalEvt task_event)
#else
static  void  AppTaskControl (void *p_arg)
#endif
{
    OS_ERR      err;
    /***********************************************
    * 描述：Task body, always written as an infinite loop.
    */
#if ( OSAL_EN == DEF_ENABLED )
#else
    TaskInitCtrl();
    
    while (DEF_TRUE) {
#endif
    /***********************************************
    * 描述： 本任务看门狗标志置位
    */
    OS_FlagPost ((OS_FLAG_GRP *)&WdtFlagGRP,
                 (OS_FLAGS     ) WDT_FLAG_CTRL,
                 (OS_OPT       ) OS_OPT_POST_FLAG_SET,
                 (CPU_TS       ) 0,
                 (OS_ERR      *) &err);
    
    /*************************************************
    * 描述： 控制LED，FAN输出
    */
    if( task_event & (OS_EVT_CTRL_TICK) ) {
        /*******************************************************************
        * 描述： 选择输出类型
        */
        switch(Ctrl.Sys.SysSts) {
            /***********************************************
            * 描述： 关机状态
            */
        case SYS_PWR_OFF:
            App_PwrOff(&Ctrl);
            break;
            /***********************************************
            * 描述： 开机状态
            */
        case SYS_PWR_ON:
            App_PwrOn(&Ctrl);
            break;
            /***********************************************
            * 描述： 
            */
        default:
            Ctrl.Sys.SysSts = SYS_PWR_OFF;
            break;
        }
        
        /***********************************************
        * 描述： WIFI配置
        */
        App_WifiConfig(&Ctrl);
        
        return ( task_event ^ (OS_EVT_CTRL_TICK) );
    }
    
    /***********************************************
    * 描述： 风机调速
    */
    if( task_event & (OS_EVT_CTRL_FAN) ) {             
        switch(Ctrl.Sys.SysSts) {
            /***********************************************
            * 描述： 关机状态
            */
        case SYS_PWR_OFF:
            App_PwrOff(&Ctrl);
            break;
            /***********************************************
            * 描述： 开机状态
            */
        case SYS_PWR_ON:
            /***********************************************
            * 描述： 风机PWM输出
            */
#if defined(TEST_RB)
            if ( Ctrl.Fan.Mode == FAN_MODE_SMART ) {
                if ( FAN_DUTY_SCALE_MAX <= Ctrl.Fan.CurScale ) {
                    Ctrl.Fan.SetScale   = FAN_DUTY_SCALE_MIN;
                } else if ( FAN_DUTY_SCALE_MIN >= Ctrl.Fan.CurScale ) {
                    Ctrl.Fan.SetScale   = FAN_DUTY_SCALE_MAX;
                }
                
                if ( Ctrl.Fan.SetScale > Ctrl.Fan.CurScale ) {
                    Ctrl.Fan.CurScale   += 10;
                    PWM_SetDuty(&FanPwm, PWM_CH_FAN,(float)Ctrl.Fan.CurScale/10.0);
                } else if ( Ctrl.Fan.SetScale < Ctrl.Fan.CurScale ) {
                    PWM_SetDuty(&FanPwm, PWM_CH_FAN,(float)Ctrl.Fan.CurScale/10.0);
                    Ctrl.Fan.CurScale   -= 10;
                } else {
                }
            } else {
                if ( Ctrl.Fan.SetScale > Ctrl.Fan.CurScale ) {
                    PWM_SetDuty(&FanPwm, PWM_CH_FAN,(float)Ctrl.Fan.CurScale/10.0);
                    Ctrl.Fan.CurScale++;
                } else if ( Ctrl.Fan.SetScale < Ctrl.Fan.CurScale ) {
                    PWM_SetDuty(&FanPwm, PWM_CH_FAN,(float)Ctrl.Fan.CurScale/10.0);
                    Ctrl.Fan.CurScale--;
                }            
            }
#else
            if ( Ctrl.Fan.SetScale > Ctrl.Fan.CurScale ) {
                PWM_SetDuty(&FanPwm, PWM_CH_FAN,(float)Ctrl.Fan.CurScale/10.0);
                Ctrl.Fan.CurScale++;
            } else if ( Ctrl.Fan.SetScale < Ctrl.Fan.CurScale ) {
                PWM_SetDuty(&FanPwm, PWM_CH_FAN,(float)Ctrl.Fan.CurScale/10.0);
                Ctrl.Fan.CurScale--;
            } 
            if ( Ctrl.Fan.CurScale < FAN_DUTY_SCALE_0 )
                Ctrl.Fan.CurScale = FAN_DUTY_SCALE_0;
#endif 
            break;
        }
        
        return ( task_event ^ (OS_EVT_CTRL_FAN) );            
    }
    
    /***********************************************
    * 描述： 睡眠（静音模式）呼吸事件
    */
    if( task_event & (OS_EVT_CTRL_LED) ) {
        if ( Ctrl.Sys.SysSts == SYS_PWR_ON ) {
            if ( Ctrl.Fan.Level == FAN_LEVEL_SILENT ) {
                /***********************************************
                * 描述： 呼吸方式：一次函数
                *
                    static float  step    = BREATHE_STEP; 
                    if( Ctrl.Led.Brightness > 60 ) {
                    Ctrl.Led.Brightness  += step * Ctrl.Led.Brightness / 60.0;
                } else {
                    Ctrl.Led.Brightness  += step;
                }
                    if( Ctrl.Led.Brightness > 100 ) {
                    Ctrl.Led.Brightness    = 100;
                    step    = -BREATHE_STEP;
                } else if( Ctrl.Led.Brightness < BREATHE_DUTY_MIN ) {
                    Ctrl.Led.Brightness    = BREATHE_DUTY_MIN;
                    step    = BREATHE_STEP;
                } 
                *//***********************************************
                * 描述： 呼吸方式：二次函数
                */  
                static int      step    = 1;
                static int      ctr     = 0;
                
                static double   x       = 0.0;
                static double   a       = 100.0 / (BREATHE_TICK_NBR/2) / (BREATHE_TICK_NBR/2);
                static double   b       = 0.0;
                static double   c       = BREATHE_DUTY_MIN;
                static double   y       = 0.0;
                
                ctr += step;
                
                if( ctr > BREATHE_TICK_NBR / 2 ) {
                    step    = -1;
                } else if( ctr < ( 0 - BREATHE_TICK_NBR / 2 ) ) {
                    step    = 1;
                }            
                
                x   = ctr;            
                y   = a * pow(x,2);
                y   = y + b*x +c;
                
                Ctrl.Led.Breathe    = y; 
                
                if( Ctrl.Led.Breathe > 100 ) {
                    Ctrl.Led.Breathe    = 100;
                } else if( Ctrl.Led.Breathe < BREATHE_DUTY_MIN ) {
                    Ctrl.Led.Breathe    = BREATHE_DUTY_MIN;
                }
                
                Ctrl.Led.SetColor   = BUILD_RGB(0,0,(INT08U)(Ctrl.Led.Breathe * Ctrl.Led.ColorScale / 100));    // R,G,B
                /***********************************************
                * 描述： 
                */  
            }
            
            /***********************************************
            * 描述： LED输出
            */
            if ( ( Ctrl.Sys.WifiSts == WIFI_STS_OK ) ) {
                LED_SetColor(Ctrl.Led.SetColor);
            }
        }
        return ( task_event ^ (OS_EVT_CTRL_LED) );
    }
    
    /***********************************************
    * 描述： WIFI入网配置状态指示
    */  
    if( task_event & (OS_EVT_CTRL_WIFI_CFG) ) {
        static INT08U   flag    = 0;
        flag    = ~flag;
        /***********************************************
        * 描述： 
        */
        if ( flag == 0) {
            Ctrl.Led.SetColor   = BUILD_RGB(0,0,0);    // R,G,B
            LED_SetColor(Ctrl.Led.SetColor);
        /***********************************************
        * 描述： 
        */
        } else {
            if ( Ctrl.Sys.SysSts != SYS_PWR_ON )
                LED_SetColor(0x00ffffff);               // 关机白色
            else
                LED_SetColor(Ctrl.Led.SetColor);        // 工作颜色
        }
        if ( Ctrl.Sys.WifiSts != WIFI_STS_OK )
            osal_start_timerEx( OS_TASK_ID_CTRL, (OS_EVT_CTRL_WIFI_CFG), 200);
        
        return ( task_event ^ (OS_EVT_CTRL_WIFI_CFG) );
    }
    
    /***********************************************
    * 描述： WIFI入网配置超时
    */  
    if( task_event & (OS_EVT_CTRL_WIFI_TMO) ) {
        osal_stop_timerEx( OS_TASK_ID_CTRL, (OS_EVT_CTRL_WIFI_TMO) );
        return ( task_event ^ (OS_EVT_CTRL_WIFI_TMO) );
    }
    
    /***********************************************
    * 描述： 滤网寿命倒计时
    */  
    if( task_event & (OS_EVT_CTRL_FILTER) ) {
        // 启动下次滤网寿命倒计时定时器
        osal_start_timerEx( OS_TASK_ID_CTRL, 
                           OS_EVT_CTRL_FILTER, 
                           OS_TICKS_PER_SEC * 60 * FILTER_LIFE_SAVE_TIME  );
        if ( Ctrl.Sys.SysSts == SYS_PWR_ON ) {
            // 计算寿命
            CalcFilterLife(Ctrl.Fan.Level, 1);
            // 计算出风量
            CalcAirVolume(Ctrl.Fan.Level, 1);
            // 保存当前出风量
            BSP_FlashWriteBytes((u32) 0*2, (u8 *)&Ctrl.Para.buf1[0], (u16) 10*2);
        }
        return ( task_event ^ (OS_EVT_CTRL_FILTER) );
    }
     
    /***********************************************
    * 描述： 出风量统计
    */  
    if( task_event & (OS_EVT_CTRL_AIR_VOLUME) ) {
        osal_start_timerEx( OS_TASK_ID_CTRL, 
                           OS_EVT_CTRL_AIR_VOLUME, 
                           OS_TICKS_PER_SEC * 60 );
        if ( Ctrl.Sys.SysSts == SYS_PWR_ON ) {
            // 计算寿命
            CalcFilterLife(Ctrl.Fan.Level, 1);
            // 计算出风量
            CalcAirVolume(Ctrl.Fan.Level, 1);
        }
        return ( task_event ^ (OS_EVT_CTRL_AIR_VOLUME) );
    }
    /***********************************************
    * 描述： 定时开关机
    */  
    if( task_event & (OS_EVT_CTRL_TIMING_ON_OFF) ) {
        static INT08U   ctr = 0;
        /***********************************************
        * 描述： 定时开机
        */ 
        if ( Ctrl.Wifi.Wr.CountDownOnMin ) {
            if ( ++ctr >= 60 ) {
                ctr = 0;
                if ( --Ctrl.Wifi.Wr.CountDownOnMin == 0 ) {
                    Power(ON);
                }
            }
        /***********************************************
        * 描述： 定时关机
        */ 
        } else if (Ctrl.Wifi.Wr.CountDownOffMin) {
            if ( ++ctr >= 60 ) {
                ctr = 0;
                if ( --Ctrl.Wifi.Wr.CountDownOffMin == 0 ) {
                    Power(OFF);
                }
            }
        } else {
            ctr = 0;
        }
        
        /***********************************************
        * 描述：报警（bety29）
        *
        union __alert {
            struct __ualert {
                uint8_t  FilterLife :1;//滤芯寿命报警
                uint8_t  AirQuality :1;//空气质量报警
                uint8_t  RSV        :6;//预留
            }udat;
            uint8_t  uAlert;
        }Alert;
        *//***********************************************
        * 描述：故障 （bety30）
        *
        union __fault {
            struct __ufault {
                uint8_t  Motor       :1;  //电机故障
                uint8_t  AirSensor   :1;  //空气传感器（VOC）故障
                uint8_t  DustSensor  :1;  //灰尘传感器故障
                uint8_t  RSV         :5;  //预留
            }udat;
            uint8_t  uFault;
        }Fault;
    
        */
        return ( task_event ^ (OS_EVT_CTRL_TIMING_ON_OFF) );
    }
        
    return 0;
}

/*******************************************************************************
 * 名    称： APP_ControlInit
 * 功    能： 任务初始化
 * 入口参数： 无
 * 出口参数： 无
 * 作　　者： Roger-WY.
 * 创建日期： 2015-03-28
 * 修    改：
 * 修改日期：
 *******************************************************************************/
void TaskInitCtrl(void)
{
    Ctrl.Fan.SetScale   = (INT16S)FAN_DUTY_SCALE_2;
    Ctrl.Fan.CurScale   = (INT16S)FAN_DUTY_SCALE_1;
    
    /***********************************************
    * 描述： 初始化本任务用到的相关硬件
    */
    ControlInit();
    
#if (STORAGE_TYPE == INTNER_FLASH ) 
    if ( (INT16U) Ctrl.Para.dat.SysSts.dat != 0 ) {
        if ( Ctrl.Para.dat.SysSts.udat.App_Fault_ISR == 1 ) {
            Ctrl.Fan.Mode    = Ctrl.Para.dat.SysSts.udat.FanMode;
            Ctrl.Fan.Level   = Ctrl.Para.dat.SysSts.udat.FanLevel;
        }
        Ctrl.Para.dat.SysSts.dat = 0;
        BSP_FlashWriteBytes((u32) 126*2, (u8 *)&Ctrl.Para.dat.SysSts.udat, (u16) 2);
    }
    
#elif (STORAGE_TYPE == INTNER_EEPROM )
#elif (STORAGE_TYPE == EXTERN_EEPROM )
#endif
    
    App_PwrOff(&Ctrl);
  
    // 计算寿命
    CalcFilterLife(0, 0);
    // 计算出风量
    CalcAirVolume(0, 0);
    /***********************************************
    * 描述： 在看门狗标志组注册本任务的看门狗标志
    */
    WdtFlags |= WDT_FLAG_CTRL;
    /*************************************************
    * 描述：启动事件查询
    */
#if ( OSAL_EN == DEF_ENABLED )
    osal_start_timerRl( OS_TASK_ID_CTRL, OS_EVT_CTRL_TICK,  OS_TICKS_PER_SEC / 5);
    osal_start_timerRl( OS_TASK_ID_CTRL, OS_EVT_CTRL_FAN,   OS_TICKS_PER_SEC / 10);
    osal_start_timerRl( OS_TASK_ID_CTRL, OS_EVT_CTRL_LED,   BREATHE_TICK);
    osal_start_timerRl( OS_TASK_ID_CTRL, OS_EVT_CTRL_TIMING_ON_OFF, OS_TICKS_PER_SEC);
#else
#endif
        
    /*************************************************
    * 描述： 如果是测试，则直接开机
    */
#if defined(TEST_RB)
    Power(ON);
#endif
}

/*******************************************************************************
 * 名    称： App_PwrOff
 * 功    能：
 * 入口参数： 无
 * 出口参数： 无
 * 作　　者： Roger-WY.
 * 创建日期： 2015-09-02
 * 修    改：
 * 修改日期：
 *******************************************************************************/
static  void    App_PwrOff              (StrCtrlDCB *dcb)
{
    PWM_SetDuty(&FanPwm, PWM_CH_FAN, FAN_DUTY_STOP);
    
    if ( dcb->Sys.WifiSts == WIFI_STS_OK ) {
        LED_SetColor(0);
    }
}

/*******************************************************************************
 * 名    称： App_PwrOff
 * 功    能：
 * 入口参数： 无
 * 出口参数： 无
 * 作　　者： Roger-WY.
 * 创建日期： 2015-09-02
 * 修    改：
 * 修改日期：
 *******************************************************************************/
static  void    App_WifiConfig         (StrCtrlDCB *dcb)
{  
    /***********************************************
    * 描述： 
    */
    switch ( dcb->Sys.WifiSts ) {
    /***********************************************
    * 描述： 
    */
    case WIFI_STS_OK:
        return;
        break;
    /***********************************************
    * 描述： 
    */
    case WIFI_STS_CONFIG: 
        if ( Ctrl.Wifi.W2dWifiSts.WifiStatus.ustatus.ConnRouter ) {
            dcb->Sys.WifiSts    = WIFI_STS_OK;
            //CYCLE_TIME_TICKS    = CONTROL_CYCLE_TIME_TICKS;
            osal_stop_timerEx( OS_TASK_ID_CTRL, (OS_EVT_CTRL_WIFI_CFG) );
            LED_SetColor(0);
        } else if ( !Ctrl.Wifi.W2dWifiSts.WifiStatus.ustatus.SoftApMode ) {
            dcb->Sys.WifiSts    = WIFI_STS_OK;
            //CYCLE_TIME_TICKS    = CONTROL_CYCLE_TIME_TICKS;
            osal_stop_timerEx( OS_TASK_ID_CTRL, (OS_EVT_CTRL_WIFI_CFG) );
            LED_SetColor(0);
            return;
        } else {
            
        }
        break;
    /***********************************************
    * 描述： 
    */
    case WIFI_STS_CONFIG_OK:
        break;
    /***********************************************
    * 描述： 
    */
    case WIFI_STS_CONFIG_ERR:
        break;
    }
}

/*******************************************************************************
 * 名    称： App_Pwron
 * 功    能：
 * 入口参数： 无
 * 出口参数： 无
 * 作　　者： Roger-WY.
 * 创建日期： 2015-09-02
 * 修    改：
 * 修改日期：
 *******************************************************************************/
static  void    App_PwrOn               (StrCtrlDCB *dcb)
{
    extern Device_ReadTypeDef               Device_ReadStruct;
    /***********************************************
    * 描述： 根据模式选择风机等级
    */
    switch(dcb->Fan.Mode) {
    case FAN_MODE_PWR_OFF:                          // 待机	上电自动运行，风机档位为0，其它正常运作
        break;
    case FAN_MODE_SILENT:                           // 静音模式	风机档位1，LED灯蓝色呼吸灯效果
        dcb->Fan.Level  = FAN_LEVEL_SILENT;
        break;
    case FAN_MODE_SMART:                            // 智能模式	风机档位根据传感器反馈的空气质量动态变化2-4，LED随空气质量改变
        dcb->Fan.Level  = FAN_LEVEL_AQL1_LOW + Ctrl.PM2D5.Level;
        break;
    case FAN_MODE_MAX_SPEED:                        // 极速模式	风机档位5，LED灯绿色
        dcb->Fan.Level  = FAN_LEVEL_MAX;
        break;
    case FAN_MODE_HAND:                             // 手动模式	手动切换模式2-4，LED灯随空气质量改变
        break;
    default:
        dcb->Fan.Level  = FAN_LEVEL_SILENT;
        break;
    }
    
    /***********************************************
    * 描述： 根据光照强度计算LED等级
    */    
#if PWM_OUT_NEGTIVE == DEF_ENABLED
    dcb->Led.Brightness     = 1.0;
#else
    static float   Brightness   = 1.0;
    
    if ( ADC_AverageValue[1] < 120 ) {
        Brightness  = 0.05;
    } else if ( ADC_AverageValue[1] > 130 ) {
        Brightness  = 1.0;
    } else if ( Brightness  < 0.05 ) {
        Brightness  = 1.0;
    }
    //Brightness      = 0.02
    //                + (float)ADC_AverageValue[1] 
    //                / 1000.0;
    //
    //if( Brightness > 1.0 )
    //    Brightness = 1.0; 
    //
    //if ( dcb->Led.Brightness == 0.0 )
    //    dcb->Led.Brightness = Brightness;
    dcb->Led.Brightness     = dcb->Led.Brightness   * 0.9 
                            + Brightness            * 0.1;
    dcb->Led.ColorScale     = (INT08U)(dcb->Led.Brightness   * 255);
#endif
    float   SetScale;
    /***********************************************
    * 描述： 分别处理风机等级及LED指示颜色
    *        出风面积：200mm*73mm
    */    
    switch(dcb->Fan.Level) {
        /***********************************************
        * 描述： 关机/待机模式：熄灭
        */
    case FAN_LEVEL_OFF:{
        SetScale            = FAN_DUTY_STOP;
        PWM_SetDuty(&FanPwm, PWM_CH_FAN, FAN_DUTY_STOP);
        LED_SetColor(0);
    } break;
    /***********************************************
    * 描述： 静音模式：蓝色呼吸 1.2m/s
    */
    case FAN_LEVEL_SILENT:{
        SetScale            = FAN_DUTY_SCALE_1;
        //dcb->Led.SetColor   = BUILD_RGB(0,0,(INT08U)(dcb->Led.Breathe * LedScale / 100));    // R,G,B
    } break;
    /***********************************************
    * 描述： 低速模式：蓝色   2.3m/s
    */
    case FAN_LEVEL_AQL1_LOW:{
        SetScale            = FAN_DUTY_SCALE_2;
        dcb->Led.SetColor   = BUILD_RGB(0,0,dcb->Led.ColorScale);          // R,G,B
    } break;
    /***********************************************
    * 描述： 中速模式：黄色   3.5m/s
    */
    case FAN_LEVEL_AQL2_MID:{
        SetScale            = FAN_DUTY_SCALE_3;
        dcb->Led.SetColor   = BUILD_RGB(dcb->Led.ColorScale,dcb->Led.ColorScale,0);    // R,G,B
    } break;
    /***********************************************
    * 描述： 高速模式：红色   4.7m/s
    */
    case FAN_LEVEL_AQL3_HIG:{
        SetScale            = FAN_DUTY_SCALE_4;
        dcb->Led.SetColor   = BUILD_RGB(dcb->Led.ColorScale,0,0);          // R,G,B
    } break;
    /***********************************************
    * 描述： 极速模式：绿色   5.3m/s
    */
    case FAN_LEVEL_MAX:{
        SetScale            = FAN_DUTY_SCALE_5;
        dcb->Led.SetColor   = BUILD_RGB(0,dcb->Led.ColorScale,0);          // R,G,B
    } break;
    default:
        SetScale            = dcb->Fan.SetScale;
        break;
    }
    
#if defined(TEST_RB)
#else
    dcb->Fan.SetScale   = (INT16S)SetScale;
#endif
}

/*******************************************************************************
 * 名    称： CalcFilterLife
 * 功    能： 计算滤网寿命
 * 入口参数： 无
 * 出口参数： 无
 * 作　　者： wumingshen
 * 创建日期： 2015-10-20
 * 修    改：
 * 修改日期：
 *******************************************************************************/
void CalcFilterLife(u8 level, u8 min)
{
    if (Ctrl.Para.dat.FilterLife.Life) {
        // 寿命减FILTER_LIFE_SAVE_TIME分钟
        Ctrl.Para.dat.FilterLife.Life   -= min;
    }
            
    if ( ( Ctrl.Para.dat.FilterLife.Life < 0 ) ||
         ( Ctrl.Para.dat.FilterLife.Life > FILTER_LIFE_TIME ) )
        Ctrl.Para.dat.FilterLife.Life    = FILTER_LIFE_TIME;
    
    Ctrl.Wifi.Rd.FilterLife         = ((INT32U)Ctrl.Para.dat.FilterLife.Life
                                    * 100)
                                    / (FILTER_LIFE_TIME);
    Ctrl.Wifi.Wr.FilterLife         = Ctrl.Wifi.Rd.FilterLife;
}

/*******************************************************************************
 * 名    称： CalcFilterLife
 * 功    能： 计算滤网寿命
 * 入口参数： 无
 * 出口参数： 无
 * 作　　者： wumingshen
 * 创建日期： 2015-10-20
 * 修    改：
 * 修改日期：
 *******************************************************************************/
void CalcAirVolume(u8 level, u8 min)
{
    u32     volume  = Ctrl.Para.dat.AirVolume.Volume;
    float   v;
    
    if ( volume  == 0xffff )
        volume      = 0;    
    
    /***********************************************
    * 描述： 分别处理风机等级及LED指示颜色
    *        出风面积：200mm*73mm
    */    
    switch(level) {
        /***********************************************
        * 描述： 关机/待机模式：熄灭
        */
    case FAN_LEVEL_OFF:         v   = 0.0;  break;
    /***********************************************
    * 描述： 静音模式：蓝色呼吸 1.2m/s
    */
    case FAN_LEVEL_SILENT:      v   = 1.8;  break;
    /***********************************************
    * 描述： 低速模式：蓝色   2.3m/s
    */
    case FAN_LEVEL_AQL1_LOW:    v   = 3.0;  break;
    /***********************************************
    * 描述： 中速模式：黄色   3.5m/s
    */
    case FAN_LEVEL_AQL2_MID:    v   = 4.0;  break;
    /***********************************************
    * 描述： 高速模式：红色   4.7m/s
    */
    case FAN_LEVEL_AQL3_HIG:    v   = 5.0;  break;
    /***********************************************
    * 描述： 极速模式：绿色   5.3m/s
    */
    case FAN_LEVEL_MAX:         v   = 6.0;  break;
    default:                    v   = 0.0;  break;
    }    
    
    //1．长方形或方形面积之出风口风量计算：（公尺单位）   
#define     s   (0.22 * 0.074)                      // 长×宽=面积（M^2）
                                                    // 面积各点的平均风速=m/s（m/s）
    float   qs  = s * v;                            // 面积（m^2）×平均风速=m^3/s（m^3/S）
    float   qm  = qs * 60;                          // m^3/s×60=m^3/minute(m^3/min)
    volume                          = (u32)(volume
                                    + qm * min);
        
    Ctrl.Para.dat.AirVolume.Volume  = volume;
    //Ctrl.Wifi.Wr.Ext.dat.AirVolume  = volume;
    memcpy(&Ctrl.Wifi.Wr.Ext.buf1[0], (INT08U *)&volume, 4);
}

/*******************************************************************************
 * 名    称： Power
 * 功    能： 开关机处理
 * 入口参数： 无
 * 出口参数： 无
 * 作　　者： wumingshen
 * 创建日期： 2015-10-20
 * 修    改：
 * 修改日期：
 *******************************************************************************/
void Power(INT08U onoff)
{
    OS_ERR  err;
    
    if ( onoff ) {
        /***********************************************
        * 描述： 开机状态初始化
        */  
        Ctrl.Sys.SysSts             = SYS_PWR_ON;
        Ctrl.Fan.CurScale           = FAN_DUTY_SCALE_0;
        Ctrl.Fan.Mode               = FAN_MODE_SMART;
        Ctrl.Wifi.Wr.CountDownOnMin = 0;
#if defined(TEST_RB)
        Ctrl.Fan.SetScale           = FAN_DUTY_SCALE_MAX;
#endif
        /***********************************************
        * 描述： 开机音
        */
        Beep( BEEP_FLAG_PWR_ON );
    } else {
        /***********************************************
        * 描述： 关机状态初始化
        */  
        Ctrl.Sys.SysSts             = SYS_PWR_OFF;
        Ctrl.Fan.Mode               = FAN_MODE_PWR_OFF;
        Ctrl.Fan.CurScale           = FAN_DUTY_SCALE_0;
        Ctrl.Wifi.Wr.CountDownOffMin= 0;
        /***********************************************
        * 描述： 关机音
        */
        Beep( BEEP_FLAG_PWR_OFF );
    }
    /***********************************************
    * 描述： 开关机状态上报
    */    
    OSFlagPost(( OS_FLAG_GRP  *)&Ctrl.Os.WifiEvtFlagGrp,
               ( OS_FLAGS      )WIFI_EVT_FLAG_REPORT,
               ( OS_OPT        )OS_OPT_POST_FLAG_SET,
               ( OS_ERR       *)&err);
}

/*******************************************************************************
 * 				end of file
 *******************************************************************************/
#endif