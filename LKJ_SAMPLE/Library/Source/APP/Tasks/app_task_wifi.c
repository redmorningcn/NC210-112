/*******************************************************************************
 *   Filename:       app_task_wifi.c
 *   Revised:        All copyrights reserved to Roger-WY.
 *   Revision:       v1.0
 *   Writer:	     Roger-WY.
 *
 *   Description:    双击选中 wifi 按 Ctrl + H, 钩选 Match the case, Replace with
 *                   输入您要的名字，点击 Replace All
 *                   双击选中 Wifi 按 Ctrl + H, 钩选 Match the case, Replace with
 *                   输入您要的名字，点击 Replace All
 *                   双击选中 WIFI 按 Ctrl + H, 钩选 Match the case, Replace with
 *                   输入您要的名字，点击 Replace All
 *                   在 app_cfg.h 中指定本任务的 优先级  （ APP_TASK_WIFI_PRIO     ）
 *                                            和 任务堆栈（ APP_TASK_WIFI_STK_SIZE ）大小
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

#ifdef VSC_INCLUDE_SOURCE_FILE_NAMES
const  CPU_CHAR  *app_task_comm__c = "$Id: $";
#endif

#define APP_TASK_WIFI_EN     DEF_DISABLED
#if APP_TASK_WIFI_EN == DEF_ENABLED
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
static  OS_TCB   AppTaskWifiTCB;

/***********************************************
* 描述： 任务堆栈（STACKS）
*/
static  CPU_STK  AppTaskWifiStk[ APP_TASK_WIFI_STK_SIZE ];

/*******************************************************************************
 * LOCAL VARIABLES
 */

/*******************************************************************************
 * GLOBAL VARIABLES
 */

/***************************************************
* 描述： 软定时器声明
*/
OS_TMR          OSTmr1;                     //定时器1

/***********************************************
* 描述： 软定时器回调函数
*/
void     OSTmr1_callback         (OS_TMR *ptmr,void *p_arg);

/*******************************************************************************
 * LOCAL FUNCTIONS
 */
static  void    AppTaskWifi          (void *p_arg);
static  void    APP_WifiInit         (void);
static  void    App_McuStatusInit           (void);
static  void    ReportDevStatusHandle       (void);
static  void    InformWifiConfigMode        (u8 mode);
/*******************************************************************************
 * GLOBAL FUNCTIONS
 */
//UART_HandleTypeDef         				UartHandle;
//Pro_Wait_AckTypeDef           	        Ctrl.Wifi.WaitAck;
////Device_WirteTypeDef   					Device_WirteStruct;
////Device_ReadTypeDef                      Device_ReadStruct;
//
//Pro_M2W_ReturnInfoTypeDef  				Ctrl.Wifi.M2wRetInfo;
//Pro_CommonCmdTypeDef      			 	Ctrl.Wifi.CommCmd;
//Pro_W2D_WifiStatusTypeDef     	 	    Ctrl.Wifi.W2dWifiSts;
//Pro_CommonCmdTypeDef     	 			Ctrl.Wifi.M2wRstCmd;
//Pro_D2W_ConfigWifiTypeDef       	    Ctrl.Wifi.D2wCfgCmd;
//Pro_D2W_ReportDevStatusTypeDef   	    Ctrl.Wifi.D2wRptSts;
//
//AttrCtrlTypeDef                         AttrCtrl;
//uint8_t									SN = 0;
//uint8_t                                 Set_LedStatus = 0;


/*******************************************************************************
 * EXTERN VARIABLES
 */

 /*******************************************************************************
 * EXTERN FUNCTIONS
 */

/*******************************************************************************/

/*******************************************************************************
 * 名    称： App_TaskWifiCreate
 * 功    能： **任务创建
 * 入口参数： 无
 * 出口参数： 无
 * 作　　者： Roger-WY.
 * 创建日期： 2015-02-05
 * 修    改：
 * 修改日期：
 * 备    注： 任务创建函数需要在app.h文件中声明
 *******************************************************************************/
