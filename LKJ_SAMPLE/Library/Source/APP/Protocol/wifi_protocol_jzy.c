/*******************************************************************************
 *   Filename:       Protocol.c
 *   Revised:        All copyrights reserved to Roger.
 *   Date:           2014-08-11
 *   Revision:       v1.0
 *   Writer:	     Roger-WY.
 *
 *   Description:    机智云WIFI通信协议
 *
 *
 *   Notes:
 *
 *   All copyrights reserved to Roger-WY
 *******************************************************************************/

#define  SNL_APP_SOURCE
#include <app.h>
#include <bsp_control.h>
#include <includes.h>
#include <bsp_flash.h>

#ifdef VSC_INCLUDE_SOURCE_FILE_NAMES
const  CPU_CHAR  *Protocol__c = "$Id: $";
#endif

//extern Pro_M2W_ReturnInfoTypeDef  Ctrl.Wifi.M2wRetInfo;
//extern Pro_P0_ControlTypeDef      Ctrl.Wifi.P0Ctrl;
//extern Pro_P0_ControlTypeDef      Ctrl.Wifi.P0Ctrl;
//extern Pro_D2W_ReportDevStatusTypeDef   Ctrl.Wifi.D2wRptSts;



/*******************************************************************************
* 名    称： MessageHandle
* 功    能： 串口有数据发生了，先检查数据是否合法，再解析数据帧，做相应处理
* 入口参数： 无
* 出口参数： 无
* 作　　者： Roger-WY.
* 创建日期： 2015-06-25
* 修    改：
* 修改日期：
* 备    注：
*******************************************************************************/
void MessageHandle(void)
{
    Pro_HeadPartTypeDef   Recv_HeadPart;
	memset(&Recv_HeadPart, 0, sizeof(Recv_HeadPart));
    memset(&Ctrl.Wifi.UartHandle.Message_Buf, 0, sizeof(16));
    /***********************************************
    * 描述： 串口收到一个完整的数据包
    */
    Ctrl.Wifi.UartHandle.Message_Len   = Ctrl.Wifi.UartHandle.UART_Cmd_len + 4;
    
    memcpy(&Ctrl.Wifi.UartHandle.Message_Buf, 
           Ctrl.Wifi.UartHandle.UART_Buf, 
           Ctrl.Wifi.UartHandle.Message_Len );
    memcpy(&Recv_HeadPart, 
           Ctrl.Wifi.UartHandle.Message_Buf, 
           sizeof(Recv_HeadPart));
    memset(&Ctrl.Wifi.UartHandle.UART_Buf, 
           0, 
           sizeof(Ctrl.Wifi.UartHandle.Message_Buf));
    
    Ctrl.Wifi.UartHandle.Package_Flag  = 0;
    Ctrl.Wifi.UartHandle.UART_Count    = 0;
    /***********************************************
    * 描述： 检查数据包是否出错
    */
    if(CheckSum(Ctrl.Wifi.UartHandle.Message_Buf,
                Ctrl.Wifi.UartHandle.Message_Len) != 
       Ctrl.Wifi.UartHandle.Message_Buf[Ctrl.Wifi.UartHandle.Message_Len - 1]) {
       /***********************************************
       * 描述： 数据包异常处理
       */
       Pro_W2D_ErrorCmdHandle();
       return ;
    }
    /***********************************************
    * 描述： 查看数据包发来的指令
    */
    switch (Recv_HeadPart.Cmd) { 
    /***********************************************
    * 描述： 获取MCU的设备信息，用于在云端绑定设备
    */
    case Pro_W2D_GetDeviceInfo_Cmd 				://= 0x01,   //WIFI到设备 获取设备信息
        Pro_W2D_GetMcuInfo();
        break;
    case Pro_D2W__GetDeviceInfo_Ack_Cmd			://= 0x02,   //设备到WIFI 回复设备信息
        break;
    /***********************************************
    * 描述： WiFi使用P0协议给设备发送命令处理函数
    */
    case Pro_W2D_P0_Cmd 						://= 0x03,   //WIFI到设备 读取设备状态
        Pro_W2D_P0CmdHandle();
        break;
    case Pro_D2W_P0_Ack_Cmd						://= 0x04,   //设备到WIFI 回复设备状态
        break;
    case Pro_D2W_P0_Cmd 						://= 0x05,   //设备到WIFI 主动上报设备状态
        break;
    case Pro_W2D_P0_Ack_Cmd						://= 0x06,   //WIFI不回复
        break;
    /***********************************************
    * 描述： WIFI模组发来的心跳包
    */
    case Pro_W2D_Heartbeat_Cmd 					://= 0x07,   //WIFI到设备 发送心跳包
        Pro_W2D_CommonCmdHandle();
        break;
    case Pro_D2W_heartbeatAck_Cmd				://= 0x08,   //设备到WIFI 回复心跳包
        break;
    case Pro_D2W_ControlWifi_Config_Cmd 		://= 0x09,   //设备到WIFI 通知WIFI进入配置模式
        break;
    case Pro_W2D_ControlWifi_Config_Ack_Cmd	    ://= 0x0A,   //WIFI到设备 回复模组进入配置模式
        break;
    case Pro_D2W_ResetWifi_Cmd 					://= 0x0B,   //设备到WIFI 发送重置WIFI模组指令
        break;
    case Pro_W2D_ResetWifi_Ack_Cmd				://= 0x0C,   //WIFI到设备 回复重置WIFI模组
        break;
    /***********************************************
    * 描述： WIFI模组向MCU通知工作状态的变化
    */
    case Pro_W2D_ReportWifiStatus_Cmd 			://= 0x0D,   //WIFI到设备 发送WIFI模组工作状态的变化
        Pro_W2D_WifiStatusHandle();
        break;
    case Pro_D2W_ReportWifiStatus_Ack_Cmd		://= 0x0E,   //设备到WIFI 回复WIFI模组工作状态的变化
        break;
    /***********************************************
    * 描述： WIFI模组请求重启MCU
    */
    case Pro_W2D_ReportWifiReset_Cmd 			://= 0x0F,   //WIFI到设备 发送请求重启MCU
        Pr0_W2D_RequestResetDeviceHandle();
        break;
    case Pro_D2W_ReportWifiReset_Ack_Cmd		://= 0x10,   //设备到WIFI 回复请求重启MCU
        break;
    /***********************************************
    * 描述： 非法指令处理
    */
    case Pro_W2D_ErrorPackage_Cmd 				://= 0x11,   //WIFI到设备 发送非法数据包错误通知
        Pro_W2D_ErrorCmdHandle();
        break;
    case Pro_D2W_ErrorPackage_Ack_Cmd			://= 0x12,   //设备到WIFI 回复非法数据包错误通知
        break;
    default:
        break;
    }
}

