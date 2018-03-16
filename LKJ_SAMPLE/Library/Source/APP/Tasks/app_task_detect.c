/*******************************************************************************
 *   Filename:       app_task_detect.c
 *   Revised:        All copyrights reserved to fourth.Peng.
 *   Revision:       v1.0
 *   Writer:	     fourth.Peng.
 *
 *   Description:
 *
 *   Notes:
 *
 *******************************************************************************/

/*******************************************************************************
 * INCLUDES
 */
#include <includes.h>
#include "bsp_adc.h"
#include "bsp_dac.h"
#include "app_ctrl.h"
#include "task.h"
#include "app_task_mater.h"
#include "app_voltage_detect.h"
#include "app.h"

#include "task_comm.h"


/*******************************************************************************
 * CONSTANTS
 */

/*******************************************************************************
 * MACROS
 */
//通道时间参数缓冲区大小
#define CH_TIMEPARA_BUF_SIZE    50

/*******************************************************************************
 * TYPEDEFS
 */

/*******************************************************************************
* Description  : 通道结构体。其中，通道时间参数辅助运算；
                             通道参数才是通道的具体指标。
* Author       : 2018/3/13 星期二, by redmorningcn
*******************************************************************************/
typedef struct
{
    /*******************************************************************************
    * Description  : 辅助运算
    * Author       : 2018/3/14 星期三, by redmorningcn
    *******************************************************************************/
    struct  {
        struct  {
            uint32  low_up_time;                            //10%位置，上升沿，中断时间 
            uint32  low_down_time;                          //10%位置，下降沿，中断时间
            uint32  hig_up_time;                            //90%位置，上升沿，中断时间
            uint32  hig_down_time;                          //90%位置，下降沿，中断时间
            uint16  low_up_cnt;
            uint16  low_down_cnt;
            uint16  hig_up_cnt;
            uint16  hig_down_cnt;
        }time[CH_TIMEPARA_BUF_SIZE];
        
        uint32  pulse_cnt;                                  //脉冲个数，判断信号有无
        uint16  p_write;                                    //缓冲区读写控制
        uint16  p_read;
    }test[2];                                               //通道检测内容
    
    /*******************************************************************************
    * Description  : 通道具体指标
    * Author       : 2018/3/14 星期三, by redmorningcn
    *******************************************************************************/
    struct _strsignalchannelpara_ {
        uint32              period;                 //周期，  0.00-2000000.00us （0.5Hz）
        uint32              freq;                   //频率，  0-100000hz              
        uint16              raise;                  //上升沿，0.00-50.00us
        uint16              fail;                   //下降沿，0.00-50.00us
        uint16              ratio;                  //占空比，0.00-100.00%
        uint16              Vol;                    //低电平，0.00-30.00V
        uint16              Voh;                    //高电平，0.00-30.00V
        uint16              status;                 //通道状态
    }para[2];
    
    uint32  ch1_2phase;                             //相位差，0.00-360.00°
}strCoupleChannel;



/*******************************************************************************
* Description  : 通道变量
* Author       : 2018/3/14 星期三, by redmorningcn
*******************************************************************************/
strCoupleChannel    ch;





ST_COLLECTOR_INFO detect_info = {0};
//#define TEST
#ifdef TEST
const ST_COLLECTOR_INFO test_info =
{
.status =0,				 /*工作状态;0:正常1:光缝被堵*/
.reserved=0,
.phase_diff = 900,			/*相位差*/
.step_counts = 650,			/*正向转动步数*/
.channel[0].vcc_vol = 15000,		/*工作电压*/
.channel[0].high_level_vol = 14900,		/*高电平电压*/
.channel[0].low_level_vol = 100,		/*低电平电压*/
.channel[0].rise_time = 100,			/* 上升沿边沿时间*/
.channel[0].fall_time = 90,			/*下降沿边沿时间*/
.channel[0].duty_ratio = 500,		/*占空比*/
.channel[1].vcc_vol = 15100,		/*工作电压*/
.channel[1].high_level_vol = 15000,		/*高电平电压*/
.channel[1].low_level_vol = 200,		/*低电平电压*/
.channel[1].rise_time = 110,			/* 上升沿边沿时间*/
.channel[1].fall_time = 99,			/*下降沿边沿时间*/
.channel[1].duty_ratio = 800,		/*占空比*/
};
#endif
/***********************************************
* 描述： 任务控制块（TCB）
*/
static  OS_TCB   AppTaskDetectTCB;
/***********************************************
* 描述： 任务接收队列
*/
extern OS_Q                DET_RxQ;


/***********************************************
* 描述： 任务堆栈（STACKS）
*/
static  CPU_STK  AppTaskDetectStk[ APP_TASK_KEY_STK_SIZE ];

/*******************************************************************************
 * LOCAL VARIABLES
 */
OS_TMR	app_detect_tmr = {0};
OS_TMR	app_sample_tmr = {0};

//ST_DET_VOL detect_vol = {0};