void  App_TaskWifiCreate(void)
{
    OS_ERR  err;

    /***********************************************
    * 描述： 任务创建
    */
    OSTaskCreate((OS_TCB     *)&AppTaskWifiTCB,              // 任务控制块  （当前文件中定义）
                 (CPU_CHAR   *)"App Task Wifi",              // 任务名称
                 (OS_TASK_PTR ) AppTaskWifi,                 // 任务函数指针（当前文件中定义）
                 (void       *) 0,                                  // 任务函数参数
                 (OS_PRIO     ) APP_TASK_WIFI_PRIO,          // 任务优先级，不同任务优先级可以相同，0 < 优先级 < OS_CFG_PRIO_MAX - 2（app_cfg.h中定义）
                 (CPU_STK    *)&AppTaskWifiStk[0],           // 任务栈顶
                 (CPU_STK_SIZE) APP_TASK_WIFI_STK_SIZE / 10, // 任务栈溢出报警值
                 (CPU_STK_SIZE) APP_TASK_WIFI_STK_SIZE,      // 任务栈大小（CPU数据宽度 * 8 * size = 4 * 8 * size(字节)）（app_cfg.h中定义）
                 (OS_MSG_QTY  ) 5u,                                 // 可以发送给任务的最大消息队列数量
                 (OS_TICK     ) 0u,                                 // 相同优先级任务的轮循时间（ms），0为默认
                 (void       *) 0,                                  // 是一个指向它被用作一个TCB扩展用户提供的存储器位置
                 (OS_OPT      )(OS_OPT_TASK_STK_CHK |               // 允许堆栈检查该任务
                                OS_OPT_TASK_STK_CLR),               // 创建任务时堆栈清零
                 (OS_ERR     *)&err);                               // 指向错误代码的指针，用于创建结果处理

}

/*******************************************************************************
 * 名    称： AppTaskWifi
 * 功    能： 控制任务
 * 入口参数： p_arg - 由任务创建函数传入
 * 出口参数： 无
 * 作　　者： Roger-WY.
 * 创建日期： 2015-02-05
 * 修    改：
 * 修改日期：
 *******************************************************************************/
