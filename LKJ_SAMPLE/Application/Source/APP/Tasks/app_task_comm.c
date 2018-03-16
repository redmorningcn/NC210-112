/*******************************************************************************
 *   Filename:       app_task_comm.c
 *   Revised:        All copyrights reserved to wumingshen.
 *   Revision:       v1.0
 *   Writer:	     wumingshen.
 *
 *   Description:    双击选中 comm 按 Ctrl + H, 钩选 Match the case, Replace with
 *                   输入您要的名字，点击 Replace All
 *                   双击选中 Comm 按 Ctrl + H, 钩选 Match the case, Replace with
 *                   输入您要的名字，点击 Replace All
 *                   双击选中 COMM 按 Ctrl + H, 钩选 Match the case, Replace with
 *                   输入您要的名字，点击 Replace All
 *                   在 app_cfg.h 中指定本任务的 优先级  （ APP_TASK_COMM_PRIO     ）
 *                                            和 任务堆栈（ APP_TASK_COMM_STK_SIZE ）大小
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
#include <app_comm_protocol.h>
#include <bsp_flash.h>
#include <iap.h>

#ifdef VSC_INCLUDE_SOURCE_FILE_NAMES
const  CPU_CHAR  *app_task_comm__c = "$Id: $";
#endif

#define APP_TASK_COMM_EN     DEF_ENABLED
#if APP_TASK_COMM_EN == DEF_ENABLED
/*******************************************************************************
 * CONSTANTS
 */

/*******************************************************************************
 * MACROS
 */
#define CYCLE_TIME_TICKS            (OS_TICKS_PER_SEC * 1)

/*******************************************************************************
 * TYPEDEFS
 */

/***********************************************
* 描述： 任务控制块（TCB）
*/
static  OS_TCB   AppTaskCommTCB;

/***********************************************
* 描述： 任务堆栈（STACKS）
*/
static  CPU_STK  AppTaskCommStk[ APP_TASK_COMM_STK_SIZE ];

/*******************************************************************************
 * LOCAL VARIABLES
 */

/*******************************************************************************
 * GLOBAL VARIABLES
 */

/***********************************************
* 描述： 软定时器声明
*/
OS_TMR          OSTmr1;                         //定时器1

/***********************************************
* 描述： 软定时器回调函数
*/
void            OSTmr1_callback             (OS_TMR *ptmr,void *p_arg);

/*******************************************************************************
 * LOCAL FUNCTIONS
 */
static void    AppTaskComm                 (void *p_arg);
static void    APP_CommInit                (void);
        void    App_McuStatusInit           (void);
        void    ReportDevStatusHandle       (void);
        void    InformCommConfigMode        (u8 mode);
static void    App_ModbusInit              (void);

INT08U          APP_CommRxDataDealCB        (MODBUS_CH  *pch);
INT08U          IAP_CommRxDataDealCB        (MODBUS_CH  *pch);
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
 * 名    称： App_TaskCommCreate
 * 功    能： **任务创建
 * 入口参数： 无
 * 出口参数： 无
 * 作　 　者： wumingshen.
 * 创建日期： 2015-02-05
 * 修    改：
 * 修改日期：
 * 备    注： 任务创建函数需要在app.h文件中声明
 *******************************************************************************/
void  App_TaskCommCreate(void)
{
    OS_ERR  err;

    /***********************************************
    * 描述： 任务创建
    */
    OSTaskCreate((OS_TCB     *)&AppTaskCommTCB,                     // 任务控制块  （当前文件中定义）
                 (CPU_CHAR   *)"App Task Comm",                     // 任务名称
                 (OS_TASK_PTR ) AppTaskComm,                        // 任务函数指针（当前文件中定义）
                 (void       *) 0,                                  // 任务函数参数
                 (OS_PRIO     ) APP_TASK_COMM_PRIO,                 // 任务优先级，不同任务优先级可以相同，0 < 优先级 < OS_CFG_PRIO_MAX - 2（app_cfg.h中定义）
                 (CPU_STK    *)&AppTaskCommStk[0],                  // 任务栈顶
                 (CPU_STK_SIZE) APP_TASK_COMM_STK_SIZE / 10,        // 任务栈溢出报警值
                 (CPU_STK_SIZE) APP_TASK_COMM_STK_SIZE,             // 任务栈大小（CPU数据宽度 * 8 * size = 4 * 8 * size(字节)）（app_cfg.h中定义）
                 (OS_MSG_QTY  ) 0u,                                 // 可以发送给任务的最大消息队列数量
                 (OS_TICK     ) 0u,                                 // 相同优先级任务的轮循时间（ms），0为默认
                 (void       *) 0,                                  // 是一个指向它被用作一个TCB扩展用户提供的存储器位置
                 (OS_OPT      )(OS_OPT_TASK_STK_CHK |               // 允许堆栈检查该任务
                                OS_OPT_TASK_STK_CLR),               // 创建任务时堆栈清零
                 (OS_ERR     *)&err);                               // 指向错误代码的指针，用于创建结果处理

}