ST_CONDITION_DETECT condition_info[MAX_ADC_CHANNEL-2] = {0};


//TIM8 捕获状态
static uint8 capture_state[4]={0x40,0x40,0x40,0x40};
uint32 m1_cycle_count,m2_cycle_count;

/*******************************************************************************
 * GLOBAL VARIABLES
 */
/*******************************************************************************
 * LOCAL FUNCTIONS
 */
void APP_Detect_Init(void);
void app_detect_time_tick(void *data, uint16 len);
static void app_detect_adc_data_update(void *data, uint16 len);

void  AppTaskDetect (void *p_arg);
void app_detect_tmr_callback(OS_TMR *ptmr,void *p_arg);
static void detect_timer_init(void);

void app_detect_task_event(ST_QUEUE *queue);
void app_calc_cha_fall_time(void *data, uint16 len);
void app_calc_cha_rise_time(void *data, uint16 len);
void app_calc_chb_fall_time(void *data, uint16 len);
void app_calc_chb_rise_time(void *data, uint16 len);
void app_sample_cycle_tick(void *data, uint16 len);
void app_check_m1_cycle(void *data, uint16 len);
 void app_check_m2_cycle(void *data, uint16 len);
void app_calc_m1_data(void *data, uint16 len);
void app_calc_m2_data(void *data, uint16 len);

void app_detect_sample_cycle_callback(void *data, uint16 len);

void Detect_exti_init(void);
void  cha_level_detect_tmrInit (void);
void cha_level_detect_tmr_set(uint32 dly_time);
void  chb_level_detect_tmrInit (void);
void chb_level_detect_tmr_set(uint32 dly_time);
int32 get_pulse_count(void);


/* comm task 接收事件对应回调函数表格*/
const ST_TASK_EVENT_TBL_TYPE app_detect_task_event_table[]=
{
{DETECT_TASK_TIME_TICK,				app_detect_time_tick			},
{DETECT_TASK_VOL_ADC_UPDATE,		app_detect_adc_data_update	    },
/*
{DETECT_TASK_CALC_CHA_FALL_TIME,	app_calc_cha_fall_time		},
{DETECT_TASK_CALC_CHA_RISE_TIME,	app_calc_cha_rise_time		},
{DETECT_TASK_CALC_CHB_FALL_TIME,	app_calc_chb_fall_time		},
{DETECT_TASK_CALC_CHB_RISE_TIME,	app_calc_chb_rise_time		},
*/
{DETECT_TASK_CHECK_M1_CYCLE,		app_check_m1_cycle		},
{DETECT_TASK_CHECK_M2_CYCLE,		app_check_m2_cycle		},
{DETECT_TASK_M1_DATA_UPDATE,		app_calc_m1_data		},
{DETECT_TASK_M2_DATA_UPDATE,		app_calc_m2_data		},
{DETECT_CALC_DETECT_INFO,			app_detect_sample_cycle_callback	},

};

/*******************************************************************************
 * GLOBAL FUNCTIONS
 */

/*******************************************************************************
 * EXTERN VARIABLES
 */
extern ST_para_storage system_para;
/*******************************************************************************
 * EXTERN FUNCTIONS
 */
 uint32 diff(uint32 a, uint32 b)
{
	uint32 d_value = a-b;
	if (d_value > 0x80000000)
		d_value = b - a;
	return d_value;
}
/*******************************************************************************/

/*******************************************************************************
 * 名    称： AppTaskKey
 * 功    能： 控制任务
 * 入口参数： p_arg - 由任务创建函数传入
 * 出口参数： 无
 * 作　　者： Roger-WY.
 * 创建日期： 2015-02-05
 * 修    改：
 * 修改日期：
 *******************************************************************************/
void  AppTaskDetect (void *p_arg)
{
	ST_QUEUE *p_mailbox = 0;
	/***********************************************
	* 描述： 任务初始化
	*/
//	APP_Detect_Init();

	/***********************************************
	* 描述：Task body, always written as an infinite loop.
	*/

	p_mailbox = (ST_QUEUE *)system_get_msg(DETECT_TASK_ID);
	if (NULL != p_mailbox)
	{
		app_detect_task_event(p_mailbox);
	}

}
/*******************************************************************************
 * 名    称： APP_GPIO_Init
 * 功    能： 任务初始化
 * 入口参数： 无
 * 出口参数： 无
 * 作　　者： fourth. peng
 * 创建日期： 2015-03-28
 * 修    改：
 * 修改日期：
 *******************************************************************************/
 static void APP_GPIO_Init()
{
    GPIO_InitTypeDef gpio_init;
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA| RCC_APB2Periph_AFIO, ENABLE);

    gpio_init.GPIO_Pin  = GPIO_Pin_11|GPIO_Pin_12;
    gpio_init.GPIO_Mode = GPIO_Mode_IPU;
    GPIO_Init(GPIOA, &gpio_init);
}
/*******************************************************************************
 * 名    称： Get_board_id
 * 功    能： 任务初始化
 * 入口参数： 无
 * 出口参数： 无
 * 作　　者： fourth. peng
 * 创建日期： 2017-05-11
 * 修    改：
 * 修改日期：
 *******************************************************************************/
 uint8 Get_board_id(void)
{
	uint8 id = 0;
	Delay_Nus(10);
	if(GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_11))
	{
		id |= 0x2;
	}
	if(GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_12))
	{
		id |= 0x1;
	}
	return id;
}
/*******************************************************************************
 * 名    称： APP_Detect_Init
 * 功    能： 任务初始化
 * 入口参数： 无
 * 出口参数： 无
 * 作　　者： fourth. peng
 * 创建日期： 2015-05-6
 * 修    改：
 * 修改日期：
 *******************************************************************************/
