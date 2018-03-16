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
#include <xprintf.h>
#include "app_task_mater.h"
#include "app_collector.h"
#include "dtc.h"
#include <math.h>
#include "task.h"
#include "task_comm.h"


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
__packed
typedef struct
{
uint8 ch;
uint8 dest;
uint8 service;
uint8 cmd;
uint8 len;
uint8 chksum;
}ST_SAMPLE_CMD_STR;
/***********************************************
* 描述： 任务控制块（TCB）
*/
static  OS_TCB      AppTaskCommTCB;
extern OS_Q                COM_RxQ;

ST_MONITOR speed_collector_monitor = {0};


OS_TMR	app_comm_tmr = {0};
#ifdef SAMPLE_BOARD
OS_TMR app_collection_info_tmr = {0};
#else
OS_TMR	app_sample_tmr = {0};
OS_TMR	collector_timerout_tmr = {0};
#endif
uint16	collector_update_count_down_tmr = 0;
//OS_Q app_comm_queue = {0};
/***********************************************
* 描述： 任务堆栈（STACKS）
*/
static  CPU_STK     AppTaskCommStk[ APP_TASK_COMM_STK_SIZE ];


const uint8 get_collector_board_info[] = {0x00,0x2,0x81,0x01,0x0};

COLLECTION_STEP collect_step =COLLECTION_IDLE;


ST_DTC_MANGER dtc ={0};

/*******************************************************************************
* Description  : 采集板应答信息全局变量初始化
* Author       : 2018/1/23 星期二, by redmorningcn
*******************************************************************************/
ST_COLLECTOR_RESPONCE collector_responce =
{
	0x01,
	0x01,
	0x80,
	0x01,
	0x00,
	{
		0,				 /*工作状态;0:正常1:光缝被堵*/
		0,				/*reserved*/
		90,				/*相位差*/
		5000,			/*正向转动步数*/
		{
			{
				49,			/*占空比*/
				15000,		/*工作电压单位为mv*/
				15000,		/*高电平电压单位为mv*/
				1500,		/*低电平电压单位为mv*/
				20,			/* 上升沿边沿时间单位0.1us*/
				20,			/*下降沿边沿时间单位0.1us*/
			},
			{
				51,			/*占空比*/
				15000,		/*工作电压单位为mv*/
				15000,		/*高电平电压单位为mv*/
				1500,		/*低电平电压单位为mv*/
				20,			/* 上升沿边沿时间单位0.1us*/
				20,			/*下降沿边沿时间单位0.1us*/
			},
		},
	},
	0x00,
};




/*******************************************************************************
 * LOCAL VARIABLES
 */

/*******************************************************************************
 * GLOBAL VARIABLES
 */

/*******************************************************************************
 * LOCAL FUNCTIONS
 */
void         AppTaskComm                 (void *p_arg);
static void         APP_CommInit                (void);
void         App_McuStatusInit           (void);
void         ReportDevStatusHandle       (void);
void         InformCommConfigMode        (u8 mode);
void         App_ModbusInit              (void);

static void dtc_init(void);
INT08U              APP_CommRxDataDealCB        (MODBUS_CH  *pch);
INT08U              IAP_CommRxDataDealCB        (MODBUS_CH  *pch);
void app_abnormal_info_update(ST_COLLECTOR_INFO *p_collector_info, BOARD_ABNORMAL_STATUS *p_abnormal_info);
static void app_dtc_update(BOARD_ABNORMAL_STATUS *p_abnormal_info);


/*******************************************************************************
 * GLOBAL FUNCTIONS
 */

/*******************************************************************************
 * EXTERN VARIABLES
 */
ST_para_storage system_para;
StrMater            Mater;


 /*******************************************************************************
 * EXTERN FUNCTIONS
 */
extern void         uartprintf                  (MODBUS_CH  *pch,const char *fmt, ...);

/*******************************************************************************/


/*******************************************************************************
 * 名    称： App_TaskCommCreate
 * 功    能： **任务创建
 * 入口参数： 无
 * 出口参数： 无
 * 作    者： wumingshen.
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




/*
void app_get_collector_info()
{
	uint8 send_buf[250] = {0};
	uint8 len;
	memcpy(send_buf, get_collector_board_info, sizeof(get_collector_board_info));
//	send_buf[4] = sizeof(get_collector_board_info);
//	memcpy(&send_buf[5], &test_collector_info, sizeof(test_collector_info));
	len = sizeof(get_collector_board_info);
	nmb_send_data(Ctrl.Com.pch, (uint8 *)&send_buf, len);
}
*/

/*******************************************************************************
* 名    称： 
* 功    能： 
* 入口参数
* 出口参数： 无
* 作  　者： fourth.peng.
* 创建日期： 2017-4-28
* 修    改：
* 修改日期：
* 备    注：
*******************************************************************************/

/*******************************************************************************
* 名    称： verify_checksum_of_nmb_frame
* 功    能： 
* 入口参数
* 出口参数： 无
* 作  　者： fourth.peng.
* 创建日期： 2017-4-28
* 修    改：
* 修改日期：
* 备    注：
*******************************************************************************/
uint8 verify_checksum_of_nmb_frame(uint8 *data, uint16 len)
{
	uint8 i;
	uint8 checksum = 0;
	for(i = 0; i< (len-1); i++)
	{
		checksum += data[i];
	}

	if(checksum == data[len-1])
	{
		return TRUE;
	}

	return FALSE;
}

 typedef struct
 {
     uint8 board_a_lost_cnt;
     uint8 board_b_lost_cnt;
     uint8 board_c_lost_cnt;
 }collector_lost_cnt_type;

collector_lost_cnt_type collector_lost = {0};

/*******************************************************************************
* 名    称： collector_responce_analyze
* 功    能： 
* 入口参数
* 出口参数： 无
* 作  　者： fourth.peng.
* 创建日期： 2017-4-28
* 修    改：
* 修改日期：
* 备    注：
*******************************************************************************/
void collector_responce_analyze( const ST_COLLECTOR_RESPONCE *p_collector_info)
{
	switch(p_collector_info->addr)
	{
		case COLLECTOR_BOARD_A_ADDR:
//			if(BOARD_A_COLLECTING == collect_step)
			{
				memcpy(&Mater.monitor.collector_board[0], (const void *)&p_collector_info->collector_info, sizeof(ST_COLLECTOR_INFO));
				collector_lost.board_a_lost_cnt = 0;
				Mater.monitor.abnormal_info[0].connected_fail = FALSE;
				system_send_msg(COMM_TASK_ID, COMM_TASK_COLLECT_BOARD_B, NULL, 0);
			}
			
			break;
			
		case COLLECTOR_BOARD_B_ADDR:
//			if(BOARD_B_COLLECTING == collect_step)
			{
				memcpy(&Mater.monitor.collector_board[1], (const void *)&p_collector_info->collector_info, sizeof(ST_COLLECTOR_INFO));
				collector_lost.board_b_lost_cnt = 0;
				Mater.monitor.abnormal_info[1].connected_fail = FALSE;
				system_send_msg(COMM_TASK_ID, COMM_TASK_COLLECT_BOARD_C, NULL, 0);
			}			
			break;
			
		case COLLECTOR_BOARD_C_ADDR:
//			if(BOARD_C_COLLECTING == collect_step)
			{
				memcpy(&Mater.monitor.collector_board[2], (const void *)&p_collector_info->collector_info, sizeof(ST_COLLECTOR_INFO));
				collector_lost.board_c_lost_cnt = 0;
				Mater.monitor.abnormal_info[2].connected_fail = FALSE;
				system_send_msg(COMM_TASK_ID, COMM_TASK_COLLECT_COMPLETE, NULL, 0);
			}
			break;
	}
}

