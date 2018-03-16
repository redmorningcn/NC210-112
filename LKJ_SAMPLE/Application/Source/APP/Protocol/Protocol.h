/*******************************************************************************
 *   Filename:       Protocol.h
 *   Revised:        All copyrights reserved to Roger.
 *   Date:           2014-08-11
 *   Revision:       v1.0
 *   Writer:	     Roger-WY.
 *
 *   Description:    机智云WIFI通信协议头文件
 *
 *
 *   Notes:
 *
 *   All copyrights reserved to Roger-WY
 *******************************************************************************/
#ifndef _PROTOCOL_H
#define _PROTOCOL_H

#include <stdio.h>
#include <stm32f10x.h>

/***********************************************
* 描述： WIFI模组复位引脚定义
*/
#define WIFIMODE_RESET_RCC      RCC_APB2Periph_GPIOB
#define WIFIMODE_RESET_PIN      GPIO_Pin_14
#define WIFIMODE_RESET_PORT     GPIOB

#define WIFIMODERESET_HIGH()    GPIO_SetBits(WIFIMODE_RESET_PORT, WIFIMODE_RESET_PIN)
#define WIFIMODERESET_LOW()     GPIO_ResetBits(WIFIMODE_RESET_PORT, WIFIMODE_RESET_PIN)


/*******************************************************************************
*
*   AppID是什么？
*
*   答：AppID是应用标识码。当开发者需要为一款智能产品开发应用（包括iOS、Android、Web应用等）时，
*   后台会自动生成一个AppID，并与此设备进行关联。应用开发时需要填入此AppID。
*   每个AppID对应的用户系统都是独立的，也就是在AppID A对应的App注册的用户，
*   无法在AppID B对应的App中登录，需要重新注册并绑定设备。
*   应用标识码（ProductKey ）和产品标识码（AppID）在产品创建时同时生成，
*   但两者之间没有直接的对应关系，一个AppID可以对应多个ProductKey，一个ProductKey也可以对应多个AppID。
*
*
*   ProductKey是什么？
*
*   答：ProductKey是产品标识码，开发者通过机智云后台创建新产品后，自动生成的一个32位字符串。
*   在机智云的数据库中是一个唯一的号码，开发者完成开发写入设备主控MCU后，机智云通过此标识码对设备进行识别并自动完成注册。
*   使用机智云的SDK开发App时，也需要使用到该ProductKey来对设备列表进行过滤，让App只能搜索到对应的设备，
*   而不会获取到属于别的产品的设备。一个ProductKey对应的是一个产品，而不是只对应该产品的单个设备。
*   例如一个ProductKey对应的是某款净化器，生产出来的该款净化器使用的都是同一个ProductKey，而不是一台一个ProductKey。
*
*
*   did是什么？
*
*   答：did是设备号（Device ID），当一个设备初次接入机智云时，机智云自动根据ProductKey以及设备Wi-Fi模块MAC地址为此设备注册一个did，
*   此did全网唯一，用于与用户的绑定及后续远程控制操作。did设计的初衷是给每台设备一个唯一标识码，
*   远程控制是通过该唯一标识码识别目标设备，同时，出于安全性考虑，该设备号需要与用户账号绑定才能远程控制。
*   另外，可以通过重置模块的方式，让模块重新在云端注册并生成新的did，这样可以让误绑定的用户无法再控制目标设备。
*
*
*   passcode是什么?
*
*   答：passcode是设备通行证，用于校验用户的绑定/控制权限。设备初次上线时，Wi-Fi模块会生成一个passcode存入设备并上传云端，
*   当用户发起设备绑定时，只要是合法操作即可获取此通行证，通过此通行证可以绑定设备，该通行证的获取时限可以在MCU串口协议中指定，
*   如果设定了获取时限，则只能在该时限内获取和绑定，避免非法用户恶意绑定设备，只有用户使用设备的启动配置功能以后，
*   才能重新获取到passcode。另外，可以通过重置模块的方式，让模块重新生成一个passcode。
*
*   productkey.JSON文件是什么？
*
*   答：机智云的SDK会在App使用的时候自动下载一个以产品的productkey命名的JSON配置文件。
*   SDK通过该配置文件，将JSON指令解析为二进制指令，传输给设备，反之同理。
*   如果SDK已经检测到该产品对应的配置文件已下载，则不会重新下载，所以更新了云端的数据点，需要重新更新配置文件。
*   可通过清空App缓存或者调用SDK的updateDeviceFromServer接口重新下载配置文件。
*
*
*******************************************************************************/