/*******************************************************************************
* 名    称： Pro_GetMcuInfo
* 功    能： WiFi模组请求设备信息
* 入口参数： 无
* 出口参数： 无
* 作　　者： Roger-WY.
* 创建日期： 2015-06-25
* 修    改：
* 修改日期：
* 备    注：
*******************************************************************************/
void Pro_W2D_GetMcuInfo(void)
{
	memcpy(&Ctrl.Wifi.CommCmd, 
           Ctrl.Wifi.UartHandle.Message_Buf, 
           sizeof(Ctrl.Wifi.CommCmd));
	Ctrl.Wifi.M2wRetInfo.Pro_HeadPart.SN    = Ctrl.Wifi.CommCmd.Pro_HeadPart.SN;
	Ctrl.Wifi.M2wRetInfo.Sum                = CheckSum((uint8_t *)&Ctrl.Wifi.M2wRetInfo,
                                                           sizeof(Ctrl.Wifi.M2wRetInfo));
	Pro_UART_SendBuf(                            (uint8_t *)&Ctrl.Wifi.M2wRetInfo,
                                                  sizeof(Ctrl.Wifi.M2wRetInfo), 0);

	//Log_UART_SendBuf((uint8_t *)&Ctrl.Wifi.M2wRetInfo,sizeof(Ctrl.Wifi.M2wRetInfo));

    /******************************输出日志*********************************************/
    //	printf("W2D_GetMcuInfo...\r\n");
    //	printf("PRO_VER:"); 	printf(PRO_VER); 		printf("\r\n");
    //	printf("P0_VER:");		printf(P0_VER);			printf("\r\n");
    //	printf("P0_VER:");      printf(HARD_VER);		printf("\r\n");
    //	printf("SOFT_VER:");    printf(SOFT_VER);		printf("\r\n");
    //	printf("PRODUCT_KEY:"); printf(PRODUCT_KEY);    printf("\r\n");
    /***********************************************************************************/
}

/*******************************************************************************
* 名    称： Pro_W2D_CommonCmdHandle
* 功    能：  1，WiFi模组与设备MCU的心跳(4.2)
*             2，设备MCU通知WiFi模组进入配置模式(4.3)
*             3，设备MCU重置WiFi模组(4.4)
*             4, WiFi模组请求重启MCU(4.6)
*             5, WiFi模组请求重启MCU ( 4.9 WiFi模组主动上报当前的状态)
*             6，设备MCU回复 (WiFi模组控制设备)
*                   4.6 	WiFi模组请求重启MCU
*                   4.9 	Wifi模组回复
*                   4.10    设备MCU回复
* 入口参数： 无
* 出口参数： 无
* 作　　者： Roger-WY.
* 创建日期： 2015-06-25
* 修    改：
* 修改日期：
* 备    注：
*******************************************************************************/
void Pro_W2D_CommonCmdHandle(void)
{
	/***********************************************
    * 描述： 将串口收到的数据包拷贝到通用命令处理结构体中
    */
    memcpy(&Ctrl.Wifi.CommCmd, 
           Ctrl.Wifi.UartHandle.Message_Buf, 
           sizeof(Ctrl.Wifi.CommCmd));
	/***********************************************
    * 描述： 对当前指令加1成为对应的回复指令
    */
    Ctrl.Wifi.CommCmd.Pro_HeadPart.Cmd    = Ctrl.Wifi.CommCmd.Pro_HeadPart.Cmd + 1;
	Ctrl.Wifi.CommCmd.Sum                 = CheckSum((uint8_t *)&Ctrl.Wifi.CommCmd, 
                                                       sizeof(Ctrl.Wifi.CommCmd));
	Pro_UART_SendBuf(                        (uint8_t *)&Ctrl.Wifi.CommCmd,
                                              sizeof(Ctrl.Wifi.CommCmd), 0);
	memset(&Ctrl.Wifi.CommCmd, 
           0, 
           sizeof(Ctrl.Wifi.CommCmd));
}

