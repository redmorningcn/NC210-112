#ifndef	APP_CTRL_H
#define	APP_CTRL_H
/*******************************************************************************
 *   Filename:       app_ctrl.h
 *   Revised:        $Date: 2009-01-08	20:15 (Fri) $
 *   Revision:       $
 *	 Writer:		 Wuming Shen.
 *
 *   Description:
 *
 *   Notes:
 *					QQ:276193028
 *     				E-mail:shenchangwei945@163.com
 *
 *   All copyrights reserved to Wuming Shen.  现代虚拟仪器仪表研究所
 *
 *******************************************************************************/

/*******************************************************************************
 * INCLUDES
 */
#include <global.h>
#include <app.h>
#include <bsp_key.h>
#include <bsp_max7219.h>
#include <protocol.h>
#include <os.h>
#include <osal.h>
#include <mb.h>

#ifdef   APP_GLOBALS
#define  APP_EXT
#else
#define  APP_EXT  extern
#endif
/*******************************************************************************
 * CONSTANTS
 */

 /*******************************************************************************
 * 				            系统参数相关宏定义                                  *
 *******************************************************************************/

/***************************************************
* 描述： RF应用数据帧队列缓冲
*/
#define AppMsgQ                            10
#define AppMsgQSize                        10
 
/***************************************************
* 描述： 定义存储设备类型
*/
#define INTNER_FLASH                        0
#define INTNER_EEPROM                       1
#define EXTERN_EEPROM                       2
#define STORAGE_TYPE                        EXTERN_EEPROM

/***************************************************
* 描述： WIFI事件标志组
*/    
#define WIFI_EVT_FLAG_HEART               (1 << 0)// 心跳包发送
#define WIFI_EVT_FLAG_RESET               (1 << 1)// WIFI复位
#define WIFI_EVT_FLAG_CONNECT             (1 << 2)// WIFI连接
#define WIFI_EVT_FLAG_RECV                (1 << 3)// 串口接收
#define WIFI_EVT_FLAG_REPORT              (1 << 4)// 上报消息
#define WIFI_EVT_FLAG_CLOSE               (1 << 5)// 断开
#define WIFI_EVT_FLAG_TIMEOUT             (1 << 6)// 超时
#define WIFI_EVT_FLAG_CONFIG              (1 << 7)// 配置

#define COMM_EVT_FLAG_HEART               (1 << 0)// 心跳包发送
#define COMM_EVT_FLAG_RESET               (1 << 1)// WIFI复位
#define COMM_EVT_FLAG_CONNECT             (1 << 2)// WIFI连接
#define COMM_EVT_FLAG_RECV                (1 << 3)// 串口接收
#define COMM_EVT_FLAG_REPORT              (1 << 4)// 上报消息
#define COMM_EVT_FLAG_CLOSE               (1 << 5)// 断开
#define COMM_EVT_FLAG_TIMEOUT             (1 << 6)// 超时
#define COMM_EVT_FLAG_CONFIG              (1 << 7)// 配置
 
/*******************************************************************************
 * 				            OSAL相关定义                                       *
 *******************************************************************************/
/*******************************************************************************
* 描述： OSAL任务优先级定义
*/
#define	OS_TASK_PRO_BEEP                OS_TASK_PRIO_HIGHEST - 0
#define	OS_TASK_PRO_KEY                 OS_TASK_PRIO_HIGHEST - 1
#define	OS_TASK_PRO_CTRL                OS_TASK_PRIO_HIGHEST - 0

#define	OS_TASK_PRO_SEN                 OS_TASK_PRIO_HIGHEST - 0
#define	OS_TASK_PRO_DISP                OS_TASK_PRIO_HIGHEST - 1
/***************************************************
* 描述： OSAL任务ID定义
*/
#define OS_TASK_ID_BEEP                 0x00
#define OS_TASK_ID_KEY                  0x01
#define OS_TASK_ID_CTRL                 0x00

