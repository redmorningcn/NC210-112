
#ifndef _TASK_COMM_DEF
#define _TASK_COMM_DEF


enum
{
MATER_TASK_ID = 0,
COMM_TASK_ID,
DTU_TASK_ID,
MODBUS_TASK_ID,
DETECT_TASK_ID,
MAX_TASK_ID,
};
typedef struct
{
uint16 event;
uint16 len;
void *data;
}ST_QUEUE;
uint8 system_send_msg(uint8 task_id, uint16 event, void *data, uint16 len);
void *system_get_msg(uint8 task_id);


#endif