/*******************************************************************************
* 名    称： Pro_W2D_WifiStatusHandle
* 功    能： 将WiFi的状态保存到 Pro_W2D_WifiStatusStruct中。并回复ACK
* 入口参数： 无
* 出口参数： 无
* 作　　者： Roger-WY.
* 创建日期： 2015-06-25
* 修    改：
* 修改日期：
* 备    注：
*******************************************************************************/
void Pro_W2D_WifiStatusHandle(void)
{
	memcpy(&Ctrl.Wifi.W2dWifiSts, 
           Ctrl.Wifi.UartHandle.Message_Buf, 
           sizeof(Ctrl.Wifi.W2dWifiSts));
	Ctrl.Wifi.W2dWifiSts.WifiStatus.uWifiStatus     = exchangeBytes(Ctrl.Wifi.W2dWifiSts.WifiStatus.uWifiStatus);
    Pro_W2D_CommonCmdHandle();
}


/*******************************************************************************
* 名    称： Pr0_W2D_RequestResetDeviceHandle
* 功    能： WiFi模组请求复位设备MCU，MCU回复ACK，并执行设备复位
* 入口参数： 无
* 出口参数： 无
* 作　　者： Roger-WY.
* 创建日期： 2015-06-25
* 修    改：
* 修改日期：
* 备    注： 为了避免 WIFI模组没有收到确认指令而重发指令造成MCU多次重启
*            需要在回复WIFI模组下等待600MS后重启
*******************************************************************************/
void Pr0_W2D_RequestResetDeviceHandle(void)
{
	OS_ERR err;

    Pro_W2D_CommonCmdHandle();

    OSTimeDly(600, OS_OPT_TIME_DLY, &err);

    /****************************在这里添加设备复位函数****************************/
	__set_FAULTMASK(1); // 关闭所有中断
	NVIC_SystemReset(); // 系统复位

    /******************************************************************************/
}

/*******************************************************************************
* 名    称： Pro_W2D_ErrorCmdHandle
* 功    能： WiFi发送收到非法信息通知，设备MCU回复ACK，并执行相应的动作
* 入口参数： 无
* 出口参数： 无
* 作　　者： Roger-WY.
* 创建日期： 2015-06-25
* 修    改：
* 修改日期：
* 备    注：
*******************************************************************************/
extern OS_SEM  SemReportStatus;            //主动上报状态信号量
void Pro_W2D_ErrorCmdHandle(void)
{
	Pro_ErrorCmdTypeDef           	 Pro_ErrorCmdStruct;       //4.7 非法消息通知
	memcpy(&Pro_ErrorCmdStruct, Ctrl.Wifi.UartHandle.Message_Buf, sizeof(Pro_ErrorCmdStruct));

	Pro_ErrorCmdStruct.Pro_HeadPart.Cmd = Pro_ErrorCmdStruct.Pro_HeadPart.Cmd;
	Pro_ErrorCmdStruct.Sum = CheckSum((uint8_t *)&Ctrl.Wifi.CommCmd, sizeof(Ctrl.Wifi.CommCmd));
	Pro_UART_SendBuf((uint8_t *)&Ctrl.Wifi.CommCmd,sizeof(Ctrl.Wifi.CommCmd), 0);

	/*************************在这里添加故障处理函数*****************************/
	switch (Pro_ErrorCmdStruct.Error_Packets)
	{
		case Error_AckSum:
        //printf("W2D Error Command ->Error_AckSum\r\n");
        break;
		case Error_Cmd:
        //printf("W2D Error Command ->Error_Cmd\r\n");
        break;
		case Error_Other:
        //printf("W2D Error Command ->Error_Other\r\n");
        break;
		default:
        break;
	}
    /******************************************************************************/
}

/*******************************************************************************
* 名    称： Pro_W2D_P0CmdHandle
* 功    能： WiFi 使用P0协议给设备发送命令处理函数
* 入口参数： 无
* 出口参数： 无
* 作　　者： Roger-WY.
* 创建日期： 2015-06-25
* 修    改：
* 修改日期：
* 备    注：
*******************************************************************************/
void Pro_W2D_P0CmdHandle(void)
{
	Pro_P0_ControlTypeDef      			 P0Ctrl; 			 //WiFi模组控制设备

	memcpy(&P0Ctrl, 
           Ctrl.Wifi.UartHandle.Message_Buf, 
           sizeof(P0Ctrl));
    
	switch (P0Ctrl.Pro_HeadPartP0Cmd.Action){
		case P0_W2D_Control_Devce_Action:       //控制设备命令
            Pro_W2D_Control_DevceHandle();                 
        break;
		case P0_W2D_ReadDevStatus_Action:       //读取设备状态命令
            Pro_W2D_ReadDevStatusHandle(0);
        break;
		case P0_D2W_ReportDevStatus_Action:     //主动上报设备状态命令
            Pro_D2W_ReportDevStatusHandle(1);
        break;

		default:
        break;
	}
}

