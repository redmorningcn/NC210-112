

/*******************************************************************************
 *   Filename:       osal.h
 *   Revised:        $Date: 2013-04-17 $
 *   Revision:       $
 *   Writer:         Wuming Shen.
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
#include <osal_timer.h>
#include <osal_event.h>
#include <osal_memory.h>

#include <bsp_timer.h>

#if (UCOS_EN == DEF_ENABLED ) && ( OS_VERSION > 30000U )
#include <os.h>
#endif

#if OSAL_TIMER_EN > 0

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
osalTimerRec_t *timerHead;
byte            timerActive;                            // Flag if hw timer active
#if OSAL_MEM_EN == 0
    osalTimerRec_t osalTimer[OSAL_MAX_TIMERS];
#endif
/*******************************************************************************
 * GLOBAL VARIABLES
 */
     
/*******************************************************************************
 * LOCAL FUNCTIONS
 */
void            osal_timer_activate         ( byte turn_on );
osalTimerRec_t *osalFindTimer               ( osalTid task_id, osalEvt event_flag );
void            osal_timer_hw_setup         ( byte turn_on );

/*******************************************************************************
 * GLOBAL FUNCTIONS
 */
INT32U          osal_system_clock;

/*******************************************************************************
 * EXTERN VARIABLES
 */

/******************************************************************************/

/*******************************************************************************
 * @fn      osalTimerInit
 *
 * @brief   Initialization for the OSAL Timer System.
 *
 * @param   none
 *
 * @return
 *******************************************************************************/
void osalTimerInit( void )
{
#if OSAL_MEM_EN == 0
    INT8U   idx;
    for ( idx = 0; idx < OSAL_MAX_TIMERS; idx++ ) {
        osalTimer[idx].next         = (void *)NULL;
        osalTimer[idx].task_id      = 0;
        osalTimer[idx].event_flag   = 0;
        osalTimer[idx].timeout      = 0;
        osalTimer[idx].reload      = 0;
    }
#endif
    timerHead   = (void *)NULL;
    // Initialize the system timer
    osal_timer_activate( FALSE );
    timerActive = FALSE;

    osal_system_clock  = 0;
}

/*******************************************************************************
 * @fn      osalAddTimer
 *
 * @brief   Add a timer to the timer list.
 *          Ints must be disabled.
 *
 * @param   task_id
 * @param   event_flag
 * @param   timeout
 *
 * @return  osalTimerRec_t * - pointer to newly created timer
 *******************************************************************************/
osalTimerRec_t *osalAddTimer( osalTid task_id, osalFlg event_flag, osalTime timeout )
{
    osalTimerRec_t  *newTimer;
    osalTimerRec_t  *srchTimer;
    // Look for an existing timer first
    newTimer = osalFindTimer( task_id, event_flag );
    if ( newTimer ) {
        // Timer is found - update it.
        newTimer->timeout = timeout;

        return ( newTimer );
    } else {
        // New Timer
#if OSAL_MEM_EN > 0
        newTimer = osal_mem_alloc( sizeof( osalTimerRec_t ) );
#else
        INT8U           idx;
        for ( idx = 0; idx < OSAL_MAX_TIMERS; idx++ ) {
            if ( !osalTimer[idx].event_flag ) {
                newTimer    = &osalTimer[idx];
                break;
            }
        }
#endif
        if ( newTimer ) {
            // Fill in new timer
            newTimer->task_id       = task_id;
            newTimer->event_flag    = event_flag;
            newTimer->timeout       = timeout;
            newTimer->next          = (void *)NULL;

            // Does the timer list already exist
            if ( timerHead == NULL ) {
                // Start task list
                timerHead = newTimer;
            } else {
                // Add it to the end of the timer list
                srchTimer = timerHead;

                // Stop at the last record
                while ( srchTimer->next )
                    srchTimer = srchTimer->next;

                // Add to the list
                srchTimer->next = newTimer;
            }

            return ( newTimer );
        } else
            return ( (osalTimerRec_t *)NULL );
    }
}

/*******************************************************************************
 * @fn      osalFindTimer
 *
 * @brief   Find a timer in a timer list.
 *          Ints must be disabled.
 *
 * @param   task_id
 * @param   event_flag
 *
 * @return  osalTimerRec_t *
 *******************************************************************************/
