/***************************************************************************
* File         : app_task_mater.h
* Summary      : 采集板结构体定义
* Version      : v0.1
* Author       : redmorningcn
* Date         : 2018/1/23 星期二
* Change Logs  :
* Date        Version      Author      Notes
* 2018/1/23 星期二           v0.1          redmorningcn    first version
***************************************************************************/

#include "global.h"
#include "display.h"

#ifndef APP_TASK_MATER_H
#define APP_TASK_MATER_H

/*******************************************************************************
* Description  :  与主板通讯，格式头尾
* Author       : 2018/1/23 星期二, by redmorningcn
*******************************************************************************/
#define DEBUG_CMD_HEAD		    0xAA55
#define DEBUG_CMD_TAIL			0x55AA


enum
{
SET_RECORD_CYCLE	 			= 0x00,
READ_RECORD_CYCLE			= 0x01,

ZERO_PRESSURE_RATIO 			= 0X50,
READ_DEVICE_ID 				= 0x51,
READ_MODEL_AND_NUMBER 		= 0x52,
GET_SYSTEM_TIME 				= 0x53,
GET_HISTORY_DATA 				= 0x54,
EARSE_FLASH					= 0x55,
EARSE_FRAM						= 0x56,
SET_VOLTAGE_SLOPE				= 0x57,
SET_CURRENT_SLOPE			= 0x58,

INCREASE_PULSE_RATE			= 0x5a,
DECREASE_PULSE_RATE			= 0x5b,
CLEAR_PULSE_RATE_ADJUST		= 0x5c,
SET_PULSE_RATE				= 0x5d,
SET_VOLTAGE_RATIO				= 0x5e,
SET_CURRENT_RATIO			= 0x5f,

SET_DISPLAY_PARA				= 0x61,
SET_PHASE_DUTY_THRESHOLD	= 0x62,
SET_THRESHOLD_OF_SENSOR		= 0x63,
SET_THRESHOLD_OF_EDGE_TIME	= 0x64,
SET_CONDITION_PARAMETER		= 0x65,
SET_RTC_BAT_VOL_FACTOR 		= 0x66,
SET_BAT_VOL_FACTOR			= 0x67,
SET_SPEED_FACTOR				= 0x68,
SET_SAMPLE_CYCLE_TIME		= 0x69,

SET_DEVICE_ID					= 0xa1,
SET_MODEL_AND_NUMBER		= 0xa2,
SET_SYSEM_TIME				= 0xa3,
DC_VOL_OFFSET_ADJUST			= 0xa4,
DC_CURR_OFFSET_ADJUST		= 0xa5,
AC_VOL_OFFSET_ADJUST			= 0xa6,
AC_CURR_OFFSET_ADJUST		= 0xa7,
VOLTAGE_GAIN_ADJUST			= 0xa8,
CURRENT_GAIN_ADJUST			= 0xa9,

CLEAR_PARA_OF_ADJUSTMENT	= 0xab,
CLEAR_PP_DATA					= 0xac,
CLEAR_NP_DATA					= 0xad,
CLEAR_PQ_DATA					= 0xae,
CLEAR_NQ_DATA					= 0xaf,

SYSTEM_RESET					= 0xf0,

};


__packed
typedef struct
{
uint8 index;
uint8 enbale;
uint16 time;
}ST_DISPLAY_PARA_SET;


/*******************************************************************************
* Description  : 光电模块参数指数（单通道）
* Author       : 2018/1/23 星期二, by redmorningcn
*******************************************************************************/
__packed
typedef struct
{
    uint16      vcc_vol;				/*工作电压*/
    uint16		high_level_vol;         /*高电平电压*/
    uint16		low_level_vol;          /*低电平电压*/
    uint16      rise_time;			    /* 上升沿边沿时间*/
    uint16      fall_time;			    /*下降沿边沿时间*/
    uint16 		duty_ratio;             /*占空比*/
    //uint16    cycle_time;             //周期时间
    //uint16    reserve;                //预留
}ST_SENSOR_INFO;


/*******************************************************************************
* Description  : 传感器通道异常
* Author       : 2018/1/24 星期三, by redmorningcn
*******************************************************************************/
__packed
typedef union
{
    struct
    {
        uint8 duty_ratio:1;         //占空比
        uint8 vcc_vol   :1;         //工作电压
        uint8 high_level:1;         //高电平
        uint8 low_level :1;         //低电平
        uint8 rise_time :1;         //上升沿
        uint8 fall_time :1;         //下降沿
        uint8 reserved  :2;         //预留
    }dtc_flag;
    uint8 share_byte;               //？
}CHANNEL_ABNORMAL_STATUS;