/*******************************************************************************
* 描述： 宏定义设备信息，协议版本号 、硬件版本号、软件版本号、产品秘钥
*/
#define		PRO_VER						"00000004"
#define		P0_VER						"00000004"
#define		HARD_VER					"00000001"
#define		SOFT_VER					"00000001"
#define		PRODUCT_KEY					"71d8060076e34c14af812c6ef606d131"

#define     EXT_DATA_LEN                16

/***********************************************
* 描述： 串口通信结构体
*/
#define Max_UartBuf             150

//#pragma pack(push)  //保存对齐状态
//#pragma pack(1)     //设定为4字节对齐

__packed	
typedef struct
{
	uint8_t				UART_RecvTimeoutCtr     : 6;    	
	uint8_t				UART_RecvFlag           : 1;    	
	uint8_t				Package_Flag            : 1;    	//判断是否接收到一个完整的串口数据包
	uint8_t				UART_Flag1;
	uint8_t				UART_Flag2;
	uint16_t 			UART_Count;					//串口缓冲区计算数据长度
	uint8_t				UART_Cmd_len;				//指令长度
	uint8_t 			UART_Buf[Max_UartBuf]; 		//串口缓冲区
	uint8_t             Message_Buf[Max_UartBuf];   //处理接收到指令的Buf
	uint8_t             Message_Len;	            //处理信息长度

}UART_HandleTypeDef;

/***********************************************
* 描述：所有协议指令集合
*/
typedef enum
{
    Pro_W2D_GetDeviceInfo_Cmd 				= 0x01,   //WIFI到设备 获取设备信息
    Pro_D2W__GetDeviceInfo_Ack_Cmd			= 0x02,   //设备到WIFI 回复设备信息

    Pro_W2D_P0_Cmd 							= 0x03,   //WIFI到设备 读取设备状态
    Pro_D2W_P0_Ack_Cmd						= 0x04,   //设备到WIFI 回复设备状态

	Pro_D2W_P0_Cmd 							= 0x05,   //设备到WIFI 主动上报设备状态
    Pro_W2D_P0_Ack_Cmd						= 0x06,   //WIFI不回复

	Pro_W2D_Heartbeat_Cmd 					= 0x07,   //WIFI到设备 发送心跳包
    Pro_D2W_heartbeatAck_Cmd				= 0x08,   //设备到WIFI 回复心跳包

	Pro_D2W_ControlWifi_Config_Cmd 			= 0x09,   //设备到WIFI 通知WIFI进入配置模式
    Pro_W2D_ControlWifi_Config_Ack_Cmd	    = 0x0A,   //WIFI到设备 回复模组进入配置模式

	Pro_D2W_ResetWifi_Cmd 					= 0x0B,   //设备到WIFI 发送重置WIFI模组指令
    Pro_W2D_ResetWifi_Ack_Cmd				= 0x0C,   //WIFI到设备 回复重置WIFI模组

	Pro_W2D_ReportWifiStatus_Cmd 			= 0x0D,   //WIFI到设备 发送WIFI模组工作状态的变化
    Pro_D2W_ReportWifiStatus_Ack_Cmd		= 0x0E,   //设备到WIFI 回复WIFI模组工作状态的变化

	Pro_W2D_ReportWifiReset_Cmd 			= 0x0F,   //WIFI到设备 发送请求重启MCU
    Pro_D2W_ReportWifiReset_Ack_Cmd			= 0x10,   //设备到WIFI 回复请求重启MCU

	Pro_W2D_ErrorPackage_Cmd 				= 0x11,   //WIFI到设备 发送非法数据包错误通知
    Pro_D2W_ErrorPackage_Ack_Cmd			= 0x12,   //设备到WIFI 回复非法数据包错误通知

}Pro_CmdTypeDef;

/***********************************************
* 描述：枚举WIFI模组进入配置模式的方法
*/
typedef enum
{
    SoftApMethod  = 0x01,       //SoftAp的方法配置
	AirLinkMethod = 0x02,       //AirLink的方法配置
                                //其他保留
}WifiConfigMethodTypeDef;