/*******************************************************************************
* 名    称： app_collection_info_tmr_callback
* 功    能： 
* 入口参数
* 出口参数： 无
* 作  　者： fourth.peng.
* 创建日期： 2017-5-11
* 修    改：
*******************************************************************************/
void app_collection_info_tmr_callback(OS_TMR *ptmr,void *p_arg)
{
	system_send_msg(COMM_TASK_ID, COMM_TASK_COLLECTION_INFO_UPDATE, NULL, 0);
}
/*******************************************************************************
* 名    称： comm_cmd_str_analyze
* 功    能： 
* 入口参数
* 出口参数： 无
* 作  　者： fourth.peng.
* 创建日期： 2017-4-28
* 修    改：
*******************************************************************************/
void comm_cmd_str_analyze(const ST_SAMPLE_CMD_STR *str)
{
#ifdef SAMPLE_BOARD
	uint16 send_dly_time = 0;
	uint16 event = (uint16)(str->service<<8) + str->cmd;
	static ST_QUEUETCB post_queue = {0};
	OS_ERR err;

	if (EVENT_REQUEST_COLLECTION_INFO == event)
	{
		if( 0 ==  str->dest)
		{
		//broad cast mode
			if ( 1 < Ctrl.sample_id)
			{
				send_dly_time = (Ctrl.sample_id-1)*INTERNAL_TIME;
#if 0
				OSTmrCreate(&app_collection_info_tmr,
								(CPU_CHAR *)"app_collection_info_tmr",
								send_dly_time, 
								send_dly_time, 
								OS_OPT_TMR_ONE_SHOT, 
								(OS_TMR_CALLBACK_PTR)app_collection_info_tmr_callback, 
								NULL, 
								&err);
				OSTmrStart(&app_collection_info_tmr, &err);
#else
				collector_update_count_down_tmr = send_dly_time;
#endif
			
			}
			else
			{
				system_send_msg(COMM_TASK_ID, COMM_TASK_COLLECTION_INFO_UPDATE, NULL, 0);
				
			}
		}
		else if(Ctrl.sample_id == str->dest)
		{
		//轮询模式，立即发送采样信息
			system_send_msg(COMM_TASK_ID, COMM_TASK_COLLECTION_INFO_UPDATE, NULL, 0);
		}
	}
#endif
}
/*******************************************************************************
* 名    称： nmb_rx_data_parser
* 功    能： 
* 入口参数
* 出口参数： 无
* 作  　者： fourth.peng.
* 创建日期： 2017-4-28
* 修    改：
* 修改日期：
* 备    注：
*******************************************************************************/
void nmb_rx_data_parser(uint8 *rx_data, uint16 len)
{
	uint8   parse_len =0;
	uint8   index = 0;
	uint8   i;
	uint8   *p_frame_start = rx_data;
	uint8   parse_data[50] = {0};

#ifdef SAMPLE_BOARD
	ST_SAMPLE_CMD_STR cmd_str = {0};
#else
	ST_COLLECTOR_RESPONCE collector_res = {0};
#endif
	for (i = 0; i< len; i++)
	{
		//接收到帧结束字符
		if (0x7e == rx_data[i])
		{
			parse_len = nmb_frame_translate(p_frame_start, parse_data,index);
			//checksum 校验
			if(verify_checksum_of_nmb_frame(parse_data, parse_len))
			{
#ifdef SAMPLE_BOARD
				memcpy(&cmd_str, (const void *)parse_data, sizeof(cmd_str));
				comm_cmd_str_analyze((const ST_SAMPLE_CMD_STR*)&cmd_str);
#else
				memcpy(&collector_res, (const void *)parse_data, sizeof(collector_res));
				collector_responce_analyze((const ST_COLLECTOR_RESPONCE *)&collector_res);
#endif
			}
			//数据包未解析完，继续处理下一帧
			if (index < len)
				{
				index =0;
				p_frame_start = rx_data + i + 1;
				}
		}
		else
		{
			index++;
		}
	}
}



/*******************************************************************************
 * 名    称： app_send_collector_update_cmd
 * 功    能： 发送更新检测信息命令给采集板
 * 入口参数： num - 采集板编号
 * 出口参数： 无
 * 作    者： fourth.peng
 * 创建日期： 2017-04-27
 * 修    改：
 * 修改日期：
 *******************************************************************************/
void app_send_collector_info_to_main_unit(void *data, uint16 len)
{
	uint8 send_buf[100] ={0};
	uint8 length = sizeof(collector_responce) -1;
	memcpy(send_buf, (const void *)&collector_responce, length);
	send_buf[1] = Ctrl.sample_id;
	nmb_send_data(Ctrl.Com.pch, (uint8 *)&send_buf, length);
}
/*******************************************************************************
 * 名    称： app_send_collector_update_cmd
 * 功    能： 发送更新检测信息命令给采集板
 * 入口参数： num - 采集板编号
 * 出口参数： 无
 * 作    者： fourth.peng
 * 创建日期： 2017-05-15
 * 修    改：
 * 修改日期：
 *******************************************************************************/
void app_update_detect_info(void *data, uint16 len)
{
	memcpy(&collector_responce.collector_info, data, sizeof(ST_COLLECTOR_INFO));
}


/*******************************************************************************
 * 名    称： app_send_collector_update_cmd
 * 功    能： 发送更新检测信息命令给采集板
 * 入口参数： num - 采集板编号
 * 出口参数： 无
 * 作    者： fourth.peng
 * 创建日期： 2017-04-27
 * 修    改：
 * 修改日期：
 *******************************************************************************/
void app_send_collector_update_cmd(uint8 num)
{
	uint8 send_buf[10] ={0};
	uint8 len = sizeof(get_collector_board_info) ;
	memcpy(send_buf, (const void *)&get_collector_board_info, len);
	send_buf[1] = 0;
	nmb_send_data(Ctrl.Com.pch, (uint8 *)&send_buf, len);
	
#ifndef SAMPLE_BOARD
	OS_ERR err;
	OSTmrStart(&collector_timerout_tmr, &err);
#endif
}

/*******************************************************************************
 * 名    称： app_start_collection
 * 功    能： comm task事件接收处理函数
 * 入口参数： 
 * 出口参数： 无
 * 作    者： fourth.peng
 * 创建日期： 2017-04-27
 * 修    改：
 * 修改日期：
 *******************************************************************************/
void app_start_collection(void *data, uint16 len)
{
	app_send_collector_update_cmd(1);
	collect_step = START_COLLECTION;
//	collect_step = BOARD_A_COLLECTING;
}
/*******************************************************************************
 * 名    称： app_collection_board_b
 * 功    能： comm task事件接收处理函数
 * 入口参数： 
 * 出口参数： 无
 * 作    者： fourth.peng
 * 创建日期： 2017-04-27
 * 修    改：
 * 修改日期：
 *******************************************************************************/