void APP_Detect_Init(void)
{
	OS_ERR err;
	/***********************************************
	* 描述： 初始化本任务用到的相关硬件
	*/
	Bsp_ADC_Init();
#ifdef SAMPLE_BOARD
	APP_GPIO_Init();
	Ctrl.sample_id = Get_board_id();
	BSP_dac_init();
	
    //detect_timer_init();
	//Detect_exti_init();
	//cha_level_detect_tmrInit();
	//chb_level_detect_tmrInit();
#endif

	/***********************************************
	* 描述： 在看门狗标志组注册本任务的看门狗标志
	*/
//	WdtFlags |= WDT_FLAG_DETECT;
#if	0
	/*创建一个周期为10ms的定时器*/
	OSTmrCreate( &app_detect_tmr,
					(CPU_CHAR *)"app_detect_tmr",
					10,
					10,
					OS_OPT_TMR_PERIODIC,
					(OS_TMR_CALLBACK_PTR)app_detect_tmr_callback,
					NULL,
					&err);
	OSTmrStart(&app_detect_tmr, &err);

	OSTmrCreate( &app_sample_tmr,
					(CPU_CHAR *)"app_detect_tmr",
					100,
					100,
					OS_OPT_TMR_PERIODIC,
					(OS_TMR_CALLBACK_PTR)app_detect_sample_cycle_callback,
					NULL,
					&err);
	OSTmrStart(&app_sample_tmr, &err);

	/*创建一个队列，用于消息传递*/
	OSQCreate(&DET_RxQ, "DET RxQ", 100, &err);
	WdtFlags |= WDT_FLAG_DETECT;
#endif
}

/*******************************************************************************
 * 名    称： app_detect_tmr_callback
 * 功    能：
 * 入口参数： 无
 * 出口参数： 无
 * 作　　者： fourth.
 * 创建日期： 2017-5-3
 * 修    改：
 * 修改日期：
 *******************************************************************************/
void app_detect_tmr_callback(OS_TMR *ptmr,void *p_arg)
{
	OS_ERR err;
	static ST_QUEUETCB mailbox = {0};
	mailbox.event = DETECT_TASK_TIME_TICK;
	mailbox.queue_data.pdata = NULL;
//	OSQPost(&DET_RxQ, &mailbox, sizeof(mailbox), OS_OPT_POST_FIFO, &err);

}
#define CHANNEL_NUM		2
/*******************************************************************************
 * 名    称： app_dac_output_update
 * 功    能：
 * 入口参数： 无
 * 出口参数： 无
 * 作　　者： fourth.
 * 创建日期： 2017-5-3
 * 修    改：
 * 修改日期：
 *******************************************************************************/
	uint16 ref_voltage_h = 0;
	uint16 ref_voltage_l = 0;

void app_dac_output_update()
{
	#define dac_value_factor			(0.1241)


	uint32 sig_h = 0;
	uint32 sig_l = 0;

	if((detect_info.channel[0].high_level_vol < (detect_info.channel[0].vcc_vol>>1))
		&&(detect_info.channel[0].high_level_vol < (detect_info.channel[1].vcc_vol>>1)))
	{
		return;
	}

	if (detect_info.channel[0].high_level_vol > detect_info.channel[1].high_level_vol)
	{
		sig_h = detect_info.channel[1].high_level_vol;
	}
	else
	{
		sig_h = detect_info.channel[0].high_level_vol;
	}

	if (detect_info.channel[0].low_level_vol > detect_info.channel[1].low_level_vol)
	{
		sig_l = detect_info.channel[0].low_level_vol;
	}
	else
	{
		sig_l = detect_info.channel[1].low_level_vol;
	}

	if(sig_h > 30000)
	{
		sig_h = 30000;
	}
	else if(sig_h < 5000)
	{
		sig_h = 5000;
	}
	if(sig_l > 2000)
	{
		sig_l = 2000;
	}

	ref_voltage_h = (uint16)((sig_l + (sig_h - sig_l)*0.9)*dac_value_factor);
	ref_voltage_l = (uint16)((sig_l + (sig_h - sig_l)*0.1)*dac_value_factor);

    /*******************************************************************************
    * Description  : 设置DAC参考值（数据不变，不能调整）
    * Author       : 2018/3/9 星期五, by redmorningcn
    *******************************************************************************/

	//DAC_SetDualChannelData(DAC_Align_12b_R, ref_voltage_h, ref_voltage_l);
    DAC_SetDualChannelData(DAC_Align_12b_R, 4096/5, 4096/5);
}
/*******************************************************************************
 * 名    称： app_detect_time_tick
 * 功    能： 	10ms中断一次
 * 入口参数： 无
 * 出口参数： 无
 * 作　　者： fourth.
 * 创建日期： 2017-5-3
 * 修    改：
 *******************************************************************************/
