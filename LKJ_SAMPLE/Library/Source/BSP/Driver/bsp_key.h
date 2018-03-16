/*******************************************************************************
 *   Filename:       bsp_key.h
 *   Revised:        All copyrights reserved to Roger.
 *   Date:           2015-08-11
 *   Revision:       v1.0
 *   Writer:	     wumingshen.
 *
 *   Description:    按键驱动模块 头文件
 *                   可以识别长按，短按，双击，旋转编码器的操作。
 *
 *
 *   Notes:
 *
 *   All copyrights reserved to wumingshen
 *******************************************************************************/
#ifndef	__BSP_KEY_H__
#define	__BSP_KEY_H__

/*******************************************************************************
 * INCLUDES
 */
 #include  <global.h>

/*******************************************************************************
 * MACROS
 */
#define	KEY_PORT_RCC	        RCC_APB2Periph_GPIOB
#define	KEY_PORT_BM_RCC	        RCC_APB2Periph_GPIOB
#define	KEY_PORT		        GPIOB
#define	KEY_PORT_BM		        GPIOB
/***********************************************
 * 描述：按键使用的引脚定义
 */
#define KEY_GPIO_NRELOAD          GPIO_Pin_13
#define KEY_GPIO_NRESET           GPIO_Pin_14
#define KEY_GPIO_PWR              GPIO_Pin_15
//#define KEY_GPIO_HOME           GPIO_Pin_2
//#define KEY_GPIO_RIGHT          GPIO_Pin_3
//#define KEY_GPIO_BACK           GPIO_Pin_4
//#define KEY_GPIO_ENTER          GPIO_Pin_5
//#define KEY_GPIO_HELP           GPIO_Pin_6
//
#define KEY_GPIO_BMA            GPIO_Pin_12
#define KEY_GPIO_BMB            GPIO_Pin_11

/***********************************************
 * 描述：键值定义
 */
#define	KEY_VAL_0		        0XC000
#define	KEY_VAL_1		        0XA000
#define	KEY_VAL_2		        0X6000
#define	KEY_VAL_3		        0X098
#define	KEY_VAL_4		        0X040
#define	KEY_VAL_5		        0X05F
#define	KEY_VAL_6		        0X03F
//
#define	KEY_VAL_7		        0X0DF
#define	KEY_VAL_8		        0X0EF

#define	KEY_VAL_NULL            0XE000
#define	KEY_VAL_ALL		        0X0000

/***********************************************
 * 描述：键盘对应值定义
 */
#define KEY_NRELOAD             KEY_VAL_0
#define KEY_NRESET              KEY_VAL_1
#define KEY_PWR                 KEY_VAL_2


//#define KEY_ESC                 KEY_VAL_0
//#define KEY_LEFT                KEY_VAL_1
//#define KEY_HOME                KEY_VAL_2
//#define KEY_RIGHT               KEY_VAL_3
//#define KEY_BACK                KEY_VAL_4
//#define KEY_ENTER               KEY_VAL_5
//#define KEY_HELP                KEY_VAL_6
#define KEY_INC                 KEY_VAL_7
#define KEY_DEC                 KEY_VAL_8
/***********************************************
 * 描述：
 */

#define KEY_KEYS                KEY_VAL_NULL

#define KEY_EVT_NULL		    0X00
#define	KEY_EVT_DOWN		    0x01
#define	KEY_EVT_UP		        0X02
#define	KEY_EVT_PRESSED 	    0x03
#define KEY_EVT_PRESSING        0X04

#define KEY_MENU_HOME           0x00
#define KEY_MENU_SV_SETTING     0x01
#define KEY_MENU_SV_SET_VAL     0x02
#define KEY_MENU_SYS_SETTING    0x10


#define KEY_STATUS_NULL         0
#define KEY_STATUS_EVENT        1
#define KEY_STATUS_PRESSED      2

/*******************************************************************************
 * TYPEDEFS
 */
 /***********************************************
 * 描述：数据类型定义
 */
 typedef struct {
    INT08U           *pkey;
    INT08U            menu;
    INT08U            lock        :2;
    INT08U            evt         :2;
    INT08U            step        :2;
    INT08U            mode        :2;
    INT16U            val;
    INT16U            cnt;
    INT32U            incCnt      :8;
    INT32U            decCnt      :24;
    INT08U            map[10];
 } StrKeyEvt;

 extern StrKeyEvt Key;


/*******************************************************************************
 * 描述： 外部函数调用
 */
 void		  KEY_Init				( void );
 INT08U  	  KEY_GetKey			( StrKeyEvt *key );
 INT08U       KEY_GetEncodingKey    ( StrKeyEvt *key );


#endif
 /*******************************************************************************
 * 				end of file
 *******************************************************************************/