/*******************************************************************************
* 名    称： Pro_W2D_Control_DevceHandle
* 功    能： WiFi 使用P0协议给设备发送命令处理函数
* 入口参数： 无
* 出口参数： 无
* 作　　者： Roger-WY.
* 创建日期： 2015-06-25
* 修    改：
* 修改日期：
* 备    注：
*******************************************************************************/
void Pro_W2D_Control_DevceHandle(void)
{
	//static Pro_P0_ControlTypeDef    	 Pro_P0_ControlStruct; 			 //4WiFi模组控制设备
	Pro_W2D_CommonCmdHandle();                                           //回复WIFI模组已收到数据
    
    static Pro_P0_ControlTypeDef    	 LastProP0Ctrl;
    u8  len = sizeof(Pro_P0_ControlTypeDef);
    
	/***********************************************
    * 描述： 拷贝串口缓冲区数据，处理并解析
    */
    memcpy(&Ctrl.Wifi.P0Ctrl, Ctrl.Wifi.UartHandle.Message_Buf, sizeof(Ctrl.Wifi.P0Ctrl));
    
    /***********************************************
    * 描述： 检测当前数据是否与上次相同，直接返回
    */
    if ( 0 == memcmp(&LastProP0Ctrl, &Ctrl.Wifi.P0Ctrl, len ) )   {
        memset(&Ctrl.Wifi.P0Ctrl, 0, sizeof(Ctrl.Wifi.P0Ctrl));
        return;
    }
    // 保存当前数据
    memcpy(&LastProP0Ctrl, &Ctrl.Wifi.P0Ctrl, len);
    
    /***********************************************
    * 描述： 处理一下接收到的数据
    */
    Ctrl.Wifi.P0Ctrl.AttrCtrl.AttrFlags.uAttrFlags
        = exchangeBytes(Ctrl.Wifi.P0Ctrl.AttrCtrl.AttrFlags.uAttrFlags);
    Ctrl.Wifi.P0Ctrl.AttrCtrl.AttrVals.SetStatusWord.uSetStatusWord
        = exchangeBytes(Ctrl.Wifi.P0Ctrl.AttrCtrl.AttrVals.SetStatusWord.uSetStatusWord);
    Ctrl.Wifi.P0Ctrl.AttrCtrl.AttrVals.SetCountDownOnMin
        = exchangeBytes(Ctrl.Wifi.P0Ctrl.AttrCtrl.AttrVals.SetCountDownOnMin);
    Ctrl.Wifi.P0Ctrl.AttrCtrl.AttrVals.SetCountDownOffMin
        = exchangeBytes(Ctrl.Wifi.P0Ctrl.AttrCtrl.AttrVals.SetCountDownOffMin);
    Ctrl.Wifi.P0Ctrl.AttrCtrl.AttrVals.SetTimingOn
        = exchangeBytes(Ctrl.Wifi.P0Ctrl.AttrCtrl.AttrVals.SetTimingOn);
    Ctrl.Wifi.P0Ctrl.AttrCtrl.AttrVals.SetTimingOff
        = exchangeBytes(Ctrl.Wifi.P0Ctrl.AttrCtrl.AttrVals.SetTimingOff);
    
    //==========================添加更多可写设备==============================//
    //首先判断设置的标志位是否置位，再往相应的参数里写值
    //========================================================================//
    /***********************************************
    * 描述： 修改开关状态
    */
    if(Ctrl.Wifi.P0Ctrl.AttrCtrl.AttrFlags.udat.Switch) {
        Ctrl.Wifi.P0Ctrl.AttrCtrl.AttrFlags.udat.Switch = 0;
        Ctrl.Sys.SysSts =  Ctrl.Wifi.P0Ctrl.AttrCtrl.AttrVals.SetStatusWord.udat.Switch;
        //       Ctrl.Wifi.D2wRptSts.Dev_Status.StatusWord.udat.Switch =  Ctrl.Sys.SysSts;
        
        if(Ctrl.Sys.SysSts) {
            /***********************************************
            * 描述： 开机声音
            */
            Ctrl.Fan.CurScale   = FAN_DUTY_SCALE_0;
            Ctrl.Fan.Mode       = FAN_MODE_SMART;
            Beep( BEEP_FLAG_PWR_ON );
        } else {
            Ctrl.Fan.Mode       = FAN_MODE_PWR_OFF;
            Ctrl.Fan.CurScale   = FAN_DUTY_SCALE_0;
            /***********************************************
            * 描述： 关机声音
            */
            Beep( BEEP_FLAG_PWR_OFF );
        }
    }
    /***********************************************
    * 描述： 修改负离子开关状态
    */
    if(Ctrl.Wifi.P0Ctrl.AttrCtrl.AttrFlags.udat.SwitchPlasma) {
        Ctrl.Wifi.P0Ctrl.AttrCtrl.AttrFlags.udat.SwitchPlasma = 0;
        Beep( BEEP_FLAG_KEY_EVT );
    }
    /***********************************************
    * 描述： 修改空气质量灯状态
    */
    if(Ctrl.Wifi.P0Ctrl.AttrCtrl.AttrFlags.udat.LedAirQuality) {
        Ctrl.Wifi.P0Ctrl.AttrCtrl.AttrFlags.udat.LedAirQuality = 0;
        Beep( BEEP_FLAG_KEY_EVT );
    }
    /***********************************************
    * 描述： 修改儿童锁状态
    */
    if(Ctrl.Wifi.P0Ctrl.AttrCtrl.AttrFlags.udat.ChildSecurityLock) {
        Ctrl.Wifi.P0Ctrl.AttrCtrl.AttrFlags.udat.ChildSecurityLock = 0;
        Ctrl.Wifi.Wr.ChildSecurityLock = Ctrl.Wifi.P0Ctrl.AttrCtrl.AttrVals.SetStatusWord.udat.ChildSecurityLock;
        Beep( BEEP_FLAG_KEY_EVT );
    }
    /***********************************************
    * 描述： 修改风速状态
    */
    if(Ctrl.Wifi.P0Ctrl.AttrCtrl.AttrFlags.udat.WindVelocity) {
        Ctrl.Wifi.P0Ctrl.AttrCtrl.AttrFlags.udat.WindVelocity = 0;
        Beep( BEEP_FLAG_KEY_EVT );
    }
    /***********************************************
    * 描述： 修改空气检测灵敏度状态
    */
    if(Ctrl.Wifi.P0Ctrl.AttrCtrl.AttrFlags.udat.AirSensitivity) {
        Ctrl.Wifi.P0Ctrl.AttrCtrl.AttrFlags.udat.AirSensitivity = 0;
        Ctrl.Led.SetLevel   = Ctrl.Wifi.P0Ctrl.AttrCtrl.AttrVals.SetStatusWord.udat.AirSensitivity;
        Beep( BEEP_FLAG_KEY_EVT );
    }
    /***********************************************
    * 描述： 修改风速状态
    */
    if(Ctrl.Wifi.P0Ctrl.AttrCtrl.AttrFlags.udat.FanLevel) {
        Ctrl.Wifi.P0Ctrl.AttrCtrl.AttrFlags.udat.FanLevel = 0;
        Ctrl.Fan.Level  = Ctrl.Wifi.P0Ctrl.AttrCtrl.AttrVals.SetStatusWord.udat.FanLevel;
        Ctrl.Fan.Mode   = FAN_MODE_HAND;
        Beep( BEEP_FLAG_KEY_EVT );
    }
    /***********************************************
    * 描述： 修改控制模式状态
    */
    if(Ctrl.Wifi.P0Ctrl.AttrCtrl.AttrFlags.udat.CtrlMode) {
        Ctrl.Wifi.P0Ctrl.AttrCtrl.AttrFlags.udat.CtrlMode = 0;
        Ctrl.Fan.Mode =  Ctrl.Wifi.P0Ctrl.AttrCtrl.AttrVals.SetStatusWord.udat.CtrlMode;        
        Beep( BEEP_FLAG_KEY_EVT );
    }
    /***********************************************
    * 描述： 修改滤芯寿命
    */
    if(Ctrl.Wifi.P0Ctrl.AttrCtrl.AttrFlags.udat.FilterLife) {
        Ctrl.Wifi.P0Ctrl.AttrCtrl.AttrFlags.udat.FilterLife = 0;
        Ctrl.Wifi.Rd.FilterLife         = 100;
        Ctrl.Wifi.Wr.FilterLife         = 100;
        Ctrl.Para.dat.FilterLife.Life   = FILTER_LIFE_TIME;
        BSP_FlashWriteBytes((u32) 0*2, (u8 *)&Ctrl.Para.dat.FilterLife, (u16) 2*2);
        Beep( BEEP_FLAG_KEY_EVT );
    }
    /***********************************************
    * 描述： 修改按周重复的值
    */
    if(Ctrl.Wifi.P0Ctrl.AttrCtrl.AttrFlags.udat.WeekRepeat) {
        Ctrl.Wifi.P0Ctrl.AttrCtrl.AttrFlags.udat.WeekRepeat = 0;
        Ctrl.Wifi.Wr.WeekRepeat = Ctrl.Wifi.P0Ctrl.AttrCtrl.AttrVals.SetWeekRepeat;
        Beep( BEEP_FLAG_KEY_EVT );
    }
    /***********************************************
    * 描述： 修改倒计时开机的值
    */
    if(Ctrl.Wifi.P0Ctrl.AttrCtrl.AttrFlags.udat.CountDownOnMin) {
        Ctrl.Wifi.P0Ctrl.AttrCtrl.AttrFlags.udat.CountDownOnMin = 0;
        Ctrl.Wifi.Wr.CountDownOnMin = Ctrl.Wifi.P0Ctrl.AttrCtrl.AttrVals.SetCountDownOnMin;
        Beep( BEEP_FLAG_KEY_EVT );
    }
    /***********************************************
    * 描述： 修改倒计时关机的值
    */
    if(Ctrl.Wifi.P0Ctrl.AttrCtrl.AttrFlags.udat.CountDownOffMin) {
        Ctrl.Wifi.P0Ctrl.AttrCtrl.AttrFlags.udat.CountDownOffMin = 0;
        Ctrl.Wifi.Wr.CountDownOffMin    = Ctrl.Wifi.P0Ctrl.AttrCtrl.AttrVals.SetCountDownOffMin;
        Beep( BEEP_FLAG_KEY_EVT );
    }
    /***********************************************
    * 描述： 修改定时开机的值
    */
    if(Ctrl.Wifi.P0Ctrl.AttrCtrl.AttrFlags.udat.TimingOn) {
        Ctrl.Wifi.P0Ctrl.AttrCtrl.AttrFlags.udat.TimingOn = 0;
        Ctrl.Wifi.Wr.TimingOn = Ctrl.Wifi.P0Ctrl.AttrCtrl.AttrVals.SetTimingOn;
        Beep( BEEP_FLAG_KEY_EVT );
    }
    /***********************************************
    * 描述： 修改定时关机的值
    */
    if(Ctrl.Wifi.P0Ctrl.AttrCtrl.AttrFlags.udat.TimingOff) {
        Ctrl.Wifi.P0Ctrl.AttrCtrl.AttrFlags.udat.TimingOff = 0;
        Ctrl.Wifi.Wr.TimingOff = Ctrl.Wifi.P0Ctrl.AttrCtrl.AttrVals.SetTimingOff;
        Beep( BEEP_FLAG_KEY_EVT );
    }
    /***********************************************
    * 描述： 修改PM2.5标定偏置值
    */
    if(Ctrl.Wifi.P0Ctrl.AttrCtrl.AttrFlags.udat.Offset) {
        Ctrl.Wifi.P0Ctrl.AttrCtrl.AttrFlags.udat.Offset = 0;
        INT16S  offset      = exchangeBytes(Ctrl.Wifi.P0Ctrl.AttrCtrl.AttrVals.Offset)
                             - 2000;
        
        /***********************************************
        * 描述： 修改PM2.5标定低值
        */
#define OFFSET_VALUE_L    400
#define OFFSET_VALUE_H    600
        /***********************************************
        * 描述： 恢复默认
        */
        if ( offset == 1000 ) {
            BSP_PM2D5Chk( PM2D5_CHK_TYPE_DEF, 0 );
        /***********************************************
        * 描述： 校准低点
        */
        } else if ( ( offset > OFFSET_VALUE_L ) && ( offset < OFFSET_VALUE_H ) ) {
            offset  = offset - OFFSET_VALUE_L;
            /***********************************************
            * 描述： 修改PM2.5标定低值
            */
            BSP_PM2D5Chk( PM2D5_CHK_TYPE_K_Y1, offset );
        /***********************************************
        * 描述： 校准高点
        */
        } else if ( offset > OFFSET_VALUE_H ) {
            offset  = offset - OFFSET_VALUE_H;
            BSP_PM2D5Chk( PM2D5_CHK_TYPE_K_Y2, offset );
        /***********************************************
        * 描述： 校准偏移
        */
        } else {
            BSP_PM2D5Chk( PM2D5_CHK_TYPE_B_OFFSET, offset );
        }
    
        Beep( BEEP_FLAG_KEY_EVT );
            
        /***********************************************
        * 描述： 保存校准值
        */
#if (STORAGE_TYPE == INTNER_FLASH ) 
        //BSP_FlashWriteBytes((u32) 10*2, (u8 *)&Ctrl.Para.dat.Pm2d5Chk.Offset, (u16) 10*2);
#elif (STORAGE_TYPE == INTNER_EEPROM )
#elif (STORAGE_TYPE == EXTERN_EEPROM )
        BSP_EEP_WriteBytes(Ctrl.Para.dat.idx->PM2D5Offset, 
                           &Ctrl.Para.buf1[2 * Ctrl.Para.dat.idx->PM2D5Offset],
                           2 );
#endif
    }
    /***********************************************
    * 描述： 修改扩展数据
    */
    if(Ctrl.Wifi.P0Ctrl.AttrCtrl.AttrFlags.udat.ExtendedData) {
        Ctrl.Wifi.P0Ctrl.AttrCtrl.AttrFlags.udat.ExtendedData = 0;
        memcpy(&Ctrl.Wifi.Wr.Ext.buf1[0],
               &Ctrl.Wifi.P0Ctrl.AttrCtrl.AttrVals.Ext.buf1[0],EXT_DATA_LEN);
        /***********************************************
        * 描述： 修改PM2.5标定低值
        */
        char i  = 0;
        if ( ( Ctrl.Wifi.Wr.Ext.dat.Pm2d5L != 0 ) ||
             ( Ctrl.Wifi.Wr.Ext.dat.Pm2d5H != 0 ) ){ 
                 
            if ( Ctrl.Wifi.Wr.Ext.dat.Pm2d5L != 0 ) {
                if ( Ctrl.PM2D5.Type    == 0 ) {
                    Ctrl.Para.dat.Pm2d5Chk.Value.y1[0]    = Ctrl.Wifi.P0Ctrl.AttrCtrl.AttrVals.Ext.dat.Pm2d5L;
                    Ctrl.Para.dat.Pm2d5Chk.Adc.x1[0]      = Ctrl.PM2D5.G1AdVal;
                } else {
                    Ctrl.Para.dat.Pm2d5Chk.Value.y1[1]    = Ctrl.Wifi.P0Ctrl.AttrCtrl.AttrVals.Ext.dat.Pm2d5L;
                    Ctrl.Para.dat.Pm2d5Chk.Adc.x1[1]      = Ctrl.PM2D5.G2AdVal;
                }
            } else if ( Ctrl.Wifi.Wr.Ext.dat.Pm2d5H != 0 ) {
                i  = 1;
                if ( Ctrl.PM2D5.Type    == 0 ) {
                    Ctrl.Para.dat.Pm2d5Chk.Value.y2[0]    = Ctrl.Wifi.P0Ctrl.AttrCtrl.AttrVals.Ext.dat.Pm2d5H;
                    Ctrl.Para.dat.Pm2d5Chk.Adc.x2[0]      = Ctrl.PM2D5.G1AdVal;
                } else {
                    Ctrl.Para.dat.Pm2d5Chk.Value.y2[1]    = Ctrl.Wifi.P0Ctrl.AttrCtrl.AttrVals.Ext.dat.Pm2d5H; 
                    Ctrl.Para.dat.Pm2d5Chk.Adc.x2[1]      = Ctrl.PM2D5.G2AdVal;
                }
            }
                
            Ctrl.Wifi.Wr.Offset = 0;
            Ctrl.Para.dat.Pm2d5Chk.Offset.b[i]       = Ctrl.Wifi.Wr.Offset;
            Ctrl.PM2D5.k[i]     = ((float)(Ctrl.Para.dat.Pm2d5Chk.Value.y2[i] - Ctrl.Para.dat.Pm2d5Chk.Value.y1[i])
                                / (float)(Ctrl.Para.dat.Pm2d5Chk.Adc.x2[i] - Ctrl.Para.dat.Pm2d5Chk.Adc.x1[i]));
            Ctrl.PM2D5.b[i]     = (Ctrl.Para.dat.Pm2d5Chk.Value.y1[i] - Ctrl.PM2D5.k[i] *Ctrl.Para.dat.Pm2d5Chk.Adc.x1[i]);  
    
            BSP_FlashWriteBytes((u32) 10*2, (u8 *)&Ctrl.Para.dat.Pm2d5Chk.Offset, (u16) 10*2);
        }
    }
    //========================================================================//
    /***********************************************
    * 描述： 主动上报设备的最新状态
    */    
	//OSSemPost ((OS_SEM  *)&SemReportStatus,
    //           (OS_OPT   )OS_OPT_POST_1,
    //           (OS_ERR  *)&err);
	memset(&Ctrl.Wifi.P0Ctrl, 0, sizeof(Ctrl.Wifi.P0Ctrl));
}


