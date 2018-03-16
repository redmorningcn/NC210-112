/*******************************************************************************
 *   Filename:       bsp_max7219.h
 *   Revised:        $Date: 2015-12-07$
 *   Revision:       $
 *	 Writer:		 Wuming Shen.
 *
 *   Description:
 *   Notes:
 *					QQ:276193028
 *     				E-mail:shenchangwei945@163.com
 *
 *   All copyrights reserved to Wuming Shen.
 *
 *******************************************************************************/
#ifndef	__BSP_MAX_7219_H__
#define	__BSP_MAX_7219_H__

#ifdef __cplusplus
extern "C" {
#endif

/*******************************************************************************
 * INCLUDES
 */
#include <includes.h>

#ifdef   BSP_GLOBALS
#define  BSP_EXT
#else
#define  BSP_EXT  extern
#endif
    
/*******************************************************************************
 * CONSTANTS
 */
#define RIGHT       0
#define LEFT        1

/*******************************************************************************
 * MACROS
 */
    
/*******************************************************************************
 * TYPEDEFS
 */
typedef struct _StrLedDisp {
    uint8	Flag;
    uint8	Cycle;
    uint8	Duty;
    uint8	Code[4];
}StrLedDisp;

/***********************************************
* 描述： 
*/    
extern StrLedDisp  LedDispCtrl[8];
extern uint8       LedDispBuf[8];
extern const uint8 LED_SEG_CODE[];

/*******************************************************************************
 * LOCAL VARIABLES
 */

/*******************************************************************************
 * GLOBAL VARIABLES
 */
 
/*******************************************************************************
 * LOCAL FUNCTIONS
 */

/*******************************************************************************
 * GLOBAL FUNCTIONS
 *
 * 返回类型         函数名称                入口参数
 */
void                BSP_MAX7219Init        ( void );
     
/*******************************************************************************
 * EXTERN VARIABLES
 */

/*******************************************************************************
 * EXTERN FUNCTIONS
 */

//-----------------------------------------------------------------------------
//函数名称: BSP_DispNum ()
//功    能: 用数码管将整型数据显示出来
//入口参数:,number
//出口参数: 无
//----------------------------------------------------------------------------
extern void BSP_DispNum(uint32 Num, uint8 Flag);

//------------------------------------------------------
//函数名称:MovBSP_DispCharacter ()
//功    能:用数码管显示字符即已定字符
//入口参数:character: 字符代码
//出口参数:无
//显示内容:   -   E   H   L   P   Black
//内容代码:	  A	  B	  C	  D	  E	  F
//-------------------------------------------------------
extern void MovBSP_DispCharacter (uint8 character);

//------------------------------------------------------
//函数名称:MovBSP_DispCharacter ()
//功    能:用数码管显示字符即已定字符
//入口参数:character: 字符代码
//出口参数:无
//显示内容:   -   E   H   L   P   Black
//内容代码:	  A	  B	  C	  D	  E	  F
//-------------------------------------------------------
extern void MovBSP_DispCharacter (uint8 Character);
//-------------------------------------------------------------------
//函数名称:     BSP_DispInit ()
//功    能:     7219初始化
//入口参数:     无
//出口参数:     无
//-------------------------------------------------------------------
extern void BSP_DispInit (void);

//=====================================================
//函数名称:BSP_DispNum()
//功    能:用数码管将整型数据显示出来
//入口参数1:    num         显示数据
//入口参数2:    DotPosition 显示点的位置，
//              低四位对应4位数码的显示点，1为显示，0不显示
//出口参数:无
//=====================================================
//
// 		uint8	Flag;
//		uint8	Cycle;
//		uint8	Duty;
//		uint8	OffX100ms;
//		uint8	Code[4];
extern void BSP_DispWrite(   int     Num,
                            char    *Fmt,
                            char    Align,
                            uint8   FmtType,
                            uint8   DotPosition,
                            uint8   Cycle,
                            uint8   Duty,
                           // uint8   OffX100ms,
                            uint8   Order);

//=====================================================
//函数名称:BSP_DispNum()
//功    能:用数码管将整型数据显示出来
//入口参数1:    num         显示数据
//入口参数2:    DotPosition 显示点的位置，
//              低四位对应4位数码的显示点，1为显示，0不显示
//出口参数:无
//=====================================================
extern void BSP_Disp(uint8 Order);

//------------------------------------------------------
//函数名称:     BSP_DispString ()
//功    能:     用数码管显示字符串代码
//入口参数:     String: 字符代码
//出口参数:     无
//显示内容:     String
//内容代码:	    调用格式  BSP_DispString("ABCD");
//-------------------------------------------------------
extern void BSP_DispString(uint8 * String);

//=====================================================
//函数名称:BSP_DispChar()
//功    能:用数码管将整型数据显示出来
//入口参数1:    num         显示数据
//入口参数2:    DotPosition 显示点的位置，
//              低四位对应4位数码的显示点，1为显示，0不显示
//出口参数:无
//=====================================================

// 		uint8	Flag;
//		uint8	Cycle;
//		uint8	Duty;
//		uint8	OffX100ms;
//		uint8	Code[4];
extern void BSP_DispWriteChar(	uint8 	*String,
							uint8 	DotPosition,
							uint8	Flag,
							uint8	Cycle,
							uint8	Duty,
							//uint8	OffX100ms,
							uint8	Order);

//------------------------------------------------------
//函数名称:BSP_DispClr ()
//功    能:清除显示缓冲区,包括显示标志位和显示内容
//入口参数:第几个显示缓冲区
//出口参数:
//-------------------------------------------------------
//=====================================================
//函数名称:BSP_DispEvtProcess()
//功    能:用数码管将整型数据显示出来
//入口参数1:    num         显示数据
//入口参数2:    DotPosition 显示点的位置，
//              低四位对应4位数码的显示点，1为显示，0不显示
//出口参数:无
//=====================================================
extern void     BSP_DispEvtProcess(void);
extern void     BSP_DispClr(uint8 order );
extern void     BSP_DispSetBrightness ( u8 bl );

extern void     BSP_DispClrAll(void);
extern void     BSP_DispOff(void);
extern void     TestSegment(void);

/*******************************************************************************
 *              end of file                                                    *
 *******************************************************************************/
#ifdef __cplusplus
}
#endif
#endif