/*******************************************************************************
 * 名    称： AppTaskComm
 * 功    能： 控制任务
 * 入口参数： p_arg - 由任务创建函数传入
 * 出口参数： 无
 * 作　 　者： wumingshen.
 * 创建日期： 2015-02-05
 * 修    改：
 * 修改日期：
 *******************************************************************************/
static  void  AppTaskComm (void *p_arg)
{
    OS_ERR      err;

    OS_TICK     dly     = CYCLE_TIME_TICKS;
    OS_TICK     ticks;
    /***********************************************
    * 描述： 任务初始化
    */
    APP_CommInit();
    
    BSP_OS_TimeDly(OS_TICKS_PER_SEC * 2);
    /***********************************************
    * 描述： 2016/01/08增加，用于非MODBBUS IAP升级通信
    */
#if defined     (IMAGE_A)
#elif defined   (IMAGE_B)
#else
    u8 status   = IAP_GetStatus();
    if ( status != IAP_STS_PROGRAMING ) 
        IAP_JumpToApp();
#endif
        
    /***********************************************
    * 描述： Task body, always written as an infinite loop.
    */
    while (DEF_TRUE) { 
        /***********************************************
        * 描述： 本任务看门狗标志置位
        */
        OS_FlagPost(( OS_FLAG_GRP *)&WdtFlagGRP,
                    ( OS_FLAGS     ) WDT_FLAG_COMM,
                    ( OS_OPT       ) OS_OPT_POST_FLAG_SET,
                    ( CPU_TS       ) 0,
                    ( OS_ERR      *) &err);        
        
        /***********************************************
        * 描述： 等待COMM操作标志位
        */
        OS_FLAGS    flags = 
        OSFlagPend( ( OS_FLAG_GRP *)&Ctrl.Os.CommEvtFlagGrp,
                    ( OS_FLAGS     ) Ctrl.Os.CommEvtFlag,
                    ( OS_TICK      ) dly,
                    ( OS_OPT       ) OS_OPT_PEND_FLAG_SET_ANY,
                    ( CPU_TS      *) NULL,
                    ( OS_ERR      *)&err);
        
        OS_ERR      terr;
        ticks   = OSTimeGet(&terr);                        // 获取当前OSTick
        
        /***********************************************
        * 描述： 串口通讯超时计数器，收到口数据时会清零，此处只增加
        */
        if ( Ctrl.Comm.ConnectTimeOut < 10 ) {
            Ctrl.Comm.ConnectTimeOut++;
        } else {
            
        }
        /***********************************************
        * 描述： 2016/01/08增加，用于非MODBBUS IAP升级通信
        */
#if MB_IAPMODBUS_EN == DEF_ENABLED
        /***********************************************
        * 描述： 如果处于升级中，3秒后没有数据退出升级
        */
        IAP_Exit(3);
#endif        
        /***********************************************
        * 描述： 没有错误,有事件发生
        */
        if ( err == OS_ERR_NONE ) {
            OS_FLAGS    flagClr = 0;
            /***********************************************
            * 描述： 心跳包发送
            */
            if       ( flags & COMM_EVT_FLAG_HEART  ) {
                flagClr |= COMM_EVT_FLAG_HEART;
                
            /***********************************************
            * 描述： COMM复位
            */
            } else if ( flags & COMM_EVT_FLAG_RESET ) {
                flagClr |= COMM_EVT_FLAG_RESET;
                
            /***********************************************
            * 描述： COMM连接
            */
            } else if ( flags & COMM_EVT_FLAG_CONNECT ) {
                flagClr |= COMM_EVT_FLAG_CONNECT;
                
            /***********************************************
            * 描述： 串口接收
            */
            } else if ( flags & COMM_EVT_FLAG_RECV ) {
                flagClr |= COMM_EVT_FLAG_RECV;
                //APP_CommRxDataDealCB(Ctrl.Comm.pch);
            /***********************************************
            * 描述： 上报消息
            */
            } else if ( flags & COMM_EVT_FLAG_REPORT ) {
                flagClr |= COMM_EVT_FLAG_REPORT;
                
            /***********************************************
            * 描述： 断开
            */
            } else if ( flags & COMM_EVT_FLAG_CLOSE ) {
                flagClr |= COMM_EVT_FLAG_CLOSE;
                
            /***********************************************
            * 描述： 超时
            */
            } else if ( flags & COMM_EVT_FLAG_TIMEOUT ) {
                flagClr |= COMM_EVT_FLAG_TIMEOUT;
                
            /***********************************************
            * 描述： 配置
            */
            } else if ( flags & COMM_EVT_FLAG_CONFIG ) {
                flagClr |= COMM_EVT_FLAG_CONFIG;
            } 
            //exit:
            /***********************************************
            * 描述： 清除标志
            */
            if ( !flagClr ) {
                flagClr = flags;
            }
            
            /***********************************************
            * 描述： 清除标志位
            */
            OSFlagPost( ( OS_FLAG_GRP  *)&Ctrl.Os.CommEvtFlagGrp,
                        ( OS_FLAGS      )flagClr,
                        ( OS_OPT        )OS_OPT_POST_FLAG_CLR,
                        ( OS_ERR       *)&err);
        /***********************************************
        * 描述： 如果超时，则发送一次心跳包
        */
        } else if ( err == OS_ERR_TIMEOUT ) {
            OSFlagPost( ( OS_FLAG_GRP  *)&Ctrl.Os.CommEvtFlagGrp,
                        ( OS_FLAGS      )Ctrl.Os.CommEvtFlag,
                        ( OS_OPT        )OS_OPT_POST_FLAG_CLR,
                        ( OS_ERR       *)&err );
        }
        
          /*************************************************
          * 描述： 发送查询命令
          */
#if MODBUS_CFG_MASTER_EN == DEF_TRUE          
          MBM_FC03_HoldingRegRd( (MODBUS_CH   *)Ctrl.Comm.pch,
                                 (CPU_INT08U   )Ctrl.Comm.pch->NodeAddr,
                                 (CPU_INT16U   )0,
                                 (CPU_INT16U  *)&Ctrl.Para.buf2[0],
                                 (CPU_INT16U   )20 );
#endif
        /***********************************************
        * 描述： 计算剩余时间
        */
        dly   = CYCLE_TIME_TICKS - ( OSTimeGet(&err) - ticks );
        if ( dly  < 1 ) {
            dly = 1;
        } else if ( dly > CYCLE_TIME_TICKS ) {
            dly = CYCLE_TIME_TICKS;
        }
    }
}

