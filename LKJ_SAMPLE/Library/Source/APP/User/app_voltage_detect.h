
/*******************************************************************************
 *   Filename:      app_voltage_detect.h
 *   Create date: 2017-07-11
 *
 *   Author:        fourth peng
 *
 *   Description:  LKJ检测装置电池电压以及工况信号采集
 *   Notes:
 *
 *
 *   All copyrights reserved to changsha nanche electronics LTD.
 *
 *******************************************************************************/

typedef enum
{
LOW_LEVEL_STATE = 0,
RISE_OR_FALL_STATE,
HIG_LEVEL_STATE,
VOLTAGE_OVERFLOW,
}CONDITION_STATUS;


enum
{
#ifdef SAMPLE_BOARD
ADC_CHANNEL_SIGNALA,
ADC_CHANNEL_SIGNALB,
ADC_CHANNEL_VCC,
ADC_CHANNEL_VCC2,
#else
ADC_CHANNEL_ZERO,
ADC_CHANNEL_TOW,
ADC_CHANNEL_BRAKE,
ADC_CHANNEL_FORWARD,
ADC_CHANNEL_BACKWARD,
ADC_CHANNEL_POWER,
ADC_CHANNEL_RTC_BAT,
#endif
MAX_ADC_CHANNEL,
};

 

typedef struct
{
    uint8 timeout_cnt;
    uint8 active;
}ST_TRANSFORM;


typedef struct
{
    ST_TRANSFORM 			transform;
    uint16					voltage;
    uint16 					peak_voltage;
    CONDITION_STATUS		status;
}ST_CONDITION_DETECT;