#define OS_TASK_ID_SEN                  0x00
#define OS_TASK_ID_DISP                 0x01
/*******************************************************************************
* 描述： OSAL事件定义
*/
/***************************************************
* 描述： OSAL事件定义：控制任务
*/
#define OS_EVT_CTRL_TICK                0X00000001
#define OS_EVT_CTRL_FAN                 0X00000002
#define OS_EVT_CTRL_LED                 0X00000004
#define OS_EVT_CTRL_WIFI_CFG            0X00000008
#define OS_EVT_CTRL_WIFI_TMO            0X00000010
#define OS_EVT_CTRL_FILTER              0X00000020
#define OS_EVT_CTRL_TIMING_ON_OFF       0X00000040
#define OS_EVT_CTRL_AIR_VOLUME          0X00000080
/***************************************************
* 描述： OSAL事件定义：SENSOR任务事件
*/
#define OS_EVT_SEN_TICKS                0X00000001
#define OS_EVT_SEN_MSEC                 0X00000002
#define OS_EVT_SEN_SEC                  0X00000004
#define OS_EVT_SEN_MIN                  0X00000008
#define OS_EVT_SEN_DEAL                 0X00000010
#define OS_EVT_SEN_UPDATE_DENSITY       0X00000020
#define OS_EVT_SEN_6                    0X00000040
#define OS_EVT_SEN_ERR_CHK              0X00000080
     
#define OS_EVT_SEN_SAMPLE               0X00000001
#define OS_EVT_SEN_PM2D5_CHK            0X00000002
#define OS_EVT_SEN_CHK_TIMEOUT          0X00000004
#define OS_EVT_SEN_UART_TIMEOUT         0X00000008
#define OS_EVT_SEN_4                    0X00000010
#define OS_EVT_SEN_5                    0X00000020
#define OS_EVT_SEN_6                    0X00000040
#define OS_EVT_SEN_7                    0X00000080
/***************************************************
* 描述： OSAL事件定义：LED任务事件
*/
#define OS_EVT_LED_0                    0X00000001
#define OS_EVT_LED_1                    0X00000002
#define OS_EVT_LED_2                    0X00000004
#define OS_EVT_LED_3                    0X00000008
#define OS_EVT_LED_4                    0X00000010
#define OS_EVT_LED_5                    0X00000020
#define OS_EVT_LED_6                    0X00000040
#define OS_EVT_LED_7                    0X00000080

/***************************************************
* 描述： OSAL事件定义：DISP任务事件
*/
#define OS_EVT_DISP_TICKS                0X00000001
#define OS_EVT_DISP_1                    0X00000002
#define OS_EVT_DISP_2                    0X00000004
#define OS_EVT_DISP_3                    0X00000008
#define OS_EVT_DISP_4                    0X00000010
#define OS_EVT_DISP_5                    0X00000020
#define OS_EVT_DISP_6                    0X00000040
#define OS_EVT_DISP_7                    0X00000080
/*******************************************************************************
 * 				            其他定义                                           *
 *******************************************************************************/
/***************************************************
* 描述： 时间定义
*/
#define FILTER_LIFE_SAVE_TIME           30          // 分钟
#define FILTER_LIFE_TIME               (3*30*24*60/1)// 分钟(3个月)
//#define FILTER_LIFE_SAVE_TIME           1          // 分钟
//#define FILTER_LIFE_TIME               (60/FILTER_LIFE_SAVE_TIME)        // 分钟(24小时，测试用)

/*******************************************************************************
 * TYPEDEFS
 */
#define PARA_TYPE_INT08S                0
#define PARA_TYPE_INT08U                1
#define PARA_TYPE_INT16S                2
#define PARA_TYPE_INT16U                3

#define PARA_TYPE   PARA_TYPE_INT16S
__packed
typedef struct {
    
#if     ( PARA_TYPE == PARA_TYPE_INT08S )
    INT08S      Min;
    INT08U      Max;
#elif   ( PARA_TYPE == PARA_TYPE_INT08U )
    INT08U      Min;
    INT08U      Max;
#elif   ( PARA_TYPE == PARA_TYPE_INT16S )
    INT16S      Min;
    INT16S      Max;
#elif   ( PARA_TYPE == PARA_TYPE_INT16U )
    INT16U      Min;
    INT16U      Max;
#else
    INT08S      Min;
    INT08U      Max;
#endif
} StrParaRange;

