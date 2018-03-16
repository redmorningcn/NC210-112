/*******************************************************************************
 *   Filename:       app_task_sensor.c
 *   Revised:        All copyrights reserved to Roger-WY.
 *   Revision:       v1.0
 *   Writer:	     Roger-WY.
 *
 *   Description:    双击选中 sensor 按 Ctrl + H, 钩选 Match the case, Replace with
 *                   输入您要的名字，点击 Replace All
 *                   双击选中 Sensor 按 Ctrl + H, 钩选 Match the case, Replace with
 *                   输入您要的名字，点击 Replace All
 *                   双击选中 SENSOR 按 Ctrl + H, 钩选 Match the case, Replace with
 *                   输入您要的名字，点击 Replace All
 *                   在 app_cfg.h 中指定本任务的 优先级  （ APP_TASK_SENSOR_PRIO ）
 *                                            和 任务堆栈（ APP_TASK_SENSOR_STK_SIZE ）大小
 *                   在 app.h 中声明本任务的     创建函数（ void  App_TaskSensorCreate(void) ）
 *                                            和 看门狗标志位 （ WDTFLAG_Sensor ）
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
//#include "bsp_pm25.h"
//#include "bsp_shtxx.h"
#include <bsp_adc7682.h>

#ifdef VSC_INCLUDE_SOURCE_FILE_NAMES
const  CPU_CHAR  *app_task_sensor__c = "$Id: $";
#endif

#define APP_TASK_SENSOR_EN     DEF_DISABLED
#if APP_TASK_SENSOR_EN == DEF_ENABLED
/*******************************************************************************
 * CONSTANTS
 */
INT32U   CYCLE_TIME_TICKS     = (OS_CFG_TICK_RATE_HZ * 1u);

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
static  OS_TCB   AppTaskSensorTCB;

/***********************************************
* 描述： 任务堆栈（STACKS）
*/
static  CPU_STK  AppTaskSensorStk[ APP_TASK_SENSOR_STK_SIZE ];

#endif
/*******************************************************************************
 * GLOBAL VARIABLES
 */

/*******************************************************************************
 * LOCAL FUNCTIONS
 */
#if ( OSAL_EN == DEF_ENABLED )
#else
static  void    AppTaskSensor           (void *p_arg);
#endif

/*******************************************************************************
 * GLOBAL FUNCTIONS
 */
/***************************************************
* 描述： 软定时器声明
*/
OS_TMR          OSTmr2;                             // 定时器1
/***********************************************
* 描述： 软定时器回调函数
*/
void            OSTmr2_callback         (OS_TMR *ptmr,void *p_arg);
/*******************************************************************************
 * EXTERN VARIABLES
 */

extern   Dev_StatusTypeDef      Dev_Status;

/*******************************************************************************/

/*******************************************************************************
 * 名    称： App_TaskSensorCreate
 * 功    能： **任务创建
 * 入口参数： 无
 * 出口参数： 无
 * 作　　者： Roger-WY.
 * 创建日期： 2015-02-05
 * 修    改：
 * 修改日期：
 * 备    注： 任务创建函数需要在app.h文件中声明
 *******************************************************************************/
void  App_TaskSensorCreate(void)
{

#if ( OSAL_EN == DEF_ENABLED )
#else
    OS_ERR  err;

    /***********************************************
    * 描述： 任务创建
    */
    OSTaskCreate((OS_TCB     *)&AppTaskSensorTCB,               // 任务控制块  （当前文件中定义）
                 (CPU_CHAR   *)"App Task Sensor",               // 任务名称
                 (OS_TASK_PTR ) AppTaskSensor,                  // 任务函数指针（当前文件中定义）
                 (void       *) 0,                              // 任务函数参数
                 (OS_PRIO     ) APP_TASK_SENSOR_PRIO,           // 任务优先级，不同任务优先级可以相同，0 < 优先级 < OS_CFG_PRIO_MAX - 2（app_cfg.h中定义）
                 (CPU_STK    *)&AppTaskSensorStk[0],            // 任务栈顶
                 (CPU_STK_SIZE) APP_TASK_SENSOR_STK_SIZE / 10,  // 任务栈溢出报警值
                 (CPU_STK_SIZE) APP_TASK_SENSOR_STK_SIZE,       // 任务栈大小（CPU数据宽度 * 8 * size = 4 * 8 * size(字节)）（app_cfg.h中定义）
                 (OS_MSG_QTY  ) 5u,                             // 可以发送给任务的最大消息队列数量
                 (OS_TICK     ) 0u,                             // 相同优先级任务的轮循时间（ms），0为默认
                 (void       *) 0,                              // 是一个指向它被用作一个TCB扩展用户提供的存储器位置
                 (OS_OPT      )(OS_OPT_TASK_STK_CHK |           // 允许堆栈检查该任务
                                OS_OPT_TASK_STK_CLR),           // 创建任务时堆栈清零
                 (OS_ERR     *)&err);                           // 指向错误代码的指针，用于创建结果处理
#endif
}