void app_detect_time_tick(void *data, uint16 len)
{
	uint8 i;
	for(i = 0; i < (MAX_ADC_CHANNEL - 2); i++)
	{
		if (TRUE == condition_info[i].transform.active )
		{
			if (condition_info[i].transform.timeout_cnt)
			{
				condition_info[i].transform.timeout_cnt--;
				if (0 == condition_info[i].transform.timeout_cnt )
				{
					//操作完成，存储操作记录
#if 0
					OS_ERR err;
				        OS_FlagPost(( OS_FLAG_GRP *)&Ctrl.Os.MaterEvtFlagGrp,
						            ( OS_FLAGS     ) COMM_EVT_SAVE_DATA,
						            ( OS_OPT       ) OS_OPT_POST_FLAG_SET,
						            ( CPU_TS       ) 0,
						            ( OS_ERR      *) &err);
#endif
				}
			}
		}
	}
}
#ifndef SAMPLE_BOARD
/*******************************************************************************
 * 名    称： app_condition_state_handler
 * 功    能： 	工况状态处理
 * 入口参数： 无
 * 出口参数： 无
 * 作　　者： fourth.
 * 创建日期： 2017-5-3
 * 修    改：
 * 修改日期：
 *******************************************************************************/
void app_condition_state_handler(uint8 ch, CONDITION_STATUS next)
{
	CONDITION_STATUS *p_curr_status = &condition_info[ch].status;
	if (*p_curr_status != next)
	{
		//
		if (FALSE == condition_info[ch].transform.active)
		{
			condition_info[ch].transform.active = TRUE;
		}

		//当前采样电压大于峰值电压
		if (Mater.monitor.peak_vol[ch]< Mater.monitor.voltage[ch])
		{
			Mater.monitor.peak_vol[ch] = Mater.monitor.voltage[ch];
		}
		condition_info[ch].transform.timeout_cnt = TRANSFORM_TIMEOUT_CNT;
	}
	else if ( VOLTAGE_OVERFLOW == *p_curr_status)
	{
		//当前采样电压大于峰值电压
		if (Mater.monitor.peak_vol[ch] < Mater.monitor.voltage[ch])
		{
			Mater.monitor.peak_vol[ch] = Mater.monitor.voltage[ch];
		}
		condition_info[ch].transform.timeout_cnt = TRANSFORM_TIMEOUT_CNT;
	}

	*p_curr_status = next;

}

/*******************************************************************************
 * 名    称： app_detect_adc_data_update
 * 功    能： 	工况状态转换
 * 入口参数： 无
 * 出口参数： 无
 * 作　　者： fourth.
 * 创建日期： 2017-5-3
 * 修    改：
 * 修改日期：
 *******************************************************************************/
 void app_condition_status_update(uint8 ch, uint16 vol)
{
	CONDITION_STATUS state = condition_info[ch].status;

	switch(state)
	{
		case LOW_LEVEL_STATE:
			if ( vol >  system_para.condition_factor.over_level_upper)
			{
				state = VOLTAGE_OVERFLOW;
			}
			else if ( vol >system_para.condition_factor.high_level_upper)
			{
				state = HIG_LEVEL_STATE;
			}
			else if ( vol > system_para.condition_factor.low_level_upper)
			{
				state = RISE_OR_FALL_STATE;
			}
			break;
		case RISE_OR_FALL_STATE:
			if ( vol < system_para.condition_factor.low_level_lower )
			{
				state = LOW_LEVEL_STATE;
			}
			else if ( vol > system_para.condition_factor.over_level_upper)
			{
				state = VOLTAGE_OVERFLOW;
			}
			else if ( vol > system_para.condition_factor.high_level_upper)
			{
				state = HIG_LEVEL_STATE;
			}
			break;
		case HIG_LEVEL_STATE:
			if ( vol < system_para.condition_factor.low_level_lower )
			{
				state = LOW_LEVEL_STATE;
			}
			else if ( vol > system_para.condition_factor.over_level_upper)
			{
				state = VOLTAGE_OVERFLOW;
			}
			else if ( vol <  system_para.condition_factor.high_level_lower)
			{
				state = RISE_OR_FALL_STATE;
			}
			break;
		case VOLTAGE_OVERFLOW:
			if ( vol < system_para.condition_factor.low_level_lower )
			{
				state = LOW_LEVEL_STATE;
			}
			else if ( vol <  system_para.condition_factor.over_level_lower)
			{
				state = HIG_LEVEL_STATE;
			}
			else if ( vol <  system_para.condition_factor.high_level_lower)
			{
				state = RISE_OR_FALL_STATE;
			}
			break;
		}

	app_condition_state_handler(ch, state);
}
 #endif