/*******************************************************************************
 * 名    称： APP_ModbusInit
 * 功    能： MODBUS初始化
 * 入口参数： 无
 * 出口参数： 无
 * 作　 　者： 无名沈
 * 创建日期： 2015-03-28
 * 修    改：
 * 修改日期：
 * 备    注： 该初始化会创建Modbus任务。
 *******************************************************************************/
void App_ModbusInit(void)
{
    MODBUS_CH   *pch;
    /***********************************************
    * 描述： uCModBus初始化，RTU时钟频率为1000HZ
    *        使用了定时器2（TIM2）
    */
    MB_Init(1000);
    /***********************************************
    * 描述： 若ModBus配置为从机（见mb_cfg.c）
    */
#if MODBUS_CFG_SLAVE_EN == DEF_TRUE
    pch         = MB_CfgCh( ModbusNode,             // ... Modbus Node # for this slave channel
                            MODBUS_SLAVE,           // ... This is a SLAVE
                            2,                      // ... 0 when a slave
                            MODBUS_MODE_RTU,        // ... Modbus Mode (_ASCII or _RTU)
                            0,                      // ... Specify UART #2
                            9600,                   // ... Baud Rate
                            USART_WordLength_8b,    // ... Number of data bits 7 or 8
                            USART_Parity_No,        // ... Parity: _NONE, _ODD or _EVEN
                            USART_StopBits_1,       // ... Number of stop bits 1 or 2
                            MODBUS_WR_EN);          // ... Enable (_EN) or disable (_DIS) writes
#endif

    /***********************************************
    * 描述： 若ModBus配置为主机（见mb_cfg.c）
    */
#if MODBUS_CFG_MASTER_EN == DEF_TRUE
    pch         = MB_CfgCh( 1,                      // ... Modbus Node # for this channel
                            MODBUS_MASTER,          // ... This is a MASTER
                            500,                    // ... One second timeout waiting for slave response
                            MODBUS_MODE_RTU,        // ... Modbus Mode (_ASCII or _RTU)
                            0,                      // ... Specify UART #3
                            9600,                   // ... Baud Rate
                            USART_WordLength_8b,    // ... Number of data bits 7 or 8
                            USART_Parity_No,        // ... Parity: _NONE, _ODD or _EVEN
                            USART_StopBits_1,       // ... Number of stop bits 1 or 2
                            MODBUS_WR_EN);          // ... Enable (_EN) or disable (_DIS) writes
#endif
    pch->AesEn          = DEF_DISABLED;             // ... AES加密禁止
    pch->NonModbusEn    = DEF_ENABLED;              // ... 支持非MODBUS通信
    pch->IapModbusEn    = DEF_ENABLED;              // ... 支持IAP MODBUS通信
    
    pch->RxFrameHead    = 0x1028;                   // ... 添加匹配帧头
    pch->RxFrameTail    = 0x102c;                   // ... 添加匹配帧尾
    
    Ctrl.Comm.pch       = pch;
}