void app_collection_board_b(void *data, uint16 len)
{
//	app_send_collector_update_cmd(2);
	collect_step = BOARD_B_COLLECTING;

}
/*******************************************************************************
 * 名    称： app_collection_board_b
 * 功    能： comm task事件接收处理函数
 * 入口参数： 
 * 出口参数： 无
 * 作    者： fourth.peng
 * 创建日期： 2017-04-27
 * 修    改：
 * 修改日期：
 *******************************************************************************/
void app_collection_board_c(void *data, uint16 len)
{
//	app_send_collector_update_cmd(3);
	collect_step = BOARD_C_COLLECTING;

}
/*******************************************************************************
 * 名    称： app_comm_time_tick
 * 功    能： comm task事件接收处理函数
 * 入口参数： 
 * 出口参数： 无
 * 作    者： fourth.peng
 * 创建日期： 2017-04-27
 * 修    改：
 * 修改日期：
 *******************************************************************************/
void app_comm_time_tick(void *data, uint16 len)
{
	static ST_QUEUETCB queue = {0};
	static uint8 collection_start_cnt =0;
	OS_ERR err;

	collection_start_cnt++;	
	if(collection_start_cnt>=5)
	{
		collection_start_cnt = 0;
	}
	
	
		

}
/*******************************************************************************
 * 名    称： app_comm_rx_collector_info
 * 功    能： comm task事件接收处理函数
 * 入口参数： 
 * 出口参数： 无
 * 作    者： fourth.peng
 * 创建日期： 2017-04-27
 * 修    改：
 * 修改日期：
 *******************************************************************************/
void app_sample_tmr_callback(OS_TMR *ptmr,void *p_arg)
{
	system_send_msg(COMM_TASK_ID, COMM_TASK_START_COLLECTION, NULL, 0);
}

/*******************************************************************************
 * 名    称： app_comm_rx_collector_info
 * 功    能： comm task事件接收处理函数
 * 入口参数： 
 * 出口参数： 无
 * 作    者： fourth.peng
 * 创建日期： 2017-04-27
 * 修    改：
 * 修改日期：
 *******************************************************************************/
void app_create_sample_tmr(void *data, uint16 len)
{
	uint16 period_time;
	OS_ERR err = 0;
	
	memcpy(&period_time, data, sizeof(period_time));
#ifndef SAMPLE_BOARD
	OSTmrCreate(&app_sample_tmr,
					(CPU_CHAR *)"app_comm_tmr",
					period_time, 
					period_time, 
					OS_OPT_TMR_PERIODIC, 
					(OS_TMR_CALLBACK_PTR)app_sample_tmr_callback, 
					NULL, 
					&err);
	if(OS_ERR_NONE != err)
	{
	}

	OSTmrStart(&app_sample_tmr, &err);
#endif
	if(OS_ERR_NONE != err)
	{
	}
}


/*******************************************************************************
 * 名    称： app_comm_rx_collector_info
 * 功    能： comm task事件接收处理函数
 * 入口参数： 
 * 出口参数： 无
 * 作    者： fourth.peng
 * 创建日期： 2017-04-27
 * 修    改：
 * 修改日期：
 *******************************************************************************/
void app_comm_rx_collector_info(void *data, uint16 len)
{
	uint8 *rx_buf = data;
	nmb_rx_data_parser(rx_buf, len);
}

/*******************************************************************************
 * 名    称： app_comm_collector_data_parser
 * 功    能：对采集的数据进行计算并分析
 * 入口参数： 
 * 出口参数： 无
 * 作    者： fourth.peng
 * 创建日期： 2017-04-28
 * 修    改记录：
 2017-5-2	fourth		增加故障信息以及DTC刷新
 *******************************************************************************/
void app_comm_collector_data_parser(void *data, uint16 len)
{
	int32 average_steps = 0;
	int32 sum_steps = 0;
	uint8 err = 0;
	uint8 err_ch = 0;
	uint8 valid_cnt = 0;
	float speed = 0;
	ST_MONITOR *p_monitor = &Mater.monitor;
	uint8 i;

	for(i = 0; i <3; i++)
	{
		if ( FALSE == p_monitor->abnormal_info[i].connected_fail )/*是否接收到采集信息*/
		{
			if ( 0 == p_monitor->collector_board[i].status ) /*采集板工作正常*/
			{
				sum_steps += p_monitor->collector_board[i].step_counts;
				valid_cnt++;
			}
			/*更新故障信息*/
			app_abnormal_info_update(&p_monitor->collector_board[i], &p_monitor->abnormal_info[i]);
		}
	}
	/*故障码刷新*/
	app_dtc_update(p_monitor->abnormal_info);
	
	average_steps = sum_steps/valid_cnt;

/*
	for (i = 0; i < 3; i++)
	{
		if ((p_monitor->collector_board[i].step_counts > (average_steps * 1.001)) 
			|| (p_monitor->collector_board[i].step_counts < (average_steps * 0.999)))
		{
		//与平均值差距大于1/1000 则认为存在异常
			err = 1;
			break;
		}
	}

	if(err)
	{
		int32 diff[3] = {0};
		diff[0] = abs(p_monitor->collector_board[0].step_counts - average_steps);
		diff[1] = abs(p_monitor->collector_board[1].step_counts - average_steps);
		diff[2] = abs(p_monitor->collector_board[2].step_counts - average_steps);
		err_ch = ((diff[0] > diff[1])&&(diff[0]>diff[2]))? 0:(((diff[1] > diff[0])&&(diff[1] > diff[2]))? 1:2);

		average_steps = (sum_steps - p_monitor->collector_board[err_ch].step_counts)>>1;
	}
*/
	
	speed = (float)average_steps*system_para.speed_factor;
	
	Mater.monitor.real_speed = speed;
}

/* comm task 接收事件对应回调函数表格*/
const ST_TASK_EVENT_TBL_TYPE app_comm_task_event_table[]=
{
{COMM_TASK_TIME_TICK,					app_comm_time_tick					},
{COMM_TASK_START_COLLECTION,			app_start_collection					},
{ COMM_TASK_COLLECT_BOARD_B,			app_collection_board_b				},
{ COMM_TASK_COLLECT_BOARD_C,			app_collection_board_c				},
{ COMM_TASK_COLLECT_COMPLETE,			app_comm_collector_data_parser		},
{COMM_TASK_RX_COLLECTOR_INFO,			app_comm_rx_collector_info			},
{COMM_TASK_CREATE_SAMPLE_TMR,			app_create_sample_tmr				},
{COMM_TASK_COLLECTION_INFO_UPDATE,		app_send_collector_info_to_main_unit	},
{COMM_TASK_UPDATE_DETECT_INFO,			app_update_detect_info				},

};


/*******************************************************************************
 * 名    称： app_comm_task_event
 * 功    能： comm task事件接收入口函数
 * 入口参数： 
 * 出口参数： 无
 * 作    者： fourth.peng
 * 创建日期： 2017-04-27
 * 修    改：
 * 修改日期：
 *******************************************************************************/