__packed
typedef struct  {    
    CPU_FP32            I1;                         // 电流1  uA为单位
    CPU_FP32            I2;                         // 电流2
    CPU_FP32            Adc1;                       // ADC1
    CPU_FP32            Adc2;                       // ADC2
    CPU_FP32            K;                          // 斜率   放大10000倍
    CPU_FP32            B;                          // 偏置
} StrAdcToIBoardChk;

__packed
typedef struct  {
    CPU_FP32              P1;                       // 压力1  Pa为单位
    CPU_FP32              P2;                       // 压力2
    CPU_FP32              I1;                       // 电流1  uA为单位
    CPU_FP32              I2;                       // 电流2
    CPU_FP32              K;                        // 斜率
    CPU_FP32              B;                        // 偏置
} StrIToPSenChk;
/***************************************************
* 描述： 传感器故障
*/
__packed
typedef struct {                                   // 状态	        1	
    INT08U                  HSenBrokenErr           : 1;        //              Bit1：上位传感器异常
    INT08U                  HSenExitDetection       : 1;        //              Bit3：液位过低，上位传感器退出检测
    INT08U                  HSenShortErr            : 1;        //              Bit7：液位过高，或传感器短路
    INT08U                  PwrDownEvt              : 1;        //              Bit4：有掉电事件（正确接收后，复0）
    INT08U                  LSenBrokenErr           : 1;        //              Bit0：下位传感器异常
    INT08U                  LSenExitDetection       : 1;        //              Bit2：液位过低，下位传感器退出检测
    INT08U                  LSenShortErr            : 1;        //              Bit6：液位过高，或传感器短路
    INT08U                  WorkStsReq              : 1;        //              Bit5：需机车工况状态信息（速度、加速度、牵引、制动、零位…）
} StrSenErr;

/*************************************************
* 描述： 油箱模型表
*/
__packed
 typedef struct {
    INT16U  Hight;                                  // 高度
    INT16U  Volume;                                 // 油量
 } _StrOilPoint;

__packed
 typedef struct {
    INT16S                  Offset;                 // 高度偏置
    INT16U                  Cnts;                   // 模型点数,14
    _StrOilPoint            Point[10];              // 高度,油量
 } StrOilVolumeTab;
 
__packed
 typedef union {
    StrOilVolumeTab         Tab;                    // 油箱模型    
    INT08U                  buf1[44];
    INT16U                  buf2[22];
    INT32U                  buf3[11];
 } StrOilTab;
 