/*******************************************************************************
 * 名    称： APP_CommInit
 * 功    能： 任务初始化
 * 入口参数： 无
 * 出口参数： 无
 * 作　 　者： wumingshen.
 * 创建日期： 2015-03-28
 * 修    改：
 * 修改日期：
 *******************************************************************************/
 void APP_CommInit(void)
{
    OS_ERR err;
    
    /***********************************************
    * 描述： 创建事件标志组
    */
    OSFlagCreate(( OS_FLAG_GRP  *)&Ctrl.Os.CommEvtFlagGrp,
                 ( CPU_CHAR     *)"App_CommFlag",
                 ( OS_FLAGS      )0,
                 ( OS_ERR       *)&err);
    
    Ctrl.Os.CommEvtFlag = COMM_EVT_FLAG_HEART       // 心跳包发送
                        + COMM_EVT_FLAG_RESET       // COMM复位
                        + COMM_EVT_FLAG_CONNECT     // COMM连接
                        + COMM_EVT_FLAG_RECV        // 串口接收
                        + COMM_EVT_FLAG_REPORT      // 串口发送
                        + COMM_EVT_FLAG_CLOSE       // 断开
                        + COMM_EVT_FLAG_TIMEOUT     // 超时
                        + COMM_EVT_FLAG_CONFIG;     // 配置
        
    /***********************************************
    * 描述： 初始化串口硬件和去服务器注册的相关内容
    */
    //BSP_UartCfg( 1, 9600);
    /***********************************************
    * 描述：创建一个软定时器 OS_OPT_TMR_ONE_SHOT, OS_OPT_TMR_PERIODIC
    *
    OSTmrCreate ((OS_TMR               *)&OSTmr1,
                 (CPU_CHAR             *)"tmr1",
                 (OS_TICK               )1,
                 (OS_TICK               )1,
                 (OS_OPT                )OS_OPT_TMR_PERIODIC,
                 (OS_TMR_CALLBACK_PTR   )OSTmr1_callback,
                 (void                 *)NULL,
                 (OS_ERR               *)&err);
    OSTmrStart ((OS_TMR  *)&OSTmr1,
                (OS_ERR  *)&err);

    *//***********************************************
    * 描述： 初始化MODBUS通信
    */        
    App_ModbusInit();
    
    /***********************************************
    * 描述： 在看门狗标志组注册本任务的看门狗标志
    */
    WdtFlags |= WDT_FLAG_COMM;
}