void app_comm_task_event(ST_QUEUE *queue)//uint16 event, ST_QUEUEDATA *data)
{
	uint8 i = 0;
	TASK_EVENT_CALLBACK p_fn = NULL;
	for(i = 0; i < ((sizeof(app_comm_task_event_table))/(sizeof(ST_TASK_EVENT_TBL_TYPE))); i++)
	{
		if(app_comm_task_event_table[i].event == queue->event)
		{
			p_fn = app_comm_task_event_table[i].p_func;
			if (NULL != p_fn)
			{
				p_fn(queue->data, queue->len);
			}
			queue->event = 0;
			queue->data = 0;
			queue->len = 0;
			break;
		}
 	}
}


/*******************************************************************************
 * 名    称： AppTaskComm
 * 功    能： 控制任务
 * 入口参数： p_arg - 由任务创建函数传入
 * 出口参数： 无
 * 作    者： wumingshen.
 * 创建日期： 2015-02-05
 * 修    改：
 * 修改日期：
 *******************************************************************************/
void  AppTaskComm (void *p_arg)
{
	ST_QUEUE *p_mailbox = 0;
    
/*******************************************************************************
* Description  : 串口通讯函数，查询事件，并根据事件，调用对应的回调函数
                 事件设置在MODBUS底层的定时器处理函数中。
* Author       : 2018/1/30 星期二, by redmorningcn
*******************************************************************************/
	p_mailbox = (ST_QUEUE *)system_get_msg(COMM_TASK_ID);

	if (NULL != p_mailbox)
	{
		app_comm_task_event(p_mailbox);//->event, p_mailbox->data,);
	}
}

/*******************************************************************************
 * 名    称： APP_ModbusInit
 * 功    能： MODBUS初始化
 * 入口参数： 无
 * 出口参数： 无
 * 作    者： 无名沈
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
    // UART1
    /***********************************************
    * 描述： 若ModBus配置为从机（见mb_cfg.c）
    *        调试升级串口
    */
#if MODBUS_CFG_SLAVE_EN == DEF_TRUE
    pch         = MB_CfgCh( ModbusNode,             // ... Modbus Node # for this slave channel
                            MODBUS_SLAVE,           // ... This is a SLAVE
                            500,                    // ... 0 when a slave
                            MODBUS_MODE_RTU,        // ... Modbus Mode (_ASCII or _RTU)
                            0,                      // ... Specify UART #1
                            115200,                 // ... Baud Rate
                            USART_WordLength_8b,    // ... Number of data bits 7 or 8
                            USART_Parity_No,        // ... Parity: _NONE, _ODD or _EVEN
                            USART_StopBits_1,       // ... Number of stop bits 1 or 2
                            MODBUS_WR_EN);          // ... Enable (_EN) or disable (_DIS) writes
    pch->AesEn          = DEF_DISABLED;             // ... AES加密禁止
    pch->NonModbusEn    = DEF_ENABLED;              // ... 支持非MODBUS通信
    pch->IapModbusEn    = DEF_ENABLED;              // ... 支持IAP MODBUS通信
    
    pch->RxFrameHead    = 0x1028;                   // ... 添加匹配帧头
    pch->RxFrameTail    = 0x102C;                   // ... 添加匹配帧尾
    
    Ctrl.Com.pch       = pch;
    
    xdev_out(xUSART1_putchar);
    xdev_in(xUSART1_getchar);  
#endif
    
    // UART2
    /***********************************************
    * 描述： 若ModBus配置为从机（见mb_cfg.c）
    *        扩展通讯串口，与思维通讯
    */
#if MODBUS_CFG_SLAVE_EN == DEF_TRUE
    pch         = MB_CfgCh( ModbusNode,             // ... Modbus Node # for this slave channel
                            MODBUS_SLAVE,           // ... This is a SLAVE
                            500,                    // ... 0 when a slave
                            MODBUS_MODE_RTU,        // ... Modbus Mode (_ASCII or _RTU)
                            1,                      // ... Specify UART #2
                            57600,                  // ... Baud Rate
                            USART_WordLength_8b,    // ... Number of data bits 7 or 8
                            USART_Parity_No,        // ... Parity: _NONE, _ODD or _EVEN
                            USART_StopBits_1,       // ... Number of stop bits 1 or 2
                            MODBUS_WR_EN);          // ... Enable (_EN) or disable (_DIS) writes
    pch->AesEn          = DEF_DISABLED;             // ... AES加密禁止
    pch->NonModbusEn    = DEF_ENABLED;              // ... 支持非MODBUS通信
    pch->IapModbusEn    = DEF_ENABLED;              // ... 支持IAP MODBUS通信
    
    pch->RxFrameHead    = 0x1028;                   // ... 添加匹配帧头
    pch->RxFrameTail    = 0x102C;                   // ... 添加匹配帧尾
    
    Ctrl.Dtu.pch        = pch;
#endif


#ifndef DISABLE_UART3_4_FOR_MODBUS
    // UART3
    /***********************************************
    * 描述： 若ModBus配置为主机（见mb_cfg.c）
    *        无线通讯串口，与公司服务器
    */
#if MODBUS_CFG_MASTER_EN == DEF_TRUE
    
    pch         = MB_CfgCh( ModbusNode,             // ... Modbus Node # for this slave channel
                            MODBUS_SLAVE,           // ... This is a SLAVE
                            500,                    // ... 0 when a slave
                            MODBUS_MODE_RTU,        // ... Modbus Mode (_ASCII or _RTU)
                            2,                      // ... Specify UART #3
                            57600,                   // ... Baud Rate
                            USART_WordLength_8b,    // ... Number of data bits 7 or 8
                            USART_Parity_No,        // ... Parity: _NONE, _ODD or _EVEN
                            USART_StopBits_1,       // ... Number of stop bits 1 or 2
                            MODBUS_WR_EN);          // ... Enable (_EN) or disable (_DIS) writes
    pch->AesEn          = DEF_DISABLED;             // ... AES加密禁止
    pch->NonModbusEn    = DEF_ENABLED;              // ... 支持非MODBUS通信
    pch->IapModbusEn    = DEF_ENABLED;              // ... 支持IAP MODBUS通信
    
    pch->RxFrameHead    = 0x1028;                   // ... 添加匹配帧头
    pch->RxFrameTail    = 0x102C;                   // ... 添加匹配帧尾
    
    Ctrl.Dtu.pch        = pch;
#endif
    // UART4
    /***********************************************
    * 描述： 若ModBus配置为主机（见mb_cfg.c）
    *        无线通讯串口，与公司服务器
    */
#if MODBUS_CFG_SLAVE_EN == DEF_TRUE
    
    pch         = MB_CfgCh( ModbusNode,             // ... Modbus Node # for this slave channel
                            MODBUS_SLAVE,           // ... This is a SLAVE
                            500,                    // ... 0 when a slave
                            MODBUS_MODE_RTU,        // ... Modbus Mode (_ASCII or _RTU)
                            3,                      // ... Specify UART #3
                            19200,                  // ... Baud Rate
                            USART_WordLength_8b,    // ... Number of data bits 7 or 8
                            USART_Parity_No,        // ... Parity: _NONE, _ODD or _EVEN
                            USART_StopBits_1,       // ... Number of stop bits 1 or 2
                            MODBUS_WR_EN);          // ... Enable (_EN) or disable (_DIS) writes
    pch->AesEn          = DEF_DISABLED;             // ... AES加密禁止
    pch->NonModbusEn    = DEF_ENABLED;              // ... 支持非MODBUS通信
    pch->IapModbusEn    = DEF_ENABLED;              // ... 支持IAP MODBUS通信
    
    pch->RxFrameHead    = 0x1028;                   // ... 添加匹配帧头
    pch->RxFrameTail    = 0x102C;                   // ... 添加匹配帧尾
    
    Ctrl.Otr.pch        = pch;