osalTimerRec_t *osalFindTimer( osalTid task_id, osalFlg event_flag )
{
    osalTimerRec_t  *srchTimer;

    // Head of the timer list
    srchTimer = timerHead;

    // Stop when found or at the end
    while ( srchTimer ) {
        if ( srchTimer->event_flag == event_flag &&
            srchTimer->task_id == task_id )
            break;
        // Not this one, check another
        srchTimer = srchTimer->next;
    }
    return ( srchTimer );
}

/*******************************************************************************
 * @fn      osalDeleteTimer
 *
 * @brief   Delete a timer from a timer list.
 *          Ints must be disabled.
 *
 * @param   table
 * @param   rmTimer
 *
 * @return  none
 *******************************************************************************/
void osalDeleteTimer( osalTimerRec_t *rmTimer )
{
    osalTimerRec_t  *srchTimer;

    // Does the timer list really exist
    if ( (timerHead != NULL) && rmTimer ) {
        // Add it to the end of the timer list
        srchTimer = timerHead;

        // First element?
        if ( srchTimer == rmTimer )  {
            timerHead = rmTimer->next;

#if OSAL_MEM_EN > 0
                osal_mem_free( rmTimer );
#else
                rmTimer->event_flag   = 0;
                rmTimer->next         = (void *)NULL;
                rmTimer->task_id      = 0;
#endif
        } else {
            // Stop when found or at the end
            while ( srchTimer->next && srchTimer->next != rmTimer ) {
                srchTimer = srchTimer->next;
            }
            // Found?
            if ( srchTimer->next == rmTimer ) {
                // Fix pointers
                srchTimer->next = rmTimer->next;
                // Deallocate the timer struct memory
#if OSAL_MEM_EN > 0
                osal_mem_free( rmTimer );
#else
                rmTimer->event_flag   = 0;
                rmTimer->next         = (void *)NULL;
                rmTimer->task_id      = 0;
                rmTimer->reload       = 0;
#endif
            }
        }
    }
}

/*******************************************************************************
 * @fn      osal_timer_activate
 *
 * @brief
 *
 *   Turns the hardware timer on or off
 *
 * @param  byte turn_on - false - turn off, true - turn on
 *
 * @return  none
 *******************************************************************************/
void osal_timer_activate( byte turn_on )
{
    osal_timer_hw_setup( turn_on );
    timerActive = turn_on;
}

/*******************************************************************************
 * @fn      osal_timer_hw_setup
 *
 * @brief
 *
 *   Setup the timer hardware.
 *
 * @param  byte turn_on
 *
 * @return  void
 *******************************************************************************/
void osal_timer_hw_setup( byte turn_on )
{
    if (turn_on) {
        //OSAL_TIMER_TICKSTART( ) ;
    } else {
        //OSAL_TIMER_TICKSTOP( );
    }
}

/*******************************************************************************
 * @fn      osal_start_timerEx
 *
 * @brief
 *
 *   This function is called to start a timer to expire in n mSecs.
 *   When the timer expires, the calling task will get the specified event.
 *
 * @param   byte taskID - task id to set timer for
 * @param   INT16U event_id - event to be notified with
 * @param   UNINT16 timeout_value - in milliseconds.
 *
 * @return  ZSUCCESS, or NO_TIMER_AVAIL.
 *******************************************************************************/
byte osal_start_timerEx( osalTid taskID, osalEvt event_id, osalTime timeout_value )
{
    halIntState_t     cpu_sr;
    osalTimerRec_t   *newTimer;
    OS_CRITICAL_ENTER();  // Hold off interrupts.

    // Add timer
    newTimer = osalAddTimer( taskID, event_id, timeout_value );
    if ( newTimer ) {
        // Does the timer need to be started?
        if ( timerActive == FALSE ) {
            osal_timer_activate( TRUE );
        }
    }

    OS_CRITICAL_EXIT();   // Re-enable interrupts.

    return ( (newTimer != NULL) ? ZSUCCESS : NO_TIMER_AVAIL );
}
/*******************************************************************************
 * @fn      osal_start_timerRl
 *
 * @brief
 *
 *   This function is called to start a timer to expire in n mSecs.
 *   When the timer expires, the calling task will get the specified event.
 *
 * @param   byte taskID - task id to set timer for
 * @param   INT16U event_id - event to be notified with
 * @param   UNINT16 timeout_value - in milliseconds.
 *
 * @return  ZSUCCESS, or NO_TIMER_AVAIL.
 *******************************************************************************/
