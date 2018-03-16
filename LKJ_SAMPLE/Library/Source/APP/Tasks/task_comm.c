


#include "global.h"
#include "task_comm.h"
#include "os.h"
#include "cfg_user.h"

#define MAX_MSG_SIZE		10

typedef struct
{
uint8 post_index;
uint8 pend_index;
ST_QUEUE msg[MAX_MSG_SIZE];
}ST_QUEUE_MSG;

typedef struct
{
uint8 task_id;
ST_QUEUE_MSG *queue_msg;
OS_Q *msg_Q;
}ST_TASK_QUEUE;


OS_Q	MTR_RxQ;
OS_Q	COM_RxQ;
OS_Q	DTU_RxQ;
OS_Q 	MB_OS_RxQ;
OS_Q 	DET_RxQ;

ST_QUEUE_MSG task_queue_msg[MAX_TASK_ID] = {0};

const ST_TASK_QUEUE task_queue_tbl[MAX_TASK_ID] = 
{
//需按TASK ID大小排序
{MATER_TASK_ID,		&task_queue_msg[MATER_TASK_ID],		&MTR_RxQ},
{COMM_TASK_ID,		&task_queue_msg[COMM_TASK_ID],		&COM_RxQ},
{DTU_TASK_ID,		&task_queue_msg[DTU_TASK_ID],		&DTU_RxQ},
{MODBUS_TASK_ID,	&task_queue_msg[MODBUS_TASK_ID],	&MB_OS_RxQ},
{DETECT_TASK_ID,	&task_queue_msg[DETECT_TASK_ID],	&DET_RxQ},

};

uint8 system_send_msg(uint8 task_id, uint16 event, void *data,    uint16 len)
{
	uint8 i;
	uint8 res = FALSE;
	OS_ERR err;
	uint8 temp_index;

	temp_index = (task_queue_tbl[task_id].queue_msg->post_index + 1);
	temp_index %= MAX_MSG_SIZE;

	if (temp_index != task_queue_tbl[task_id].queue_msg->pend_index)
	{
		task_queue_tbl[task_id].queue_msg->msg[temp_index].event = event;
		task_queue_tbl[task_id].queue_msg->msg[temp_index].len = len;
		task_queue_tbl[task_id].queue_msg->msg[temp_index].data = data;
#if	0//ndef SAMPLE_BOARD
		OSQPost(task_queue_tbl[task_id].msg_Q,
				&task_queue_tbl[task_id].queue_msg->msg[temp_index], 
				len, 
				OS_OPT_POST_FIFO,
				&err);
		if(OS_ERR_NONE == err)
		{
			res = TRUE;
			task_queue_tbl[task_id].queue_msg->post_index = temp_index;
		}
#else
		res = TRUE;
		task_queue_tbl[task_id].queue_msg->post_index = temp_index;
#endif
	}

	return res;
}

void *system_get_msg(uint8 task_id)
{
	void *msg = 0;
	
	if ( task_queue_tbl[task_id].queue_msg->pend_index != task_queue_tbl[task_id].queue_msg->post_index)
	{
		task_queue_tbl[task_id].queue_msg->pend_index++;
		task_queue_tbl[task_id].queue_msg->pend_index %= MAX_MSG_SIZE;
		msg = &task_queue_tbl[task_id].queue_msg->msg[task_queue_tbl[task_id].queue_msg->pend_index];
	}
	return msg;
}



 