__packed
typedef union _uEXTENDED {
    struct _sEXTENDED {
        u32     AirVolume;
        u8      Pm2d5L;
        u8      Pm2d5H;
    } dat;
    u8          buf1[EXT_DATA_LEN];
    u16         buf2[EXT_DATA_LEN/2];
    u32         buf3[EXT_DATA_LEN/4];
} UnionExtData;
    
/***********************************************
* 描述：只读设备列表，以后要添加只读设备都添加到这里
*/
__packed	
typedef struct
{
    uint8_t         FilterLife;
	uint16_t        Voc;
    int8_t          Temperature;
	uint8_t         Humidity;
    uint16_t        PM25Val;
    uint8_t         PM25Level;
}Device_ReadTypeDef;

/***********************************************
* 描述：可写设备列表，以后要添加可写设备都添加到这里
*/
__packed	
typedef struct
{
    /***********************************************
    * 描述： 儿童锁标志位
    */
    uint8_t  ChildrenLockFlag;
    /***********************************************
    * 描述：滤网寿命
    */
    uint8_t  FilterLife;
    /***********************************************
    * 描述：按周重复
    */
    uint8_t  WeekRepeat;
    /***********************************************
    * 描述：倒计时开机
    */
    uint16_t CountDownOnMin;
    /***********************************************
    * 描述：倒计时关机
    */
    uint16_t CountDownOffMin;
    /***********************************************
    * 描述：定时开机
    */
    uint16_t TimingOn;
    /***********************************************
    * 描述：定时关机
    */
    uint16_t TimingOff;
    /***********************************************
    * 描述：儿童锁
    */
    uint8_t  ChildSecurityLock ;
    /***********************************************
    * 描述：PM2.5偏置
    */
    uint16_t Offset;
    /***********************************************
    * 描述：扩展通信
    */
    UnionExtData    Ext;

} Device_WirteTypeDef;

/***********************************************
* 描述：控制指令枚举
*/
typedef enum
{
    SetLED_OnOff    = 0x01,
	SetLED_Color    = 0x02,
	SetLED_R       	= 0x04,
	SetLED_G    	= 0x08,
	SetLED_B  		= 0x10,
	SetMotor		= 0x20,
}Attr_FlagsTypeDef;

typedef enum
{
    LED_OnOff       = 0x00,
	LED_OnOn        = 0x01,
	LED_Costom      = 0x00,
	LED_Yellow    	= 0x02,
	LED_Purple  	= 0x04,
	LED_Pink		= 0x06,

}LED_ColorTypeDef;

/***********************************************
* 描述：设备报警，故障等信息
*/
__packed	
typedef struct
{
	uint8_t		    Alert;
	uint8_t		    Fault;

}Device_RestsTypeDef;

/***********************************************
* 描述：包含设备所有参数的结构体
*/
__packed	
typedef struct
{
	Device_WirteTypeDef     Device_Wirte;
	Device_ReadTypeDef      Device_Read;
	Device_RestsTypeDef     Device_Rests;
}Device_AllTypeDef;