static  void  AppTaskWifi (void *p_arg)
{
    OS_ERR      err;

    OS_TICK     dly;
    OS_TICK     ticks;
    /***********************************************
    * 描述： 任务初始化
    */
    APP_WifiInit();
    /***********************************************
    * 描述： Task body, always written as an infinite loop.
    */
    while (DEF_TRUE) { 
        /***********************************************
        * 描述： 本任务看门狗标志置位
        */
        OS_FlagPost(( OS_FLAG_GRP *)&WdtFlagGRP,
                    ( OS_FLAGS     ) WDTFLAG_Wifi,
                    ( OS_OPT       ) OS_OPT_POST_FLAG_SET,
                    ( CPU_TS       ) 0,
                    ( OS_ERR      *) &err);        
        
        /***********************************************
        * 描述： 等待WIFI操作标志位
        */
        OS_FLAGS    flags = 
        OSFlagPend( ( OS_FLAG_GRP *)&Ctrl.Os.WifiEvtFlagGrp,
                    ( OS_FLAGS     ) Ctrl.Os.WifiEvtFlag,
                    ( OS_TICK      ) dly,                                       //超时等待PendTimeOutMS ms
                    ( OS_OPT       ) OS_OPT_PEND_FLAG_SET_ANY,                  //任何一位置一
                    ( CPU_TS      *) NULL,
                    ( OS_ERR      *)&err);
        
        OS_ERR      terr;
        ticks   = OSTimeGet(&terr);                        // 获取当前OSTick
        
        /***********************************************
        * 描述： 没有错误,有事件发生
        */
        if ( err == OS_ERR_NONE ) {
            OS_FLAGS    flagClr = 0;
            /***********************************************
            * 描述： 心跳包发送
            */
            if       ( flags & WIFI_EVT_FLAG_HEART  ) {
                flagClr |= WIFI_EVT_FLAG_HEART;
                
            /***********************************************
            * 描述： WIFI复位
            */
            } else if ( flags & WIFI_EVT_FLAG_RESET ) {
                flagClr |= WIFI_EVT_FLAG_RESET;
                GPIO_InitTypeDef GPIO_InitStructure;
                /**************************************************
                * 描述： 复位端口设为输出
                */
                RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB,ENABLE);
                GPIO_InitStructure.GPIO_Pin = KEY_GPIO_NRESET;
                GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
                
                GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
                GPIO_Init(KEY_PORT, &GPIO_InitStructure);
                /**************************************************
                * 描述： 拉低WIFI模块复位引脚
                */
                GPIO_ResetBits(GPIOB,GPIO_Pin_14);
                BSP_OS_TimeDly(OS_TICKS_PER_SEC / 50);
                GPIO_SetBits(GPIOB,GPIO_Pin_14);
                
                /**************************************************
                * 描述： 复位端口设为输入
                */
                GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
                GPIO_Init(KEY_PORT, &GPIO_InitStructure);    
            /***********************************************
            * 描述： WIFI连接
            */
            } else if ( flags & WIFI_EVT_FLAG_CONNECT ) {
                flagClr |= WIFI_EVT_FLAG_CONNECT;
                
            /***********************************************
            * 描述： 串口接收
            */
            } else if ( flags & WIFI_EVT_FLAG_RECV ) {
                flagClr |= WIFI_EVT_FLAG_RECV;
                /***********************************************
                * 描述： 串口接受WIFI模组的消息并处理
                */
                Ctrl.Wifi.TimeOut    = 0;                    // 超时计数器清零
                MessageHandle();
            /***********************************************
            * 描述： 上报消息
            */
            } else if ( flags & WIFI_EVT_FLAG_REPORT ) {
                flagClr |= WIFI_EVT_FLAG_REPORT;
                /***********************************************
                * 描述： 定时向WIFI模组主动上报设备状态
                */
                ReportDevStatusHandle();
            /***********************************************
            * 描述： 断开
            */
            } else if ( flags & WIFI_EVT_FLAG_CLOSE ) {
                flagClr |= WIFI_EVT_FLAG_CLOSE;
                
            /***********************************************
            * 描述： 超时
            */
            } else if ( flags & WIFI_EVT_FLAG_TIMEOUT ) {
                flagClr |= WIFI_EVT_FLAG_TIMEOUT;
                
            /***********************************************
            * 描述： 配置
            */
            } else if ( flags & WIFI_EVT_FLAG_CONFIG ) {
                flagClr |= WIFI_EVT_FLAG_CONFIG;
                /***********************************************
                * 描述： 通知WIFI进入配置模式
                */
                InformWifiConfigMode(AirLinkMethod);
                //InformWifiConfigMode(SoftApMethod); 
            } 
            
            if ( !flagClr ) {
                flagClr = flags;
            }
            
            /***********************************************
            * 描述： 清除标志位
            */
            OSFlagPost( ( OS_FLAG_GRP  *)&Ctrl.Os.WifiEvtFlagGrp,
                        ( OS_FLAGS      )flagClr,
                        ( OS_OPT        )OS_OPT_POST_FLAG_CLR,
                        ( OS_ERR       *)&err);
        /***********************************************
        * 描述： 如果超时，则发送一次心跳包
        */
        } else if ( err == OS_ERR_TIMEOUT ) {
            OSFlagPost( ( OS_FLAG_GRP  *)&Ctrl.Os.WifiEvtFlagGrp,
                        ( OS_FLAGS      )Ctrl.Os.WifiEvtFlag,
                        ( OS_OPT        )OS_OPT_POST_FLAG_CLR,
                        ( OS_ERR       *)&err);
            
            Ctrl.Wifi.UartHandle.Package_Flag  = 0;
            if ( ++Ctrl.Wifi.TimeOut > 180 ) {
                Ctrl.Wifi.TimeOut    = 170;
                OSFlagPost(( OS_FLAG_GRP  *)&Ctrl.Os.WifiEvtFlagGrp,
                           ( OS_FLAGS      )WIFI_EVT_FLAG_RESET,
                           ( OS_OPT        )OS_OPT_POST_FLAG_SET,
                           ( OS_ERR       *)&err);
            }
        }
        
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
 * 名    称： APP_WifiInit
 * 功    能： 任务初始化
 * 入口参数： 无
 * 出口参数： 无
 * 作　　者： Roger-WY.
 * 创建日期： 2015-03-28
 * 修    改：
 * 修改日期：
 *******************************************************************************/
static void APP_WifiInit(void)
{
    OS_ERR err;
    /***********************************************
    * 描述： 初始化串口硬件和去服务器注册的相关内容
    */
    BSP_UartCfg( 2, 9600);
    App_McuStatusInit();

    /***********************************************
    * 描述： 初始化汉枫监测模块输入引脚
    */
    GPIO_InitTypeDef GPIO_InitStructure;
    /* Enable FSMC, GPIOD, GPIOE, GPIOF, GPIOG and AFIO clocks */
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB |
                           RCC_APB2Periph_AFIO,ENABLE);

    /***********************************************
    * 描述： 禁止串行Jtag 调试端口 SWJ-DP
    *        STM32中PA.13,PA.14,PA.15,PB.03,PB.04是默认的JTAG引脚映射
    */
    //#define GPIO_Remap_SWJ_NoJTRST      ((uint32_t)0x00300100)  /*!< Full SWJ Enabled (JTAG-DP + SW-DP) but without JTRST */
    //#define GPIO_Remap_SWJ_JTAGDisable  ((uint32_t)0x00300200)  /*!< JTAG-DP Disabled and SW-DP Enabled */
    //#define GPIO_Remap_SWJ_Disable      ((uint32_t)0x00300400)  /*!< Full SWJ Disabled (JTAG-DP + SW-DP) */

    /***********************************************
    * 描述： 改变指定管脚的映射 GPIO_Remap_SWJ_Disable SWJ 完全启用（JTAG+SW-DP），但复位禁止
    */
    //GPIO_PinRemapConfig(GPIO_Remap_SWJ_NoJTRST, ENABLE);
    /***********************************************
    * 描述： 改变指定管脚的映射 GPIO_Remap_SWJ_Disable SWJ 完全禁用（JTAG+SW-DP）
    */
    //GPIO_PinRemapConfig(GPIO_Remap_SWJ_Disable, ENABLE);
    /***********************************************
    * 描述： 改变指定管脚的映射 GPIO_Remap_SWJ_JTAGDisable ，JTAG-DP 禁用 + SW-DP 使能
    */
    GPIO_PinRemapConfig(GPIO_Remap_SWJ_JTAGDisable , ENABLE);
    /***********************************************
    * 描述： 禁止异步跟踪，释放PB3
    */
    #define DBGMCU_CR    (*((volatile unsigned long *)0xE0042004))    //在顶部加上宏定义
    DBGMCU_CR &= 0xFFFFFFDF;

    /* Set PB.3 ~ PB.6 GPIO_Mode_IPU*/
    GPIO_InitStructure.GPIO_Pin     = GPIO_Pin_0
                                    | GPIO_Pin_1
                                    | GPIO_Pin_2
                                    | GPIO_Pin_3
                                    | GPIO_Pin_4
                                    | GPIO_Pin_5;
    GPIO_InitStructure.GPIO_Speed   = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode    = GPIO_Mode_IPU;
	GPIO_Init(GPIOB, &GPIO_InitStructure);

    /***********************************************
    * 描述： 创建蓝牙事件标志组
    */
    OSFlagCreate(( OS_FLAG_GRP  *)&Ctrl.Os.WifiEvtFlagGrp,
                 ( CPU_CHAR     *)"App_WifiFlag",
                 ( OS_FLAGS      )0,
                 ( OS_ERR       *)&err);
    
    Ctrl.Os.WifiEvtFlag = WIFI_EVT_FLAG_HEART               // 心跳包发送
                        + WIFI_EVT_FLAG_RESET               // WIFI复位
                        + WIFI_EVT_FLAG_CONNECT             // WIFI连接
                        + WIFI_EVT_FLAG_RECV                // 串口接收
                        + WIFI_EVT_FLAG_REPORT              // 串口发送
                        + WIFI_EVT_FLAG_CLOSE               // 断开
                        + WIFI_EVT_FLAG_TIMEOUT             // 超时
                        + WIFI_EVT_FLAG_CONFIG;             // 配置
        
    /***********************************************
    * 描述：创建一个软定时器 OS_OPT_TMR_ONE_SHOT, OS_OPT_TMR_PERIODIC
    */
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
    /***********************************************
    * 描述： 在看门狗标志组注册本任务的看门狗标志
    */
    WdtFlags |= WDTFLAG_Wifi;
}