/*******************************************************************************
 * 名    称： APP_CommRxDataDealCB
 * 功    能： 串口数据处理回调函数，由MB_DATA.C调用
 * 入口参数： 无
 * 出口参数： 无
 * 作　 　者： 无名沈
 * 创建日期： 2016-01-04
 * 修    改：
 * 修改日期：
 * 备    注： 
 *******************************************************************************/
INT08U APP_CommRxDataDealCB(MODBUS_CH  *pch)
{
    /***********************************************
    * 描述： 2016/01/08增加，用于非MODBBUS IAP升级通信
    */
#if MB_IAPMODBUS_EN == DEF_ENABLED
    /***********************************************
    * 描述： 如果程序处在升级模式
    */
    if ( ( Iap.Status != IAP_STS_DEF ) && 
         ( Iap.Status != IAP_STS_SUCCEED ) &&
         ( Iap.Status != IAP_STS_FAILED ) ) {
        return IAP_CommRxDataDealCB(pch);
    }
#endif
    /***********************************************
    * 描述： 获取帧头
    */
    CPU_SR_ALLOC();
    CPU_CRITICAL_ENTER();
    u8  Len     = pch->RxBufByteCtr;
    memcpy( (INT08U *)&pch->RxFrameData, (INT08U *)pch->RxBuf, Len );
    CPU_CRITICAL_EXIT();
    
    INT08U  *DataBuf    = pch->RxFrameData;
    /***********************************************
    * 描述： 接收数据处理
    */
    u8  DstAddr = 
    CSNR_GetData(	(unsigned char *)pch->RxFrameData,          //RecBuf,
                    (unsigned char	 )Len,                       //RecLen, 
                    (unsigned char *)DataBuf,                   //DataBuf,
                    (unsigned short *)&Len);                     //InfoLen)

    /***********************************************
    * 描述： 如果数据长度或者地址为0，则不处理
    */
    if ( ( Len == 0 ) || ( DstAddr == 0 ) )
        return FALSE;
    
    /***********************************************
    * 描述： 获取帧头
    */
    OS_CRITICAL_ENTER();
    memcpy( (INT08U *)&Ctrl.Comm.Rd.Head, (INT08U *)pch->RxFrameData, 8 );
    OS_CRITICAL_EXIT();

    /***********************************************
    * 描述： 地址比对，不是本机地址，直接返回
    */
    if ( Ctrl.Comm.Rd.Head.DstAddr != Ctrl.Comm.SlaveAddr ) {
        return FALSE;
    }
    
    /***********************************************
    * 描述： 检测升级指令
    */
    if ( ( Len == 18) && 
         ( 0 >= memcmp((const char *)"IAP_pragram start!",(const char *)&pch->RxFrameData[8], 18) ) ) {
#if defined     (IMAGE_A) || defined   (IMAGE_B)
             
#else
        /***********************************************
        * 描述： 发送数据处理
        */
        DataSend_CSNR(	(unsigned char  ) Ctrl.Comm.SlaveAddr,         // SourceAddr,
                        (unsigned char  ) Ctrl.Comm.Rd.Head.SrcAddr,   // DistAddr,
                        (unsigned char *)&pch->RxFrameData[8],         // DataBuf,
                        (unsigned int	 ) Len);                        // DataLen 
#endif
        IAP_Reset();
        return TRUE;
    }    

    /***********************************************
    * 描述： 获取数据
    */
    OS_CRITICAL_ENTER();
    memcpy( (INT08U *)&Ctrl.Comm.Rd.Data, (INT08U *)&pch->RxFrameData[8], Len );
    OS_CRITICAL_EXIT();
    
    /***********************************************
    * 描述： 写数据
    */
    if ( ( Ctrl.Comm.Rd.Head.PacketCtrl & 0x0f ) == 0x01 ) {
        
    /***********************************************
    * 描述： 读数据
    */
    } else if ( ( Ctrl.Comm.Rd.Head.PacketCtrl & 0x0f ) == 0x00  ) {
        OS_CRITICAL_ENTER();
        
        Ctrl.Comm.Rd.Data.Oil           = SW_INT16U((Ctrl.Comm.Rd.Data.Oil>>16));
        /***********************************************
        * 描述： 将数据打包给发送结构
        */
        Ctrl.Comm.Wr.Data.Hight         = SW_INT16U(Ctrl.Sen.C.Hight);          // 高度
        Ctrl.Comm.Wr.Data.HightDiff     = (INT16U)Ctrl.Sen.C.CalcDiffHight;     // 高度差
        Ctrl.Comm.Wr.Data.Pressure[0]   = (INT16S)Ctrl.Sen.H.PressureSample;    // 高位传感器压力
        Ctrl.Comm.Wr.Data.Pressure[1]   = (INT16S)Ctrl.Sen.L.PressureSample;    // 低位传感器压力
        Ctrl.Comm.Wr.Data.Density       = (INT16U)(Ctrl.Sen.C.Density * 10000.0);//密度
        
        Ctrl.Comm.Wr.Data.AdcValue[0]   = (INT16U)(Ctrl.Sen.H.AdcValueSample);  // 高位ADC值
        Ctrl.Comm.Wr.Data.AdcValue[1]   = (INT16U)(Ctrl.Sen.L.AdcValueSample);  // 低位ADC值
        Ctrl.Comm.Wr.Data.Volume        = (INT16U)(Ctrl.Sen.C.Volume);          // 2016.04.16增加
        Ctrl.Comm.Wr.Data.Weight        = (INT16U)(Ctrl.Sen.C.Weight);          // 2016.04.16增加
        
        Ctrl.Comm.Wr.Data.CalcDensity   = (INT16S)(Ctrl.Sen.C.CalcDensity * 10000.0);
        Ctrl.Comm.Wr.Data.Gravity       = (INT16S)(Ctrl.Sen.C.Gravity * 10000.0);
        Ctrl.Comm.Wr.Data.CalcGravity   = (INT16S)(Ctrl.Sen.C.CalcGravity * 10000.0);
        
        Ctrl.Comm.Wr.Data.Sts.Dat       = Ctrl.Para.dat.SenSts.Dat[1];          // 状态标志
        
        Ctrl.Comm.Wr.Head.DataLen       = 12+14;                                 // 2016.04.16增加4字节
        OS_CRITICAL_EXIT();

        /***********************************************
        * 描述： 发送数据处理
        */
        DataSend_CSNR( (INT08U ) Ctrl.Comm.SlaveAddr,                           // SourceAddr,
                       (INT08U ) Ctrl.Comm.Rd.Head.SrcAddr,                     // DistAddr,
                       (INT08U*)&Ctrl.Comm.Wr.Data,                             // DataBuf,
                       (INT16U ) Ctrl.Comm.Wr.Head.DataLen);                    // DataLen 
    }
    
    /***********************************************
    * 描述： 串口接受COMM模组的消息并处理
    */
    Ctrl.Comm.ConnectTimeOut    = 0;                // 超时计数器清零
    Ctrl.Comm.ConnectFlag       = TRUE;             // 转连接成功标志
    
    return TRUE;
}