/***********************************************
* 描述：WiFi模组读取设备的当前状态（上报）
*/
__packed
typedef struct {
    /***********************************************
    * 描述：状态字 （bety0/bety1）
    */
    union __statusword{
        struct __ustatusword {
            uint16_t  Switch              :1;//电源开关        bool型
            uint16_t  SwitchPlasma        :1;//等离子开关      bool型
            uint16_t  LedAirQuality       :1;//空气质量指示灯  bool型
            uint16_t  ChildSecurityLock   :1;//儿童锁          bool型
            uint16_t  WindVelocity        :2;//风速
            uint16_t  AirSensitivity      :3;//空气检测灵敏度
            uint16_t  FanLevel            :3;//风机转速
            uint16_t  CtrlMode            :3;//控制模式
            uint16_t  RSV                 :1;//预留
        } udat;
        uint16_t uStatusWord;
    } StatusWord;
    /***********************************************
    * 描述：滤网寿命 （bety2）
    */
    uint8_t  FilterLife;
    /***********************************************
    * 描述：按周重复 （bety3）
    */
    uint8_t  WeekRepeat;
    /***********************************************
    * 描述：倒计时开机（bety4/bety5）
    */
    uint16_t CountDownOnMin;
    /***********************************************
    * 描述：倒计时关机 （bety6/bety7）
    */
    uint16_t CountDownOffMin;
    /***********************************************
    * 描述：定时开机 （bety8/bety9）
    */
    uint16_t TimingOn;
    /***********************************************
    * 描述：定时关机 （bety10/bety11）
    */
    uint16_t TimingOff;
    /***********************************************
    * 描述：偏置 （bety12/bety13）
    */
    uint16_t Offset;
    /***********************************************
    * 描述：扩展通信 （bety14/bety29）
    */
    UnionExtData    Ext;
    /***********************************************
    * 描述：空气质量（评级） （bety30）
    */
    uint8_t  AirQualty;
    /***********************************************
    * 描述：空气质量异味（VOC） （bety31）
    */
    uint8_t  PeculiarAirQuality;
    /***********************************************
    * 描述： 温度值 （bety32）
    */
    int8_t  Temperature;
    /***********************************************
    * 描述： 温度值 （bety33）
    */
    uint8_t  Humidity;
    /***********************************************
    * 描述：空气质量粉尘（PM2.5） （bety34、35）
    */
    uint16_t  DustAirQuality;
    /***********************************************
    * 描述：报警（bety36）
    */
    union __alert {
        struct __ualert {
            uint8_t  FilterLife :1;//滤芯寿命报警
            uint8_t  AirQuality :1;//空气质量报警
            uint8_t  RSV        :6;//预留
        }udat;
        uint8_t  uAlert;
    }Alert;
    /***********************************************
    * 描述：故障 （bety37）
    */
    union __fault {
        struct __ufault {
            uint8_t  Motor       :1;  //电机故障
            uint8_t  AirSensor   :1;  //空气传感器（VOC）故障
            uint8_t  DustSensor  :1;  //灰尘传感器故障
            uint8_t  RSV         :5;  //预留
        }udat;
        uint8_t  uFault;
    }Fault;
}Dev_StatusTypeDef;

/***********************************************
* 描述：WIFI模组控制设备结构体
*/
__packed
typedef struct
{
    /***********************************************
    * 描述：设置标志位 (只有当设置标志位位1时，后面的数据值才真正有效)
    */
    union __attrflags{
        struct __uattrflags {
           uint16_t  Switch              :1;
           uint16_t  SwitchPlasma        :1;
           uint16_t  LedAirQuality       :1;
           uint16_t  ChildSecurityLock   :1;
           uint16_t  WindVelocity        :1;
           uint16_t  AirSensitivity      :1;
           uint16_t  FanLevel            :1;
           uint16_t  CtrlMode            :1;
           uint16_t  FilterLife          :1;
           uint16_t  WeekRepeat          :1;
           uint16_t  CountDownOnMin      :1;
           uint16_t  CountDownOffMin     :1;
           uint16_t  TimingOn            :1;
           uint16_t  TimingOff           :1;
           uint16_t  Offset              :1;
           uint16_t  ExtendedData        :1;
        }udat;
        uint16_t uAttrFlags;
    }AttrFlags;

    struct __attrvals {
        /***********************************************
        * 描述：状态字 （bety0/bety1）
        */
        union __setstatusword{
            struct __usetstatusword {
                uint16_t  Switch              :1;//电源开关        bool型
                uint16_t  SwitchPlasma        :1;//等离子开关      bool型
                uint16_t  LedAirQuality       :1;//空气质量指示灯  bool型
                uint16_t  ChildSecurityLock   :1;//儿童锁    bool型
                uint16_t  WindVelocity        :2;//风速
                uint16_t  AirSensitivity      :3;//空气检测灵敏度
                uint16_t  FanLevel            :3;//风机转速
                uint16_t  CtrlMode            :3;//控制模式
                uint16_t  RSV                 :1;//预留
            }udat;
            uint16_t uSetStatusWord;
        }SetStatusWord;
        /***********************************************
        * 描述：滤网寿命 （bety2）
        */
        uint8_t  SetFilterLife;
        /***********************************************
        * 描述：按周重复 （bety3）
        */
        uint8_t  SetWeekRepeat;
        /***********************************************
        * 描述：倒计时开机（bety4/bety5）
        */
        uint16_t SetCountDownOnMin;
        /***********************************************
        * 描述：倒计时关机 （bety6/bety7）
        */
        uint16_t SetCountDownOffMin;
        /***********************************************
        * 描述：定时开机 （bety8/bety9）
        */
        uint16_t SetTimingOn;
        /***********************************************
        * 描述：定时关机 （bety10/bety11）
        */
        uint16_t SetTimingOff;
        /***********************************************
        * 描述：PM2.5偏置 （bety12/bety13）
        */
        uint16_t Offset;
        /***********************************************
        * 描述：扩展通信 （bety14/bety23）
        */
        UnionExtData    Ext;
    }AttrVals;
}AttrCtrlTypeDef;