byte osal_start_timerRl( osalTid taskID, osalEvt event_id, osalTime timeout_value )
{
    halIntState_t     cpu_sr;
    osalTimerRec_t   *newTimer;
    // Hold off interrupts.
    OS_CRITICAL_ENTER();

    // Add timer
    newTimer = osalAddTimer( taskID, event_id, timeout_value );
    if ( newTimer ) {
        newTimer->reload    = newTimer->timeout;
        // Does the timer need to be started?
        if ( timerActive == FALSE ) {
            osal_timer_activate( TRUE );
        }
    }

    OS_CRITICAL_EXIT();   // Re-enable interrupts.

    return ( (newTimer != NULL) ? ZSUCCESS : NO_TIMER_AVAIL );
}
/*******************************************************************************
 * @fn      osal_stop_timerEx
 *
 * @brief
 *
 *   This function is called to stop a timer that has already been started.
 *   If ZSUCCESS, the function will cancel the timer and prevent the event
 *   associated with the timer from being set for the calling task.
 *
 * @param   byte task_id - task id of timer to stop
 * @param   INT16U event_id - identifier of the timer that is to be stopped
 *
 * @return  ZSUCCESS or INVALID_EVENT_ID
 *******************************************************************************/
byte osal_stop_timerEx( osalTid task_id, osalEvt event_id )
{
    halIntState_t     cpu_sr;
    osalTimerRec_t   *foundTimer;

    CPU_CRITICAL_ENTER();  // Hold off interrupts.

    // Find the timer to stop
    foundTimer = osalFindTimer( task_id, event_id );
    if ( foundTimer ) {
        osalDeleteTimer( foundTimer );
    }

    CPU_CRITICAL_EXIT();   // Re-enable interrupts.

    return ( (foundTimer != NULL) ? ZSUCCESS : INVALID_EVENT_ID );
}

/*******************************************************************************
 * @fn      osalTimerUpdate
 *
 * @brief   Update the timer structures for a timer tick.
 *
 * @param   none
 *
 * @return  none
 *******************************************************************************/
void osalTimerUpdate( void )
{
    osalTimerRec_t  *srchTimer;
    osalTimerRec_t  *prevTimer;
    osalTimerRec_t  *saveTimer;

    // Update the system time
    osal_system_clock += OSAL_TICK_TIME_MS;

    // Look for open timer slot
    if ( timerHead != NULL ) {
        // Add it to the end of the timer list
        srchTimer = timerHead;
        prevTimer = (void *)NULL;

        // Look for open timer slot
        while ( srchTimer ) {
            // Decrease the correct amount of time
            if (srchTimer->timeout <= OSAL_TICK_TIME_MS)
                srchTimer->timeout = 0;
            else
                srchTimer->timeout = srchTimer->timeout - OSAL_TICK_TIME_MS;

            // When timeout, execute the task
            if ( srchTimer->timeout == 0 ) {
                osal_set_event( srchTimer->task_id, srchTimer->event_flag );
                if ( srchTimer->reload ) {
                  // Restart srchTimer
                  srchTimer->timeout    = srchTimer->reload;
                  prevTimer = srchTimer;
                  // Get next
                  srchTimer = srchTimer->next;
                  continue;
                }
                // Take out of list
                if ( prevTimer == NULL )
                    timerHead = srchTimer->next;
                else
                    prevTimer->next = srchTimer->next;
                // Next
                saveTimer = srchTimer->next;
                // Free memory
#if OSAL_MEM_EN > 0
                osal_mem_free( srchTimer );
#else
                srchTimer->event_flag   = 0;
                srchTimer->next         = (void *)NULL;
                srchTimer->task_id      = 0;
                srchTimer->reload       = 0;
#endif
                srchTimer = saveTimer;
            } else {
                // Get next
                prevTimer = srchTimer;
                srchTimer = srchTimer->next;
            }
        }
    }
}

/*******************************************************************************
 * 				     end of file                               *
 *******************************************************************************/
#endif