#endif
#endif
    
    Ctrl.Com.SlaveAddr  = 0x84;
}


/*******************************************************************************
 * 名    称： app_comm_tmr_callback
 * 功    能： comm task 的软定时器的回调函数
 * 入口参数： 无
 * 出口参数： 无
 * 作　　者： pengwending
 * 创建日期： 2017-4-26
 * 修    改：
 * 修改日期：
 *******************************************************************************/
void app_comm_tmr_callback(OS_TMR *ptmr,void *p_arg)
{
	system_send_msg(COMM_TASK_ID, COMM_TASK_TIME_TICK, NULL, 0);
}

/*******************************************************************************
 * 名    称： app_collector_timeout_tmr_call_back
 * 功    能： comm task 的软定时器的回调函数
 * 入口参数： 无
 * 出口参数： 无
 * 作　　者： pengwending
 * 创建日期： 2017-4-26
 * 修    改：
 * 修改日期：
 *******************************************************************************/
void app_collector_timeout_tmr_call_back(OS_TMR *ptmr,void *p_arg)
{
	BOARD_ABNORMAL_STATUS *p_abnormal_info = Mater.monitor.abnormal_info;

	switch ( collect_step )
	{
		case BOARD_A_COLLECTING:
			collector_lost.board_a_lost_cnt++;
			if(collector_lost.board_a_lost_cnt > 5)
			{
				memset(&Mater.monitor.collector_board[0], 0, sizeof(ST_COLLECTOR_INFO));
				p_abnormal_info[0].connected_fail = TRUE;
			}
			system_send_msg(COMM_TASK_ID, COMM_TASK_COLLECT_BOARD_B, NULL, 0);
			break;
		case BOARD_B_COLLECTING:
			collector_lost.board_b_lost_cnt++;
			if(collector_lost.board_b_lost_cnt > 5)
			{
				memset(&Mater.monitor.collector_board[1], 0, sizeof(ST_COLLECTOR_INFO));
				p_abnormal_info[1].connected_fail = TRUE;
			}
			system_send_msg(COMM_TASK_ID, COMM_TASK_COLLECT_BOARD_C, NULL, 0);
			break;
		case BOARD_C_COLLECTING:
			collector_lost.board_c_lost_cnt++;
			if(collector_lost.board_c_lost_cnt > 5)
			{
				memset(&Mater.monitor.collector_board[2], 0, sizeof(ST_COLLECTOR_INFO));
				p_abnormal_info[2].connected_fail = TRUE;
			}
			
			system_send_msg(COMM_TASK_ID, COMM_TASK_COLLECT_COMPLETE, NULL, 0);
			break;
		case START_COLLECTION:
			collector_lost.board_a_lost_cnt++;
			if(collector_lost.board_a_lost_cnt > 5)
			{
				memset(&Mater.monitor.collector_board[0], 0, sizeof(ST_COLLECTOR_INFO));
				p_abnormal_info[0].connected_fail = TRUE;
			}
			
			collector_lost.board_b_lost_cnt++;
			if(collector_lost.board_b_lost_cnt > 5)
			{
				memset(&Mater.monitor.collector_board[1], 0, sizeof(ST_COLLECTOR_INFO));
				p_abnormal_info[1].connected_fail = TRUE;
			}
			
			collector_lost.board_c_lost_cnt++;
			if(collector_lost.board_c_lost_cnt > 5)
			{
				memset(&Mater.monitor.collector_board[2], 0, sizeof(ST_COLLECTOR_INFO));
				p_abnormal_info[2].connected_fail = TRUE;
			}
			system_send_msg(COMM_TASK_ID, COMM_TASK_COLLECT_COMPLETE, NULL, 0);
			break;
	}

}

/*******************************************************************************
 * 名    称： APP_CommInit
 * 功    能： 任务初始化
 * 入口参数： 无
 * 出口参数： 无
 * 作    者： wumingshen.
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
                        + COMM_EVT_FLAG_CONFIG      // 配置
                        + COMM_EVT_FLAG_IAP_START   // IAP开始
                        + COMM_EVT_FLAG_IAP_END    // IAP结束
                        +COMM_EVT_FLAG_TIME_TICK;
        
    /***********************************************
    * 描述： 初始化MODBUS通信
    */        
    App_ModbusInit();
    
    /***********************************************
    * 描述： 在看门狗标志组注册本任务的看门狗标志
    */

//    WdtFlags |= WDT_FLAG_COMM;

	/*创建一个周期为20ms的定时器*/
	OSTmrCreate(&app_comm_tmr,
					(CPU_CHAR *)"app_comm_tmr",
					20, 
					20, 
					OS_OPT_TMR_PERIODIC, 
					(OS_TMR_CALLBACK_PTR)app_comm_tmr_callback, 
					NULL, 
					&err);
	#ifndef SAMPLE_BOARD
	OSTmrCreate(&collector_timerout_tmr,
				(CPU_CHAR *)"collector time out tmr",
				50, 
				50, 
				OS_OPT_TMR_ONE_SHOT, 
				(OS_TMR_CALLBACK_PTR)app_collector_timeout_tmr_call_back, 
				NULL, 
				&err);
	OSTmrStart(&app_comm_tmr, &err);
	#endif
	#if	0	//def DISABLE_MATER_TASK
	ST_QUEUEDATA data;
	uint16 internal_time = 100;
	data.pdata = &internal_time;
	data.len = sizeof(internal_time);
	app_create_sample_tmr(&data);
	#endif

	/*创建一个队列，用于消息传递*/
	OSQCreate(&COM_RxQ, "comm RxQ", 100, &err);
	
	
}

/*******************************************************************************
 * 名    称： APP_CommRxDataDealCB
 * 功    能： 串口数据处理回调函数，由MB_DATA.C调用
 * 入口参数： 无
 * 出口参数： 无
 * 作    者： 无名沈
 * 创建日期： 2016-01-04
 * 修    改：
 * 修改日期：
 * 备    注： 
 *******************************************************************************/
extern CPU_BOOLEAN APP_CSNC_CommHandler(MODBUS_CH  *pch);
                extern void UpdateRecordPoint(uint8_t storeflag);

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
//    CPU_SR_ALLOC();
//    CPU_CRITICAL_ENTER();
    uint16_t Len     = pch->RxBufByteCtr;
    memcpy( (INT08U *)&pch->RxFrameData, (INT08U *)pch->RxBuf, Len );
//    CPU_CRITICAL_EXIT();
    
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
    * 描述： 如果地址为0，则不处理
    */
    if ( DstAddr == 0 )
        return FALSE;
    
    /***********************************************
    * 描述： 获取帧头
    */
//    OS_CRITICAL_ENTER();
    memcpy( (INT08U *)&Ctrl.Com.Rd.Head, (INT08U *)pch->RxFrameData, 8 );