/***************************************************
* 描述： 控制器参数：共256 + 4 = 260字节
*/
__packed
typedef struct _StrCtrlPara {
    /***************************************************
    * 描述： 系统参数：起始地址 = 000
    */    
    INT16U              Password;                       // 用于MODEBUS通讯确认密钥，默认为6237，防止非法改数据
    /***************************************************
    * 描述： 密度：起始地址 = 002
    */
    CPU_FP32            LastDensity;                    // 上次测量密度
    
    /***************************************************
    * 描述： 偏置：起始地址 = 006
    */
    INT16S              Offset;
    /***************************************************
    * 描述： 预留：起始地址 = 008
    */     
    CPU_FP32            LastGravity;                    // 上次测量重力加速度
    
    CPU_FP32            SetDensity;                     // 设置密度
    CPU_FP32            SetGravity;                     // 设置重力加速度   
    
    //INT16U              Rsv1[3];
    /***************************************************
    * 描述： 校准参数：起始地址 = 020
    */
    StrAdcToIBoardChk   Check[2]; 
    /***************************************************
    * 描述： 校准参数：起始地址 = 068
    */
    StrIToPSenChk       SenChk[2];    
    /***************************************************
    * 描述： 校准参数：起始地址 = 116
    */
    union __uSENSOR_CHECK_TYPE {
        struct __sSENSOR_CHECK_TYPE {
            INT08U      RstFlag             : 1;        // 复位标志,标志位位置和寄存器位置不能随意调整，MB_DATA.C中的复位也要跟着调整
            INT08U      Test                : 1;        // 测试标志
            INT08U      Rsv                 : 6;        //
            StrSenErr   Err;                            // 错误标志
        }Udat;
        INT08U Dat[2];
    } SenSts;
    /***************************************************
    * 描述： 校准参数：起始地址 = 116
    */
    union __uSEL_TYPE {
        struct __sSEL_TYPE {
            INT16U      FilterSel           : 4;        // 传感器滤波级别选择：0~4
            INT16U      SensorSel           : 2;        // 传感器选择:0低、1高、2平均、3智能
            INT16U      DensitySel          : 2;        // 密度选择：0设定、1测量、2设定测量平均值、3智能
            INT16U      GravitySel          : 2;        // 重力加速度选择：0设定、1测量、2设定测量平均、3智能
            INT16U      CalcSel             : 2;        // 计算方法选择：0A-I-P、1A-P
            INT16U      DispSel             : 2;        // 显示模式：0连接显示油量，断开显示高度；1显示高度；2；轮流显示 3：关闭显示
            INT16U      Rsv                 : 2;        // 预留
        }udat;
        INT16U dat;
    } Sel;
    
    /***************************************************
    * 描述： 用户参数：起始地址 = 118
    */
    StrOilTab   Vtab;
    /***************************************************
    * 描述： 用户参数：起始地址 = 118
    */
    INT16S              RSV[44];                        // 保留13个参数
    /***********************************************
    * 描述：状态字 ：起始地址 = 252
    */
    union __sSYSTEM_STATUS {
        struct __uSYSTEM_STATUS {
            uint16_t  AbnormalPowerReset    : 1;        // 异常掉电标志
            uint16_t  App_Fault_ISR         : 1;        // 异常中断
            uint16_t  ResetFlag             : 1;        // 重启标志位
            uint16_t  rsv1                  : 1;        // 
            
            uint16_t  ModuleType            : 2;        // 模块类型
            uint16_t  Channels              : 2;        // 通道数
            
            INT16U    FirstPwrFlag          : 1;        // 首次上电标志位
            INT16U    SenChkFlag            : 1;        // PM2D5校准标志位
            uint16_t  RSV2                  : 6;        //
        }udat;
        INT16S dat;
    } SysSts;

    /***************************************************
    * 描述： 恢复出厂设置：起始地址 = 254
    */
    INT16S          RestoreSetting;                     // 恢复出厂设置

    const struct _StrCtrlPara *idx;                    // 序号指针，占4字节

} StrCtrlPara;

/***************************************************
* 描述：
*/
__packed
typedef struct {
    INT08U                 *pEdit;                      // 编辑参数指针
    INT08U                 *pMax;                       // 编辑参数的最大值指针
    INT08U                 *pMin;                       // 编辑参数的最小值指针
    INT08U                 *pStart;                     // 编辑的起始参数
    INT08U                 *pEnd;                       // 编辑的结束参数
    INT08U                  EditIdx;                    // 编辑项目序号
    INT08U                  Edit;                       // 编辑参数
} StrEdit;

__packed
typedef struct {
    INT08U                  Mode;                       // 参数修改标志
    StrLedDisp             *Led;
} StrDisp;

__packed
typedef struct {
    INT08U                 *p08;
    INT16U                 *p16;
    INT32U                 *p32;
    INT16U                  len;
} NOTIFY_EEP_SAVE_TYPE;

__packed
typedef struct {
    OS_FLAG_GRP             WifiEvtFlagGrp;
    OS_FLAGS                WifiEvtFlag;
    OS_FLAG_GRP             CommEvtFlagGrp;
    OS_FLAGS                CommEvtFlag;
} StrCtrlOS;


__packed
typedef struct {    
    INT08U                  SysSts                  : 4;        // 系统状态
    INT08U                  WifiSts                 : 4;        // WIFI状态
        
    INT16U                  FirstPwrFlag            : 1;        // 首次上电标志位
    INT16U                  PM2D5ChkFlag            : 1;        // PM2D5校准标志位
    INT16U                  RsvFlag                 :14;        // 保留    
} StrSystem;

/***************************************************
* 描述： 帧头：长沙南睿轨道交通车辆装置异步串行通信通用通信协议
*/
__packed
typedef struct {    
    INT16U                  Head;                               // 帖头
    INT08U                  SrcAddr;                            // 源地址
    INT08U                  DstAddr;                            // 目的地址   
    INT08U                  PacketSn;                           // 报文序号
    INT08U                  PacketCtrl;                         // 报文控制字： b0000
    INT08U                  DataLen;                            // 数据区长度： 12
    INT08U                  PacketIdx;                          // 报文索引
    INT08U                  PacketIdxChk;                       // 报文索引校验
} StrCommHead;