/*******************************************************************************
* Description  : 采集板异常信息
* Author       : 2018/1/24 星期三, by redmorningcn
*******************************************************************************/
__packed
typedef struct
{
    uint8 connected_fail;                       //通许异常（如果连接不上，该消息咋传出）        
    uint8 phase_diff_abnomal;                   //双通道相位异常
    uint8 pulse_count_error;                    //脉冲数异常
    CHANNEL_ABNORMAL_STATUS channel[2];
}BOARD_ABNORMAL_STATUS;
__packed

/*******************************************************************************
* Description  : 双通道传感器参数（增加相位差（-180~180 &  0~360））
* Author       : 2018/1/23 星期二, by redmorningcn
*******************************************************************************/
typedef struct
{
    uint8		    status;				 /*工作状态;0:正常1:光缝被堵*/
    uint8 		    reserved;
    uint16 		    phase_diff;			/*相位差*/
    int32		    step_counts;		/*正向转动步数*/
    ST_SENSOR_INFO	channel[2];		    /*channel a信息*/
}ST_COLLECTOR_INFO;


/*******************************************************************************
* Description  : 检测板应答信息
* Author       : 2018/1/23 星期二, by redmorningcn
*******************************************************************************/
__packed
typedef struct
{
    uint8               channel;
    uint8               addr;
    uint8               service_id;
    uint8               cmd_id;
    uint8               len;
    ST_COLLECTOR_INFO   collector_info;
    uint8               checksum;
}ST_COLLECTOR_RESPONCE;

__packed

typedef struct
{
float		in1;
float		in2;
float		in3;
float		in4;
float		in5;
float		in6;
}ST_OPERATION_STATE;

__packed

typedef struct
{
uint16 zero_vol;
uint16 tow_vol;
uint16 brake_vol;
uint16 forward_vol;
uint16 backward_vol;
uint16 power_vol;
uint16 bat_vol;
}ST_DET_VOL;

/*******************************************************************************
* Description  : 采集板应答信息
* Author       : 2018/1/24 星期三, by redmorningcn
*******************************************************************************/
__packed
typedef struct
{
    ST_COLLECTOR_INFO			collector_board[3];	        /*采集板信息(波形信息)*/
    BOARD_ABNORMAL_STATUS 		abnormal_info[3];           //采集板的工作状态信息（异常）
    uint16						real_speed;					/*实速*/
    uint16 						voltage[7];
    uint16						peak_vol[5];
}ST_MONITOR;

__packed
typedef struct
{
uint8 enable;
uint16 times;
}DISPLAY_SETTING;
__packed
typedef struct
{
uint8	phase_threshold_low;
uint8	phase_threshold_hig;
uint8	duty_threshold_low;
uint8	duty_threshold_hig;
uint8	edge_time_threshold;
uint16	sensor_vcc_threshold_low;
uint16	sensor_vcc_threshold_hig;
uint8	hig_level_threshold_factor;
uint8	low_level_threshold_factor;
}ST_COLLECTOR_THRESHOLD;
__packed
typedef struct
{
uint8 low_level_lower;
uint8 low_level_upper;
uint8 high_level_lower;
uint8 high_level_upper;
uint8 over_level_lower;
uint8 over_level_upper;
}ST_CONDITION_FACTOR;

__packed
typedef struct
{
DISPLAY_SETTING				display_setting[DISPLAY_STEP_MAX];
ST_COLLECTOR_THRESHOLD		collector_threshold;
ST_CONDITION_FACTOR		condition_factor;
float							bat_vol_factor;
float 						voltage_factor;
float 						speed_factor;
uint16 						sample_cycle;
uint8 						checksum;
}ST_para_storage;

__packed

typedef    struct __sTIME__ {
    uint32            Sec                 : 6;            // D5~D0:秒
    uint32            Min                 : 6;            // D11~D6:分
    uint32            Hour                : 5;            // D16~D12:时
    uint32            Day                 : 5;            // D21~D17：日
    uint32            Mon                 : 4;            // D25~D22：月
    uint32            Year                : 6;            // D26~D31:年
    } st_TIME;

__packed

typedef struct
{    
    /***************************************************
    * 描述： 头:13 bytes
    */
    uint8             CmdType;                            // 命令类型
    uint16            LocoTyp;                            // 机车型号
    uint16            LocoNbr;                            // 机车号
    uint32            DataType            : 8;            // 数据类型
    uint32            Speed               :10;            // 时速
    uint32            Rsv1                :14;            // 预留   
    
    /***************************************************
    * 描述： 时间:4 bytes
    */
	st_TIME Time;
	
    ST_MONITOR	monitor;

    uint32            RecordTime;
    uint32            Head;
    uint32            Tail;
    uint32            RecordNbr;                          // 记录号
    uint16            chk;
}StrMater;
__packed

typedef struct
{
	 uint8 	        Time[6];			    //时间
	uint16 	        Crc16;					//CRC校验
}SDAT;


extern StrMater            Mater;
extern StrMater            History;

#endif