//    OS_CRITICAL_EXIT();

    /***********************************************
    * 描述： 地址比对，不是本机地址，直接返回
    */
    if ( Ctrl.Com.Rd.Head.DstAddr != Ctrl.Com.SlaveAddr ) {
        /***********************************************
        * 描述： 转发数据
        */
        //NMB_Tx(Ctrl.Otr.pch,
        //       (CPU_INT08U  *)pch->RxBuf,
        //       (CPU_INT16U   )pch->RxBufByteCtr); 
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
        CSNC_SendData(	(MODBUS_CH      *)Ctrl.Com.pch,
                        (unsigned char  ) Ctrl.Com.SlaveAddr,         // SourceAddr,
                        (unsigned char  ) Ctrl.Com.Rd.Head.SrcAddr,   // DistAddr,
                        (unsigned char *)&pch->RxFrameData[8],         // DataBuf,
                        (unsigned int	 ) Len);                        // DataLen 
#endif
        IAP_Reset();
        Iap.FrameIdx    = 0;
        return TRUE;
    }    

    /***********************************************
    * 描述： 如果数据长度或者地址为0，则不处理
    */
    if ( Len == 0 ) {
        if ( pch->PortNbr == Ctrl.Dtu.pch->PortNbr ) {
            if ( GetRecvFrameNbr() == Ctrl.Com.Rd.Head.PacketSn ) {
                /***********************************************
                * 描述： 清除定时上传标志
                */
                SetSendFrameNbr();
#ifndef SAMPLE_BOARD
                UpdateRecordPoint(1);
#endif
                pch->StatNoRespCtr  = 0;
                
#ifndef SAMPLE_BOARD
                osal_set_event( OS_TASK_ID_TMR, OS_EVT_TMR_MIN);
#endif
                
                /***********************************************
                * 描述： 串口接受COMM模组的消息并处理
                */
                Ctrl.Com.ConnectTimeOut    = 0;                // 超时计数器清零
                Ctrl.Com.ConnectFlag       = TRUE;             // 转连接成功标志
                return TRUE;
            }
        }
        return FALSE;
    }
    /***********************************************
    * 描述： 获取数据
    */
    if( Len > 256 )
        return FALSE;
    
//    OS_CRITICAL_ENTER();
//    memcpy( (INT08U *)&Ctrl.Com.Rd.Data, (INT08U *)&pch->RxFrameData[8], Len );
//    OS_CRITICAL_EXIT();
    
    /***********************************************
    * 描述： 升级程序A
    */
    if ( ( Ctrl.Com.Rd.Head.PacketCtrl & 0x0f ) == 0x04 ) {
        
    /***********************************************
    * 描述： 升级程序B
    */
    } else if ( ( Ctrl.Com.Rd.Head.PacketCtrl & 0x0f ) == 0x03 ) {
        
    /***********************************************
    * 描述： 参数读取
    */
    } else if ( ( Ctrl.Com.Rd.Head.PacketCtrl & 0x0f ) == 0x02 ) {
        
    /***********************************************
    * 描述： 参数设置
    */
    } else if ( ( Ctrl.Com.Rd.Head.PacketCtrl & 0x0f ) == 0x01 ) {
        
    /***********************************************
    * 描述： 读数据
    */
    } else if ( ( Ctrl.Com.Rd.Head.PacketCtrl & 0x0f ) == 0x00  ) {        
        return TRUE;//APP_CSNC_CommHandler(pch);
    }
exit:
    /***********************************************
    * 描述： 串口接受COMM模组的消息并处理
    */
    Ctrl.Com.ConnectTimeOut    = 0;                // 超时计数器清零
    Ctrl.Com.ConnectFlag       = TRUE;             // 转连接成功标志
    
    return TRUE;
}