/*******************************************************************************
 * 名    称： AppTaskSensor
 * 功    能： 控制任务
 * 入口参数： p_arg - 由任务创建函数传入
 * 出口参数： 无
 * 作　　者： Roger-WY.
 * 创建日期： 2015-02-05
 * 修    改：
 * 修改日期：
 *******************************************************************************/

#if ( OSAL_EN == DEF_ENABLED )
osalEvt  TaskSensorEvtProcess(INT8U task_id, osalEvt task_event)
#else
static  void  AppTaskSensor (void *p_arg)
#endif
{
    OS_ERR      err;
    INT32U      ticks;
    INT32S      dly;
#if defined(PM2D5_UART_CHK)
    static INT08U       UartSendFlag    = 0;
#endif
    
    /***********************************************
    * 描述：Task body, always written as an infinite loop.
    */
#if ( OSAL_EN == DEF_ENABLED )
#else
    TaskInitSensor();
    
    while (DEF_TRUE) {
#endif
        /***********************************************
        * 描述： 本任务看门狗标志置位
        */
        OS_FlagPost ((OS_FLAG_GRP *)&WdtFlagGRP,
                     (OS_FLAGS     ) WDT_FLAG_SEN,
                     (OS_OPT       ) OS_OPT_POST_FLAG_SET,
                     (CPU_TS       ) 0,
                     (OS_ERR      *) &err);
        /***********************************************
        * 描述： 得到系统当前时间
        */
        ticks = OSTimeGet(&err);
        
#if ( OSAL_EN == DEF_ENABLED )
        if( task_event & OS_EVT_SEN_SAMPLE ) {
#else
#endif
            static INT8U       step    = 0;
            static INT08U      ctr     = 0;
            /***********************************************
            * 描述： 获得SHT温湿度传感器的值
            */
            switch(step) {
            case 0:
                if(BSP_ShtMeasureOnce(0,0) == 0) {
                    step = 1;
                } else {
                    step = 0;
                }
                CYCLE_TIME_TICKS = 350;
                break;
            case 1:
                if(BSP_ShtMeasureOnce(0,1) == 0) {
                    step = 2;
                } else{
                    step = 0;
                }
                CYCLE_TIME_TICKS = 10;
                break;
                
            case 2:
                if(BSP_ShtMeasureOnce(1,0) == 0) {
                    step = 3;
                } else {
                    step = 0;
                }
                CYCLE_TIME_TICKS = 350;
                break;
                
            case 3:
                if(BSP_ShtMeasureOnce(1,1) == 0) {
                    step = 4;
                } else {
                    step = 0;
                }
                CYCLE_TIME_TICKS = 10;
                break;
                
            case 4:
                if(BSP_ShtMeasureOnce(2,1) == 0) {
                    step = 0;
                    //Ctrl.Wifi.Rd.Temperature   = (int8_t)SHT_Data.Temp;
                    //Ctrl.Wifi.Rd.Humidity      = (uint8_t)SHT_Data.Humi;
                    
                } else {
                    step = 0;
                }
                
                CYCLE_TIME_TICKS = 500;
                break;
            }
            
            /***********************************************
            * 描述： 获得PM2.5的值和PM2.5的等级
            */
            if( ++ctr >= 4 ) {
                ctr = 0;
                /***********************************************
                * 描述： 读取实际值
                */
                Ctrl.PM2D5.G1Val        = BSP_PM2D5GetValue(0); // 读取一代值
                Ctrl.PM2D5.G2Val        = BSP_PM2D5GetValue(1); // 读取二代值

                /***********************************************
                * 描述： 选择传感器
                */
                if ( Ctrl.PM2D5.Type == 0 ) {
                    Ctrl.PM2D5.Val      = Ctrl.PM2D5.G1Val;
                } else {
                    Ctrl.PM2D5.Val      = Ctrl.PM2D5.G2Val;
                }
                
                Ctrl.PM2D5.ChkedVal = Ctrl.PM2D5.Val + Ctrl.Para.dat.Pm2d5Chk.Offset.b[Ctrl.PM2D5.Type]; 
                /***********************************************
                * 描述： 计算校准以后的值
                */
                /***********************************************
                * 描述： 参数过滤
                */
                Ctrl.PM2D5.ChkedVal     = (Ctrl.PM2D5.ChkedVal >   0) ? Ctrl.PM2D5.ChkedVal : 1;
                Ctrl.PM2D5.ChkedVal     = (Ctrl.PM2D5.ChkedVal < 999) ? Ctrl.PM2D5.ChkedVal : 999;
                
                Ctrl.Wifi.Rd.PM25Val    = Ctrl.PM2D5.ChkedVal;
                /***********************************************
                * 描述： 计算等级
                */
                Ctrl.Wifi.Rd.PM25Level  = BSP_PM2D5GetLevel(Ctrl.PM2D5.ChkedVal);
                Ctrl.PM2D5.Level        = Ctrl.Wifi.Rd.PM25Level;
#if defined(PM2D5_UART_CHK)
                if ( UartSendFlag ) {
                    u8  Sendbuf[11];
                    Sendbuf[0] = 0xFF;
                    Sendbuf[1] = 0xA5;
                    Sendbuf[2] = 6;
                    /***********************************************
                    * 描述： 计算PM25的值
                    */
                    Sendbuf[3] = HI_INT16U(Ctrl.Wifi.Rd.PM25Val);
                    Sendbuf[4] = LO_INT16U(Ctrl.Wifi.Rd.PM25Val);
                    /***********************************************
                    * 描述： 获取温湿度的值
                    */
                    Sendbuf[5] = HI_INT16U(Ctrl.Wifi.Rd.Temperature);
                    Sendbuf[6] = LO_INT16U(Ctrl.Wifi.Rd.Temperature);
                    
                    Sendbuf[7] = HI_INT16U(Ctrl.Wifi.Rd.Humidity);
                    Sendbuf[8] = LO_INT16U(Ctrl.Wifi.Rd.Humidity);

                    Sendbuf[9] = LO_INT16U(Sendbuf[3] + Sendbuf[4] + Sendbuf[5] + Sendbuf[6] + Sendbuf[7] + Sendbuf[8]);
                    Sendbuf[10] = 0x5a;
                    USART_WriteBytes(USART1, Sendbuf, 11);
                }
#endif
            }
            /***********************************************
            * 描述： 获得VOC传感器的值(暂时未启用)
            */
            Ctrl.Wifi.Rd.Voc = 0;
            /***********************************************
            * 描述： 去除任务运行的时间，等到一个控制周期里剩余需要延时的时间
            */
            dly   = CYCLE_TIME_TICKS - ( OSTimeGet(&err) - ticks );
            if ( dly  <= 0 ) {
                dly   = 1;
            }
#if ( OSAL_EN == DEF_ENABLED )
            osal_start_timerEx( OS_TASK_ID_SEN,
                               OS_EVT_SEN_SAMPLE,
                                dly);
            
            return ( task_event ^ OS_EVT_SEN_SAMPLE );
        }
        /***********************************************
        * 描述： 修改PM2.5标定低值
        */
        if( task_event & OS_EVT_SEN_PM2D5_CHK ) {
            BSP_PM2D5Chk( PM2D5_CHK_TYPE_B_OFFSET, Ctrl.PM2D5.ChkVal );
            BeepExt(3, 3000, 500);
            return ( task_event ^ OS_EVT_SEN_PM2D5_CHK );
        }

        /***********************************************
        * 描述： PM2.5低值校验
        */
        if( task_event & OS_EVT_SEN_CHK_TIMEOUT ) {  
            Ctrl.PM2D5.ChkCtr   = 0; 
            return ( task_event ^ OS_EVT_SEN_CHK_TIMEOUT );
        }
        
        /***********************************************
        * 描述： PM2.5串口校验
        */
#if defined(PM2D5_UART_CHK)
        if( task_event & OS_EVT_SEN_UART_TIMEOUT ) { 
            extern u8 Uart1RxBuf[];
            extern u8 Uart1RxCtr;
            if ( ( Uart1RxBuf[0] == 0xFF ) && 
                 ( Uart1RxBuf[1] == 0XA5 ) && 
                 ( Uart1RxBuf[6] == 0x5A ) ) {
                 INT16S     value   = BUILD_INT16U(Uart1RxBuf[4],Uart1RxBuf[3]);
                 switch (Uart1RxBuf[2]) {
                 case 0:
                    BSP_PM2D5Chk( PM2D5_CHK_TYPE_DEF, 0 );
                    BeepExt(1, 3000, 500);
                    break;
                 case 1:
                    PM2D5ChkVal = value;
                    BSP_PM2D5Chk( PM2D5_CHK_TYPE_B_OFFSET, value );
                    BeepExt(1, 3000, 200);
                    break;
                 case 2:
                    BSP_PM2D5Chk( PM2D5_CHK_TYPE_K_Y1, value );
                    BeepExt(2, 3000, 200);
                    break;
                 case 3:
                    BSP_PM2D5Chk( PM2D5_CHK_TYPE_K_Y2, value );
                    BeepExt(3, 3000, 200);
                    break;
                 case 0xff:
                    UartSendFlag    = !UartSendFlag;
                    BeepExt(1, 3000, 200);
                    break;
                }
            }
            Uart1RxCtr  = 0;
            memset(Uart1RxBuf, 0, 11);
            return ( task_event ^ OS_EVT_SEN_UART_TIMEOUT );
        }
#endif 
        return 0;
#else
        BSP_OS_TimeDly(dly);
    }
#endif
}