/*******************************************************************************
* 名    称： Pro_W2D_ReadDevStatusHandle
* 功    能： 读取设备状态处理
* 入口参数： 无
* 出口参数： 无
* 作　　者： Roger-WY.
* 创建日期： 2015-06-25
* 修    改：
* 修改日期：
* 备    注：
*******************************************************************************/
void Pro_W2D_ReadDevStatusHandle(uint8_t ucmd)
{
	Pro_D2W_ReportDevStatusHandle(ucmd);
}

/*******************************************************************************
* 名    称： Pro_D2W_ReportDevStatusHandle
* 功    能： 上报设备状态处理
* 入口参数： ucmd;处理是主动上报还是询问回复 0：表示询问回复 1：表示主动上报
* 出口参数： 无
* 作　　者： Roger-WY.
* 创建日期： 2015-06-25
* 修    改：
* 修改日期：
* 备    注：
*******************************************************************************/
void Pro_D2W_ReportDevStatusHandle(uint8_t ucmd)
{
	Ctrl.Wifi.D2wRptSts.Pro_HeadPartP0Cmd.Pro_HeadPart.SN = Ctrl.Wifi.SN++;
    switch(ucmd) {
        case 0:{
           Ctrl.Wifi.D2wRptSts.Pro_HeadPartP0Cmd.Action = P0_D2W_ReadDevStatus_Action_ACK;//WIFI模组询问设备状态//回复读取设备状态命令
        };break;
        case 1:{
           Ctrl.Wifi.D2wRptSts.Pro_HeadPartP0Cmd.Action = P0_D2W_ReportDevStatus_Action;  //主动上报设备状态
        };break;
        default:break;

    }
	/***********************************************
    * 描述： 上报设备状态字
    */
    Ctrl.Wifi.D2wRptSts.Dev_Status.StatusWord.udat.Switch            = Ctrl.Sys.SysSts;
    Ctrl.Wifi.D2wRptSts.Dev_Status.StatusWord.udat.ChildSecurityLock = Ctrl.Wifi.Wr.ChildSecurityLock;
    Ctrl.Wifi.D2wRptSts.Dev_Status.StatusWord.udat.FanLevel          = Ctrl.Fan.Level;
    Ctrl.Wifi.D2wRptSts.Dev_Status.StatusWord.udat.CtrlMode          = Ctrl.Fan.Mode;
    Ctrl.Wifi.D2wRptSts.Dev_Status.StatusWord.udat.AirSensitivity    = Ctrl.Led.SetLevel;

    Ctrl.Wifi.D2wRptSts.Dev_Status.StatusWord.uStatusWord = exchangeBytes(Ctrl.Wifi.D2wRptSts.Dev_Status.StatusWord.uStatusWord);
    /***********************************************
    * 描述： 上报设备滤网寿命
    */
    Ctrl.Wifi.D2wRptSts.Dev_Status.FilterLife       = Ctrl.Wifi.Rd.FilterLife;
    /***********************************************
    * 描述： 上报设备按周重复的值
    */
    Ctrl.Wifi.D2wRptSts.Dev_Status.WeekRepeat       = Ctrl.Wifi.Wr.WeekRepeat;
    /***********************************************
    * 描述： 上报设备倒计时开机的值
    */
    Ctrl.Wifi.D2wRptSts.Dev_Status.CountDownOnMin   = exchangeBytes(Ctrl.Wifi.Wr.CountDownOnMin);
    /***********************************************
    * 描述： 上报设备倒计时关机的值
    */
    Ctrl.Wifi.D2wRptSts.Dev_Status.CountDownOffMin  = exchangeBytes(Ctrl.Wifi.Wr.CountDownOffMin);
    /***********************************************
    * 描述： 上报设备定时开机的值
    */
    Ctrl.Wifi.D2wRptSts.Dev_Status.TimingOn         = exchangeBytes(Ctrl.Wifi.Wr.TimingOn);
    /***********************************************
    * 描述： 上报设备定时关机的值
    */
    Ctrl.Wifi.D2wRptSts.Dev_Status.TimingOff        = exchangeBytes(Ctrl.Wifi.Wr.TimingOff);
    
    /***********************************************
    * 描述： 上报PM2.5偏置
    */
    Ctrl.Wifi.D2wRptSts.Dev_Status.Offset           = exchangeBytes(Ctrl.Wifi.Wr.Offset);
    
    /***********************************************
    * 描述： 上报扩展数据
    */
    memcpy(&Ctrl.Wifi.D2wRptSts.Dev_Status.Ext.buf1[0],
           &Ctrl.Wifi.Wr.Ext.buf1[0],EXT_DATA_LEN);    
    
    /***********************************************
    * 描述： PM2.5的值
    */
    Ctrl.Wifi.D2wRptSts.Dev_Status.DustAirQuality  = exchangeBytes(Ctrl.Wifi.Rd.PM25Val);
    Ctrl.Wifi.D2wRptSts.Dev_Status.AirQualty       = Ctrl.Wifi.Rd.PM25Level;

    /***********************************************
    * 描述： 温湿度的值
    */
    Ctrl.Wifi.D2wRptSts.Dev_Status.Temperature     = Ctrl.Wifi.Rd.Temperature + 45;//协议中中，加上了最小值-45，所以加上45才是真实值
    Ctrl.Wifi.D2wRptSts.Dev_Status.Humidity        = Ctrl.Wifi.Rd.Humidity;


	Ctrl.Wifi.D2wRptSts.Sum = CheckSum((uint8_t *)&Ctrl.Wifi.D2wRptSts, sizeof(Ctrl.Wifi.D2wRptSts));

    Pro_UART_SendBuf((uint8_t *)&Ctrl.Wifi.D2wRptSts,sizeof(Ctrl.Wifi.D2wRptSts), 0);
}