void app_comm_speed_collection_rx_data(MODBUS_CH *pch)
{
	uint8 rx_buf[100]={0};
	uint8 i;
	uint8 checksum = 0;
	ST_COLLECTOR_INFO collector_info = {0};

	uint8 len = pch->RxBufByteCtr-1;

	memcpy (rx_buf, pch->RxBuf, len);
	len -= 1;
	for (i =0; i < len; i++)
	{
		checksum += rx_buf[i];
	}

	if(checksum == rx_buf[len])
	{
		memcpy((void *)&collector_info, &rx_buf[5], sizeof(collector_info));
	}
	
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
 * 作    者： 无名沈
 * 创建日期： 2016-01-04
 * 修    改：
 * 修改日期：
 * 备    注： 
 *******************************************************************************/
INT08U IAP_CommRxDataDealCB(MODBUS_CH  *pch)
{
//    CPU_SR_ALLOC();
//    CPU_CRITICAL_ENTER();
    u8  Len     = pch->RxBufByteCtr;
    memcpy( (INT08U *)&pch->RxFrameData, (INT08U *)pch->RxBuf, Len );
//    CPU_CRITICAL_EXIT();
    
    /***********************************************
    * 描述： 获取帧头
    */    
    INT08U  *DataBuf    = pch->RxFrameData;
    
    /***********************************************
    * 描述： 接收数据处理
    */
    u8  DstAddr = 
    CSNC_GetData(	(unsigned char *)pch->RxFrameData,          //RecBuf,
                    (unsigned char	 )Len,                       //RecLen, 
                    (unsigned char *)DataBuf,                   //DataBuf,
                    (unsigned short *)&Len);                    //InfoLen)
    /***********************************************
    * 描述： 检测接收结束
    */
    if ( ( Len == 16) && 
         ( 0 >= memcmp((const char *)"IAP_pragram end!",(const char *)&pch->RxFrameData[8], 16) ) ) {
        /***********************************************
        * 描述： 发送数据处理
        */
        CSNC_SendData(	(MODBUS_CH      *)Ctrl.Com.pch,
                        (unsigned char  ) Ctrl.Com.SlaveAddr,          // SourceAddr,
                        (unsigned char  ) Ctrl.Com.Rd.Head.SrcAddr,    // DistAddr,
                        (unsigned char *)&pch->RxFrameData[8],          // DataBuf,
                        (unsigned short ) Len); 
        /***********************************************
        * 描述： 置位IAP结束标志位
        */
/*
        OS_ERR err;
        OS_FlagPost(( OS_FLAG_GRP *)&Ctrl.Os.CommEvtFlagGrp,
                    ( OS_FLAGS     ) COMM_EVT_FLAG_IAP_END,
                    ( OS_OPT       ) OS_OPT_POST_FLAG_SET,
                    ( CPU_TS       ) 0,
                    ( OS_ERR      *) &err);
*/        
        return TRUE;
    }
    
    if ( ( Len == 18) && 
         ( 0 >= memcmp((const char *)"IAP_pragram start!",(const char *)&pch->RxFrameData[8], 18) ) ) {
        
#if defined     (IMAGE_A) || defined   (IMAGE_B)
#else
        /***********************************************
        * 描述： 发送数据处理
        */
        CSNC_SendData(	(MODBUS_CH      *)Ctrl.Com.pch,
                        (unsigned char  ) Ctrl.Com.SlaveAddr,          // SourceAddr,
                        (unsigned char  ) Ctrl.Com.Rd.Head.SrcAddr,    // DistAddr,
                        (unsigned char *)&pch->RxFrameData[8],         // DataBuf,
                        (unsigned int	 ) Len);                        // DataLen 
#endif
        IAP_Restart();
        Iap.FrameIdx    = 0;
        return TRUE;
    }
    
    if ( Iap.Status < IAP_STS_START )
        return TRUE;
    /***********************************************
    * 描述： 写数据
    */
    /***********************************************
    * 描述： 将数据打包给发送结构
    */
    Ctrl.Com.Wr.Head.DataLen       = 0;
    /***********************************************
    * 描述： 编辑应答内容
    */
    char str[20];
    usprintf(str,"\n%d",Iap.FrameIdx);
    str[19]  = 0;
    /***********************************************
    * 描述： 写数据到Flash
    */
    IAP_Program((StrIapState *)&Iap, (INT16U *)&pch->RxFrameData[8], Len, (INT16U )GetRecvFrameNbr() );
    /***********************************************
    * 描述： 发送数据处理
    */
    CSNC_SendData(	(MODBUS_CH     *) Ctrl.Com.pch,
                    (unsigned char  ) Ctrl.Com.SlaveAddr,                    // SourceAddr,
                    (unsigned char  ) Ctrl.Com.Rd.Head.SrcAddr,              // DistAddr,
                    (unsigned char *) str,                                    // DataBuf,
                    (unsigned short ) strlen(str));                           // DataLen 
    if ( Len < 128 ) {
        /***********************************************
        * 描述： 置位IAP结束标志位
        */
/*        OS_ERR err;
        OS_FlagPost(( OS_FLAG_GRP *)&Ctrl.Os.CommEvtFlagGrp,
                    ( OS_FLAGS     ) COMM_EVT_FLAG_IAP_END,
                    ( OS_OPT       ) OS_OPT_POST_FLAG_SET,
                    ( CPU_TS       ) 0,
                    ( OS_ERR      *) &err);*/
    } else {
        IAP_Programing();                               // 复位超时计数器
    }
        
    /***********************************************
    * 描述： 串口接受COMM模组的消息并处理
    */
    Ctrl.Com.ConnectTimeOut    = 0;                // 超时计数器清零
    Ctrl.Com.ConnectFlag       = TRUE;             // 转连接成功标志
    
    return TRUE;
}
#endif

/*******************************************************************************
 * 名    称： NMBS_FCxx_Handler
 * 功    能： 非MODBUS串口数据处理回调函数，由mbs_core.d调用
 * 入口参数： 无
 * 出口参数： 无
 * 作    者： 无名沈
 * 创建日期： 2017-02-03
 * 修    改：
 * 修改日期：
 * 备    注： 
 *******************************************************************************/
extern OS_Q                MTR_RxQ;
extern OS_Q                COM_RxQ;
extern OS_Q                DTU_RxQ;
CPU_BOOLEAN  NMBS_FCxx_Handler (MODBUS_CH  *pch)
{
	static ST_QUEUETCB post_queue = {0};
    OS_ERR      err;
    CPU_INT16U  head    = BUILD_INT16U(pch->RxBuf[1], pch->RxBuf[0]);
    CPU_INT16U  tail    = BUILD_INT16U(pch->RxBuf[pch->RxBufByteCtr-1],
                                       pch->RxBuf[pch->RxBufByteCtr-2]);
    /***********************************************
    * 描述： 长沙南车DTU协议处理
    */
    if ( ( pch->RxFrameHead == head ) &&
         ( pch->RxFrameTail == tail ) ) {
        APP_CommRxDataDealCB(pch);
        /***********************************************
        * 描述： 电表调试协议协议,电表协议协议处理处理
        */
    } else if ( ( 0xAA55 == head ) || ( 0xAAAA == head ) ) {	//此分支没有校验 彭文丁
        OS_ERR  err;
        
//        CPU_SR_ALLOC();
//        OS_CRITICAL_ENTER();
        memcpy( (INT08U *)&pch->RxFrameData, (INT08U *)pch->RxBuf, pch->RxBufByteCnt );
//        OS_CRITICAL_EXIT();
        
#ifndef SAMPLE_BOARD
/*        (void)OSQPost((OS_Q         *)&MTR_RxQ,
                      (void         *) pch,
                      (OS_MSG_SIZE   ) pch->RxBufByteCtr,
                      (OS_OPT        ) OS_OPT_POST_FIFO,
                      (OS_ERR       *)&err);
         BSP_OS_TimeDly(5);		//此处为什么delay? 彭文丁*/
	system_send_msg(MATER_TASK_ID, MATER_TASK_RX_NMB_DATA, pch, pch->RxBufByteCtr);
#endif
         /***********************************************
         * 描述： 电表协议协议处理
         */  
    }
	else if(0x7e == (0xff&tail))
     	{
//        CPU_SR_ALLOC();
//        OS_CRITICAL_ENTER();
        memcpy( (INT08U *)&pch->RxFrameData, (INT08U *)pch->RxBuf, pch->RxBufByteCnt );
//        OS_CRITICAL_EXIT();

/*
	post_queue.event = COMM_TASK_RX_COLLECTOR_INFO;
	post_queue.queue_data.pdata = pch->RxFrameData;
	post_queue.queue_data.len = pch->RxBufByteCtr;
        (void)OSQPost((OS_Q         *)&COM_RxQ,
                      (void         *) &post_queue,
                      (OS_MSG_SIZE   ) sizeof(post_queue),
                      (OS_OPT        ) OS_OPT_POST_FIFO,
                      (OS_ERR       *)&err);
*/
	system_send_msg(COMM_TASK_ID, COMM_TASK_RX_COLLECTOR_INFO, pch->RxFrameData, pch->RxBufByteCtr);
//		app_comm_speed_collection_rx_data(pch);
     	}
	else {
        return DEF_FALSE;
    }
    return DEF_TRUE;
}

/*******************************************************************************
 * 名    称： dtc_init
 * 功    能： 故障码初始化为无故障
 * 入口参数： 无
 * 出口参数： 无
 * 作    者： fourth.peng
 * 创建日期： 2017-05-2
 * 修    改：
 * 修改日期：
 *******************************************************************************/
static void dtc_init(void)
{
	uint8 i;

	memset((void *)&dtc, NO_FAULT, sizeof(dtc));

}
/*******************************************************************************
 * 名    称： app_abnormal_info_update
 * 功    能： 刷新故障码
 * 入口参数： 无
 * 出口参数： 无
 * 作    者： fourth.peng
 * 创建日期： 2017-05-2
 * 修    改：
 * 修改日期：
 *******************************************************************************/
 void app_abnormal_info_update(ST_COLLECTOR_INFO *p_collector_info, BOARD_ABNORMAL_STATUS *p_abnormal_info)
{
	/*相位差小于45°或大于135°认为相位差异常*/
	const ST_COLLECTOR_THRESHOLD *p_threshold = &system_para.collector_threshold;
	if (( p_threshold->phase_threshold_low > p_collector_info->phase_diff )
		|| (p_threshold->phase_threshold_hig < p_collector_info->phase_diff))
	{
		p_abnormal_info->phase_diff_abnomal = TRUE;
	}
	else
	{
		p_abnormal_info->phase_diff_abnomal = FALSE;
	}
	
	/*占空比小于30%或者大于%70*/
	if (( p_threshold->duty_threshold_low > p_collector_info->channel[0].duty_ratio ) || ( p_threshold->phase_threshold_hig < p_collector_info->channel[0].duty_ratio))
	{
		p_abnormal_info->channel[0].dtc_flag.duty_ratio = 1;
	}
	else
	{
		p_abnormal_info->channel[0].dtc_flag.duty_ratio = 0;
	}
	if (( p_threshold->duty_threshold_low > p_collector_info->channel[1].duty_ratio ) || ( p_threshold->phase_threshold_hig < p_collector_info->channel[1].duty_ratio))
	{
		p_abnormal_info->channel[1].dtc_flag.duty_ratio = 1;
	}
	else
	{
		p_abnormal_info->channel[1].dtc_flag.duty_ratio = 0;
	}
	
	/*边沿时间大于4.2us*/
	if ( p_threshold->edge_time_threshold < p_collector_info->channel[0].rise_time)
	{
		p_abnormal_info->channel[0].dtc_flag.rise_time = 1;
	}
	else
	{
		p_abnormal_info->channel[0].dtc_flag.rise_time = 0;
	}
	if ( p_threshold->edge_time_threshold < p_collector_info->channel[0].fall_time)
	{
		p_abnormal_info->channel[0].dtc_flag.fall_time = 1;
	}
	else
	{
		p_abnormal_info->channel[0].dtc_flag.fall_time = 0;
	}
	if ( p_threshold->edge_time_threshold < p_collector_info->channel[1].rise_time)
	{
		p_abnormal_info->channel[1].dtc_flag.rise_time = 1;
	}
	else
	{
		p_abnormal_info->channel[1].dtc_flag.rise_time = 0;
	}
	if ( p_threshold->edge_time_threshold < p_collector_info->channel[1].fall_time)
	{
		p_abnormal_info->channel[1].dtc_flag.fall_time = 1;
	}
	else
	{
		p_abnormal_info->channel[1].dtc_flag.fall_time = 0;
	}
	/*工作电压*/
	if ((p_threshold->sensor_vcc_threshold_low > p_collector_info->channel[0].vcc_vol)
		||(p_threshold->sensor_vcc_threshold_hig < p_collector_info->channel[0].vcc_vol))
	{
		p_abnormal_info->channel[0].dtc_flag.vcc_vol = 1;
	}
	else
	{
		p_abnormal_info->channel[0].dtc_flag.vcc_vol = 0;
	}
	
	if ((p_threshold->sensor_vcc_threshold_low > p_collector_info->channel[1].vcc_vol)
		||(p_threshold->sensor_vcc_threshold_hig < p_collector_info->channel[1].vcc_vol))
	{
		p_abnormal_info->channel[1].dtc_flag.vcc_vol = 1;
	}
	else
	{
		p_abnormal_info->channel[1].dtc_flag.vcc_vol = 0;
	}
	/*高低电平*/
	if ((p_threshold->hig_level_threshold_factor *p_collector_info->channel[0].vcc_vol / 100) > p_collector_info->channel[0].high_level_vol)
	{
		p_abnormal_info->channel[0].dtc_flag.high_level = 1;
	}
	else
	{
		p_abnormal_info->channel[0].dtc_flag.high_level = 0;
	}

	if ((p_threshold->low_level_threshold_factor *p_collector_info->channel[0].vcc_vol / 100) < p_collector_info->channel[0].low_level_vol)
	{
		p_abnormal_info->channel[0].dtc_flag.low_level = 1;
	}
	else
	{
		p_abnormal_info->channel[0].dtc_flag.low_level = 0;
	}

	if ((p_threshold->hig_level_threshold_factor *p_collector_info->channel[1].vcc_vol / 100) > p_collector_info->channel[1].high_level_vol)
	{
		p_abnormal_info->channel[1].dtc_flag.high_level = 1;
	}
	else
	{
		p_abnormal_info->channel[1].dtc_flag.high_level = 0;
	}

	if ((p_threshold->low_level_threshold_factor *p_collector_info->channel[1].vcc_vol / 100) < p_collector_info->channel[1].low_level_vol)
	{
		p_abnormal_info->channel[1].dtc_flag.low_level = 1;
	}
	else
	{
		p_abnormal_info->channel[1].dtc_flag.low_level = 0;
	}

	
}
/*******************************************************************************
 * 名    称： app_dtc_update
 * 功    能： 刷新故障码
 * 入口参数： 无
 * 出口参数： 无
 * 作    者： fourth.peng
 * 创建日期： 2017-05-2
 * 修    改：
 * 修改日期：
 *******************************************************************************/
static void app_dtc_update(BOARD_ABNORMAL_STATUS *p_abnormal_info)
{
	uint8 i;
	uint8 fault_cnt = 0;
	
	for(i = 0; i <3; i++)
	{
		if(0 == p_abnormal_info[i].connected_fail)
		{
			dtc.curr_dtc[COLLECTOR_I_CONNECT_FAIL+i] = NO_FAULT;
			
			if ( TRUE == p_abnormal_info[i].phase_diff_abnomal)
			{
				dtc.curr_dtc[COLLECTOR_I_PHASE_ABNORMAL+i] = FAULT;
			}
			else
			{
				dtc.curr_dtc[COLLECTOR_I_PHASE_ABNORMAL+i] = NO_FAULT;
			}

			if ( p_abnormal_info[i].channel[0].share_byte )
			{
				dtc.curr_dtc[SENSOR_I_IS_ABNORMAL + (2*i)] = FAULT;
			}
			else
			{
				dtc.curr_dtc[SENSOR_I_IS_ABNORMAL + (2*i)] = NO_FAULT;
			}
			
			if ( p_abnormal_info[i].channel[1].share_byte )
			{
				dtc.curr_dtc[SENSOR_I_IS_ABNORMAL +1 + (2*i)] = FAULT;
			}
			else
			{
				dtc.curr_dtc[SENSOR_I_IS_ABNORMAL +1 + (2*i)] = NO_FAULT;
			}		
		}
		else
		{
			dtc.curr_dtc[COLLECTOR_I_CONNECT_FAIL+i] = FAULT;
			dtc.curr_dtc[COLLECTOR_I_PHASE_ABNORMAL+i] = NO_FAULT;
			dtc.curr_dtc[SENSOR_I_IS_ABNORMAL + (2*i)] = NO_FAULT;
			dtc.curr_dtc[SENSOR_I_IS_ABNORMAL +1 + (2*i)] = NO_FAULT;
		}

	}
	
	memset( dtc.fault_list, MAX_DTC_NUM, sizeof(dtc.fault_list) );
	for(i = 0;i < MAX_DTC_NUM; i++)
	{
		if ( FAULT == dtc.curr_dtc[i] )
			dtc.fault_list[fault_cnt++] = i;
	}
	dtc.count = fault_cnt;

	/*与上次故障码是否一致，如果不一致则保存当前数据*/
	if(0 != memcmp(dtc.prev_dtc, dtc.curr_dtc, sizeof(dtc.curr_dtc)))
	{
/*		OS_ERR err;
		
	        OS_FlagPost(( OS_FLAG_GRP *)&Ctrl.Os.MaterEvtFlagGrp,
			            ( OS_FLAGS     ) COMM_EVT_SAVE_DATA,
			            ( OS_OPT       ) OS_OPT_POST_FLAG_SET,
			            ( CPU_TS       ) 0,
			            ( OS_ERR      *) &err);
*/
		memcpy(dtc.prev_dtc, dtc.curr_dtc, sizeof(dtc.prev_dtc));
	}
}

/*******************************************************************************
 * 				end of file
 *******************************************************************************/
#endif
