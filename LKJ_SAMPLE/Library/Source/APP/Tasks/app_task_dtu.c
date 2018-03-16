/*******************************************************************************
 *   Filename:       app_task_dtu.c
 *   Revised:        All copyrights reserved to wumingshen.
 *   Revision:       v1.0
 *   Writer:	     wumingshen.
 *
 *   Description:    双击选中 dtu 按 Ctrl + H, 钩选 Match the case, Replace with
 *                   输入您要的名字，点击 Replace All
 *                   双击选中 Dtu 按 Ctrl + H, 钩选 Match the case, Replace with
 *                   输入您要的名字，点击 Replace All
 *                   双击选中 DTU 按 Ctrl + H, 钩选 Match the case, Replace with
 *                   输入您要的名字，点击 Replace All
 *                   在 app_cfg.h 中指定本任务的 优先级  （ APP_TASK_DTU_PRIO     ）
 *                                            和 任务堆栈（ APP_TASK_DTU_STK_SIZE ）大小
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
#include <xprintf.h>
#ifdef PHOTOELECTRIC_VELOCITY_MEASUREMENT
#include <Speed_sensor.h>
#include "app_task_mater.h"
#else
#include <power_macro.h>
#endif
#include <mx25.h>
#include <FM24CL64.h>
#include <crccheck.h>

#ifdef VSC_INCLUDE_SOURCE_FILE_NAMES
const  CPU_CHAR  *app_task_dtu__c = "$Id: $";
#endif
#ifdef SAMPLE_BOARD
#define APP_TASK_DTU_EN     DEF_DISABLED
#else
#define APP_TASK_DTU_EN     DEF_ENABLED
#endif
OS_Q                DTU_RxQ;

#if APP_TASK_DTU_EN == DEF_ENABLED
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
static  OS_TCB      AppTaskDtuTCB;

/***********************************************
* 描述： 任务堆栈（STACKS）
*/
static  CPU_STK  AppTaskDtuStk[ APP_TASK_DTU_STK_SIZE ];

/*******************************************************************************
 * LOCAL VARIABLES
 */

/*******************************************************************************
 * GLOBAL VARIABLES
 */

/*******************************************************************************
 * LOCAL FUNCTIONS
 */
static void    AppTaskDtu                 (void *p_arg);
static void    APP_DtuInit                (void);
       void     App_McuStatusInit          (void);
       void     ReportDevStatusHandle      (void);
       void     InformDtuConfigMode        (u8 mode);
       
       void     App_SendDataFromHistory     (void);

INT08U          APP_DtuRxDataDealCB        (MODBUS_CH  *pch);
INT08U          IAP_DtuRxDataDealCB        (MODBUS_CH  *pch);
/*******************************************************************************
 * GLOBAL FUNCTIONS
 */

/*******************************************************************************
 * EXTERN VARIABLES
 */

 /*******************************************************************************
 * EXTERN FUNCTIONS
 */
extern void     uartprintf              (MODBUS_CH  *pch,const char *fmt, ...);

/*******************************************************************************/

/*******************************************************************************
 * 名    称： App_TaskDtuCreate
 * 功    能： **任务创建
 * 入口参数： 无
 * 出口参数： 无
 * 作    者： wumingshen.
 * 创建日期： 2015-02-05
 * 修    改：
 * 修改日期：
 * 备    注： 任务创建函数需要在app.h文件中声明
 *******************************************************************************/
void  App_TaskDtuCreate(void)
{
    OS_ERR  err;

    /***********************************************
    * 描述： 任务创建
    */
    OSTaskCreate((OS_TCB     *)&AppTaskDtuTCB,                      // 任务控制块  （当前文件中定义）
                 (CPU_CHAR   *)"App Task Dtu",                      // 任务名称
                 (OS_TASK_PTR ) AppTaskDtu,                         // 任务函数指针（当前文件中定义）
                 (void       *) 0,                                  // 任务函数参数
                 (OS_PRIO     ) APP_TASK_DTU_PRIO,                 // 任务优先级，不同任务优先级可以相同，0 < 优先级 < OS_CFG_PRIO_MAX - 2（app_cfg.h中定义）
                 (CPU_STK    *)&AppTaskDtuStk[0],                   // 任务栈顶
                 (CPU_STK_SIZE) APP_TASK_DTU_STK_SIZE / 10,         // 任务栈溢出报警值
                 (CPU_STK_SIZE) APP_TASK_DTU_STK_SIZE,              // 任务栈大小（CPU数据宽度 * 8 * size = 4 * 8 * size(字节)）（app_cfg.h中定义）
                 (OS_MSG_QTY  ) 0u,                                 // 可以发送给任务的最大消息队列数量
                 (OS_TICK     ) 0u,                                 // 相同优先级任务的轮循时间（ms），0为默认
                 (void       *) 0,                                  // 是一个指向它被用作一个TCB扩展用户提供的存储器位置
                 (OS_OPT      )(OS_OPT_TASK_STK_CHK |               // 允许堆栈检查该任务
                                OS_OPT_TASK_STK_CLR),               // 创建任务时堆栈清零
                 (OS_ERR     *)&err);                               // 指向错误代码的指针，用于创建结果处理

}