/***********************************************
* 描述： 2016/01/08增加，用于非MODBBUS IAP升级通信
*/
#if MB_IAPMODBUS_EN == DEF_ENABLED
/*******************************************************************************
 * 名    称： APP_CommRxDataDealCB
 * 功    能： 串口数据处理回调函数，由MB_DATA.C调用
 * 入口参数： 无
 * 出口参数： 无
 * 作　 　者： 无名沈
 * 创建日期： 2016-01-04
 * 修    改：
 * 修改日期：
 * 备    注： 
 *******************************************************************************/
INT08U IAP_CommRxDataDealCB(MODBUS_CH  *pch)
{
    CPU_SR_ALLOC();
    CPU_CRITICAL_ENTER();
    u8  Len     = pch->RxBufByteCtr;
    memcpy( (INT08U *)&pch->RxFrameData, (INT08U *)pch->RxBuf, Len );
    CPU_CRITICAL_EXIT();
    
    /***********************************************
    * 描述： 获取帧头
    */    
    INT08U  *DataBuf    = pch->RxFrameData;
    
    /***********************************************
    * 描述： 接收数据处理
    */
    u8  DstAddr = 
    CSNR_GetData(	(unsigned char *)pch->RxFrameData,          //RecBuf,
                    (unsigned char	 )Len,                       //RecLen, 
                    (unsigned char *)DataBuf,                   //DataBuf,
                    (unsigned short *)&Len);                     //InfoLen)
    /***********************************************
    * 描述： 检测接收结束
    */
    if ( ( Len == 16) && 
         ( 0 >= memcmp((const char *)"IAP_pragram end!",(const char *)&pch->RxFrameData[8], 16) ) ) {
        /***********************************************
        * 描述： 发送数据处理
        */
        DataSend_CSNR(	(unsigned char  ) Ctrl.Comm.SlaveAddr,          // SourceAddr,
                        (unsigned char  ) Ctrl.Comm.Rd.Head.SrcAddr,    // DistAddr,
                        (unsigned char *)&pch->RxFrameData[8],          // DataBuf,
                        (unsigned short ) Len); 
        
        IAP_Finish();
        return TRUE;
    }
    
    if ( ( Len == 18) && 
         ( 0 >= memcmp((const char *)"IAP_pragram start!",(const char *)&pch->RxFrameData[8], 18) ) ) {
        
#if defined     (IMAGE_A) || defined   (IMAGE_B)
#else
        /***********************************************
        * 描述： 发送数据处理
        */
        DataSend_CSNR(	(unsigned char  ) Ctrl.Comm.SlaveAddr,          // SourceAddr,
                        (unsigned char  ) Ctrl.Comm.Rd.Head.SrcAddr,    // DistAddr,
                        (unsigned char *)&pch->RxFrameData[8],            // DataBuf,
                        (unsigned int	 ) Len);   // DataLen 
#endif
        IAP_Reset();
        return TRUE;
    }
    /***********************************************
    * 描述： 写数据
    */
    /***********************************************
    * 描述： 将数据打包给发送结构
    */
    Ctrl.Comm.Wr.Head.DataLen       = 0;
    /***********************************************
    * 描述： 写数据到Flash
    */
    IAP_Program((StrIapState *)&Iap, (INT16U *)&pch->RxFrameData[8], Len );    
    /***********************************************
    * 描述： 编辑应答内容
    */
    char str[20];
    if ( Len < 128 ) {
        usprintf(str,"\n%d\nFinished\n",Iap.FrameIdx);
    } else {
        usprintf(str,"\n%d",Iap.FrameIdx);
    }
    str[19]  = 0;
    /***********************************************
    * 描述： 发送数据处理
    */
    DataSend_CSNR(	(unsigned char  ) Ctrl.Comm.SlaveAddr,                    // SourceAddr,
                    (unsigned char  ) Ctrl.Comm.Rd.Head.SrcAddr,              // DistAddr,
                    (unsigned char *) str,                                    // DataBuf,
                    (unsigned short ) strlen(str));                            // DataLen 
    if ( Len < 128 ) {
        BSP_OS_TimeDly(100);
        IAP_Finish();
    } else {
        IAP_Programing();                           // 复位超时计数器
    }
    /***********************************************
    * 描述： 串口接受COMM模组的消息并处理
    */
    Ctrl.Comm.ConnectTimeOut    = 0;                // 超时计数器清零
    Ctrl.Comm.ConnectFlag       = FALSE;            // 转连接成功标志
    
    return TRUE;
}
#endif
/*******************************************************************************
 * 名    称： OSTmr1_callback
 * 功    能： 软定时器1的回调函数
 * 入口参数： 无
 * 出口参数： 无
 * 作　 　者： 无名沈
 * 创建日期： 2015-03-28
 * 修    改：
 * 修改日期：
 * 备    注： 定时器回调函数不能使用延时函数
 *******************************************************************************/
void OSTmr1_callback(OS_TMR *ptmr,void *p_arg)
{

}


/*******************************************************************************
 * 				end of file
 *******************************************************************************/
#endif