/***************************************************
* 描述： 接收数据：长沙南睿轨道交通车辆装置异步串行通信通用通信协议
*/
// 接收数据结构：主机-->油尺
__packed
typedef struct {
    INT32U                  Oil;                                // 油量	        4	数值范围0-9999L 
    INT16S                  Accelerations;                      // 加速度	    2	-9.000-9.000（m/s2）(1000倍)
    INT16U                  Speed;                              // 速度	        2	数值0-600（Km/H）
    struct __recv_work__ {                                     // 机车工况	    1
    INT08U                  Null                    : 1;        //              0x01:零位
    INT08U                  Tow                     : 1;        //              0x02:牵引
    INT08U                  Brake                   : 1;        //              0x04:制动
    INT08U                  Rsv                     : 5;        // 其他：未定义
    } Work;
    INT08U                  Rsv[3+10];                          // 备用	        3
} StrCommRecvData;

/***************************************************
* 描述： 发送数据：长沙南睿轨道交通车辆装置异步串行通信通用通信协议
*/
// 应答数据结构：油尺-->主机
__packed
typedef struct {
    INT16U                  Hight;                              // 液位高度	    2	xxx.x毫米（10倍）
    INT16U                  Density;                            // 高度计算密度	2	0.xxxx克/立方厘米（10000倍）
    union __usend_status {
    StrSenErr               Udat;
    INT08U                  Dat;
    } Sts;
    INT08U                  RefLiqTemp;                         // 液体参考温度	1	0.25~63.50度（分别用1-254表示）或者-49.5~77.0度（分别用1-254表示，100表示0度）0：过低；255：过高
    INT16U                  AdcValue[2];                        // Adc值
    INT16U                  HightDiff;                          // 高度差	    2	0~1000x0.1mm(两传感器高度差)
    INT16U                  Volume;                             // 体积  	    2	0~10000L(油量)
    INT16U                  Weight;                             // 重量	        2	0~10000kg(重量)
    INT16S                  Pressure[2];                        // 压强	        2	0~10000pa
    INT16S                  CalcDensity;                        // 测量密度     x10000
    INT16S                  Gravity;                            // 计算加速度   x10000
    INT16S                  CalcGravity;                        // 测量加速度   x10000
    //INT08U                  Rsv[2+10];                        // 备用	        2	
} StrCommSendData;

/***************************************************
* 描述： 数据：长沙南睿轨道交通车辆装置异步串行通信通用通信协议
*/
__packed
typedef struct { 
    INT16U                  Check;                              // 校验码，高在前，低在后
    INT16U                  Tail;                               // 帖尾
} StrCommTail;

__packed
typedef struct { 
    /***************************************************
    * 描述： 长沙南睿轨道交通车辆装置异步串行通信通用通信协议
    */
    StrCommHead             Head;                               // 头
    StrCommRecvData         Data;                               // 接收数据区
    StrCommTail             Tail;                               // 尾
} StrCommRecvPack;

__packed
typedef struct { 
    /***************************************************
    * 描述： 长沙南睿轨道交通车辆装置异步串行通信通用通信协议
    */
    StrCommHead             Head;                               // 头
    StrCommSendData	        Data;                               // 发送数据区
    StrCommTail             Tail;                               // 尾
} StrCommSendPack;

__packed
typedef struct {     
    StrCommRecvPack         Rd;                                 // 接收数据区
    StrCommSendPack	        Wr;                                 // 发送数据区
    /***************************************************
    * 描述： 串口控制组
    */
    INT08U                  SlaveAddr;                          // 从机地址    
    MODBUS_CH               *pch;                               // MODBUS句柄
    
    INT08U                  ConnectTimeOut;                     // 连接超时计数器，秒为单位
    INT08U                  ConnectFlag;                        // 连接标志
} StrDevComm;