/*******************************************************************************
 * 名    称： App_McuStatusInit
 * 功    能： 任务初始化
 * 入口参数： 无
 * 出口参数： 无
 * 作　　者： Roger-WY.
 * 创建日期： 2015-03-28
 * 修    改：
 * 修改日期：
 * 备    注：
 *******************************************************************************/
static void App_McuStatusInit(void)
{
    /***********************************************
    * 描述： 初始化串口结构体
    */
	memset(&Ctrl.Wifi.UartHandle, 0, sizeof(Ctrl.Wifi.UartHandle));

	/***********************************************
    * 描述： 初始化通用命令处理结构体
    */
	memset(&Ctrl.Wifi.CommCmd, 0, sizeof(Ctrl.Wifi.CommCmd));
	Ctrl.Wifi.CommCmd.Pro_HeadPart.Head[0] = 0XFF;
	Ctrl.Wifi.CommCmd.Pro_HeadPart.Head[1] = 0XFF;

    /***********************************************
    * 描述： 初始化回复设备信息结构体
    */
	memset(&Ctrl.Wifi.M2wRetInfo, 0, sizeof(Ctrl.Wifi.M2wRetInfo));
	Ctrl.Wifi.M2wRetInfo.Pro_HeadPart.Head[0] = 0XFF;
	Ctrl.Wifi.M2wRetInfo.Pro_HeadPart.Head[1] = 0XFF;
	Ctrl.Wifi.M2wRetInfo.Pro_HeadPart.Len = exchangeBytes(sizeof(Ctrl.Wifi.M2wRetInfo) - 4);
	Ctrl.Wifi.M2wRetInfo.Pro_HeadPart.Cmd = Pro_D2W__GetDeviceInfo_Ack_Cmd;
	memcpy(Ctrl.Wifi.M2wRetInfo.Pro_ver, PRO_VER, strlen(PRO_VER));
	memcpy(Ctrl.Wifi.M2wRetInfo.P0_ver, P0_VER, strlen(P0_VER));
	memcpy(Ctrl.Wifi.M2wRetInfo.Hard_ver, HARD_VER, strlen(HARD_VER));
	memcpy(Ctrl.Wifi.M2wRetInfo.Soft_ver, SOFT_VER, strlen(SOFT_VER));
	memcpy(Ctrl.Wifi.M2wRetInfo.Product_Key, PRODUCT_KEY, strlen(PRODUCT_KEY));
	Ctrl.Wifi.M2wRetInfo.Binable_Time = exchangeBytes(0);

	/***********************************************
    * 描述： 初始化上报状态结构体
    */
	memset(&Ctrl.Wifi.D2wRptSts, 0, sizeof(Ctrl.Wifi.D2wRptSts));
	Ctrl.Wifi.D2wRptSts.Pro_HeadPartP0Cmd.Pro_HeadPart.Head[0] = 0XFF;
	Ctrl.Wifi.D2wRptSts.Pro_HeadPartP0Cmd.Pro_HeadPart.Head[1] = 0XFF;
	Ctrl.Wifi.D2wRptSts.Pro_HeadPartP0Cmd.Pro_HeadPart.Len = exchangeBytes(sizeof(Ctrl.Wifi.D2wRptSts) - 4);
	Ctrl.Wifi.D2wRptSts.Pro_HeadPartP0Cmd.Pro_HeadPart.Cmd = Pro_D2W_P0_Cmd;

	/***********************************************
    * 描述： 初始化重启命令结构体
    */
	memset(&Ctrl.Wifi.M2wRstCmd, 0, sizeof(Ctrl.Wifi.M2wRstCmd));
	Ctrl.Wifi.M2wRstCmd.Pro_HeadPart.Head[0] = 0XFF;
	Ctrl.Wifi.M2wRstCmd.Pro_HeadPart.Head[1] = 0XFF;
	Ctrl.Wifi.M2wRstCmd.Pro_HeadPart.Len = exchangeBytes(sizeof(Ctrl.Wifi.M2wRstCmd) - 4);
	Ctrl.Wifi.M2wRstCmd.Pro_HeadPart.Cmd = Pro_D2W_ResetWifi_Cmd;

	/***********************************************
    * 描述： 初始化设备通知WIFI模组进入配置模式结构体
    */
	memset(&Ctrl.Wifi.D2wCfgCmd, 0, sizeof(Ctrl.Wifi.D2wCfgCmd));
	Ctrl.Wifi.D2wCfgCmd.Pro_HeadPart.Head[0] = 0XFF;
	Ctrl.Wifi.D2wCfgCmd.Pro_HeadPart.Head[1] = 0XFF;
	Ctrl.Wifi.D2wCfgCmd.Pro_HeadPart.Len = exchangeBytes(sizeof(Ctrl.Wifi.D2wCfgCmd) - 4);
	Ctrl.Wifi.D2wCfgCmd.Pro_HeadPart.Cmd = Pro_D2W_ControlWifi_Config_Cmd;

	/***********************************************
    * 描述： 初始化写命令结构体
    */
	memset(&Ctrl.Wifi.Wr, 0, sizeof(Ctrl.Wifi.Wr));

	/***********************************************
    * 描述： 初始化读命令结构体
    */
	memset(&Ctrl.Wifi.Rd, 0, sizeof(Ctrl.Wifi.Rd));
}