/***********************************************
* 描述：ACK 回复参数
*/
#define Send_MaxTime   200
#define Send_MaxNum    3

__packed	
typedef struct
{
	uint8_t			SendTime;
	uint8_t			SendNum;

}Pro_Wait_AckTypeDef;

/***********************************************
* 描述：通信协议的公用部分
*/
__packed	
typedef struct
{
	uint8_t			Head[2];
	uint16_t		Len;
	uint8_t			Cmd;
	uint8_t			SN;
	uint8_t			Flags[2];

}Pro_HeadPartTypeDef;

/***********************************************
* 描述：WiFi模组请求设备信息 （用于在云端注册该设备）
*/
__packed	
typedef struct
{
	Pro_HeadPartTypeDef  		    Pro_HeadPart;
	uint8_t							Pro_ver[8];
	uint8_t							P0_ver[8];
	uint8_t							Hard_ver[8];
	uint8_t							Soft_ver[8];
	uint8_t							Product_Key[32];
	uint16_t						Binable_Time;
	uint8_t							Sum;

}Pro_M2W_ReturnInfoTypeDef;

/***********************************************
* 描述：通用命令回复帧 （心跳、ack等可以复用此帧）
*       适用一下命令帧的回复：
*       WiFi模组与设备MCU的心跳
*       设备MCU重置WiFi模组
*       WiFi模组请求重启MCU
*       Wifi模组回复
*       设备MCU回复
*/
__packed	
typedef struct
{
	Pro_HeadPartTypeDef    	Pro_HeadPart;
	uint8_t					Sum;
}Pro_CommonCmdTypeDef;

/***********************************************
* 描述：设备MCU通过WiFi模组进入配置模式
*/
__packed	
typedef struct
{
	Pro_HeadPartTypeDef  	Pro_HeadPart;
	uint8_t                 Config_Method;
	uint8_t					Sum;
}Pro_D2W_ConfigWifiTypeDef;


/***********************************************
* 描述：WiFi模组向设备MCU通知WiFi模组工作状态的变化
*/
__packed	
typedef struct
{
	Pro_HeadPartTypeDef    	Pro_HeadPart;
	union __wifistatus {
        struct __wifi_status {
            uint16_t   SoftApMode       : 1; //是否开启SoftAP模式 0：关闭 1：开启
            uint16_t   StationMode      : 1; //是否开启Station模式 0：关闭 1：开启
            uint16_t   ConfigMode       : 1; //是否开启配置模式 0：关闭 1：开启
            uint16_t   BindingMode      : 1; //是否开启绑定模式 0：关闭 1：开启
            uint16_t   ConnRouter       : 1; //WIFI模组是否成功连接路由器 0：未连接 1：连接
            uint16_t   ConnClouds       : 1; //WIFI模组是否成功连接云端   0：未连接 1：连接
            uint16_t   RSV              : 2; //bit6-bit7预留
            uint16_t   Signal           : 3; //仅当WIFI模组已成功连接路由器时才有效，表示WIFI模组当前连接AP的信号强度（RSSI） 范围是0-7 （7的信号强度最强）
            uint16_t   Phone            : 1; //是否已有绑定的手机上线 0：关闭 1：开启
            uint16_t   ProductionTest   : 1; //是否处于产测模式中 0：没有   1：有
            uint16_t   RSV1             : 3; //bit13-bit15预留
        }ustatus;
        uint16_t       uWifiStatus;
    }WifiStatus;
	uint8_t					Sum;
}Pro_W2D_WifiStatusTypeDef;

/***********************************************
* 描述：非法信息通知枚举列表
*/
typedef enum
{
    Error_AckSum = 0x01,        //校验和错误
    Error_Cmd 	 = 0x02,		//命令不可识别
    Error_Other  = 0x03,        //其他错误
}Error_PacketsTypeDef;

