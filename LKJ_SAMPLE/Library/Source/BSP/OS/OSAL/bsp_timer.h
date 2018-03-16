#ifndef	TIMER_H
#define	TIMER_H
/*********************************************************************
 *   Filename:       Uart.h
 *   Revised:        $Date: 2009-01-08	20:15 (Fri) $
 *   Revision:       $
 *	 Writer:		 Wuming Shen.
 *
 *   Description:    
 *
 *   Notes:        
 *					QQ:276193028                            			                  
 *     				E-mail:shenchangwei945@163.com    
 *			
 *   All copyrights reserved to Wuming Shen.  现代虚拟仪器仪表研究所
 *
 *********************************************************************/

/*********************************************************************
 * INCLUDES
 */
 #include  <global.h>

/*********************************************************************
 * CONSTANTS
 */
#ifndef OS_TICKS_PER_SEC
#define OS_TICKS_PER_SEC               1000
#endif
#define MHz                         /1
#define TIME1_CLK_DIV               2
#define SYSTEM_CLOCK                72MHz
#define	TIMER_DEF_CNT               ( 65536 - ( 1000000L / OS_TICKS_PER_SEC / TIME1_CLK_DIV ) )
   
/*********************************************************************
 * MACROS
 */
 #define	TIMER_PORT		        PORTA
   
/*********************************************************************
 * TYPEDEFS
 */
 /***********************************************
 * 描述：
 */
typedef struct {
    INT16U       hour;
    INT16U       min;
    INT16U       sec;
    INT16U       msec;
} StrTime;

extern StrTime SysTime;

/*********************************************************************
 * GLOBAL VARIABLES
 */

/*********************************************************************
 * GLOBAL FUNCTIONS
 */

/*********************************************************************
 * EXTERN VARIABLES
 */

/*********************************************************************
 * EXTERN FUNCTIONS
 */

void       OSAL_TIMER_TICKINIT              (void);
void       OSAL_TIMER_TICKSTART             (void );
void       OSAL_TIMER_TICKSTOP              (void );

void       OS_CPU_PendSVHandler             (void);
                                                  /* See OS_CPU_C.C                                    */
void       OS_CPU_SysTickHandler            (void);
//void       OS_CPU_SysTickInit               (void);
                                                  /* See BSP.C                                         */
INT32U     OS_CPU_SysTickClkFreq            (void);

#define    OSGetTicks()                     osal_system_clock
#define    OSSetTicks(ticks)                {osal_system_clock = ticks;}

/*********************************************************************
 * 				end of file
 *********************************************************************/
#endif	/* GLOBLES_H */