/*******************************************************************************
 * 名    称： InformWifiConfigMode
 * 功    能： 通知WIFI模组进入配置模式
 * 入口参数： 无
 * 出口参数： 无
 * 作　　者： Roger-WY.
 * 创建日期： 2015-03-28
 * 修    改：
 * 修改日期：
 * 备    注： //01为SoftAP配置模式、02 为Air Link配置模式、其他值为保留值
 *******************************************************************************/
OPTIMIZE_NONE static void InformWifiConfigMode(u8 mode)
{
    Ctrl.Wifi.D2wCfgCmd.Pro_HeadPart.SN = Ctrl.Wifi.SN++;

    //Ctrl.Wifi.D2wCfgCmd.Config_Method = SoftApMethod; // AirLinkMethod;
    Ctrl.Wifi.D2wCfgCmd.Config_Method = mode;//AirLinkMethod;
    Ctrl.Wifi.D2wCfgCmd.Sum = CheckSum((uint8_t *)&Ctrl.Wifi.D2wCfgCmd, sizeof(Ctrl.Wifi.D2wCfgCmd));
    Pro_UART_SendBuf((uint8_t *)&Ctrl.Wifi.D2wCfgCmd,sizeof(Ctrl.Wifi.D2wCfgCmd), 0);
}

/*******************************************************************************
 * 名    称： ReportDevStatusHandle
 * 功    能： 主动上班设备状态
 * 入口参数： 无
 * 出口参数： 无
 * 作　　者： Roger-WY.
 * 创建日期： 2015-03-28
 * 修    改：
 * 修改日期：
 * 备    注：
 *******************************************************************************/
