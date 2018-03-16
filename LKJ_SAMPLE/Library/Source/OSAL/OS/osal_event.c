/*******************************************************************************
 *   Filename:      @osal_event.c
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
#include <osal.h>
#include <osal_event.h>
#include <osal_memory.h>

#if OSAL_EVENT_EN > 0
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
OsalTadkREC_t   *TaskHead;      //the point to link'head
OsalTadkREC_t   *TaskActive;
osalTid           Task_id;

#if OSAL_MEM_EN == 0
OsalTadkREC_t   Task[OSAL_MAX_TASKS];
#endif

/***********************************************
* 描述： OS接口
*/
#if (UCOS_EN == DEF_ENABLED ) && ( OS_VERSION > 30000U )
OS_SEM           Osal_EvtSem;               // 信号量
#endif

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
 * @fn       osal_init_TaskHead
 *
 * @brief   init task link's head
 *
 * @param   none
 *
 * @return
 *******************************************************************************/
void  osal_init_TaskHead(void )
{
#if OSAL_MEM_EN == 0
    INT8U       idx;
    for ( idx = 0; idx < OSAL_MAX_TASKS; idx++ ) {
        Task[idx].next              = (void *)NULL;
        Task[idx].events            = 0;
        Task[idx].taskID            = 0;
        Task[idx].taskPriority      = 0;
        Task[idx].pfnInit           = (void *)0;
        Task[idx].pfnEventProcessor = (void *)0;
    }
#endif
    TaskHead    = (OsalTadkREC_t *)NULL;
    TaskActive  = (OsalTadkREC_t *)NULL;
    Task_id     = 0;

 }

/*******************************************************************************
 * @fn       osal_Task_init
 *
 * @brief   init task
 *
 * @param   none
 *
 * @return
 *******************************************************************************/
void  osal_Task_init(void)
{
    TaskActive = TaskHead;

    while(TaskActive) {
        if(TaskActive->pfnInit) {
             TaskActive->pfnInit();
        }
        TaskActive = TaskActive->next;
    }
    TaskActive = (OsalTadkREC_t *)NULL;
}
/*******************************************************************************
* 修改记录：20140510 增加INT8U id参数，用以任务ID自由分配取代任务ID系统分配
*/
 void  osal_add_Task(pTaskInitFn pfnInit,
                  pTaskEventHandlerFn pfnEventProcessor,
                  INT8U taskPriority,INT8U id)
{
    OsalTadkREC_t  *TaskNew;
    OsalTadkREC_t  *TaskSech;
    OsalTadkREC_t  **TaskPTR;
#if OSAL_MEM_EN > 0
    TaskNew = osal_mem_alloc(sizeof( OsalTadkREC_t));
#else
    INT8U           idx;
    for ( idx = 0; idx < OSAL_MAX_TASKS; idx++ ) {//查找空闲的任务资源，如找到则进行分配；
        if ( !Task[idx].pfnEventProcessor ) {
            TaskNew = &Task[idx];
            break;
        }
    }
#endif
    if(TaskNew) {
        TaskNew->pfnInit =                      pfnInit;
        TaskNew->pfnEventProcessor =            pfnEventProcessor;
        TaskNew->taskID =                       id;//Task_id++;
        TaskNew->events =                       0;
        TaskNew->taskPriority =                 taskPriority;
        TaskNew->next =                         (OsalTadkREC_t *)NULL;

        TaskPTR = &TaskHead;
        TaskSech = TaskHead;

        while(TaskSech)  {
            if(TaskNew->taskPriority > TaskSech->taskPriority) {
                TaskNew->next = TaskSech;
                *TaskPTR = TaskNew;
                return;
            }
            TaskPTR = &TaskSech->next;
            TaskSech = TaskSech->next;
        }
        *TaskPTR = TaskNew;
    }
    return;
}

/*******************************************************************************
 * @fn      osalNextActiveTask
 *
 * @brief   This function will return the next active task.
 *
 * NOTE:    Task queue is in priority order. We can stop at the
 *          first task that is "ready" (events element non-zero)
 *
 * @param   none
 *
 * @return  pointer to the found task, NULL if not found
 *******************************************************************************/
OsalTadkREC_t *osalNextActiveTask( void )
{
    OsalTadkREC_t  *TaskSech;

    // Start at the beginning
    TaskSech = TaskHead;

    // When found or not
    while (  TaskSech )  {
        if ( TaskSech->events)  {
            // task is highest priority that is ready
            return  TaskSech;
        }
        TaskSech =  TaskSech->next;
    }
    return NULL;
}

/*******************************************************************************
 * @fn      osalFindActiveTask
 *
 * @brief   This function will return the taskid task.
 *
 * NOTE:    Task queue is in priority order. We can stop at the
 *          first task that is "ready" (events element non-zero)
 *
 * @param   task_id
 *
 * @return  pointer to the found task, NULL if not found
 *******************************************************************************/
OsalTadkREC_t *osalFindTask( osalTid taskID )
{
    OsalTadkREC_t *TaskSech;
    TaskSech = TaskHead;
    while(TaskSech) {
        if(TaskSech->taskID == taskID) {
                return (TaskSech);
        }
        TaskSech = TaskSech->next;
    }
    return ((OsalTadkREC_t *)NULL);
}
/*******************************************************************************
 * 				end of file                                    *
 *******************************************************************************/
#endif