/*******************************************************************************
 * 名    称： app_detect_adc_data_update
 * 功    能： 	ADC数据处理
 * 入口参数： 无
 * 出口参数： 无
 * 作　　者： fourth.
 * 创建日期： 2017-5-3
 * 修    改：
 * 修改日期：
 *******************************************************************************/
static void app_detect_adc_data_update(void *data, uint16 len)
{
#ifndef SAMPLE_BOARD
	uint16_t adc_value[Channel_Times][Channel_Number] = {0};
	int16 adc_average_value[Channel_Number] = {0};
	uint8 i,count;
	uint16 sum = 0;
	uint32 voltage;
	int16 high_ref, low_ref;

	memcpy(adc_value, data->pdata, data->len);
	for (i=0; i < Channel_Number; i++)
	{
		for(count=0 ;count < Channel_Times; count++)
		{
			sum += adc_value[count][i];
		}

		adc_average_value[i] = sum/Channel_Times;
		sum = 0;
	}

	for (i = 0; i < (MAX_ADC_CHANNEL -2); i++)
	{
		voltage = (uint32)(((adc_average_value[i]*3300/4096)*system_para.voltage_factor)/1000);
		app_condition_status_update(i, voltage);
		Mater.monitor.voltage[i] = voltage;
	}
	Mater.monitor.voltage[ADC_CHANNEL_POWER] = (uint16)(((adc_average_value[ADC_CHANNEL_POWER]*3300/4096)*system_para.voltage_factor)/1000);
	Mater.monitor.voltage[ADC_CHANNEL_RTC_BAT] = (uint16)((adc_average_value[ADC_CHANNEL_POWER]*3300/4096)*system_para.bat_vol_factor);
#endif
}



/*******************************************************************************
* Description  : 通道时间参数计算
                 根据通道buf的读写指针，确定是否需要进运算
* Author       : 2018/3/13 星期二, by redmorningcn
*******************************************************************************/
void    app_calc_ch_timepara(void)
{
    uint8   p_write;
    uint8   p_read;
    uint64  starttime;
    uint64  endtime;
    uint16  cnt;
    
    uint64  periodtime;
    uint64  ratiotime;
    uint64  phasetime;
    
    uint8   i;
        
    for(i = 0;i< 2;i++)
    {
        p_write = ch.test[i].p_write;
        p_read  = ch.test[i].p_read;
        
        /*******************************************************************************
        * Description  : 速度通道时间参数运算
        * Author       : 2018/3/13 星期二, by redmorningcn
        *******************************************************************************/
        if(     ( p_write > p_read) &&  (p_write > p_read+10)           
           ||   ( p_write < p_read) &&  (p_write + CH_TIMEPARA_BUF_SIZE > p_read+10)           
               )  
        {

            /*******************************************************************************
            * Description  : 计算周期(0.01us) 周期信号任意一点再次出现，取low_up中断为标准
            * Author       : 2018/3/13 星期二, by redmorningcn
            *******************************************************************************/
            p_read      =   ch.test[i].p_read;
            starttime   =   ch.test[i].time[p_read].low_up_time  * 65536 
                        +   ch.test[i].time[p_read].low_up_cnt  ;
            
            p_read      =   (ch.test[i].p_read + 1) % CH_TIMEPARA_BUF_SIZE;       //防止越界
            endtime     =   ch.test[i].time[p_read].low_up_time  * 65536 
                        +   ch.test[i].time[p_read].low_up_cnt  ;
            
            if(starttime > endtime)             //防止翻转
            {
                endtime += 65536;
            }
            periodtime = endtime - starttime;
            
            ch.para[i].period = (periodtime * 1000*1000*100 )/ sys.cpu_freq;
            
            if(periodtime){
                ch.para[i].freq = sys.cpu_freq  / periodtime;   //计算频率
                
                if(((sys.cpu_freq *10) % periodtime)> 4 )       //四舍五入
                    ch.para[i].freq += 1;
            }
            
            /*******************************************************************************
            * Description  : 计算占空比(xx.xx%)，( hig_down -  low_up ) / period
            * Author       : 2018/3/13 星期二, by redmorningcn
            *******************************************************************************/
            p_read      =   ch.test[i].p_read;
            starttime   =   ch.test[i].time[p_read].low_up_time  * 65536 
                        +   ch.test[i].time[p_read].low_up_cnt  ;
            endtime     =   ch.test[i].time[p_read].low_down_time  * 65536 
                        +   ch.test[i].time[p_read].low_down_cnt  ;            
            if(starttime > endtime)             //防止翻转
            {
                endtime += 65536;
            }
            ratiotime = endtime - starttime;
            
            if( periodtime )
                ch.para[i].ratio = ( ratiotime * 100* 100 ) / periodtime; 
            
            /*******************************************************************************
            * Description  : 计算上升沿（0.01us）
            * Author       : 2018/3/13 星期二, by redmorningcn
            *******************************************************************************/
            p_read      =   ch.test[i].p_read;
            starttime   =   ch.test[i].time[p_read].low_up_time  * 65536 
                        +   ch.test[i].time[p_read].low_up_cnt  ;
            endtime     =   ch.test[i].time[p_read].hig_up_time  * 65536 
                        +   ch.test[i].time[p_read].hig_up_cnt  ;            
            if(starttime > endtime)             //防止翻转
            {
                endtime += 65536;
            }

            ch.para[i].raise = ( endtime - starttime)/sys.cpu_freq;
            
            /*******************************************************************************
            * Description  : 计算下降沿(0.01us)
            * Author       : 2018/3/13 星期二, by redmorningcn
            *******************************************************************************/
            p_read      =   ch.test[i].p_read;
            starttime   =   ch.test[i].time[p_read].hig_down_time  * 65536 
                        +   ch.test[i].time[p_read].hig_down_cnt  ;
            endtime     =   ch.test[i].time[p_read].low_down_time  * 65536 
                        +   ch.test[i].time[p_read].low_down_cnt  ;            
            if(starttime > endtime)             //防止翻转
            {
                endtime += 65536;
            }
            
            ch.para[i].fail = ( endtime - starttime)/sys.cpu_freq;
            

            /*******************************************************************************
            * Description  : 计算相位差(xx.xx°)
            * Author       : 2018/3/13 星期二, by redmorningcn
            *******************************************************************************/
            if(i == 1)      
            {
                p_read      =   ch.test[i].p_read;
                starttime   =   ch.test[0].time[p_read].low_up_time  * 65536 
                            +   ch.test[0].time[p_read].low_up_cnt  ;
                endtime     =   ch.test[1].time[p_read].low_up_time  * 65536 
                            +   ch.test[1].time[p_read].low_up_cnt  ;            
                if(starttime > endtime)             //防止翻转
                {
                    endtime += periodtime;          //加一周期时间
                }
                
                ch.ch1_2phase = (endtime - starttime) / periodtime; 
            }

            //读指正++
            ch.test[i].p_read++ ;
            ch.test[i].p_read %= CH_TIMEPARA_BUF_SIZE; 
        }
    }
}