#define ADC_SAMPLE_CNTS_PER_SEC     10                          // 每个通道一秒钟采样次数
#define ADC_SAMPLE_SECS             5                           // 秒级滤波次数
#define ADC_SAMPLE_CNTS_MAX         18                          // 
#define ADC_SAMPLE_TIME             (OS_CFG_TICK_RATE_HZ / ADC_SAMPLE_CNTS_PER_SEC / 2)                          // 

#define ADC_SIGNAL_CYCLE            1500                        // 带通滤波器最小信号周期
#define ADC_SIGNAL_CYCLE_CNT        10                          // 采集周期个数
#define ADC_SAMPLE_CYCLE            100                         // 深度数据处理采样周期
#define ADC_SAMPLE_CNT              (1)                         // 深度数据处理采样周期
#define ADC_SAMPLE_CNTS_DEEPTH      (ADC_SIGNAL_CYCLE * ADC_SIGNAL_CYCLE_CNT / ADC_SAMPLE_CYCLE) // 数据处理深度

__packed
typedef struct { 
    INT16U                  AdcValue;                           // 采样ADC值
    INT16U                  AdcValueSec;                        // 每秒钟ADC值
    INT16U                  AdcValueMin;                        // 每分钟ADC值
    INT16U                  AdcValueAll;                        // 所有钟ADC值
    INT16U                  AdcValueSample;                     // 所有电压
    CPU_FP32                Pressure;                           // 压力
    CPU_FP32                PressureSec;                        // 每秒钟压力
    CPU_FP32                PressureMin;                        // 每分钟压力
    CPU_FP32                PressureAll;                        // 所有压力
    CPU_FP32                PressureSample;                         // 所有电压
    INT16S                  Hight;                              // 高度
    INT16S                  HightSec;                           // 每秒钟高度
    INT16S                  HightMin;                           // 每分钟高度
    INT16S                  HightAll;                           // 所有高度
    INT16S                  HightSample;                         // 所有电压
    INT16S                  Voltage;                            // 电压0.1MV
    INT16S                  VoltageSec;                         // 每秒钟电压
    INT16S                  VoltageMin;                         // 每分钟电压
    INT16S                  VoltageAll;                         // 所有电压
    INT16S                  VoltageSample;                      // 所有电压
    
    CPU_FP32                Current;                            // 电流
    CPU_FP32                CurrentSec;                         // 每秒钟电流
    CPU_FP32                CurrentMin;                         // 每分钟电流
    CPU_FP32                CurrentAll;                         // 所有电流
    CPU_FP32                CurrentSample;                      // 所有电流
    
    INT08U                  Head;                               // 滤波参数头指针
    INT08U                  Tail;                               // 滤波参数尾指针
    INT08U                  HeadSec;                            // 滤波参数头指针
    INT08U                  TailSec;                            // 滤波参数尾指针
    INT08U                  HeadMin;                            // 滤波参数头指针
    INT08U                  TailMin;                            // 滤波参数尾指针
    
    INT16U                  AdcFilter   [ADC_SAMPLE_CNTS_PER_SEC];// 100毫秒内ADC值
    INT16U                  AdcFilterSec[ADC_SAMPLE_SECS];      // 1秒内ADC值
    INT16U                  AdcFilterMin[ADC_SAMPLE_CNTS_MAX];  // 1分内ADC值
    INT16U                  Rsv[100-38-ADC_SAMPLE_CNTS_PER_SEC-ADC_SAMPLE_SECS-ADC_SAMPLE_CNTS_MAX]; 
}__StrSen;

