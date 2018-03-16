#ifndef	BPQ_H
#define	BPQ_H
/*******************************************************************************
 *   Filename:       bsp_bpq.h
 *   Revised:        All copyrights reserved to Wuming Shen.
 *   Date:           2014-03-24
 *   Revision:       v1.0
 *   Writer:	     Wuming Shen.
 *
 *   Description:
 *
 *   Notes:
 *					QQ:276193028
 *     				E-mail:shenchangwei945@163.com
 *
 *******************************************************************************/

/*******************************************************************************
 * INCLUDES
 */
 #include  <global.h>

/*******************************************************************************
 * CONSTANTS
 */
/***********************************************
 * √Ë ˆ£∫
 */
#define BPQ_STATUS_STOP         0
#define BPQ_STATUS_FORWARD      1
#define BPQ_STATUS_REVERSION    2

#define BPQ_DIR_NONE            0
#define BPQ_DIR_FORWARD         1
#define BPQ_DIR_REVERSION       2

/*******************************************************************************
 * MACROS
 */

/*******************************************************************************
 * TYPEDEFS
 */
typedef struct {
  float         freq;
  float         min;
  float         max;
  INT16U        dir     : 2;

  INT16U        irun    : 1;
  INT16U        ierr    : 1;

  INT16U        ofor    : 1;
  INT16U        orev    : 1;
  INT16U        orst    : 1;

  INT16U        up      : 1;
  INT16U        down    : 1;

  INT16U        rsv     : 7;

} StrBpq;

extern StrBpq   bpq;

/*******************************************************************************
 * GLOBAL VARIABLES
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
 StrBpq	   *BPQ_Init				(void);
 void	    BPQ_Stop		        (void);
 BOOL       BPQ_Start               (INT8U  dir, float freq);

 void       BPQ_SetFreq             (float freq);
 void       BPQ_SetDir              (INT8U dir);
 BOOL       BPQ_GetStatus           (StrBpq *pbpq);
 BOOL       BPQ_SetStatus           (StrBpq *pbpq);
 void       BPQ_ErrClr              (void);

 BOOL       BPQ_GetInput            (StrBpq *pbpq );
 BOOL       BPQ_SetOutput           (StrBpq *pbpq );

/*******************************************************************************
 * 				end of file
 *******************************************************************************/
#endif	/* GLOBLES_H */