/*******************************************************************************
* 名    称： Pro_UART_SendBuf
* 功    能： 向串口发送数据帧
* 入口参数： buf:数据起始地址； packLen:数据长度； tag=0,不等待ACK；tag=1,等待ACK；
* 出口参数： 无
* 作　　者： Roger-WY.
* 创建日期： 2015-06-25
* 修    改：
* 修改日期：
* 备    注： 若等待ACK，按照协议失败重发3次；数据区出现FF，在其后增加55
*******************************************************************************/
void Pro_UART_SendBuf(uint8_t *Buf, uint16_t PackLen, uint8_t Tag)
{
	uint16_t i;
	Pro_HeadPartTypeDef   Send_HeadPart;
	Pro_HeadPartTypeDef   Recv_HeadPart;

	for(i=0;i<PackLen;i++){
		USART_Send_Char(USART3,Buf[i]);
		if(i >=2 && Buf[i] == 0xFF) USART_Send_Char(USART3,0x55);
	}

	if(Tag == 0) return ;

	memcpy(&Send_HeadPart, Buf, sizeof(Send_HeadPart));
	memset(&Ctrl.Wifi.WaitAck, 0, sizeof(Ctrl.Wifi.WaitAck));

	while(Ctrl.Wifi.WaitAck.SendNum < Send_MaxNum) {
		if(Ctrl.Wifi.WaitAck.SendTime < Send_MaxTime) {
			if(Ctrl.Wifi.UartHandle.Package_Flag) {
				memcpy(&Recv_HeadPart, Ctrl.Wifi.UartHandle.UART_Buf, sizeof(Recv_HeadPart));
				Ctrl.Wifi.UartHandle.Package_Flag = 0;
				if((Send_HeadPart.Cmd == (Recv_HeadPart.Cmd - 1)) && (Send_HeadPart.SN == Recv_HeadPart.SN))
					break;
			}
		} else {
			Ctrl.Wifi.WaitAck.SendTime = 0;
			for(i=0;i<PackLen;i++) {
				USART_Send_Char(USART3,Buf[i]);;
				if(i >=2 && Buf[i] == 0xFF)
					USART_Send_Char(USART3,0x55);
			}
			Ctrl.Wifi.WaitAck.SendNum++ ;
		}
	}
}