__packed
typedef struct {     
    INT16S                  AverHight;                          // 平均高度
    INT16S                  AverHightSec;                       // 平均高度
    INT16S                  AverHightMin;                       // 平均高度
    INT16S                  AverHightAll;                       // 平均高度
    INT16S                  AverHightSample;                    // 所有电压
    INT16S                  AverPressure;                       // 平均压力
    INT16S                  AverPressureSec;                    // 平均压力
    INT16S                  AverPressureMin;                    // 平均压力
    INT16S                  AverPressureAll;                    // 平均压力
    INT16S                  AverPressureSample;                 // 所有电压
    CPU_FP32                DiffPressure;                       // 压力差
    CPU_FP32                DiffPressureSec;                    // 压力差
    CPU_FP32                DiffPressureMin;                    // 压力差
    CPU_FP32                DiffPressureAll;                    // 压力差
    CPU_FP32                DiffPressureSample;                 // 所有电压
    INT16S                  CalcHight;                          // 计算高度差
    INT16S                  CalcHightSec;                       // 计算高度差
    INT16S                  CalcHightMin;                       // 计算高度差
    INT16S                  CalcHightAll;                       // 计算高度差
    INT16S                  CalcHightSample;                    // 所有电压
    INT16S                  OffsetHight;                        // 高度偏差
    INT16S                  CalcDiffHight;                      // 计算高度差
    INT16S                  CalcDiffHightSec;                   // 计算高度差
    INT16S                  CalcDiffHightMin;                   // 计算高度差
    INT16S                  CalcDiffHightAll;                   // 计算高度差
    INT16S                  CalcDiffHightSample;                // 校准高度差
    INT16S                  ChkedDiffHight;                     // 校准高度差
    CPU_FP32                ChkedDensity;                       // 密度
    CPU_FP32                CalcDensity;                        // 密度
    CPU_FP32                CalcDensitySec;                     // 密度
    CPU_FP32                CalcDensityMin;                     // 密度
    CPU_FP32                CalcDensityAll;                     // 密度
    CPU_FP32                CalcDensitySample;                  // 所有电压
    CPU_FP32                Kp;                                 // 
    CPU_FP32                KpSample;                           // 
    CPU_FP32                Volume;                             // 体积
    CPU_FP32                Weight;                             // 重量
    INT16S                  Hight;                              // 上传高度   
    CPU_FP32                Density;                            // 计算密度
    CPU_FP32                Gravity;                            // 计算加速度
    CPU_FP32                CalcGravity;                        // 测量加速度
    INT08U                  Rsv[200-118];//100-57];                         // 

} __StrSenCom;

/***************************************************
* 描述： 控制器参数：共256 + 4 = 260字节
*/
__packed
typedef struct _StrSenPara {
    /***************************************************
    * 描述： （2）：系统参数：起始地址 = 000
    */    
    CPU_FP32                ChkedDiffHight;             // 校准高度差
    CPU_FP32                DiffPNbr;                   // 压强差数据组个数
    CPU_FP32                PINbr;                      // 压强电流数据组个数    
    CPU_FP32                TestDensity;                // 测试密度
    CPU_FP32                TestGravity;                // 测试加速度
    
    INT32U                  SenId;                      // 传感器编号，上在前，下在后
    CPU_FP32                Offset;                     // 保留5个参数
    /***************************************************
    * 描述： 校准参数：起始地址 = 28
    */
    union __uSENSOR_CHECK_TYPE2 {
        struct __sSENSOR_CHECK_TYPE2 {
            INT16U      ChkType             : 4;        // 高位传感器校准类型：0不校准；1压强差分段；2：压强差线性；3：压力电流分段；4：压力电流线性
            INT16U      Test                : 4;        // 低位传感器校准类型：0不校准；1压强差分段；2：压强差线性；3：压力电流分段；4：压力电流线性
            INT16U      Rst                 : 4;        // 复位标志
            INT16U      Rsv                 : 4;
        }udat;
        INT32U dat;
    } SenSts;
    CPU_FP32                RSV1[2];                    // 保留5个参数
    /***************************************************
    * 描述： （2）：系统参数：起始地址 = 040 
    */
    CPU_FP32                DiffP[10];
    /***************************************************
    * 描述： （3）：系统参数：起始地址 = 080
    */ 
    CPU_FP32                PI[2][6][2];
    /***************************************************
    * 描述： （4）：系统参数：起始地址 = 176
    */ 
    StrIToPSenChk           Check[2];
    /***************************************************
    * 描述： （1）：用户参数：起始地址 = 200
    */
    CPU_FP32                RSV2[7];                     // 保留5个参数

    /***************************************************
    * 描述： 校验码：起始地址 = 252
    */
    INT16U                  CheckSum;                   // 校验码
    /***************************************************
    * 描述： 校验码：起始地址 = 254
    */
    INT16U                  ReadFlag;                   // 已读取标志，0X5A为新数据，0XA5为已读，其他为无效
} StrSenPara;