/*******************************************************************************
* Description  : 全局时间累积。真实时间 time = strSys.time * 65536 + TIM_GetCounter  
                              再乘以单周期时间。
* Author       : 2018/3/13 星期二, by redmorningcn
*******************************************************************************/
void TIM8_OVER_IRQHandler(void)
{
	if(TIM_GetITStatus(TIM8,TIM_IT_Update)!=RESET)  //计数器溢出中断
	{
		TIM_ClearITPendingBit(TIM8,TIM_IT_Update);  //清除中断标志
        sys.time++;                                 //系统是时间累加
	}
}

/*******************************************************************************
* Description  : 全局时钟，为所有信号提供统一时间标准
* Author       : 2018/3/13 星期二, by redmorningcn
*******************************************************************************/
void Timer8_Iint(void)
{
	TIM_TimeBaseInitTypeDef	TIM_BaseInitStructure;
//	TIM_ICInitTypeDef TIM_ICInitStruct;
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM8,ENABLE);//使能定时器时钟
	//初始化定时器8
	TIM_BaseInitStructure.TIM_Period = 65535;                   //计数器自动重装值
	TIM_BaseInitStructure.TIM_Prescaler = 0;                    //不分频
	TIM_BaseInitStructure.TIM_ClockDivision = TIM_CKD_DIV1;     //时钟不分割
	TIM_BaseInitStructure.TIM_CounterMode = TIM_CounterMode_Up; //向上计数
	TIM_BaseInitStructure.TIM_RepetitionCounter = 0;            //重复计数设置
    
	TIM_TimeBaseInit(TIM8,&TIM_BaseInitStructure);              //初始化时钟
    
	TIM_ClearFlag(TIM8, TIM_FLAG_Update);                       //清楚中断标志位
	TIM_ITConfig(TIM8,TIM_IT_Update,ENABLE);                    //不允许更新中断
	TIM_Cmd(TIM8,ENABLE);
    
  
	BSP_IntVectSet(TIM8_UP_IRQn, TIM8_OVER_IRQHandler);
	BSP_IntEn(TIM8_UP_IRQn);
    
    sys.time = 0;                                               //系统时间置0
}