/*******************************************************************************
 * 名    称： APP_SensorInit
 * 功    能： 任务初始化
 * 入口参数： 无
 * 出口参数： 无
 * 作　　者： Roger-WY.
 * 创建日期： 2015-03-28
 * 修    改：
 * 修改日期：
 *******************************************************************************/
void TaskInitSensor(void)
{
    OS_ERR err;
    /***********************************************
    * 描述： 初始化本任务用到的相关硬件
    */
    //BSP_PM2D5Init();
    //BSP_ShtxxInit();
    BSP_ADCInit();
    /***********************************************
    * 描述：创建一个软定时器 OS_OPT_TMR_ONE_SHOT, OS_OPT_TMR_PERIODIC
    */
    OSTmrCreate ((OS_TMR               *)&OSTmr2,
                 (CPU_CHAR             *)"tmr2",
                 (OS_TICK               )10,
                 (OS_TICK               )10,
                 (OS_OPT                )OS_OPT_TMR_PERIODIC,
                 (OS_TMR_CALLBACK_PTR   )OSTmr2_callback,
                 (void                 *)NULL,
                 (OS_ERR               *)&err);
    OSTmrStart ((OS_TMR  *)&OSTmr2,
                (OS_ERR  *)&err);
    /***********************************************
    * 描述： 在看门狗标志组注册本任务的看门狗标志
    */
    WdtFlags |= WDT_FLAG_SEN;
    Ctrl.PM2D5.ChkVal   = 25;
    /*************************************************
    * 描述：启动事件查询
    */
#if ( OSAL_EN == DEF_ENABLED )
    osal_start_timerEx( OS_TASK_ID_SEN,
                        (1<<0),
                        1);
#else
#endif
}
    extern void BSP_PM2D5GetAdcValueCB(void);

/*******************************************************************************
 * 名    称： OSTmr2_callback
 * 功    能： 软定时器1的回调函数
 * 入口参数： 无
 * 出口参数： 无
 * 作　　者： 无名沈
 * 创建日期： 2015-03-28
 * 修    改：
 * 修改日期：
 * 备    注： 定时器回调函数不能使用延时函数
 *******************************************************************************/
void OSTmr2_callback(OS_TMR *ptmr,void *p_arg)
{
    BSP_PM2D5GetAdcValueCB();
}
/*******************************************************************************
 * 				end of file
 *******************************************************************************/
#endif