/***********************************************
* 描述：非法消息通知
*/
__packed	
typedef struct
{
	Pro_HeadPartTypeDef  		Pro_HeadPart;
	Error_PacketsTypeDef		Error_Packets;
	uint8_t						Sum;
}Pro_ErrorCmdTypeDef;

/***********************************************
* 描述：P0 命令列表
*/
typedef enum
{
	P0_W2D_Control_Devce_Action 		= 0x01,  //控制设备命令
    P0_W2D_ReadDevStatus_Action 		= 0x02,  //读取设备状态命令
	P0_D2W_ReadDevStatus_Action_ACK     = 0x03,  //回复读取设备状态命令
	P0_D2W_ReportDevStatus_Action       = 0X04,  //主动上报设备状态命令

}P0_ActionTypeDef;


/***********************************************
* 描述：带P0指令的公共部分
*/
__packed	
typedef struct
{
	Pro_HeadPartTypeDef  		Pro_HeadPart;
	P0_ActionTypeDef            Action;
}Pro_HeadPartP0CmdTypeDef;

/***********************************************
* 描述：WiFi模组读取设备的当前状态  （WIFI模组询问）
*/
__packed	
typedef struct
{
	Pro_HeadPartP0CmdTypeDef 			Pro_HeadPartP0Cmd;
	uint8_t								Sum;
}Pro_W2D_ReadDevStatusTypeDef;

/***********************************************
* 描述：读取设备的当前状态  （设备主动上报）
*/
__packed	
typedef struct
{
	Pro_HeadPartP0CmdTypeDef 			Pro_HeadPartP0Cmd;
	//Device_AllTypeDef       			Device_All;
    Dev_StatusTypeDef                   Dev_Status;
	uint8_t								Sum;
}Pro_D2W_ReportDevStatusTypeDef;

/***********************************************
* 描述：WiFi模组控制设备
*       WiFi模组发送
*/
__packed	
typedef struct
{

	Pro_HeadPartP0CmdTypeDef 			Pro_HeadPartP0Cmd;
	//Attr_FlagsTypeDef                 Attr_Flags;
	//Device_WirteTypeDef     			Device_Wirte;
    AttrCtrlTypeDef                     AttrCtrl;
	uint8_t							  	Sum;
}Pro_P0_ControlTypeDef;
//#pragma pack(pop)   //保存对齐状态

//extern UART_HandleTypeDef  				UART_HandleStruct;
//extern Pro_M2W_ReturnInfoTypeDef        Pro_M2W_ReturnInfoStruct;
//extern Pro_HeadPartTypeDef              Pro_HeadPartStruct;
//extern Pro_CommonCmdTypeDef 			Pro_CommonCmdStruct;
//extern Pro_Wait_AckTypeDef              Wait_AckStruct;
//extern Pro_D2W_ReportDevStatusTypeDef   Pro_D2W_ReportStatusStruct;
//extern Pro_W2D_WifiStatusTypeDef        Pro_W2D_WifiStatusStruct;
//extern Dev_StatusTypeDef                Dev_Status;
//extern AttrCtrlTypeDef                  AttrCtrl;

/*******************************************************************************
 * GLOBAL FUNCTIONS
 */
void        MessageHandle                       (void);
void        Pro_W2D_GetMcuInfo                  (void);
void        Pro_W2D_CommonCmdHandle             (void);
void        Pro_W2D_WifiStatusHandle            (void);
void        Pr0_W2D_RequestResetDeviceHandle    (void);
void        Pro_W2D_ErrorCmdHandle              (void);
void        Pro_W2D_P0CmdHandle                 (void);
void        Pro_W2D_Control_DevceHandle         (void);
void        Pro_W2D_ReadDevStatusHandle         (uint8_t ucmd);
void        Pro_D2W_ReportDevStatusHandle       (uint8_t ucmd);
    
void        Pro_UART_SendBuf                    (uint8_t *Buf, uint16_t PackLen, uint8_t Tag);
int16_t	    exchangeBytes                       (int16_t	value);
uint8_t     CheckSum                            ( uint8_t *buf, int packLen );

/*******************************************************************************
 *              end of file                                                    *
 *******************************************************************************/
#endif /*_PROTOCOL_H*/