__packed
typedef union {
    StrSenPara             dat;
    INT08U                  buf1[256];
    INT16U                  buf2[128];
    INT32U                  buf3[64];
} UnionSenPara;

__packed
typedef struct { 
    __StrSen                H;                                  // 高点传感器
    __StrSen                L;                                  // 低点传感器
    __StrSenCom             C;                                  // 传感器公共部分
    UnionSenPara            Para;
    INT16U                  Filter[2][ADC_SAMPLE_CNTS_DEEPTH];  //深度数据处理
    INT08U                  DensityStep;                        //密度获取状态机
    INT08U                  DensitySts;                         //密度获取状态机
    INT16U                  DensityCtr;                         //密度获取状态机
} StrSen; 


__packed
typedef union {
    StrCtrlPara             dat;
    INT08U                  buf1[256];
    INT16U                  buf2[128];
    INT32U                  buf3[64];
} UnionCtrlPara;

//APP_EXT StrOilTab       OilTab;

__packed
typedef struct {
    /***************************************************
    * 描述： 数据结构，顺序不能随意调整
    */
    UnionCtrlPara           Para;                               // 控制参数
    StrSen                  Sen;                                // 传感器属性 
    StrOilTab               Tab;                                // 油箱模型
    StrDevComm              Comm;                               // COMM属性 
    
    StrOilTab              *VTab;                               // 油箱容积表
    StrParaRange           *Prge;                               // 控制参数范围
    
    StrCtrlOS               Os;                                 // OS属性        
    StrDisp                 Disp;                               // 显示属性        
    StrSystem               Sys;                                // 系统状态
} StrCtrlDCB;

extern StrCtrlDCB           Ctrl;

/*******************************************************************************
 * 				                  其他                                         *
 *******************************************************************************/
/***************************************************
* 描述： 调试信息数据结构，用于发送
*/
__packed
typedef struct {
  INT08U                tail;                                   // 尾
  INT08U                head;                                   // 头
  INT08U                cnt;                                    // 计数器
  INT08U                qbuf[AppMsgQ][AppMsgQSize];             // 消息队列
} StrMsgInfo;

/*******************************************************************************
 * GLOBAL VARIABLES
 */
/***************************************************
* 描述： 结构体声明
*/
APP_EXT NOTIFY_EEP_SAVE_TYPE    NotifyEepSave;

/*******************************************************************************
 * GLOBAL FUNCTIONS
 */

/*******************************************************************************
 * EXTERN VARIABLES
 */

/*******************************************************************************
 * EXTERN FUNCTIONS
 */
void        Beep                        (INT32U flag);
void        BeepExt                     (u8 cnt, u16 freq, u16 dly);
void        Power                       (INT08U onoff);
/***************************************************
* 描述： 应用层函数体声明
*/
BOOL        App_PraInit                 (void);
BOOL        App_GetParaFromEeprom       (void);
BOOL        App_SetParaToEeprom         (void);
void        App_Error                   (void);
void        App_FanHandCtrl             (INT08U step);
void        App_FanAutoCtrl             (INT08U step);
void        App_ParaFilter              (void);

/***************************************************
* 描述： OSAL任务初始化函数体声明
*/
void        TaskInitBeep                ( void );
void        TaskInitKey                 ( void );
void        TaskInitCtrl                ( void );
void        TaskInitSen                 ( void );
void        TaskInitDisp                ( void );
/***************************************************
* 描述： OSAL任务事件处理函数体声明
*/
osalEvt     TaskBeepEvtProcess          (INT8U task_id, osalEvt task_event);
osalEvt     TaskKeyEvtProcess           (INT8U task_id, osalEvt task_event);
osalEvt     TaskCtrlEvtProcess          (INT8U task_id, osalEvt task_event);
osalEvt     TaskSenEvtProcess           (INT8U task_id, osalEvt task_event);
osalEvt     TaskDispEvtProcess          (INT8U task_id, osalEvt task_event);

/***************************************************
* 描述： AES加减密函数体声明
*/
/***************************************************
* 描述： 看门狗函数体声明
*/
void        App_MsgWriteCmd             ( INT16U cmd );
/*******************************************************************************
 * 				end of file                                                    *
 *******************************************************************************/
#endif	/* GLOBLES_H */