/*******************************************************************************
* Description  : 节约中断时间，处理函数只赋值。
                 1、取系统定时器时间sys.time
                 2、取捕获计算器产生时间cnt，捕获产生的实际时间为sys.time * 65536 + cnt;
                 3、方波信号记录，如下时间，波形参数时间确定 
                    上升沿在10%位置时间；
                    上升沿在90%位置时间；
                    下降沿在10%位置时间；
                    下降沿在90%位置时间。
* Author       : 2018/3/16 星期五, by redmorningcn
*******************************************************************************/
void TIM8_CC_IRQHandler(void)
{

    uint16      cnt;
    u32         time;                                //时间等于 sys.time * 65536+TIM_CNT     

    //cnt  = TIM_CNT;
    time = sys.time;                                //时间等于 sys.time * 65536+TIM_CNT     
	
	if(TIM8->SR&0x04)								//CH2捕获中断 
	{
        cnt = TIM8->CCR2;

        if(GPIO_ReadInputDataBit(GPIOC,GPIO_Pin_7) == SET)      //高位（90%）的上升触发
        {     
            TIM_OC2PolarityConfig(TIM8,TIM_ICPolarity_Falling); //设置为下降沿捕获			

            ch.test[0].time[ch.test[0].p_write].hig_up_time  = time;     
            ch.test[0].time[ch.test[0].p_write].hig_up_cnt   = cnt;   
        }
        else                                                    //高位（90%）下降沿触发
        {    
            TIM_OC2PolarityConfig(TIM8,TIM_ICPolarity_Rising);  //设置为上升沿捕获

            ch.test[0].time[ch.test[0].p_write].hig_down_time   = time;     
            ch.test[0].time[ch.test[0].p_write].hig_down_cnt    = cnt;   
        }
        
        TIM_ClearITPendingBit(TIM8,TIM_IT_CC2);                 //清除中断标志
	}
    
	if(TIM8->SR&0x02)								            //CC1捕获中断 
	{

        cnt = TIM8->CCR1;
        if(GPIO_ReadInputDataBit(GPIOC,GPIO_Pin_6) == SET)      //低位（10%）的上升触发
        {  
            TIM_OC1PolarityConfig(TIM8,TIM_ICPolarity_Falling);//设置为下降沿捕获			

            ch.test[0].time[ch.test[0].p_write].low_up_time    =  time;     
            ch.test[0].time[ch.test[0].p_write].low_up_cnt     =  cnt;    
        }else                                                   //低位（10%）的下降触发
        {           
            TIM_OC1PolarityConfig(TIM8,TIM_ICPolarity_Rising);//设置为上升沿捕获

            ch.test[0].time[ch.test[0].p_write].low_down_time    =  time;     
            ch.test[0].time[ch.test[0].p_write].low_down_cnt     =  cnt; 
            
            ch.test[0].pulse_cnt++;                             //周期结束放置在后面
            ch.test[0].p_write           =      ch.test[0].pulse_cnt 
                % CH_TIMEPARA_BUF_SIZE;
        }
        
        TIM_ClearITPendingBit(TIM8,TIM_IT_CC1);//清除中断标志
	}    
  
	

	if(TIM8->SR&0x10)//CH4捕获中断 在CH4上升沿中断中记录的值即为两波形的上升沿时间差
	{
        
        cnt =  TIM8->CCR4;
        if(GPIO_ReadInputDataBit(GPIOC,GPIO_Pin_9) == SET)      //高位（90%）的上升触发
        {     
            TIM_OC4PolarityConfig(TIM8,TIM_ICPolarity_Falling);//设置为下降沿捕获			

            ch.test[1].time[ch.test[1].p_write].hig_up_time  = time;     
            ch.test[1].time[ch.test[1].p_write].hig_up_cnt   = cnt;   
        }
        else                                                    //高位（90%）下降沿触发
        {           
            TIM_OC4PolarityConfig(TIM8,TIM_ICPolarity_Rising);//设置为上升沿捕获		

            ch.test[1].time[ch.test[1].p_write].hig_down_time   = time;     
            ch.test[1].time[ch.test[1].p_write].hig_down_cnt    = cnt;   
        }
        
        TIM_ClearITPendingBit(TIM8,TIM_IT_CC4);//清除中断标志

	}
    
	if(TIM8->SR&0x08)//CH3捕获中断  
	{
        cnt = TIM8->CCR3;
        if(GPIO_ReadInputDataBit(GPIOC,GPIO_Pin_8) == SET)      //低位（10%）的上升触发
        {  
            TIM_OC3PolarityConfig(TIM8,TIM_ICPolarity_Falling);//设置为下降沿捕获			

            ch.test[1].time[ch.test[1].p_write].low_up_time    =  time;     
            ch.test[1].time[ch.test[1].p_write].low_up_cnt     =  cnt;    
        }
        else                                                    //低位（10%）的下降触发
        {     
            TIM_OC3PolarityConfig(TIM8,TIM_ICPolarity_Rising);//设置为上升沿捕获		

            ch.test[1].time[ch.test[1].p_write].low_down_time    =  time;     
            ch.test[1].time[ch.test[1].p_write].low_down_cnt     =  cnt;  
            
            
            ch.test[1].pulse_cnt++;                             //周期结束放置在后面
            ch.test[1].p_write             =        ch.test[1].pulse_cnt 
                % CH_TIMEPARA_BUF_SIZE;
        }
            
        TIM_ClearITPendingBit(TIM8,TIM_IT_CC3);//清除中断标志
	}
    
}