OPTIMIZE_NONE static  void ReportDevStatusHandle (void)
{
    Pro_D2W_ReportDevStatusHandle(1);
}

/*******************************************************************************
 * 名    称： OSTmr1_callback
 * 功    能： 软定时器1的回调函数
 * 入口参数： 无
 * 出口参数： 无
 * 作　　者： 无名沈
 * 创建日期： 2015-03-28
 * 修    改：
 * 修改日期：
 * 备    注： 定时器回调函数不能使用延时函数
 *******************************************************************************/
void OSTmr1_callback(OS_TMR *ptmr,void *p_arg)
{
    OS_ERR err;
    static  uint16_t ReportTimeCount = 0;
    /***********************************************
    * 描述：注意：回调函数中执行的时间不宜过长
    */
    ReportTimeCount++;
    Ctrl.Wifi.WaitAck.SendTime++;
    
	if(ReportTimeCount >= 3000) {
		ReportTimeCount = 0;
        OSFlagPost(( OS_FLAG_GRP  *)&Ctrl.Os.WifiEvtFlagGrp,
                   ( OS_FLAGS      )WIFI_EVT_FLAG_REPORT,
                   ( OS_OPT        )OS_OPT_POST_FLAG_SET,
                   ( OS_ERR       *)&err);
	}
    
    /***********************************************
    * 描述： 串口接收超时计数器
    */
    if ( Ctrl.Wifi.UartHandle.UART_RecvFlag == 1 ) {   // 接收到数据
        Ctrl.Wifi.UartHandle.UART_RecvFlag = 0;
        if( ++Ctrl.Wifi.UartHandle.UART_RecvTimeoutCtr > 50 ) {
            Ctrl.Wifi.UartHandle.Package_Flag  = 0;
            Ctrl.Wifi.UartHandle.Package_Flag  = 0;
            Ctrl.Wifi.UartHandle.UART_Count    = 0;
            Ctrl.Wifi.UartHandle.UART_Flag1    = 0;
            Ctrl.Wifi.UartHandle.UART_Flag2    = 0;
            
            memset(&Ctrl.Wifi.UartHandle.UART_Buf, 
                   0, 
                   sizeof(Ctrl.Wifi.UartHandle.Message_Buf));
            
        }
    } else {
    }
}


/*******************************************************************************
 * 				end of file
 *******************************************************************************/
#endif