/*******************************************************************************
* 名    称： Log_UART_SendBuf
* 功    能： 向串口发送数据帧
* 入口参数： buf:数据起始地址； packLen:数据长度； tag=0,不等待ACK；tag=1,等待ACK；
* 出口参数： 无
* 作　　者： Roger-WY.
* 创建日期： 2015-06-25
* 修    改：
* 修改日期：
* 备    注： 若等待ACK，按照协议失败重发3次；数据区出现FF，在其后增加55
*******************************************************************************/
void Log_UART_SendBuf(uint8_t *Buf, uint16_t PackLen)
{
	uint16_t i;
	for(i=0;i<PackLen;i++){
		USART_Send_Char(USART1,Buf[i]);
		if(i >=2 && Buf[i] == 0xFF)
            USART_Send_Char(USART1,0x55);
	}
}

/*******************************************************************************
* 名    称： exchangeBytes
* 功    能： 模拟的htons 或者 ntohs，如果系统支字节序更改可直接替换成系统函数
* 入口参数： value
* 出口参数： 更改过字节序的short数值
* 作　　者： Roger-WY.
* 创建日期： 2015-06-25
* 修    改：
* 修改日期：
* 备    注：
*******************************************************************************/
int16_t	exchangeBytes(int16_t	value)
{
	int16_t		tmp_value;
	uint8_t		*index_1, *index_2;

	index_1 = (uint8_t *)&tmp_value;
	index_2 = (uint8_t *)&value;

	*index_1 = *(index_2+1);
	*(index_1+1) = *index_2;

	return tmp_value;
}

/*******************************************************************************
* 名    称： CheckSum
* 功    能： 校验和算法
* 入口参数： buf:数据起始地址； packLen:数据长度；
* 出口参数： 校验码
* 作　　者： Roger-WY.
* 创建日期： 2015-06-25
* 修    改：
* 修改日期：
* 备    注：
*******************************************************************************/
uint8_t CheckSum( uint8_t *buf, int packLen )
{
    uint16_t	i;
	uint8_t		sum;
	if(buf == NULL || packLen <= 0) return 0;
	sum = 0;
	for(i=2; i<packLen-1; i++)
		sum += buf[i];

	return sum;
}
/*******************************************************************************
 * 				end of file
 *******************************************************************************/