/*******************************************************************************
* Description  : 配置定时器外部捕获
                定时器的双边捕获有bug，不能产生。在中断服务程序中改变触发边缘，实现
                双边捕获功能。
                
* Author       : 2018/3/16 星期五, by redmorningcn
*******************************************************************************/
void Timer8_Configuration(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
//	TIM_TimeBaseInitTypeDef	TIM_BaseInitStructure;
	TIM_ICInitTypeDef TIM_ICInitStruct;
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM8,ENABLE);//使能定时器时钟
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC,ENABLE);//使能IO口时钟
	
	GPIO_InitStructure.GPIO_Pin =        GPIO_Pin_6
                                        |GPIO_Pin_7  
                                        |GPIO_Pin_8
                                        |GPIO_Pin_9;
                           
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;//下拉输入
	GPIO_Init(GPIOC,&GPIO_InitStructure);//初始化
	
	//初始化TIM8输入捕获参数
	TIM_ICInitStruct.TIM_Channel = TIM_Channel_1;//输入端映射到TI1
    
	TIM_ICInitStruct.TIM_ICPolarity = TIM_ICPolarity_Rising;//上升沿捕获
    //TIM_ICInitStruct.TIM_ICPolarity = TIM_ICPolarity_BothEdge;//上升沿捕获
    
	TIM_ICInitStruct.TIM_ICSelection = TIM_ICSelection_DirectTI;//映射到TI1上
	TIM_ICInitStruct.TIM_ICPrescaler = TIM_ICPSC_DIV1;//输入不分频
	TIM_ICInitStruct.TIM_ICFilter = 0x00;//输入不滤波
	TIM_ICInit(TIM8,&TIM_ICInitStruct);//初始化TIM8
	
	TIM_ICInitStruct.TIM_Channel = TIM_Channel_2;//输入端映射到TI2
	TIM_ICInitStruct.TIM_ICPolarity = TIM_ICPolarity_Rising;//上升沿捕获
	TIM_ICInitStruct.TIM_ICSelection = TIM_ICSelection_DirectTI;//映射到TI2上
	TIM_ICInitStruct.TIM_ICPrescaler = TIM_ICPSC_DIV1;//输入不分频
	TIM_ICInitStruct.TIM_ICFilter = 0x00;//输入不滤波
	TIM_ICInit(TIM8,&TIM_ICInitStruct);//初始化TIM8
	
	TIM_ICInitStruct.TIM_Channel = TIM_Channel_3;//输入端映射到TI3
	TIM_ICInitStruct.TIM_ICPolarity = TIM_ICPolarity_Rising;//上升沿捕获
	TIM_ICInitStruct.TIM_ICSelection = TIM_ICSelection_DirectTI;//映射到TI3上
	TIM_ICInitStruct.TIM_ICPrescaler = TIM_ICPSC_DIV1;//输入不分频
	TIM_ICInitStruct.TIM_ICFilter = 0x00;//输入不滤波
	TIM_ICInit(TIM8,&TIM_ICInitStruct);//初始化TIM8
	
	TIM_ICInitStruct.TIM_Channel = TIM_Channel_4;//输入端映射到TI4
	TIM_ICInitStruct.TIM_ICPolarity = TIM_ICPolarity_Rising;//上升沿捕获
	TIM_ICInitStruct.TIM_ICSelection = TIM_ICSelection_DirectTI;//映射到TI4上
	TIM_ICInitStruct.TIM_ICPrescaler = TIM_ICPSC_DIV1;//输入不分频
	TIM_ICInitStruct.TIM_ICFilter = 0x00;//输入不滤波
	TIM_ICInit(TIM8,&TIM_ICInitStruct);//初始化TIM8
	
	TIM_ITConfig(TIM8,TIM_IT_Update|TIM_IT_CC1|TIM_IT_CC2|TIM_IT_CC3|TIM_IT_CC4,ENABLE);//不允许更新中断 CC1IE捕获中断
	
	BSP_IntVectSet(TIM8_CC_IRQn, TIM8_CC_IRQHandler);
	BSP_IntEn(TIM8_CC_IRQn);
	
	TIM_Cmd(TIM8,ENABLE);//开启定时器8
}

/*******************************************************************************
* Description  : 通道采样初始化
                初始化全局定时器和采样通道的外部中断；
                以及运行的全局变量；
* Author       : 2018/3/13 星期二, by redmorningcn
*******************************************************************************/
void    init_ch_timepara_detect(void)
{
    //Detect_exti_init();     //外部检测通道
    
    //Detect_exti01_init();
    
    Timer8_Iint();          //启动全局定时器
    
    Timer8_Configuration();
    
    //初始化运算参数
}


/*******************************************************************************
 * 				end of file
 *******************************************************************************/