/*******************************************************************************
 * 名    称： AppTaskDtu
 * 功    能： 控制任务
 * 入口参数： p_arg - 由任务创建函数传入
 * 出口参数： 无
 * 作    者： wumingshen.
 * 创建日期： 2015-02-05
 * 修    改：
 * 修改日期：
 *******************************************************************************/
static  void  AppTaskDtu (void *p_arg)
{
    OS_ERR      err;

    OS_TICK     dly     = CYCLE_TIME_TICKS;
    OS_TICK     ticks;
        
    APP_DtuInit();
    
    BSP_OS_TimeDly(OS_TICKS_PER_SEC / 2);
    /***********************************************
    * 描述： Task body, always written as an infinite loop.
    */
    while (DEF_TRUE) { 
        /***********************************************
        * 描述： 本任务看门狗标志置位
        */
        OS_FlagPost(( OS_FLAG_GRP *)&WdtFlagGRP,
                    ( OS_FLAGS     ) WDT_FLAG_DTU,
                    ( OS_OPT       ) OS_OPT_POST_FLAG_SET,
                    ( CPU_TS       ) 0,
                    ( OS_ERR      *) &err);
        
        /***********************************************
        * 描述： 等待DTU数据接收消息队列
        *
        OS_MSG_SIZE p_msg_size;
        
        MODBUS_CH *pch = 
       (MODBUS_CH *)OSQPend ((OS_Q*)&DTU_RxQ,
                    (OS_TICK       )dly,
                    (OS_OPT        )OS_OPT_PEND_BLOCKING,//OS_OPT_PEND_NON_BLOCKING,
                    (OS_MSG_SIZE  *)&p_msg_size,
                    (CPU_TS       *)0,
                    (OS_ERR       *)&err);
        // DTU收到消息
        if ( OS_ERR_NONE == err ) {
            // 消息处理
            APP_DtuRxDataDealCB(pch);
        }
        *//***********************************************
        * 描述： 等待DTU操作标志位
        */
        OS_FLAGS    flags = 
        OSFlagPend( ( OS_FLAG_GRP *)&Ctrl.Os.DtuEvtFlagGrp,
                    ( OS_FLAGS     ) Ctrl.Os.DtuEvtFlag,
                    ( OS_TICK      ) dly,
                    ( OS_OPT       ) OS_OPT_PEND_FLAG_SET_ANY,
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
            if       ( flags & COMM_EVT_FLAG_HEART  ) {
                flagClr |= COMM_EVT_FLAG_HEART;
            
            /***********************************************
            * 描述： DTU复位
            */
            } 
            if ( flags & COMM_EVT_FLAG_RESET ) {
                flagClr |= COMM_EVT_FLAG_RESET;
                
            /***********************************************
            * 描述： DTU连接
            */
            } 
            if ( flags & COMM_EVT_FLAG_CONNECT ) {
                flagClr |= COMM_EVT_FLAG_CONNECT;
                
            /***********************************************
            * 描述： DTU串口接收数据
            */
            } 
            if ( flags & COMM_EVT_FLAG_RECV ) {
                flagClr |= COMM_EVT_FLAG_RECV;
                APP_DtuRxDataDealCB(Ctrl.Dtu.pch);
            /***********************************************
            * 描述： 上报消息
            */
            } 
            if ( flags & COMM_EVT_FLAG_REPORT ) {
                /***********************************************
                * 描述： 发送历史数据到服务器
                */
                App_SendDataFromHistory();
            /***********************************************
            * 描述： 断开
            */
            } 
            if ( flags & COMM_EVT_FLAG_CLOSE ) {
                flagClr |= COMM_EVT_FLAG_CLOSE;
                
            /***********************************************
            * 描述： 超时
            */
            } 
            if ( flags & COMM_EVT_FLAG_TIMEOUT ) {
                flagClr |= COMM_EVT_FLAG_TIMEOUT;
                
            /***********************************************
            * 描述： 配置
            */
            } else if ( flags & COMM_EVT_FLAG_CONFIG ) {
                flagClr |= COMM_EVT_FLAG_CONFIG;
            
            /***********************************************
            * 描述： IAP结束
            */
            } 
            if ( flags & COMM_EVT_FLAG_IAP_END ) {
                flagClr |= COMM_EVT_FLAG_IAP_END;
                
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
            OSFlagPost( ( OS_FLAG_GRP  *)&Ctrl.Os.DtuEvtFlagGrp,
                        ( OS_FLAGS      )flagClr,
                        ( OS_OPT        )OS_OPT_POST_FLAG_CLR,
                        ( OS_ERR       *)&err);
            
        /***********************************************
        * 描述： 如果超时，则发送一次心跳包
        */
        } else if ( err == OS_ERR_TIMEOUT ) {
            
        }
//next:
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
 * 名    称： APP_DtuInit
 * 功    能： 任务初始化
 * 入口参数： 无
 * 出口参数： 无
 * 作    者： wumingshen.
 * 创建日期： 2015-03-28
 * 修    改：
 * 修改日期：
 *******************************************************************************/
 void APP_DtuInit(void)
{
    OS_ERR err;
    
    /***********************************************
    * 描述： 创建事件标志组
    */
    OSFlagCreate(( OS_FLAG_GRP  *)&Ctrl.Os.DtuEvtFlagGrp,
                 ( CPU_CHAR     *)"App_DtuFlag",
                 ( OS_FLAGS      )0,
                 ( OS_ERR       *)&err);
    
    Ctrl.Os.DtuEvtFlag = COMM_EVT_FLAG_HEART       // 心跳包发送
                        + COMM_EVT_FLAG_RESET       // DTU复位
                        + COMM_EVT_FLAG_CONNECT     // DTU连接
                        + COMM_EVT_FLAG_RECV        // 串口接收
                        + COMM_EVT_FLAG_REPORT      // 串口发送
                        + COMM_EVT_FLAG_CLOSE       // 断开
                        + COMM_EVT_FLAG_TIMEOUT     // 超时
                        + COMM_EVT_FLAG_CONFIG      // 配置
                        + COMM_EVT_FLAG_IAP_START   // IAP开始
                        + COMM_EVT_FLAG_IAP_END;    // IAP结束
            
    OSQCreate ( (OS_Q        *)&DTU_RxQ,
                (CPU_CHAR    *)"RxQ",
                (OS_MSG_QTY   ) OS_CFG_INT_Q_SIZE,
                (OS_ERR      *)&err);
    /***********************************************
    * 描述： 在看门狗标志组注册本任务的看门狗标志
    */
    WdtFlags |= WDT_FLAG_DTU;
}

/*******************************************************************************
 * 名    称： APP_DtuRxDataDealCB
 * 功    能： 串口数据处理回调函数，由MB_DATA.C调用
 * 入口参数： 无
 * 出口参数： 无
 * 作    者： 无名沈
 * 创建日期： 2016-01-04
 * 修    改：
 * 修改日期：
 * 备    注： 
 *******************************************************************************/
INT08U APP_DtuRxDataDealCB(MODBUS_CH  *pch)
{
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
    CSNC_GetData(	(unsigned char *)pch->RxFrameData,          //RecBuf,
                    (unsigned short )Len,                       //RecLen, 
                    (unsigned char *)DataBuf,                   //DataBuf,
                    (unsigned short *)&Len);                    //InfoLen)

    /***********************************************
    * 描述： 如果数据长度或者地址为0，则不处理
    */
    if ( ( Len == 0 ) || ( DstAddr == 0 ) )
        return FALSE;
    
    /***********************************************
    * 描述： 获取帧头
    */
    OS_CRITICAL_ENTER();
    memcpy( (INT08U *)&Ctrl.Dtu.Rd.Head, (INT08U *)pch->RxFrameData, 8 );
    OS_CRITICAL_EXIT();

    /***********************************************
    * 描述： 地址比对，不是本机地址，直接返回
    */
    if ( Ctrl.Dtu.Rd.Head.DstAddr != Ctrl.Dtu.SlaveAddr ) {
        return FALSE;
    }
    
    /***********************************************
    * 描述： 获取数据
    */
    if( Len > sizeof(StrCommRecvData))
        return FALSE;
    
    OS_CRITICAL_ENTER();
    memcpy( (INT08U *)&Ctrl.Dtu.Rd.Data, (INT08U *)&pch->RxFrameData[8], Len );
    OS_CRITICAL_EXIT();
    
    /***********************************************
    * 描述： 升级程序A
    */
    if ( ( Ctrl.Dtu.Rd.Head.PacketCtrl & 0x0f ) == 0x04 ) {
        
    /***********************************************
    * 描述： 升级程序B
    */
    } else if ( ( Ctrl.Dtu.Rd.Head.PacketCtrl & 0x0f ) == 0x03 ) {
        
    /***********************************************
    * 描述： 参数读取
    */
    } else if ( ( Ctrl.Dtu.Rd.Head.PacketCtrl & 0x0f ) == 0x02 ) {
        
    /***********************************************
    * 描述： 参数设置
    */
    } else if ( ( Ctrl.Dtu.Rd.Head.PacketCtrl & 0x0f ) == 0x01 ) {
        
    /***********************************************
    * 描述： 读数据
    */
    } else if ( ( Ctrl.Dtu.Rd.Head.PacketCtrl & 0x0f ) == 0x00  ) {
        /***********************************************
        * 描述： 如果是显示模块，则不应答
        */
        if ( Ctrl.Para.dat.Sel.udat.DevSel == 2 ) {        
            Ctrl.Dtu.Rd.Data.Oil           = SW_INT16U((Ctrl.Dtu.Rd.Data.Oil>>16));
            goto exit;
        }
    }
exit:
    /***********************************************
    * 描述： 串口接受DTU模组的消息并处理
    */
    Ctrl.Dtu.ConnectTimeOut    = 0;                // 超时计数器清零
    Ctrl.Dtu.ConnectFlag       = TRUE;             // 转连接成功标志
    
    return TRUE;
}

/*******************************************************************************
* 名    称： UpdateRecordPoint
* 功    能： 
* 入口参数： 无
* 出口参数： 无
* 作  　者： wumingshen.
* 创建日期： 2017-02-07
* 修    改：
* 修改日期：
* 备    注：
*******************************************************************************/
void UpdateRecordPoint(uint8_t storeflag)
{
    uint32_t    size    = sizeof(StrMater);             // 获取记录长度
    Mater.Tail  += size;
    
    if ( Mater.Tail >= MAX_ADDR ) {
        Mater.Tail = 0; 
    } else if ( Mater.Tail >= Mater.Head ) {
        Mater.Tail  = Mater.Head; 
    }
    if ( storeflag ) {
        //WriteFM24CL64(72, (uint8_t *)&Mater.Head, 4);    
        WriteFM24CL64(300 + 4, (uint8_t *)&Mater.Tail, 4); 
        WriteFM24CL64(312 + 4, (uint8_t *)&Mater.Tail, 4); 
        WriteFM24CL64(324 + 4, (uint8_t *)&Mater.Tail, 4); 
        //WriteFM24CL64(80, (uint8_t *)&Mater.RecordNbr, 4);
    }
}
/*******************************************************************************
 * 名    称： app_save_record_number_and_index
 * 功    能：
 * 入口参数：
 * 出口参数： 无
 * 作 　 者： fourth peng
 * 创建日期： 2017-5-5
 * 修    改：
 * 修改日期：
 *******************************************************************************/
void app_save_record_number_and_index()
{
	//数据头尾以及流水号保存3份
	WriteFM24CL64(300, (uint8_t *)&Mater.Head, 12); 
	WriteFM24CL64(312, (uint8_t *)&Mater.Head, 12); 
	WriteFM24CL64(324, (uint8_t *)&Mater.Head, 12); 
}


/*******************************************************************************
 * 名    称： App_SaveDataToHistory
 * 功    能：
 * 入口参数：
 * 出口参数： 无
 * 作 　 者： 无名沈
 * 创建日期： 2017-02-7
 * 修    改：
 * 修改日期：
 *******************************************************************************/
void  App_SaveDataToHistory( void )
{
    /**************************************************
    * 描述： 保存当前数据
    */
    uint32_t    size = sizeof(StrMater);                    // 数据长度
        
            CPU_SR  cpu_sr;
    OS_CRITICAL_ENTER();
	Mater.chk = GetCrc16Check((uint8_t *)&Mater,size-2);    // 获取存储校验码
    OS_CRITICAL_EXIT();
	/*写入之前没有判断写入地址是否溢出彭文丁*/
    SaveOneREcord(Mater.Head,(uint8_t *)&Mater,size);       // 保存数据
	Mater.Head += size;                                     // 更改头指针
    
    if ( Mater.Head >= MAX_ADDR ) {                         // 头指针溢出
        Mater.Head = 0;                                     // 溢出指针归零
        if ( Mater.Tail == 0 )                              // 检查尾指针是否为零
            Mater.Tail  += size;                            // 为零更改尾指针
    } else {                                                // 头指针没有溢出
        if ( Mater.Tail >= Mater.Head ) {                   // 尾指针大于等于头指针
            Mater.Tail  += size;                            // 更改尾指针
            if ( Mater.Tail >= MAX_ADDR )                   // 尾指针溢出
                Mater.Tail = 0;                             // 尾指针归零
        }
    }


	//fourth add for clear the peak voltage	2017-5-6
	memset(&Mater.monitor.peak_vol, 0, sizeof(Mater.monitor.peak_vol));
    
    Mater.RecordNbr++;                                      // 记录号+1
    
    /**************************************************
    * 描述： 保存当前记录号和数据指针
    */
    app_save_record_number_and_index();
/*    
	WriteFM24CL64(72, (uint8_t *)&Mater.Head, 4); 
	WriteFM24CL64(76, (uint8_t *)&Mater.Tail, 4);
	WriteFM24CL64(80, (uint8_t *)&Mater.RecordNbr, 4);
*/   
    osal_set_event( OS_TASK_ID_TMR, OS_EVT_TMR_MIN);
}

/*******************************************************************************
 * 名    称： App_SendDataFromHistory
 * 功    能： 数据转储,数据低字节在前，高字节在后 
 * 入口参数：
 * 出口参数： 无
 * 作 　 者： 无名沈
 * 创建日期： 2017-02-7
 * 修    改：
 * 修改日期：
 *******************************************************************************/
void App_SendDataFromHistory(void)
{	    
    /***********************************************
    * 描述： 如果程序处在升级模式
    */
    if ( ( Iap.Status != IAP_STS_DEF ) && 
         ( Iap.Status != IAP_STS_SUCCEED ) &&
         ( Iap.Status != IAP_STS_FAILED ) ) {
        return;
    }
    
    /***********************************************
    * 描述： 头指针在前面（缓冲区未满）
    */
    if ( Mater.Head == Mater.Tail ) {
        osal_start_timerRl( OS_TASK_ID_TMR, OS_EVT_TMR_MIN ,  OS_TICKS_PER_SEC * 10);
    /***********************************************
    * 描述： 头指针在后面（缓冲区已满）
    */
    } else {
        uint32_t    size    = sizeof(StrMater);             // 获取记录长度
		MX25L3206_RD(Mater.Tail, size,(uint8_t*)&History);  // 读取一条记录
        
        /**************************************************
        * 描述： 数据校验
        */
		uint16_t    CRC_sum1 = GetCrc16Check((uint8_t*)&History,sizeof(StrMater)-2);		
		uint16_t    CRC_sum2 = History.chk;
        uint32_t    timeout = OS_TICKS_PER_SEC * 5;
        /**************************************************
        * 描述： 发送记录至服务器
        */
		if(CRC_sum1 == CRC_sum2) {						    //进行累加和计算
            if (Ctrl.Dtu.pch->RxBufByteCtr == 0) {
                CSNC_SendData( (MODBUS_CH      *) Ctrl.Dtu.pch,
                               (CPU_INT08U      ) Ctrl.Com.SlaveAddr,                     // SourceAddr,
                               (CPU_INT08U      ) 0xCB,                     // DistAddr,
                               (CPU_INT08U     *)&History,                    // DataBuf,
                               (CPU_INT08U      ) size);                    // DataLen 
                Ctrl.Dtu.pch->StatNoRespCtr++;
                if ( Ctrl.Dtu.pch->StatNoRespCtr < 5 ) {
                    timeout     = OS_TICKS_PER_SEC * 10; 
                } else if ( Ctrl.Dtu.pch->StatNoRespCtr < 20 ) {
                    timeout     = OS_TICKS_PER_SEC * 30; 
/*                } else if ( Ctrl.Dtu.pch->StatNoRespCtr < 100 ) {
                    timeout     = OS_TICKS_PER_SEC * 30; */
                } else {                
                    timeout     = OS_TICKS_PER_SEC * 60; 
                }
            }
            osal_start_timerRl( OS_TASK_ID_TMR, OS_EVT_TMR_MIN, timeout );
        } else {
            UpdateRecordPoint(0);
            
            osal_set_event( OS_TASK_ID_TMR, OS_EVT_TMR_MIN);
            //timeout = OS_TICKS_PER_SEC * 1;
        }
        
    }
}

/*******************************************************************************
 * 				end of file
 *******************************************************************************/
#endif
