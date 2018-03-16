/*******************************************************************************
 *   Filename:      @osal.c
 *   Revised:
 *   Revision:
 *   Writer:        @Wuming Shen.
 *
 *   Description:
 *
 *   Notes:
 *
 *
 *   All copyrights reserved to Wuming Shen.
 *
 *******************************************************************************/

/*******************************************************************************
 * INCLUDES
 */
#include <global.h>
#include <includes.h>
#include <bsp_hal.h>

#if OSAL_EN > 0
/*******************************************************************************
 * CONSTANTS
 */

/*******************************************************************************
 * TYPEDEFS
 */
/*******************************************************************************
 * MACROS
 */


/*******************************************************************************
 * LOCAL VARIABLES
 */

/*******************************************************************************
 * GLOBAL VARIABLES
 */
//osal_msg_q_t    osal_qHead;

/*******************************************************************************
 * LOCAL FUNCTIONS
 */

/*******************************************************************************
 * GLOBAL FUNCTIONS
 */

/*******************************************************************************
 * EXTERN VARIABLES
 */

 /*******************************************************************************
 * EXTERN FUNCTIONS
 */

 /*******************************************************************************
 * @fn      osal_set_event
 *
 * @brief
 *
 *    This function is called to set the event flags for a task.  The
 *    event passed in is OR'd into the task's event variable.
 *
 * @param   byte task_id - receiving tasks ID
 * @param   byte event_flag - what event to set
 *
 * @return  ZSUCCESS, INVALID_TASK
 *******************************************************************************/
byte osal_set_event( osalTid task_id, osalFlg event_flag )
{
    OsalTadkREC_t  *srchTask;
    halIntState_t   cpu_sr;

    srchTask = osalFindTask( task_id );
    if ( srchTask ) {
        // Hold off interrupts
        CPU_CRITICAL_ENTER();
        // Stuff the event bit(s)
        srchTask->events |= event_flag;
        // Release interrupts
        CPU_CRITICAL_EXIT();
#if (UCOS_EN == DEF_ENABLED ) && ( OS_VERSION > 30000U )
        BSP_OS_SemPost(&Osal_EvtSem);                     // 发送信号量
#endif
    } else
        return ( INVALID_TASK );
    return ( ZSUCCESS );
}

/*******************************************************************************
 * @fn      osal_init_system
 *
 * @brief
 *
 *   This function initializes the "task" system by creating the
 *   tasks defined in the task table (OSAL_Tasks.h).
 *
 * @param   void
 *
 * @return  ZSUCCESS
 *******************************************************************************/
byte osal_init_system( void )
{
    // Initialize the Memory Allocation System
#if OSAL_MEM_EN > 0
    osal_mem_init();
#endif
    // Initialize the message queue
    // osal_qHead = NULL;

#if defined( OSAL_TOTAL_MEM )
  osal_msg_cnt = 0;
#endif

    // Initialize the timers
    osalTimerInit();

    // Initialize the tasking system
    osal_init_TaskHead();

    return ( ZSUCCESS );
}

/*******************************************************************************
 * @fn      osal_start_system
 *
 * @brief
 *
 *   This function is the main loop function of the task system.  It
 *   will look through all task events and call the task_event_processor()
 *   function for the task with the event.  If there are no events (for
 *   all tasks), this function puts the processor into Sleep.
 *   This Function doesn't return.
 *
 * @param   void
 *
 * @return  none
 *******************************************************************************/
void osal_start_system( void )
{
    osalEvt       events;
    halIntState_t cpu_sr;

    /***********************************************
    * 描述：OSAL任务查询和执行
    */
    while(1) {
        /***********************************************
        * 描述：用于需要系统总是查询的的事件，如：
        *       串口接收事件，按钮扫描事件等
        */
        HAL_ProcessPoll();
        /***********************************************
        * 描述：查找是否有任务产生事件
        */
        TaskActive = osalNextActiveTask();

        /***********************************************
        * 描述：如果当前任务有事件发生
        */
        if ( TaskActive ) {
            CPU_CRITICAL_ENTER();
            events = TaskActive->events;
            /***********************************************
            * 描述：清除当前任务事件
            */
            TaskActive->events = 0;
            CPU_CRITICAL_EXIT();

            /***********************************************
            * 描述：再次检查事件是否存在
            */
            if ( events != 0 ) {
                /***********************************************
                * 描述：判断任务函数是否存在
                */
                if ( TaskActive->pfnEventProcessor ) {
                    /***********************************************
                    * 描述：执行任务处理高优先级事件并返回未处理完的事件
                    *       事件值越小，优先级越高
                    */
                    events = (TaskActive->pfnEventProcessor)( TaskActive->taskID, events );
                    /***********************************************
                    * 描述：添加返回未处理完的事件到当前任务，等待下次执行
                    */
                    CPU_CRITICAL_ENTER();
                    TaskActive->events |= events;
                    CPU_CRITICAL_EXIT();
                }
            }
        }
    }
}
/*******************************************************************************
 * 				end of file                                    *
 *******************************************************************************/
#endif